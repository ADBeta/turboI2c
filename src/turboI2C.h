/*******************************************************************************
* Ultra Light-weight and fast Software I2C Library designed for devices 
* without hardware I2C like the ATTiny devices;
* or to communicate with multiple identical devices on one port.
* Uses Port Manipulation for high speed, using my chASM2 library
*
* Unlike SoftwareI2C this library doesn't have any special functions, i.e
* setSpeed, verbose, listDevices etc. If you want those features, use 
* SoftwareI2C : https://github.com/ADBeta/SoftwareI2C
*
* 27 Aug 2023
* Version 1.0
* ADBeta 
*******************************************************************************/
#ifndef TURBOSWI2C_H
#define TURBOSWI2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include <chASM2.h>

typedef struct {
	uint8_t i2c_address;
	chasm_t p_SCL;
	chasm_t p_SDA;
	
	bool active;
} turboi2c_t;


typedef enum {I2C_SCL, I2C_SDA} i2c_pin_t;
typedef enum {I2C_ACK = 0, I2C_NACK = 1} i2c_ack_t;

//Initialize a turboi2c struct
//Pass turboi2c pointer, SCL, SDA and the address of the device to use
void turboi2c_init(turboi2c_t *, const uint8_t scl, const uint8_t sda, 
                   const uint8_t addr);

/*** Hardware Control *********************************************************/
//Stop or start the Bus communications. Can be used with tx_byte or rx_byte
//to do a manual control I2C bus
void turboi2c_start(turboi2c_t *); 
void turboi2c_stop(turboi2c_t *);

//tx or rx a byte to/from the bus
bool turboi2c_tx_byte(turboi2c_t *, uint8_t);
uint8_t turboi2c_rx_byte(turboi2c_t *, const i2c_ack_t);

//Write a single byte to the bus (returns exit status)
bool turboi2c_write_byte(turboi2c_t *, const uint8_t);

//Write a byte to a register (returns exit status)
bool turboi2c_write_register(turboi2c_t *, const uint8_t reg, 
                             const uint8_t dat);
//Read a byte from a register
uint8_t turboi2c_read_register(turboi2c_t *, const uint8_t reg);

//From start register, read to or write from an array, n number of bytes
//Return the exit status
bool turboi2c_write_array(turboi2c_t *, const uint8_t reg, uint8_t *, 
                          const uint8_t n);
bool turboi2c_read_array(turboi2c_t *, const uint8_t reg, uint8_t *,
                         const uint8_t n);



#ifdef __cplusplus
}
#endif
#endif
