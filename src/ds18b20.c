#include "ds18b20.h"
#include "stm8s.h"
#include "delay.h"
#include "uart.h"

static inline void write_zero() {
    PD_ODR &= ~(1 << ONE_WIRE_BUS);
    delay_us(60);
    PD_ODR |= (1 << ONE_WIRE_BUS);
    delay_us(6);
}

static inline void write_one() {
    PD_ODR &= ~(1 << ONE_WIRE_BUS);
    delay_us(12);
    PD_ODR |= (1 << ONE_WIRE_BUS);
    delay_us(56);
}

static uint8_t read_bit() {
    // Pull low for one clock to enable read slot
    PD_ODR &= ~(1 << ONE_WIRE_BUS);
    delay_us(6);
    PD_ODR |= (1 << ONE_WIRE_BUS);
    PD_DDR &= ~(1 << ONE_WIRE_BUS);     // Enable as input
    delay_us(12);   // Sample within the first 15 us
    uint8_t res = ((PD_IDR & (1 << ONE_WIRE_BUS)) >> ONE_WIRE_BUS);
    PD_DDR |= (1 << ONE_WIRE_BUS);
    delay_us(56);
    return res;    // 0x01 or 0x00
}

static void write_byte(uint8_t data) {
    PD_DDR |= (1 << ONE_WIRE_BUS);
    for (int i = 0; i < 8; i++) {   // Bits written LSB first 
        if ((data & (1 << i)) >> i) {
            write_one();
        } else {
            write_zero();
        }
    }
}

static void read_bytes(void *buf, uint8_t num_bytes) {
    for (uint8_t i = num_bytes; i >= 1; i--) {
        for (uint8_t j = 0; j < 8; j++) {
           *((uint8_t*)buf + i - 1) |= (read_bit() << j);    // Bits arrive LSB first 
        }
    }
}

static void reset() {
    while (1) {
        // Send initilization pulse (pull down for min of 480us)
        PD_ODR &= ~(1 << ONE_WIRE_BUS);    // Pull down
        delay_us(550);

        PD_ODR |= (1 << ONE_WIRE_BUS);    // Pull up
        PD_DDR &= ~(1 << ONE_WIRE_BUS);   // Enable as input

        delay_us(60);   // Wait for one time window for DS18B20(s) to respond

        if (!(PD_IDR & (1 << ONE_WIRE_BUS))) {
            delay_us(430); // Wait for the end of slave pulse
            break;
        } 
        else {
            PRINTF("ERROR: No sensors detected on the 1-wire bus\n");
            delay_ms(500);
        }         
    }
}

// For debugging with UART
static void print_buf(uint8_t* buf, uint8_t num_bytes) {
    PRINTF("%d Bytes at %p:\n", num_bytes, buf);
    for (uint8_t i = 0; i < num_bytes; i++) {
        PRINTF("0x%02x\n", buf[i]);
    }
}

// For debugging with UART
static void print_temp(uint8_t* temp_bytes) {       // Only the first 2 bytes will be read in the order MSB, LSB
    // Assume the default 12 bit precision
    int8_t whole = (((temp_bytes[1] & 0b11110000) >> 4) | ((temp_bytes[0] & 0b00001111) << 4));
    if (temp_bytes[0] >> 7 == 1) whole += 1;    // Negative number correction
    
    // Manual 4 bit precision floating point
    uint8_t floating = ((temp_bytes[1] & 0b00001111));
    uint16_t decimal = 0;

    if ((floating & 0b00001000) >> 3) decimal += 5000;
    if ((floating & 0b00000100) >> 2) decimal += 2500;
    if ((floating & 0b00000010) >> 1) decimal += 1250;
    if (floating & 0b00000001) decimal += 625;

    PRINTF("%d.%04d degrees Celsius\n", whole, decimal);
}

void ONE_WIRE_init() {
    // Setup PIN A4 for input/output for 1-wire bus
    PD_DDR |= (1 << ONE_WIRE_BUS);    // Enable as output
    PD_CR1 |= (1 << ONE_WIRE_BUS);    // Enable as push pull when output and pull up when input
    PD_ODR |= (1 << ONE_WIRE_BUS);    // Pull up
}

uint8_t ONE_WIRE_check_crc(uint8_t* data, uint8_t data_size) {    // CRC of zero is a successful message
    // 8 bit shift register with XOR's at bit positions 0, 4, and 6
    uint8_t crc = 0x00;
    // Start from the least significant byte of the data
    for (uint8_t i = data_size; i >= 1; i--) {
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t next_bit = (data[i - 1] & (1 << j)) >> j;

            // LSB XOR with input
            next_bit ^= (crc & 0b00000001);

            // Input XOR with bit 3 and bit 4
            crc ^= next_bit << 3;           // 0b0000x000
            crc ^= next_bit << 4;           // 0b000x0000

            // Shift register
            crc >>= 1;
            crc |= next_bit << 7;
        }
    }
    return crc;
}

