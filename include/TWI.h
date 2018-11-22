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
    TWI_START                   = 0x08,     /*!< A START condition has been transmitted.*/
    TWI_RESTART                 = 0x10,     /*!< A repeated START condition has been transmitted.*/
    TWI_MT_SLA_ACK              = 0x18,     /*!< SLA+W has been transmitted; ACK has been received.*/
    TWI_MT_SLA_NACK             = 0x20,     /*!< SLA+W has been transmitted; NOT ACK has been received.*/
    TWI_MT_DATA_ACK             = 0x28,     /*!< Data byte has been transmitted; ACK has been received.*/
    TWI_MT_DATA_NACK            = 0x30,     /*!< Data byte has been transmitted; NOT ACK has been received.*/
    TWI_M_ARB_LOST              = 0x38,     /*!< Arbitration lost in SLA+W or data bytes  (Transmitter); Arbitration
                                            * lost in SLA+R or NOT ACK bit (Receiver).*/
    TWI_MR_SLA_ACK              = 0x40,     /*!< SLA+R has been transmitted; ACK has been received*/
    TWI_MR_SLA_NACK             = 0x48,     /*!< SLA+R has been transmitted; NOT ACK has been received*/
    TWI_MR_DATA_ACK             = 0x50,     /*!< Data byte has been received; ACK has been returned*/
    TWI_MR_DATA_NACK            = 0x58,     /*!< Data byte has been received; NOT ACK has been returned*/

    /** Slave transmitter mode **/
    TWI_ST_SLA_ACK              = 0xA8,     /*!< Own SLA+R has been received; ACK has been returned */
    TWS_ST_SLA_ACK_M_ARB_LOST   = 0xB0,     /*!< Arbitration lost in SLA+R/W as Master; own SLA+R has been received;
                                            * ACK has been returned*/
    TWS_ST_DATA_ACK             = 0xB8,     /*!< Data byte in TWDR has been transmitted; ACK has been received */
    TWS_ST_DATA_NACK            = 0xC0,     /*!< Data byte in TWDR has been transmitted;  NOT ACK has been received */
    TWS_ST_DATA_ACK_LAST_BYTE   = 0xC8,     /*!< Last data byte in TWDR has been transmitted (TWEA = ; ACK has been
                                            * received */

};

/****************************************************************/
/* Transmission commands                                        */
/****************************************************************/
enum class TWICommand
{
    START           = 0,        /*!< Transmit START condition */
    STOP            = 1,        /*!< Transmit STOP condition */
    TRANSMIT_DATA   = 2,        /*!< Transmit data in master transmitter mode */
    TRANSMIT_ACK    = 3,        /*!< Transmit ACK */
    TRANSMIT_NACK   = 4,        /*!< Transmit NACK */
    ENABLE_SLAVE    = 5         /*!< Enable slave mode */
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

/****************************************************************/
/* Enumeration of TWI Bit Rate Prescaler                        */
/****************************************************************/
enum class TWIMode
{
    Master  = 0x00,     /*!< Set TWI communication as Master */
    Slave   = 0x01      /*!< Set TWI communication as Slave */
};

ISR(TWI_vect);

class TWI {
    friend void TWI_vect(void);

public:
    explicit TWI();

    void TWISetMode(TWIMode requestedMode = TWIMode::Master,
                    uint8_t setSlaveAddress = 0x01,
                    PrescalerValue value = PrescalerValue::PRESCALE_VALUE_1,
                    uint32_t twiFrequency = 100000);

    void setPrescaler(PrescalerValue value);

    void setBitRate(uint32_t twiFrequency);

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

    void TWIWrite(const char *const data);

    void TWIRead(const uint8_t slaveAddress,
        uint8_t* data,
        uint8_t readBytesLen,
        const bool repeatedStart = false);
    
    bool GetAvailability();

private:
    uint8_t TWIPrescalerValue;
    TWIMode mode;
    uint8_t slaveModeAddress;


    // Function for handling the TWI_vect interrupt calls
    inline void twi_interrupt_handler();

    /** Static variables **/
    // Buffer Setup
    // Transmission buffer - Rx
    static uint8_t txBuffer[TX_BUFFER_SIZE]; /*!< Transmission buffer to hold values before being sent on the TWI */
    static uint8_t txIndex;    /*!< Current index within the transmission buffer (txBuffer) */
    static uint8_t txBufferLen; /*!< Current size of the transmission buffer. Depends on the length of the data to be
 * transmitted */

    // Buffer Setup
    // Receiver buffer - Rx
    static uint8_t rxBuffer[TX_BUFFER_SIZE]; /*!< Reception buffer to hold values before being sent on the TWI */
    static uint8_t rxIndex; /*!< Current index within the receiver buffer (txBuffer) */
    static uint8_t rxBufferLen; /*!< Current size of the receiver buffer. Depends on the length of the data to be
 * transmitted */

    static TWIInfoStruct TWIInfo;
};
extern TWI twi;

#endif //ATMEGA_TWI_TWI_H