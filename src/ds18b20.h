#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Pin mappings
DS18B20 -- STM8S   
DQ -> PC3
*/

#define ONE_WIRE_BUS 3

/* Setup 1-wire bus */
void ONE_WIRE_init();

/* Checks the CRC following the CRC Generator in the datasheet.
   Returns 0 on valid data */
uint8_t ONE_WIRE_check_crc(uint8_t* data, uint8_t data_size);

/* Fetches each serial rom from the sensors on the bus
   Each ROM is 64 bits, buffer must be sized accordingly */
void ONE_WIRE_searchROMs(uint8_t *buf, uint8_t num_sensors);

/* Instructs all sensors on the 1-wire bus to convert temperature from
   analog to digital. For 12 bit presicion this may take up to 750 ms */
void ONE_WIRE_convertTemperature();

/* Fetches the scratchpad memory from the sensor with matching the ROM
   ROM must be 64 bits exactly and buf must be at least 9 bytes */
void ONE_WIRE_readScratchPad(uint8_t* rom, uint8_t* buf);

/* Pass in a 9 byte buffer and writes formatted temperature to the buffer
   in the format of SWWW.DDDD */
void ONE_WIRE_temperature_to_string(uint8_t* temp_bytes, char* buf);

/* Returns the signed whole number part of the temperature
   Can be within -55 Celsius to +125 Celsius */
int16_t ONE_WIRE_whole_temperature(uint8_t* temp_bytes);

/* Returns the decimal part of the temperature stored as an unsigned integer 
   10,000 equals 1 degree Celsius */
uint16_t ONE_WIRE_decimal_temperature(uint8_t* temp_bytes);


#endif