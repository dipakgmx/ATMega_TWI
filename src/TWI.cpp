//
// Created by dipak on 09.11.18.
//

#include <TWI.h>
#include "util/delay.h"

TWI twi;
uint8_t TWI::txBuffer[TX_BUFFER_SIZE] = {0};
uint8_t TWI::txIndex = 0;
uint8_t TWI::txBufferLen = 0;
uint8_t TWI::rxBuffer[TX_BUFFER_SIZE] = {0};
uint8_t TWI::rxIndex = 0;
uint8_t TWI::rxBufferLen = 0;
TWIInfoStruct TWI::TWIInfo = {Available, false};

TWI::TWI(PrescalerValue value, long twiFrequency)
{
    /** default communication settings **/
    //Set value of pre-scaler to 0
    this->setPrescaler(value);
    //Set frequency of TWI communication
    //Default value of 100kHz is set in the constructor call
    this->setBitRate(twiFrequency);

    /** Enable TWI communication **/
    TWCR = (1 << TWEN) | (1 << TWIE);

    txBuffer[TX_BUFFER_SIZE] = {0};
    txIndex = 0;
    txBufferLen = 0;
    rxBuffer[TX_BUFFER_SIZE] = {0};
    rxIndex = 0;
    rxBufferLen = 0;
    TWIInfo.state = Available;
    TWIInfo.repStart = false;
}

void TWI::setPrescaler(PrescalerValue value)
{
    /** TWPS: TWI Prescaler Bits **/
    switch(value) {
    case PrescalerValue::PRESCALE_VALUE_1:
        TWSR |= 0;
        this->TWIPrescalerValue = 1;
        break;
    case PrescalerValue::PRESCALE_VALUE_4:
        TWSR |= 1;
        this->TWIPrescalerValue = 4;
        break;
    case PrescalerValue::PRESCALE_VALUE_16:
        TWSR |= 2;
        this->TWIPrescalerValue = 16;
        break;
    case PrescalerValue::PRESCALE_VALUE_64:
        TWSR |= 3;
        this->TWIPrescalerValue = 64;
        break;
    }
}

void TWI::setBitRate(long twiFrequency)
{
    /** TWBR – TWI Bit Rate Register **/
    //TWBR selects the division factor for the bit rate generator.
    TWBR = static_cast<uint8_t>(((F_CPU / twiFrequency) - 16) / (2 * this->TWIPrescalerValue));
}

void TWI::TWIPerform(TWICommand command)
{
    switch (command) {
    case TWICommand::START:
        TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
        break;
    case TWICommand::STOP:
        TWCR = (1<<TWINT) | (1<<TWEN)| (1<<TWSTO);
        break;
    case TWICommand::TRANSMIT_DATA:
    case TWICommand::TRANSMIT_NACK:
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
        break;
    case TWICommand::TRANSMIT_ACK:
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
        break;
    default:
        break;
    }
}

bool TWI::isTWIReady()
{
    return (TWIInfo.state == Available)
    || (TWIInfo.state == RepeatedStartSent);
}

void TWI::TWIWrite( uint8_t slaveAddress,
                    const uint8_t *data,
                    uint8_t dataLen,
                    bool repeatedStart,
                    bool TWIReadRequest)
{
    // Transmission shall only be performed as long as dataLen is lesser
    // than the buffer size
    if (dataLen <= TX_BUFFER_SIZE) {
        while (!isTWIReady()) {
            _delay_us(1);
        }
        // Set repeated start state
        TWIInfo.repStart = repeatedStart;
        if (TWIReadRequest) {
            //In case the TWIWrite function is called from the TWIRead function, the slave address with the read
            // command is already sent. Hence it is not necessary to perform the shift operation again
            txBuffer[0] = slaveAddress;
        }
        else {
            // Copy slave address into the tx buffer.
            txBuffer[0] = slaveAddress << 1;
        }

        //Copy all information in data to txBuffer.
        //Start for loop from index 1 since, 0 has the address
        for (uint8_t index = 0; index <= dataLen; index++) {
            txBuffer[index + 1] = data[index];
        }
        //Set the data length for transmission now
        txBufferLen = static_cast<uint8_t>(dataLen + 1);
        txIndex = 0;

        // If a repeated stop is already send, devices are expecting data
        // and not another start condition
        if (TWIInfo.state == RepeatedStartSent) {
            TWIInfo.state = Initializing;
            TWDR = txBuffer[txIndex++];
            TWIPerform(TWICommand::TRANSMIT_DATA);
        }
        else {
            TWIInfo.state = Initializing;
            TWIPerform(TWICommand::START);
        }
    }
}

