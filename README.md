# STM8S Temperature Controller for 3D Printer Enclosure

## Description
This repository contains the source code for a STM8S temperature controller for maintaining a constant ambient temperature in an 3D printer enclosure. It is developed for the sdcc targeting STM8S003K3.

## Parts List

 1. STM8S003K3 MCU: This particular example uses a custom PCB with the TSSOP20 package. Requires at least 8 Kbytes of flash.
 2. 3 x DS18B20 temperature sensors.
 3. HD44780 LCD1602 Display with or without an I2C I/O expander
 4. 4-wire PWM controllable fan
 5. EC11 rotary encoder
 6. RT6224D Step-Down Buck Converter

 For a full list of all parts with part numbers see /docs/parts.csv 


## Building the Source

### Environment Setup

 - Install sdcc found here: https://sdcc.sourceforge.net/
    - Installed into ~/local/sdcc
 - Install stm8flash found here: https://github.com/vdudouyt/stm8flash
    - Installed into ~/local/sdcc/bin

 ### Building
 Build the source with default options (4-bit parallel for LCD):

    make

 Build the source with UART debugging enabled:

     make UART=1

 Build the source for use with I2C LCD display:

    make I2C=1

 
 Flash the code on the microcontroller:

    make flash
