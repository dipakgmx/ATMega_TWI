//
// Created by dipak on 09.11.18.
//

#include <TWI.h>
#include "util/delay.h"

TWI twi;

TWI::TWI(PrescalerValue value, long twiFrequency)
{
    TWIInfo.mode = Ready;
    TWIInfo.errorCode = 0xFF;
    TWIInfo.repStart = 0;
    /** default communication settings **/
    //Set value of pre-scaler to 0
    this->setPrescaler(value);
    //Set frequency of TWI communication
    //Default value of 100kHz is set in the constructor call
    this->setBitRate(twiFrequency);

    /** Enable TWI communication with the interrupt **/
    /** TWCR – TWI Control Register **/
    /** TWIE: TWI Interrupt Enable **/
    // When this bit is written to one, and the I-bit in SREG is set, the TWI interrupt request will be activated for as
    // long as the TWINT Flag is high.
    /** TWEN: TWI Enable Bit **/
    // The TWEN bit enables TWI operation and activates the TWI interface. When TWEN is written to one, the TWI
    //takes control over the I/O pins connected to the SCL and SDA pins, enabling the slew-rate limiters and spike filters.
    //If this bit is written to zero, the TWI is switched off and all TWI transmissions are terminated, regardless of any
    //ongoing operation.
    TWCR = (1 << TWIE) | (1 << TWEN);
}

