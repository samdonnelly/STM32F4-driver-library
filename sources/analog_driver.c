/**
 * @file analog_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog (ADC and DAQ) data functions 
 * 
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

// Drivers 
#include "analog_driver.h"

// Libraries 

//=======================================================================================


//================================================================================
// Initialization 

// ADC port init 
void adc_port_init(void)
{
    // Enable the ADC1 clock 
    RCC->APB2ENR |= (SET_BIT << SHIFT_8); 
}


// ADC init 
void adc_pin_init(void)
{
    // Notes: 
    //  - This function gets called for each ADC channel being initialized so don't put 
    //    code that only needs to be called once in here. 

    // Configure the clock 

    // Configure the GPIOs for analog mode 

    // Configure the ADC 
}

//================================================================================
