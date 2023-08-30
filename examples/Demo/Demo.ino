/*******************************************************************************
* turboI2C Demonstation.
* This is a demo of the most useful funtions, but this program as a whole has no
* practical use.
*******************************************************************************/
#include <turbo.h>

#define I2C_SDA 10
#define I2C_SCL 11
#define I2C_ADDR 0xD0

turboi2c_t i2c;

//Array to read or write from
uint8_t demoArray[4] = {0xaa, 0xbb, 0xcc, 0xdd};

void setup() {
	//Initialise the turboi2c object
	turboi2c_init(&i2c, I2C_SDA, I2C_SCL, I2C_ADDR);
	
	//Write a single byte to the bus
	turboi2c_write_byte(&i2c, 0x50);
	
	//Write an array to the bus from register 00
	turboi2c_write_array(&i2c, 0x00, demoArray, 4);
	
	//Read an array from the bus from register 00
	turboi2c_read_array(&i2c, 0x00, demoArray, 4);
}

void loop() {

}
