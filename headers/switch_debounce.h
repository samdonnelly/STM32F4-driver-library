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

#define DEBOUNCE_NUM_SAMPLES 8 

#define DEBOUNCE_PULLUP_XOR 0xFF 

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
 *          Note that the driver support up to 8 buttons at this point. 
 * 
 * @param pull_mask 
 */
void debounce_init(
    uint8_t pull_mask); 


/**
 * @brief Switch debounce calculation 
 * 
 * @details 
 *          This function should be called using a periodic interrupt. The frequency at which 
 *          the function is called is determined by the frquency of the periodic interrupt. 
 *          The number of button samples needed to determine if a button is pressed or 
 *          released is determined by the DEBOUNCE_NUM_SAMPLES macro above. The frequency 
 *          between function calls and the number of samples gives you the debounce time. 
 *          If the buttons are for user input then the debounce time should be less than or 
 *          equal to about 50ms to prevent the user from noticing a delay between input and 
 *          action. 
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
