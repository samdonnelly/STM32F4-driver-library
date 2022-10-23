/**
 * @file analog_driver.h
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

#ifndef _ANALOG_DRIVER_H_
#define _ANALOG_DRIVER_H_

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 

// Other drivers 

//=======================================================================================


//================================================================================
// Function prototypes 

/**
 * @brief ADC port initialization 
 * 
 */
void adc_port_init(void); 


/**
 * @brief ADC pin initialization 
 * 
 * @details 
 * 
 */
void adc_pin_init(void); 

//================================================================================

#endif  // _ANALOG_DRIVER_H_