void TWI::TWIWrite( const uint8_t slaveAddress,
                    const char *const data,
                    const bool repeatedStart)
{
    uint8_t dataLen = 0;
    const char * ptr;
    ptr = data;
    while (  ((*ptr) != '\0')
           &&(dataLen < TX_BUFFER_SIZE)) {
        dataLen++;
        ptr++;
    };
    TWIWrite(slaveAddress, reinterpret_cast<const uint8_t *>(data), dataLen, repeatedStart, false);
}

void TWI::TWIRead(  const uint8_t slaveAddress,
                    uint8_t *data,
                    uint8_t readBytesLen,
                    const bool repeatedStart)
{
    if (readBytesLen <= RX_BUFFER_SIZE) {
        rxIndex = 0;
        rxBufferLen = readBytesLen;
        //Create a temp variable to send
        // Slave address + Write
        auto slaveAddressWrite = static_cast<uint8_t>((slaveAddress << 1) | 0x01);
        //Calling the TWIWrite function to transmit the data
        TWIWrite(slaveAddressWrite, nullptr, 0, repeatedStart, true);

        //Wait until buffer is filled with received data
        while (rxIndex != rxBufferLen - 1) {
            _delay_us(1);
        }
        for (uint8_t index = 0; index < rxBufferLen; index++) {
            data[index] = rxBuffer[index];
        }
    }
}

void TWI::twi_interrupt_handler()
{
    switch (TWI_STATUS) {

        /** SLA+W has been transmitted; ACK has been received. **/
        case TWI_MT_TX_SLA_ACK:
            TWIInfo.state = MasterTransmitter;

        /** A START condition has been transmitted. **/
        case TWI_START:

        /** Data byte has been transmitted; ACK has been received. **/
        case TWI_MT_TX_DATA_ACK:
            if (txIndex < txBufferLen) {
                TWDR = txBuffer[txIndex++];
                TWIPerform(TWICommand::TRANSMIT_DATA);
            }
            else if (TWIInfo.repStart) {
                TWIPerform(TWICommand::START);
            }
            else {
                TWIInfo.state = Available;
                TWIPerform(TWICommand::STOP);
            }
            break;

        /** A repeated START condition has been transmitted. **/
        case TWI_RESTART:
            TWIInfo.state = RepeatedStartSent;
        break;

        /** SLA+R has been transmitted; ACK has been received **/
        case TWI_SL_RX_SLA_ACK:
            TWIInfo.state = MasterReceiver;
            // Checking if more than 1 byte is expected. If yes, send ACK, else send NACK
            if (rxIndex < rxBufferLen - 1) {
                TWI::TWIPerform(TWICommand::TRANSMIT_ACK);
            }
            else {
                TWI::TWIPerform(TWICommand::TRANSMIT_NACK);
            }
        break;

        /** Data byte has been received; ACK has been returned **/
        case TWI_SL_RX_DATA_ACK:
            rxBuffer[rxIndex++] = TWDR;
            // Checking if more than 1 byte is expected. If yes, send ACK, else send NACK
            if (rxIndex < rxBufferLen - 1) {
                TWI::TWIPerform(TWICommand::TRANSMIT_ACK);
            }
            else {
                TWI::TWIPerform(TWICommand::TRANSMIT_NACK);
            }
        break;

        /** Data byte has been received; NOT ACK has been returned **/
        case TWI_SL_RX_DATA_NACK:
            rxBuffer[rxIndex++] = TWDR;
            if (TWIInfo.repStart) {
                TWIPerform(TWICommand::START);
            }
            else {
                TWIInfo.state = Available;
                TWIPerform(TWICommand::STOP);
            }
        break;

    default:
            TWIInfo.state = Available;
            break;
    }
}

ISR(TWI_vect) {
    twi.twi_interrupt_handler();
}






