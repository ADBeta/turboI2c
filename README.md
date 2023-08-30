# turboI2C
turboI2C is an Arduino Library that is extrememly lightweight, very fast and 
very simple to use; it supports the ATTiny family natively, and provides near
hardware I2C speeds.

This library can be used to control multiple divices with the same address,
by using different SCL or SDA pins, for when the I2C Device cannot have its 
address changed.

**NOTE you will need [chASM2](https://github.com/ADBeta/chASM2) installed in
order to use this library**

## Usage
To create a turboI2C device ready for use, you need to create a struct, then  
initialise it with the values required
```c
turboi2c_t device;

void setup() {
	turboi2c_init(&device, scl_pin, sda_pin, address);
}
```

These are the general use functions available from turboI2C
```c
//Write a single byte to the bus (returns exit status)
bool turboi2c_write_byte(turboi2c_t *, const uint8_t);

//Write a byte to a register (returns exit status)
bool turboi2c_write_register(turboi2c_t *, const uint8_t reg, const uint8_t dat);

//Read a byte from a register
uint8_t turboi2c_read_register(turboi2c_t *, const uint8_t reg);

//From a register, read to or write from an array, n number of bytes. Return the exit status
bool turboi2c_write_array(turboi2c_t *, const uint8_t reg, uint8_t *, const uint8_t n);

bool turboi2c_read_array(turboi2c_t *, const uint8_t reg, uint8_t *, const uint8_t n);
```

These functions are direct hardware control, they can be used to create custom
data structures.
```c
//Start an I2C transmission, with double start handling
void turboi2c_start(turboi2c_t *); 

//Stop the I2C transmission
void turboi2c_stop(turboi2c_t *);

//Transmit or Receive a byte from the I2C bus. does not handle stop/start of
//the I2C bus
bool turboi2c_tx_byte(turboi2c_t *, uint8_t);
//i2c_ack_t is I2C_ACK or I2C_NACK
uint8_t turboi2c_rx_byte(turboi2c_t *, const i2c_ack_t);
```

## TODO
* Do stricter arbitration and stress tests
* add actual wait() delay

--------------------------------------------------------------------------------
<b>(c) ADBeta 2023</b>
turboI2C is under the GPL2.0 License, please see LICENSE for information
