//
// Created by dipak on 09.11.18.
//

#ifndef ATMEGA_TWI_TWI_H
#define ATMEGA_TWI_TWI_H
#include <avr/interrupt.h>
#include <stdlib.h>

// Transmit buffer length
#ifndef TX_BUFFER_SIZE
    #define TX_BUFFER_SIZE 32
#endif

// Receiver buffer length
#ifndef RX_BUFFER_SIZE
    #define RX_BUFFER_SIZE 32
#endif

// Defining the TWI status
#define TWI_STATUS	(TWSR & 0xF8)

/****************************************************************/
/* Enumeration to hold the status of TWI bus                    */
/****************************************************************/
enum TWIStatus
{
    Ready,
    Initializing,
    RepeatedStartSent,
    MasterTransmitter,
    MasterReceiver,
    SlaveTransmitter,
    SlaveReciever
};

/****************************************************************/
/* Enumeration for TWI status codes                             */
/****************************************************************/
enum {
    /** Bus error due to illegal START or STOP condition. **/
    TWI_BUSERROR                   = 0x00,

    /** A START condition has been transmitted. **/
    TWI_START                      = 0x08,

    /** A repeated START condition has been transmitted. **/
    TWI_RSTART                     = 0x10,

    /****************************************************************/
    /** Status codes for Master Transmitter Mode **/
    /****************************************************************/
    /** SLA+W has been transmitted;
     * ACK has been received. **/
    TWI_MT_SLA_ACK                 = 0x18,

    /** SLA+W has been transmitted;
     * NOT ACK has been received.**/
    TWI_MT_SLA_NACK                = 0x20,

    /** Data byte has been transmitted;
     * ACK has been received. **/
    TWI_MT_DATA_ACK                = 0x28,

    /** Data byte has been transmitted;
     * NOT ACK has been received. **/
    TWI_MT_DATA_NACK               = 0x30,

    /** Arbitration lost in SLA+W or data bytes (Transmitter);
     Arbitration lost in SLA+R or NOT ACK bit (Receiver). **/
    TWI_M_ARB_LOST                 = 0x38,

    /****************************************************************/
    /** Status codes for Master Receiver Mode **/
    /****************************************************************/
    /**SLA+R has been transmitted;
     * ACK has been received.*/
    TWS_MR_SLA_ACK                 = 0x40,
    /**SLA+R has been transmitted;
     * NOT ACK has been received.*/
    TWS_MR_SLA_NACK                = 0x48,
    /**Data byte has been received;
     * ACK has been returned.**/
    TWS_MR_DATA_ACK                = 0x50,
    /**Data byte has been received;
     * NOT ACK has been returned.**/
    TWS_MR_DATA_NACK               = 0x58,
    TWS_ST_SLA_ACK                 = 0xA8, /* ! Own SLA+R has been received;
	                                        * ACK has been returned */
    TWS_ST_SLA_ACK_M_ARB_LOST      = 0xB0, /* ! Arbitration lost in SLA+R/W as Master;
	                                        *own SLA+R has been received;
	                                        *ACK has been returned */
    TWS_ST_DATA_ACK                = 0xB8, /* ! Data byte in TWDR has been transmitted;
	                                        *ACK has been received */
    TWS_ST_DATA_NACK               = 0xC0, /* ! Data byte in TWDR has been transmitted;
	                                        * NOT ACK has been received */
    TWS_ST_DATA_ACK_LAST_BYTE      = 0xC8, /* ! Last data byte in TWDR has been transmitted
	                                        * (TWEA = ; ACK has been received */
    TWS_SR_SLA_ACK                 = 0x60, /* ! Own SLA+W has been received
	                                        *ACK has been returned */
    TWS_SR_SLA_ACK_M_ARB_LOST      = 0x68, /* ! Arbitration lost in  SLA+R/W as Master;
	                                        * own SLA+W has been received;
	                                        * ACK has been returned */
    TWS_SR_GEN_ACK                 = 0x70, /* ! General call address has been received;
											ACK has been returned */
    TWS_SR_GEN_ACK_M_ARB_LOST      = 0x78, /* ! Arbitration lost in SLA+R/W as Master;
	                                        * General call address has been received;
	                                        *  ACK has been returned */
    TWS_SR_SLA_DATA_ACK            = 0x80, /* ! Previously addressed with own SLA+W;
	                                        * data has been received;  ACK
	                                        * has been returned */
    TWS_SR_SLA_DATA_NACK           = 0x88, /* ! Previously addressed with own SLA+W;
	                                        * data has been received;NOT ACK has been
	                                        * returned */
    TWS_SR_GEN_DATA_ACK            = 0x90, /* ! Previously addressed with general call;
	                                        * data has been received; ACK
	                                        * has been returned */
    TWS_SR_GEN_DATA_NACK           = 0x98, /* ! Previously addressed with general call;
	                                        *  data has been received;
	                                        *  NOT ACK has been returned */
    TWS_SR_STOP_RESTART            = 0xA0  /* ! A STOP condition or repeated START condition
	                                        *  has been received while still addressed
	                                        *  as Slave */
};


/****************************************************************/
/* Transmission commands                                        */
/****************************************************************/
enum class TWICommand
{
    START = 0,
    STOP = 1,
    TRANSMIT_DATA = 2,
    TRANSMIT_ACK = 3,
    TRANSMIT_NACK = 4
};

typedef struct TWIInfoStruct{
    TWIStatus mode;
    uint8_t errorCode;
    uint8_t repStart;
}TWIInfoStruct;

/****************************************************************/
/* Enumeration of TWI Bit Rate Prescaler                        */
/****************************************************************/
enum class PrescalerValue
{
    PRESCALE_VALUE_1  = 0x00,
    PRESCALE_VALUE_4  = 0x01,
    PRESCALE_VALUE_16 = 0x02,
    PRESCALE_VALUE_64 = 0x03
};
ISR(TWI_vect);

class TWI {
    friend void TWI_vect(void);
public:
    explicit TWI(PrescalerValue value = PrescalerValue::PRESCALE_VALUE_1, long twiFrequency = 100000);
    void setPrescaler(PrescalerValue value);
    void setBitRate(long twiFrequency);
    void TWIPerform(TWICommand command);
    bool isTWIReady();
    bool TWIWrite(uint8_t slaveAddress, const uint8_t *data, uint8_t dataLen);
    bool TWIRead(const uint8_t slaveAddress, const uint8_t *data, const uint8_t dataLen);
private:
    TWIInfoStruct TWIInfo;
    uint8_t TWIPrescalerValue;
    // Buffer Setup
    // Receiving buffer - Rx
    uint8_t rxBuffer[RX_BUFFER_SIZE];
    volatile uint8_t rxIndex;
    uint8_t rxBufferLen;

    // Transmission buffer - Rx
    uint8_t txBuffer[TX_BUFFER_SIZE];
    volatile uint8_t txIndex;
    uint8_t txBufferLen;
    inline void twi_interrupt_handler();

};
extern TWI twi;

#endif //ATMEGA_TWI_TWI_H
