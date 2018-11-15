//
// Created by dipak on 09.11.18.
//

#include <TWI.h>
#include "util/delay.h"

TWI twi;
uint8_t TWI::txBuffer[TX_BUFFER_SIZE] = {0};
uint8_t TWI::txIndex = 0;
uint8_t TWI::txBufferLen = 0;

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

bool TWI::TWIWrite(const uint8_t slaveAddress, const uint8_t *data, const uint8_t dataLen)
{
    // Transmission shall only be performed as long as dataLen is lesser
    // than the buffer size
    if (dataLen <= TX_BUFFER_SIZE) {
        // Copy slave address into the tx buffer.
        txBuffer[0] = slaveAddress << 1;

        //Copy all information in data to txBuffer.
        //Start for loop from index 1 since, 0 has the address
        for (uint8_t index = 1; index <= dataLen; index++) {
            txBuffer[index] = data[index - 1];
        }
        //Set the data length for transmission now
        txBufferLen = static_cast<uint8_t>(dataLen + 1);
        txIndex = 0;
        TWIPerform(TWICommand::START);
    }
    else {
        return false;
    }
    return true;
}

void TWI::twi_interrupt_handler()
{

    switch (TWI_STATUS) {
        /** A START condition has been transmitted. **/
        case TWI_START:

        /** A repeated START condition has been transmitted. **/
        case TWI_RESTART:

        /** SLA+W has been transmitted; ACK has been received. **/
        case TWI_MT_TX_SLA_ACK:

        /** Data byte has been transmitted; ACK has been received. **/
        case TWI_MT_TX_DATA_ACK:
            if (txIndex < txBufferLen) {
                PORTB |= (1 << PINB7);
                TWDR = txBuffer[txIndex++];
                TWIPerform(TWICommand::TRANSMIT_DATA);
            }
            else {
                TWIPerform(TWICommand::STOP);
            }
            break;

        default:
        break;
    }
}

ISR(TWI_vect) {
    twi.twi_interrupt_handler();
}

