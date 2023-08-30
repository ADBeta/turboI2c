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
* ADBeta 
*******************************************************************************/
#include <chASM2.h>
#include "turboSWI2C.h"

/*** Initialization ***********************************************************/
void turboi2c_init(turboi2c_t *ptr, const uint8_t scl, const uint8_t sda, 
                   const uint8_t addr)
{
	ptr->i2c_address = addr;
	
	chasm_init(&ptr->p_SCL, scl);
	chasm_init(&ptr->p_SDA, sda);
	
	chasm_write(&ptr->p_SCL, LOW);
	chasm_write(&ptr->p_SDA, LOW);
	
	turboi2c_stop(ptr);
}

/** Hardware control **********************************************************/
static void wait()
{
	//TODO add an adaptable delay to this using no-op
}

static int8_t clock_stretch(turboi2c_t *ptr) {
	uint8_t clock_waits = 0;
	
	//While SCL is being pulled LOW, wait
	while(chasm_read(&ptr->p_SCL) == 0) 
	{
		wait();
		++clock_waits;
		
		//Error state, upon 10 attempts
		if(clock_waits == 10) 
		{
			return -1;
		}
	}
	
	return 0;
}

//Pulls the i2c pin low
static void assert_pin(turboi2c_t *ptr, const i2c_pin_t pin)
{
	if(pin == I2C_SCL) chasm_setmode(&ptr->p_SCL, OUTPUT);
	if(pin == I2C_SDA) chasm_setmode(&ptr->p_SDA, OUTPUT);
}

//Lets the i2c pin pullup high
static void release_pin(turboi2c_t *ptr, const i2c_pin_t pin)
{
	if(pin == I2C_SCL) chasm_setmode(&ptr->p_SCL, INPUT);
	if(pin == I2C_SDA) chasm_setmode(&ptr->p_SDA, INPUT);
}

//Sends a single bit over the bus
static void tx_bit(turboi2c_t *ptr, const bool bit) 
{
	if(bit == 0)
	{
		assert_pin(ptr, I2C_SDA);
	} else {
		release_pin(ptr, I2C_SDA);
	}
	
	//Toggle SCL pin
	wait();
	release_pin(ptr, I2C_SCL);
	wait();
	
	//Exit if clock stretching fails
	if(clock_stretch(ptr) != 0) return;
	wait();
	
	assert_pin(ptr, I2C_SCL);
}

//Reads a single bit from the bus
static bool rx_bit(turboi2c_t *ptr) 
{
	bool bit = 0;
	
	//Let the slave device drive the data line
	release_pin(ptr, I2C_SDA);
	wait();
	
	//Set clock line high to request new bit
	release_pin(ptr, I2C_SCL);
	
	//Exit if clock stretching fails
	if(clock_stretch(ptr) != 0) return 0;
	wait();
	
	//read the SDA line and set bit accordingly
	if(chasm_read(&ptr->p_SDA) == 1) 
	{
		bit = 1;
	} else {
		bit = 0;
	}
	
	assert_pin(ptr, I2C_SCL);
	return bit;
}

/*** API Functions ************************************************************/
void turboi2c_start(turboi2c_t *ptr) 
{
	//Start condition is defined by SDA going low while SCL is high
	//If repeat start is needed, first do this 
	if(ptr->active)
	{
		release_pin(ptr, I2C_SDA); //SDA high
		wait();
		release_pin(ptr, I2C_SCL); //SCL high
		//Exit if clock stretching fails
		if(clock_stretch(ptr) != 0) return;
		wait();
	}
	//Do normal start
	assert_pin(ptr, I2C_SDA); //SDA low
	wait();
	assert_pin(ptr, I2C_SCL); //SCL low
	
	ptr->active = true;
}

void turboi2c_stop(turboi2c_t *ptr)
{
	//Stop condition is defined by SDA going high while SCL is high
	assert_pin(ptr, I2C_SDA); //SDA low
	wait();
	release_pin(ptr, I2C_SCL);//SCL high
	
	//Exit if clock stretching fails
	if(clock_stretch(ptr) != 0) return;
	wait();
	
	//Set SDA high while SCL is high
	release_pin(ptr, I2C_SDA);
	wait();
	
	//If SDA is not what we expect, something went wrong
	//TODO
	
	ptr->active = false;
}

