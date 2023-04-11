/**
 * @file switch_debounce.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Switch debouce code 
 * 
 * @version 0.1
 * @date 2023-04-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _SWITCH_DEBOUNCE_H_ 
#define _SWITCH_DEBOUNCE_H_ 

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define DEBOUNCE_NUM_BUTTONS 8 

//=======================================================================================


//=======================================================================================
// Structures 
//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief Switch debounce initialization 
 * 
 * @details 
 * 
 * @param pull_mask 
 */
void debounce_init(
    uint8_t pull_mask); 


/**
 * @brief Switch debounce calculation 
 * 
 * @details 
 * 
 * @param button_status 
 */
void debounce(
    uint8_t button_status); 


/**
 * @brief Switch debounce pressed buttons 
 * 
 * @details 
 * 
 * @param button_select 
 * @return uint8_t 
 */
uint8_t debounce_pressed(
    uint8_t button_select); 


/**
 * @brief Switch debounce released buttons 
 * 
 * @details 
 * 
 * @param button_select 
 * @return uint8_t 
 */
uint8_t debounce_released(
    uint8_t button_select); 

//=======================================================================================

#endif   // _SWITCH_DEBOUNCE_H_ 
