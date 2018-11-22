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

TWI::TWI()
{

}

/*!
 * Function to send the transmission mode - Master or Slave
 * @param requestedMode Sets the transmission mode. Default mode is set to Master. For master - TWIMode::Slave, for
 * slave - TWIMode::Slave
 * @param setSlaveAddress Sets the address when slave mode is selected. Default value is set to 0x01. Value should be
 * between 0x01 -
 * @param value Sets the prescaler value. Default prescaler of 1 (prescaling to 0). Use
 * PRESCALE_VALUE_1
 * PRESCALE_VALUE_4
 * PRESCALE_VALUE_16
 * PRESCALE_VALUE_64
 * @param twiFrequency Sets the TWI communication frequency. Default value of 100000 (100 kHz)
 */
void TWI::TWISetMode(TWIMode requestedMode,
                     uint8_t setSlaveAddress,
                     PrescalerValue value,
                     uint32_t twiFrequency)
{
    /* Set indexes to 0 */
    txIndex = 0;
    txBufferLen = 0;
    rxIndex = 0;
    rxBufferLen = 0;
    TWIInfo.state = Available;
    TWIInfo.repStart = false;

    /** default communication settings **/
    //Set value of pre-scaler to 0
    this->setPrescaler(value);
    //Set frequency of TWI communication
    //Default value of 100kHz is set in the constructor call
    this->setBitRate(twiFrequency);
    this->mode = requestedMode;

    if (requestedMode == TWIMode::Master) {
        /** Clear any address present in the TWAR register **/
        TWAR = 0;
        /** Enable TWI communication **/
        TWCR = (1 << TWEN) | (1 << TWIE);
    }
    else {
        this->slaveModeAddress = setSlaveAddress;
        TWAR = slaveModeAddress << 1;
        TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins.
            (0<<TWIE)|(0<<TWINT)|                      // Disable TWI Interupt.
            (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Do not ACK on any requests, yet.
            (0<<TWWC);
    }

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

void TWI::setBitRate(uint32_t twiFrequency)
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

        case TWICommand::ENABLE_SLAVE:
            TWCR =  (1 << TWEN) |                               /* Enable TWI-interface and release TWI pins */
                    (1 << TWIE) | (1 << TWINT) |                /* Keep interrupt enabled and clear the flag */
                    (1 << TWEA) | (0 << TWSTA) | (0 << TWSTO) | /* Acknowledge on any new requests */
                    (0 << TWWC);
            break;

        default:
            break;
    }
}

bool TWI::GetAvailability()
{
    return TWIInfo.state == Available;
}

bool TWI::isTWIReady()
{
    return (TWIInfo.state == Available)
    || (TWIInfo.state == RepeatedStartSent);
}

/*!
 * Master transmitter function to write data into the TWI bus
 * @param slaveAddress Address of the TWI slave device (7 bit wide)
 * @param data Data to be transmitted. Sent in as an array
 * @param dataLen Length of the data to be transmitted
 * @param repeatedStart Boolean value, set as default to false. To be set to true if a repeated start is performed
 * @param TWIReadRequest Boolean value, set as default to false. This value is used by the TWIRead function only!
 */

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

/*!
 * Overload of TWIWrite - Master transmitter function to write data into the TWI bus
 * Function is used to send char array to the TWIWrite function. This is necessary since the length of the character
 * array is counted here
 * @param slaveAddress
 * @param data
 * @param repeatedStart
 */
void TWI::TWIWrite( const uint8_t slaveAddress,
                    const char *const data,
                    const bool repeatedStart)
{
    uint8_t dataLen = 0;
    const char * ptr;
    ptr = data;
    // Count number of elements present in the input char array
    while (  ((*ptr) != '\0')   // Looping until end of char array '\0' is encountered
           &&(dataLen < TX_BUFFER_SIZE)) { // or until the buffer size
        dataLen++;
        ptr++;
    };
    TWIWrite(slaveAddress, reinterpret_cast<const uint8_t *>(data), dataLen, repeatedStart, false);
}

/*!
 * Slave transmitter function to write data into the TWI bus when requested by master
 * @param data Data to be transferred
 */
void TWI::TWIWrite(const char *const data)
{
    txIndex = 0;
    uint8_t dataLen = 0;
    const char * ptr;
    ptr = data;
    while (  ((*ptr) != '\0')
        &&(dataLen < TX_BUFFER_SIZE)) {
        txBuffer[dataLen++] = static_cast<uint8_t>(*ptr);
        ptr++;
    };
    TWI::TWIPerform(TWICommand::ENABLE_SLAVE);
}

/**
 * Function to read data in Master Receiver mode
 */
//!
//! \param slaveAddress Address of the TWI slave device
//! \param data pointer to the array where the data shall be saved to
//! \param readBytesLen Number of bytes the received data shall be
//! \param repeatedStart Boolean value, set as default to false. To be set to true if a repeated start is performed
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
        case TWI_MT_SLA_ACK:
            TWIInfo.state = MasterTransmitter;

        /** A START condition has been transmitted. **/
        case TWI_START:

        /** Data byte has been transmitted; ACK has been received. **/
        case TWI_MT_DATA_ACK:
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
        case TWI_MR_SLA_ACK:
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
        case TWI_MR_DATA_ACK:
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
        case TWI_MR_DATA_NACK:
            rxBuffer[rxIndex++] = TWDR;
            if (TWIInfo.repStart) {
                TWIPerform(TWICommand::START);
            }
            else {
                TWIInfo.state = Available;
                TWIPerform(TWICommand::STOP);
            }
            break;

        /** Own SLA+R has been received; ACK has been returned **/
        case TWI_ST_SLA_ACK:
            // Reset buffer pointer
            txIndex = 0;
            TWIInfo.state = SlaveTransmitter;

        /** Data byte in TWDR has been transmitted; ACK has been received **/
        case TWS_ST_DATA_ACK:
            // Copy data from current buffer position
            TWDR = txBuffer[txIndex++];
            TWI::TWIPerform(TWICommand::ENABLE_SLAVE);
            break;

        /** Data byte in TWDR has been transmitted;  NOT ACK has been received */
        case TWS_ST_DATA_NACK:
            TWIInfo.state = Available;
            TWI::TWIPerform(TWICommand::ENABLE_SLAVE);
            break;

        default:
            TWIPerform(TWICommand::STOP);
            TWIInfo.state = Available;
            break;
    }
}

ISR(TWI_vect) {
    twi.twi_interrupt_handler();
}