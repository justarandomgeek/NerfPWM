/* ========================================================================== */
/*                                                                            */
/*   twi.c                                                                    */
/*   (c) 2013 Thomas Deal                                                     */
/*                                                                            */
/* ========================================================================== */


#include <util/twi.h>
#include "nerfpwm.h"


// packet used in master mode
struct packet
{
packet *next;		// pointer to next packet when in a tx/rx queue, else null
uint8_t address;	// I2C address to tx packet to, or where packet was rx'd from.
				// sort rx'd to handlers by device, then on data. 0 when addressed as slave?
uint8_t size;		// the size of the data[] buffer
uint8_t data[];	// data to transmit to the device
}


typedef void(*packet_callback)(packet*,void*)

/*

send: add packet to tx queue for ISR.
recieve:

packet *Send(*packet);
void Listen(uint8_t addr, packet_callback callback);
*/




// address in uppper 7 bits, r/w bit ignored
void init_twi(void)
{
	TWBR = 2; // TWBR=2,PS=0 at 8MHz produces 400kHz
	TWAR = 0xFE; // default value from datasheet
	TWSR = 0;
	TWCR = _BV(TWEN)|_BV(TWIE);

}

volatile uint8_t command;
volatile uint8_t buff[10];
volatile uint8_t curr;
volatile uint8_t end;




ISR(TWI_vect)
{
	uint8_t status = TW_STATUS;
	uint8_t data;
	
	
	switch (status)
	{
		case TW_ST_SLA_ACK:
			// reserved system command(s)
			if(command<CMD_USER_START)
			{
				//Currently only defined is 0x00/MY_ADDR, but all undef system commands reply with address too...
				twi_send_single_byte(TWAR & ~_BV(TWGCE));
			} else {
				//TODO: call a callback function with command
				// twi_read_callback(command)
			}		
			break;
		case TW_ST_DATA_ACK:
			// put next byte in TWDR from buffer
			TWDR = buff[curr++];
			if (curr==end) { TWCR &= ~_BV(TWEA); } else { TWCR |= _BV(TWINT)|_BV(TWEA); }
			break;
		case TW_ST_DATA_NACK:		
		case TW_ST_LAST_DATA:
			// we're out of data, switch to non-addressed and wait for address again
			TWDR = buff[curr];
			TWCR |= _BV(TWINT)|_BV(TWEA);
			break;
		
		case TW_SR_SLA_ACK:
			// we've been addressed for writing...
			curr = 0;
			TWCR |= _BV(TWINT)|_BV(TWEA);
			break;
		
		case TW_SR_DATA_ACK:
			if (curr++) // second and subsequent bytes
			{ /* no current mult-byte commands TODO: callback here*/ }
			else // first byte
			{
				// first byte received is command
				command = TWDR;
				
				/*switch (command)
				{
					// writeable addresses that take 1 byte
					case RAW_LEDS:
					case MY_ADDR:
						//TCWR &= ~_BV(TWEA);
						//break;				
					
					// read-only addresses, these are only used to set the
					// command for reading
					// RAW_BUTTONS0 RAW_BUTTONS1 RAW_BUTTONS2
					// RAW_PINKY RAW_RING RAW_MIDDLE RAW_INDEX LAST_SIGN
					// ALL_BUTTONS ALL_FLEXES SIGN_AND_BUTTONS
					default:
						TWCR &= ~_BV(TWEA);
						break;
					
				
				}*/
			}
			break;
		
		case TW_SR_DATA_NACK:
			data = TWDR;
			/*switch (command)
			{
				// writeable addresses that take 1 byte
				case MY_ADDR:
					// switch to new address
					TWAR = data & ~_BV(TWGCE);
				
					// write new address to EEPROM
					EEAR = (uintptr_t)&address_ee;
					EEDR = data & ~_BV(TWGCE);
					// no need for cli() protection here, ISR runs with interrupts disabled
					EECR |= _BV(EEMPE);
					EECR |= _BV(EEPE);
					
					
					break;				
					
				case RAW_LEDS:
					// set LEDs as specified...
					// note that wrist LED is used as an indicator in 
					// sign-recognition mode, and that state will override
					// this.
					
					//TODO: use some of the lower bits to control use of 
					// SPI lines as interrupts to notify host/master of
					// new data (button change/new raw flex/new char)
					PORTB &= ~(_BV(LED_HAND)|_BV(LED_WRIST));
					PORTB |= data & (_BV(LED_HAND)|_BV(LED_WRIST));
					break;
				
				// read-only addresses, these are only used to set the
				// command for reading
				// RAW_BUTTONS0 RAW_BUTTONS1 RAW_BUTTONS2
				// RAW_PINKY RAW_RING RAW_MIDDLE RAW_INDEX LAST_SIGN
				// ALL_BUTTONS ALL_FLEXES SIGN_AND_BUTTONS
				default:
					break;
			}
			*/
			TWCR |= _BV(TWINT)|_BV(TWEA);
			break;
		
		case TW_SR_STOP:
			TWCR |= _BV(TWINT)|_BV(TWEA);
			break;
		
		case TW_BUS_ERROR:
			// bus error, reset state
			TWCR |= _BV(TWINT)|_BV(TWSTO)|_BV(TWEA);
			break;
		case TW_NO_INFO:
		default:
			break;
	}

}