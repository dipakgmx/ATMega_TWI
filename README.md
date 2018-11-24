# ATMega TWI(I2C) interrupt based library

A C++ implementation of TWI communication of the ATMega2560.

###  Functions used:

### Function to set the transmission mode - Master or Slave
```
void TWI::TWISetMode(TWIMode requestedMode, 
                     uint8_t setSlaveAddress, 
                     PrescalerValue value, 
                     uint32_t twiFrequency)
```
`requestedMode` Sets the transmission mode. Default mode is set to Master. For master - `TWIMode::Master`, for slave - `TWIMode::Slave`  
`setSlaveAddress` Sets the address when slave mode is selected. Default value is set to 0x01.  
`value` Sets the prescaler value. Default prescaler of 1 (prescaling to 0). Use
             * `PRESCALE_VALUE_1`
             * `PRESCALE_VALUE_4`
             * `PRESCALE_VALUE_16`
             * `PRESCALE_VALUE_64`  
`twiFrequency` Sets the TWI communication frequency. Default value of 100000 (100 kHz)

### Master transmitter function to write data into the TWI bus
```
void TWI::Write(uint8_t slaveAddress,
                const uint8_t *data,
                uint8_t dataLen,
                bool repeatedStart,
                bool TWIReadRequest)
```
`slaveAddress` Address of the TWI slave device (7 bit wide)  
`data` Data to be transmitted. Sent in as an array  
`dataLen` Length of the data to be transmitted  
`repeatedStart` Boolean value, set as default to false. To be set to true if a repeated start is performed  
`TWIReadRequest` Boolean value, set as default to false. This value is used by the TWIRead function only!  
