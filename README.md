# STM32F4 Library

This repository contains device and peripheral drivers as well as general purpose software tools for the STM32F411RE chip. This library is general purpose meaning it can be applied to any STM32F411RE application that needs it. The library is written in C to provide easier compatibility with both C and C++ applications. 

Test code for this library can be found in the <a href="https://github.com/samdonnelly/STM32F4-driver-test">STM32-driver-test</a> repository. 

The library is broken into three main sections: 

### Devices 

Device drivers for external hardware. The drivers are written based on manufacturers datasheets and they utilize the peripheral drivers in this library to establish communication with the devices. These devices must be physically connected to the STM32F411RE for them to work. A list of devices supported include: 

* ESC for brushless motor 
* HC-05 Bluetooth module 
* HD44780U LCD screen 
* LSM303AGR magnetometer 
* M8Q GPS module 
* MPU-6050 IMU 
* nrF24L01 radio module 
* SD card (HW-125 SPI adapter) 
* WS2812 LEDs (Neopixels) 

### Peripherals 

Hardware peripheral drivers. These drivers are written based on the STM32F411RE datasheet and are used both internally by the chip and by the device drivers for data input and output. A list of peripherals supported include: 

* ADC 
* DMA 
* GPIO 
* I2C 
* Interrupts 
* SPI 
* Timers 
* UART 

### Tools 

General software tools to aid application and library code. These software tools include: 

* Linked list driver: 
  * Provides support for creating and reading nodes of a linked list. These lists are intented for keeping track of instances of device information. 
* STM32F411XE customized code: 
  * This code is taken from the STM32 generated code (via CubeMX) and modified to read certain chip information. More specifically, it's used to read the clock frequencies of the chip which are used for timing purposes. 
* Switch debouncing: 
  * Used to provide a software debounce for physical switch/button inputs (GPIO input via a physical switch). 
* Tools: 
  * General purpose functions and data that can be used within the library or any application. 
