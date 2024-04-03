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

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define DEBOUNCE_NUM_SAMPLES 8        // Number of samples used to determine button status 

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief Switch debounce initialization 
 * 
 * @details Initializes the data structure that holds button debounce information. This 
 *          must be called during setup before using the other driver functions. The 
 *          pull_mask argument specifies the configuration of buttons used: 
 *          
 *          - 1 --> pull-up (normally high, goes low when pressed) 
 *          - 0 --> pull-down (normally low, goes high when pressed) 
 *          
 *          For example, if the pull_mask is 00000101, this tells the driver that 
 *          buttons 1 and 3 are pull-up buttons and the remainder are pull-downs. 
 *          Currently the driver supports up to 8 buttons, however if you use less 
 *          than 8 buttons then just leave the mask bits at 0 and don't read those 
 *          unused buttons/bits from the getters. 
 * 
 * @param pull_mask : mask to specify the configuration of the buttons 
 */
void debounce_init(
    uint8_t pull_mask); 


/**
 * @brief Switch debounce calculation 
 * 
 * @details Debounces switch input and determines the current status of each button - pressed, 
 *          released or other. The status of the switches can be read using debounce_pressed 
 *          and debounce_release. 
 *          
 *          Mechanical switches are prone to bounce which can result in unreliable inputs to 
 *          the system. To counteract this, this function samples a buttons input multiple 
 *          times to allow for bounce to settle, and only when the button state is the same 
 *          across all samples will the button be known to be pressed or released. If the 
 *          state reads neither pressed or released it means the switch is in limbo either 
 *          from bounce or because it hasn't been sampled in a single state long enough yet. 
 *          
 *          To properly use this function, it should be called using a periodic interrupt. 
 *          The number of samples needed to determine the state of a switch is defined by 
 *          the DEBOUNCE_NUM_SAMPLES macro, which in conjunction with the periodic interrupt 
 *          frequency gives you the minimum time needed to determine the state of the switch. 
 *          The button_status argument provides the current state of the switches used (GPIO 
 *          input state for the switch). Use each bit of the status argument to represent a 
 *          switch. For example if you have two switches, the first GPIOC pin 0 and the 
 *          second GPIOA pin 6, pass the status of GPIOC pin 1 as bit zero and the status of 
 *          GPIOA pin 6 as bit 1. An easy method for this is to assign all the switches to 
 *          a port (say GPIOC) and just read and pass the ports data register so you don't 
 *          have to format the bits yourself. The bit order defined by button_status will be 
 *          the same bit order read from the getters. 
 * 
 * @see debounce_pressed 
 * @see debounce_release 
 * 
 * @param button_status : GPIO input status of the switches used 
 */
void debounce(
    uint8_t button_status); 


/**
 * @brief Get pressed button status 
 * 
 * @details Returns the pressed button status. The driver supports up to 8 buttons so the 
 *          status is 1 byte (1-bit per button status). A 1 indicates the button is pressed 
 *          and a zero otherwise. To get the button released status, use debounce_released. 
 *          
 *          The button_select argument is a mask to filter which button status you want. For 
 *          example, if the mask passed here is 00010010, button 2 and 5 status will be 
 *          returned in the form of 000X00X0 where X will be either 1 for pressed or 0 
 *          otherwise. Note that the bit number in the status does not directly relate the 
 *          the pin number the button is on. The button/pin that the bit corresponds to is 
 *          defined by the button_status argument in the debounce function. 
 * 
 * @param button_select : mask to filter the button status you want 
 * @return uint8_t : button status 
 */
uint8_t debounce_pressed(
    uint8_t button_select); 


/**
 * @brief Get released button status 
 * 
 * @details Returns the released button status. The driver supports up to 8 buttons so the 
 *          status is 1 byte (1-bit per button status). A 1 indicates the button is released 
 *          and a zero otherwise. To get the button pressed status, use debounce_pressed. 
 *          
 *          The button_select argument is a mask to filter which button status you want. For 
 *          example, if the mask passed here is 01010001, button 1, 5 and 7 status will be 
 *          returned in the form of 0X0X000X where X will be either 1 for released or 0 
 *          otherwise. Note that the bit number in the status does not directly relate the 
 *          the pin number the button is on. The button/pin that the bit corresponds to is 
 *          defined by the button_status argument in the debounce function. 
 * 
 * @param button_select : mask to filter the button status you want 
 * @return uint8_t : button status 
 */
uint8_t debounce_released(
    uint8_t button_select); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _SWITCH_DEBOUNCE_H_ 
