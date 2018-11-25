# ATMega TWI(I2C) interrupt based library

A C++ implementation of TWI communication of the ATMega2560.
Master and slave communication is possible.

## How to use:
Modify the `CMakeLists.txt` file:

### Set the flash option:
Setting this to `YES` flashes the generated `.hex` file into the micro-controller
```
SET(FLASH YES)
```
### Set the baud rate for flashing:
```
SET(BAUD 115200)
```
### Set the programmer for flashing:
The default Arduino programmer was used in the `CMakeLists.txt`.
```
SET(PROGRAMMER wiring) 
```
### Select the correct port for flashing:
```
set(PORT /dev/ttyACM0)
```

### Building:
Run the following within the extracted folder.
```
mkdir Build
cd Build
cmake ..
make
```
Generated files would be found in the `bin` folder within the parent folder.
If `SET(FLASH ???)` was set as `YES`, as explained earlier, the file is flashed onto the ATMega2560

### Using the library:
Typical usage  
* Calling the constructor:
  ```
  TWI twi;
  ```
* To set I2C mode as slave:
  ```
  twi.TWISetMode(TWIMode::Slave, 0x01);
  ``` 
  Here, 0x01 is the slave address.
* To set I2C mode as master:
  ```
  twi.TWISetMode(TWIMode::Master);
  ```
* To read or recieve use Read() function- depends on Master or slave:
  ```
  twi.Read();
  ```
* To transmit use Write() function- depends on Master or slave:
  ```
  twi.Write();
  ```
###  Functions used:

### Function to set the transmission mode - Master or Slave
```
void TWI::TWISetMode(TWIMode requestedMode, 
                     uint8_t setSlaveAddress, 
                     PrescalerValue value, 
                     uint32_t twiFrequency)
```
- `requestedMode` Sets the transmission mode. Default mode is set to Master. For master - `TWIMode::Master`, for 
slave - `TWIMode::Slave`  
- `setSlaveAddress` Sets the address when slave mode is selected. Default value is set to 0x01.  
- `value` Sets the prescaler value. Default prescaler of 1 (prescaling to 0). Use  
`PRESCALE_VALUE_1`  
`PRESCALE_VALUE_4`  
`PRESCALE_VALUE_16`  
`PRESCALE_VALUE_64`    
- `twiFrequency` Sets the TWI communication frequency. Default value of 100000 (100 kHz)

### Master transmitter function to write data into the TWI bus
```
void TWI::Write(uint8_t slaveAddress,
                const uint8_t *data,
                uint8_t dataLen,
                bool repeatedStart,
                bool TWIReadRequest)
```
- `slaveAddress` Address of the TWI slave device (7 bit wide)  
- `data` Data to be transmitted. Sent in as an array  
- `dataLen` Length of the data to be transmitted  
- `repeatedStart` Boolean value, set as default to false. To be set to true if a repeated start is needed  
- `TWIReadRequest` Boolean value, set as default to false. This value is used by the Read function only!  

### Overload of Write - Master transmitter function to write data into the TWI bus
Function is used to send char array to the Write function. This is necessary since the length of the character array is determined here.
```
void TWI::Write(const uint8_t slaveAddress,
                const char *const data,
                const bool repeatedStart)
```
- `slaveAddress` Address of the TWI slave device (7 bit wide)  
- ` data` char array to be transmitted  
- `repeatedStart` Boolean value, set as default to false. To be set to true if a repeated start is needed

### Slave transmitter function to write data into the TWI bus when requested by master
```
void TWI::Write(const char *const data)
```
- `data` Data to be transferred

### Function to read data in Master Receiver mode
```
void TWI::Read(const uint8_t slaveAddress,
               uint8_t *data,
               uint8_t readBytesLen,
               const bool repeatedStart)
```

- `slaveAddress` Address of the TWI slave device  
- `data` pointer to the array where the data shall be saved to  
- `readBytesLen` Number of bytes the received data shall be  
- `repeatedStart` Boolean value, set as default to false. To be set to true if a repeated start is needed  

### Overload of Read - Slave transmitter function to write data into the TWI bus
```
void TWI::Read()
```