bool turboi2c_tx_byte(turboi2c_t * ptr, uint8_t byte)
{
	//transmit bits from MSB First
	for(uint8_t bit = 0; bit < 8; ++bit)
	{
		uint8_t c_bit = (byte & 0x80) != 0;
		tx_bit(ptr, c_bit);
		//Shift byte up by one
		byte = byte << 1;
	}
	
	bool ack = rx_bit(ptr);
	return ack;
}

uint8_t turboi2c_rx_byte(turboi2c_t *ptr, const i2c_ack_t ack)
{
	uint8_t byte = 0;
	
	//Read bits MSB first
	for(uint8_t bit = 0; bit < 8; ++bit)
	{
		byte = (byte << 1) | rx_bit(ptr);
	}
	
	//Write ACK bit. ACK(LOW) = ready for more data; NACK(HIGH) = stop sending
	tx_bit(ptr, ack);
	return byte;

}

bool turboi2c_write_byte(turboi2c_t *ptr, const uint8_t dat)
{
	turboi2c_start(ptr);
	
	//Send I2C Address in Write mode
	if(turboi2c_tx_byte(ptr, ptr->i2c_address) != 0) return 1;
	
	//Send single byte
	if(turboi2c_tx_byte(ptr, dat) != 0) return 1;
	
	turboi2c_stop(ptr);
	
	return 0;
}

bool turboi2c_write_register(turboi2c_t *ptr, const uint8_t reg, 
                             const uint8_t dat)
{ 	
	turboi2c_start(ptr);
	
	//Send I2C Address in Write mode
	if(turboi2c_tx_byte(ptr, ptr->i2c_address) != 0) return 1;
	//Send Register Byte
	if(turboi2c_tx_byte(ptr, reg) != 0) return 1;
	//Send Data Byte
	if(turboi2c_tx_byte(ptr, dat) != 0) return 1;
	
	turboi2c_stop(ptr);
	return 0;
}

uint8_t turboi2c_read_register(turboi2c_t *ptr, const uint8_t reg)
{ 	
	turboi2c_start(ptr);
	
	//Send I2C Address in Write mode
	if(turboi2c_tx_byte(ptr, ptr->i2c_address) != 0) return 0;
	
	//Send Register Byte
	if(turboi2c_tx_byte(ptr, reg) != 0) return 0;
	
	//Repeat start sequence
	turboi2c_start(ptr);
	
	//Send I2C Address in Read mode
	if(turboi2c_tx_byte(ptr, ptr->i2c_address | 0x01) != 0) return 0;
	
	//Read data in, Send NACK bit to end read request
	uint8_t receivedData = turboi2c_rx_byte(ptr, I2C_NACK);

	turboi2c_stop(ptr);
	return receivedData;
}

bool turboi2c_write_array(turboi2c_t *ptr, const uint8_t reg, uint8_t *arr,
                          const uint8_t n)
{
	turboi2c_start(ptr);
	
	//Send I2C Address in Write mode
	if(turboi2c_tx_byte(ptr, ptr->i2c_address) != 0) return 1;
	
	//Send Register Byte
	if(turboi2c_tx_byte(ptr, reg) != 0) return 1;
	
	//Copy read bytes to the array
	for(uint8_t b = 0; b < n; b++) {
		//Send data in the current array index.
		turboi2c_tx_byte(ptr, arr[b]);
	}
	
	turboi2c_stop(ptr);
	return 0;
}

bool turboi2c_read_array(turboi2c_t *ptr, const uint8_t reg, uint8_t *arr,
                         const uint8_t n)
{
	turboi2c_start(ptr);
	
	//Send I2C Address in Write mode
	if(turboi2c_tx_byte(ptr, ptr->i2c_address) != 0) return 1;
	
	//Send Register Byte
	if(turboi2c_tx_byte(ptr, reg) != 0) return 1;
	
	//Repeat start sequence
	turboi2c_start(ptr);
	
	//Send I2C Address in Read mode
	//Send I2C Address in Read mode
	if(turboi2c_tx_byte(ptr, ptr->i2c_address | 0x01) != 0) return 1;
	
	//Acknowledge, will be switched at last byte read
	i2c_ack_t read_ack = I2C_ACK;
	
	//Copy read bytes to the array
	for(uint8_t b = 0; b < n; b++) {
		//If the current byte is the last to be read (n - 1), change ACK to NACK
		if(b == (n-1)) read_ack = I2C_NACK;
		
		//Read data in, Send ACK or NACK.
		arr[b] = turboi2c_rx_byte(ptr, read_ack);
	}
	
	turboi2c_stop(ptr);
	return 0;
}