void ONE_WIRE_searchROMs(uint8_t *buf, uint8_t num_sensors) {
    reset();
    write_byte(0xF0);   // search ROM 0xF0

    // By process of elimination fetch each 64 bit serial number from all the ROMs on the bus
    uint8_t conflict_flag = 0;
    for (uint32_t i = num_sensors * 64; i >= 1 ; i--) {
        // Once the full 64 bits have been received from one sensor reset and run again
        if (i < (num_sensors * 64) && i % 64 == 0) {
            if (ONE_WIRE_check_crc(buf, 8) != 0) {
                // Redo last ROM search
                PRINTF(" SearchROM CRC incorrect\n");
                i -= 64;
            }
            reset();
            write_byte(0xF0);       // search ROM 0xF0
        }
        uint8_t position = 7 - ((i - 1) % 8);

        // Logical AND of all the sensor(s) ROMs connected at the current bit position
        uint8_t b1 = read_bit(); 
        // ~Complement of its LSB of its rom code at the current position
        uint8_t b2 = read_bit();        

        // 00 -> indicates device conflict (there is a zero and a one in the position)
        // 10 -> all devices have one in the current position
        // 01 -> all devices have zero in the current position

        // Master writes either a one or a zero to select devices
        // 0 selects devices with 0 at position and 1 selects devices with 1 at position 
        // Once deselected reset required to make device respond
        if (b1 && !b2) {        // 10
            write_one();
            buf[(i - 1) / 8] |= (1 << position);
        }
        else if (!b1 && b2) {   // 01
            write_zero();
        }
        else {                  // 00
            // Check the last successfully written ROM and write the opposite bit (this will start as zero)
            // For each conflict check the last conflicts and choose the XOR of all the bit choices
            for (uint8_t j = 0; j < num_sensors; j++) {
                conflict_flag ^= ((buf[((i - 1) / 8) + ((8 * j) - 1)] & (1 << (position))) >> position);
            }
            if (conflict_flag) {
                write_one();
                buf[(i - 1) / 8] |= (1 << position);
            } 
            else {
                write_zero();
            }
        }
    }
}

void ONE_WIRE_convertTemperature() {
        // Convert temperature on each sensor connected to the bus
        reset();
        
        write_byte(0xCC);   // Skip rom command: 0xCC
        write_byte(0x44);   // Convert temperature request: 0x44

        while(!read_bit()); // Wait for sensor(s) to finish the temp conversion
}

void ONE_WIRE_readScratchPad(uint8_t* rom, uint8_t* buf) {
    uint8_t crc = 1;
    while(crc != 0) {
        reset();
        write_byte(0x55);
        write_byte(rom[7]);
        write_byte(rom[6]);
        write_byte(rom[5]);
        write_byte(rom[4]);
        write_byte(rom[3]);
        write_byte(rom[2]);
        write_byte(rom[1]);
        write_byte(rom[0]);

        // Read scratchpad: 0xBE
        write_byte(0xBE);  

        read_bytes(buf, 9);
        crc = ONE_WIRE_check_crc(buf, 9);
    }
}

void ONE_WIRE_temperature_to_string(uint8_t* temp_bytes, char* buf) {
    int8_t whole = ONE_WIRE_whole_temperature(temp_bytes);
    uint16_t decimal = ONE_WIRE_decimal_temperature(temp_bytes);

    sprintf(buf, "%4d.%04d", whole, decimal);
}

int16_t ONE_WIRE_whole_temperature(uint8_t* temp_bytes) {
    uint8_t whole = (((temp_bytes[1] & 0b11110000) >> 4) | ((temp_bytes[0] & 0b00001111) << 4));
    if (temp_bytes[0] >> 7 == 1) whole += 1;    // Negative number correction
    return whole;
}

uint16_t ONE_WIRE_decimal_temperature(uint8_t* temp_bytes) {
    // Assume 12 bit precision
    // Manual 4 bit precision floating point
    uint8_t floating = ((temp_bytes[1] & 0b00001111));
    uint16_t decimal = 0;
    if ((floating & 0b00001000) >> 3) decimal += 5000;
    if ((floating & 0b00000100) >> 2) decimal += 2500;
    if ((floating & 0b00000010) >> 1) decimal += 1250;
    if (floating & 0b00000001) decimal += 625;
    return decimal;
}