void TWI::setPrescaler(PrescalerValue value)
{
    /** TWPS: TWI Prescaler Bits **/
    switch(value) {
    case PrescalerValue::PRESCALE_VALUE_1:
        TWSR |= 1;
        this->TWIPrescalerValue = 1;
        break;
    case PrescalerValue::PRESCALE_VALUE_4:
        TWSR |= 4;
        this->TWIPrescalerValue = 4;
        break;
    case PrescalerValue::PRESCALE_VALUE_16:
        TWSR |= 16;
        this->TWIPrescalerValue = 16;
        break;
    case PrescalerValue::PRESCALE_VALUE_64:
        TWSR |= 64;
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
/**TWINT: TWI Interrupt Flag **/
// This bit is set by hardware when the TWI has finished its current job and expects application software response.
// While the TWINT Flag is set, the SCL low period is stretched. This allows the software to ensure that the
// data is processed before the next data bit is sent/received. The TWINT flag must be cleared by software by
// writing a logic 1 to it. If interrupts are enabled and the TWIE flag is also set, then the MCU will jump to the
// TWI interrupt vector when TWINT gets set.
/** TWSTA: TWI START Condition Bit **/
// The TWEA bit controls the generation of the acknowledge pulse. If the TWEA bit is written to one, the ACK pulse
// is generated on the TWI bus
/**TWEN: TWI Enable Bit **/
//The TWEN bit enables TWI operation and activates the TWI interface. When TWEN is written to one, the TWI
//takes control over the I/O pins connected to the SCL and SDA pins, enabling the slew-rate limiters and spike
// filters. If this bit is written to zero, the TWI is switched off and all TWI transmissions are terminated,
// regardless of any ongoing operation.
/** TWIE: TWI Interrupt Enable **/
//When this bit is written to one, and the I-bit in SREG is set, the TWI interrupt request will be activated for as
// long as the TWINT Flag is high.
/** TWSTO: TWI STOP Condition Bit **/
// Writing the TWSTO bit to one in Master mode will generate a STOP condition on the 2-wire Serial Bus. When the
//STOP condition is executed on the bus, the TWSTO bit is cleared automatically. In Slave mode, setting the
//TWSTO bit can be used to recover from an error condition. This will not generate a STOP condition, but the TWI
//returns to a well-defined unaddressed Slave mode and releases the SCL and SDA lines to a high impedance state.
/** TWEA: TWI Enable Acknowledge Bit **/
//The TWEA bit controls the generation of the acknowledge pulse.
    switch (command) {
    case TWICommand::START:
        TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
        break;
    case TWICommand::STOP:
        TWCR = (1<<TWINT) | (1<<TWEN)| (1<<TWSTO) | (1<<TWIE);
        break;
    case TWICommand::TRANSMIT_DATA:
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
        break;
    case TWICommand::TRANSMIT_ACK:
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
        break;
    case TWICommand::TRANSMIT_NACK:
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
        break;
    default:break;
    }
}

bool TWI::isTWIReady()
{
    return (TWIInfo.mode == Ready) | (TWIInfo.mode == RepeatedStartSent);
}

bool TWI::TWIWrite(const uint8_t slaveAddress, const uint8_t *data, const uint8_t dataLen)
{
    // Transmission shall only be performed as long as dataLen is lesser
    // than the buffer size
    if (dataLen <= txBufferLen) {

        // Wait until ready
        //while (!isTWIReady()) {_delay_us(1);}

        // Copy slave address into the tx buffer.
        txBuffer[0] = slaveAddress << 1;
        //Copy all information in data to txBuffer.
        //Start for loop from index 1 since, 0 has the address
        for (uint8_t index = 1; index < dataLen; index++) {
            txBuffer[index] = data[index - 1];
        }

        //Set the data length for transmission now
        txBufferLen = static_cast<uint8_t>(dataLen + 1);
        txIndex = 0;
        TWIPerform(TWICommand::START);

/*        if (TWIInfo.mode == RepeatedStartSent)
        {
            TWIInfo.mode = Initializing;
            TWDR = txBuffer[txIndex++]; // Load data to transmit buffer
            TWIPerform(TWICommand::TRANSMIT_DATA);
        }
        else // Otherwise, just send the normal start signal to begin transmission.
        {
            TWIInfo.mode = Initializing;
            TWIPerform(TWICommand::START);
        }*/
    }
    else {
        return false;
    }
    return true;
}

bool TWI::TWIRead(const uint8_t slaveAddress, const uint8_t *data, const uint8_t dataLen)
{
    // Copy slave address into the rx buffer.
    rxBuffer[0] = static_cast<uint8_t>(slaveAddress << 1 | 0x01);

    rxBufferLen = static_cast<uint8_t>(dataLen+1);
    return true;
}

void TWI::twi_interrupt_handler()
{
    switch (TWI_STATUS) {
        /****************************************************************/
        /** Status codes for Master Transmitter Mode **/
        /****************************************************************/


    case TWI_START:
        /*A START condition has been transmitted.*/
        /** SLA+W has been transmitted;

    /** A repeated START condition has been transmitted. **/
    case TWI_RSTART:
        TWIInfo.mode = RepeatedStartSent;
        txIndex = 0;

    /** * ACK has been received. **/
    case TWI_MT_SLA_ACK:
        // Set mode to Master Transmitter
        TWIInfo.mode = MasterTransmitter;

    /** Data byte has been transmitted;
    * ACK has been received. **/
    case TWI_MT_DATA_ACK:
        if (txIndex < txBufferLen) {
            TWDR = txBuffer[txIndex++];
            TWIPerform(TWICommand::TRANSMIT_DATA);
        }
        else {
            TWIInfo.mode = Ready;
            TWIPerform(TWICommand::STOP);
        }
        break;



        /** SLA+W has been transmitted;
         * NOT ACK has been received.**/
    case TWI_MT_SLA_NACK:
        /** Data byte has been transmitted;
        * ACK has been received. **/
    case TWI_MT_DATA_NACK:
        /** Arbitration lost in SLA+W or data bytes (Transmitter) **/
    case TWI_M_ARB_LOST:
        TWIInfo.errorCode = static_cast<uint8_t>(TWI_STATUS);
        TWIInfo.mode = Ready;
        TWIPerform(TWICommand::STOP);

    default:
        break;
    }
}

ISR(TWI_vect) {
    twi.twi_interrupt_handler();
}


