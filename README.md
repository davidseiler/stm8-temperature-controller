# STM8S Temperature Controller for 3D Printer Enclosure

## Description
This repository contains the source code for a STM8S temperature controller created for maintaining a constant ambient temperature in an 3D printer enclosure. It developed for the sdcc compiler targeting STM8S.

## Part List

 1. STM8S MCU: This particular example uses a NUCLEO-8S207K8 dev board with the LQFP32 package. Requires at least 16 Kbytes of flash.
 2. 3 x DS18B20 temperature sensors.
 3. HD44780 LCD1602 Display

## Building Source

### Environment Setup

 - Install sdcc found here: https://sdcc.sourceforge.net/
    - Installed into ~/local/sdcc
 - Install stm8flash found here: https://github.com/vdudouyt/stm8flash
    - Installed into ~/local/sdcc/bin

 ### Building
 Build the source:

    make

 Build the source with UART debugging enabled:

    make debug

 
 Flash the code on the microcontroller:

    make flash

 Notes: 
  - The UART is setup through to use the same UART/pins that are used to flash the device
  - The dev board I am using has a built in stlink the version is hardcoded in the Makefile that may need to be updated for different setups
