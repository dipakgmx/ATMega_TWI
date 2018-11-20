//
// Created by dipak on 09.11.18.
//

#ifndef ATMEGA_TWI_TWI_H
#define ATMEGA_TWI_TWI_H
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>

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
/* Enumeration to determine the current state of TWI            */
/****************************************************************/

typedef enum {
    Available,
    Initializing,
    RepeatedStartSent,
    MasterTransmitter,
    MasterReceiver,
    SlaveTransmitter,
    SlaveReciever
} TWIState;

/****************************************************************/
/* TWI information struct                                       */
/****************************************************************/

typedef struct TWIInfoStruct{
    TWIState state;
    bool repStart;
}TWIInfoStruct;



/****************************************************************/
/* Enumeration for TWI status codes                             */
/****************************************************************/
enum {
    TWI_START           = 0x08,     /*A START condition has been transmitted.*/
    TWI_RESTART         = 0x10,     /*A repeated START condition has been transmitted.*/
    TWI_MT_TX_SLA_ACK   = 0x18,     /*SLA+W has been transmitted; ACK has been received.*/
    TWI_MT_TX_SLA_NACK  = 0x20,     /*SLA+W has been transmitted; NOT ACK has been received.*/
    TWI_MT_TX_DATA_ACK  = 0x28,     /*Data byte has been transmitted; ACK has been received.*/
    TWI_MT_TX_DATA_NACK = 0x30,     /*Data byte has been transmitted; NOT ACK has been received.*/
    TWI_M_ARB_LOST      = 0x38,     /*Arbitration lost in SLA+W or data bytes
                                    *(Transmitter); Arbitration lost in SLA+R or
                                    *NOT ACK bit (Receiver).*/
    TWI_SL_RX_SLA_ACK   = 0x40,     /*SLA+R has been transmitted; ACK has been received*/
    TWI_SL_RX_SLA_NACK  = 0x48,     /*SLA+R has been transmitted; NOT ACK has been received*/
    TWI_SL_RX_DATA_ACK  = 0x50,     /*Data byte has been received; ACK has been returned*/
    TWI_SL_RX_DATA_NACK = 0x58,     /*Data byte has been received; NOT ACK has been returned*/
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
    explicit TWI(PrescalerValue value = PrescalerValue::PRESCALE_VALUE_1,
        long twiFrequency = 100000);

    void setPrescaler(PrescalerValue value);

    void setBitRate(long twiFrequency);

    void TWIPerform(TWICommand command);

    bool isTWIReady();

    void TWIWrite(uint8_t slaveAddress,
                      const uint8_t *data,
                      uint8_t dataLen,
                      bool repeatedStart = false,
                      bool TWIReadRequest = false);
    void TWIWrite(const uint8_t slaveAddress,
        const char *const data,
        const bool repeatedStart = false);

    template <typename T>
    void TWIWrite(const uint8_t slaveAddress,
        const T data,
        const bool repeatedStart = false)
    {
        TWIWrite(slaveAddress, (uint8_t *) &data, sizeof(data), repeatedStart, false);
    }

    void TWIRead(const uint8_t slaveAddress,
        uint8_t* data,
        uint8_t readBytesLen,
        const bool repeatedStart = false);


private:
    uint8_t TWIPrescalerValue;

    // Function for handling the TWI_vect interrupt calls
    inline void twi_interrupt_handler();

    /** Static variables **/
    // Buffer Setup
    // Transmission buffer - Rx
    static uint8_t txBuffer[TX_BUFFER_SIZE];
    static uint8_t txIndex;
    static uint8_t txBufferLen;

    // Buffer Setup
    // Receiver buffer - Rx
    static uint8_t rxBuffer[TX_BUFFER_SIZE];
    static uint8_t rxIndex;
    static uint8_t rxBufferLen;

    static TWIInfoStruct TWIInfo;
};
extern TWI twi;

#endif //ATMEGA_TWI_TWI_H