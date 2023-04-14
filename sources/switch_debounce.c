/**
 * @file switch_debounce.c
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

//=======================================================================================
// Includes 

#include "switch_debounce.h"

//=======================================================================================


//=======================================================================================
// Global variables 

// Data record to hold the debounce information 
typedef struct switch_debounce_s 
{
    // Circular buffer to store multiple button state instances 
    // The data type will determine the number of buttons supported - 1-bit per button 
    uint8_t state[DEBOUNCE_NUM_SAMPLES]; 

    // Circular buffer index 
    uint8_t index; 

    // Pull mask used to specify which buttons/bits are pull-up and pull-down 
    // Pull-down --> 0 
    // Pull-up ----> 1 
    uint8_t pull_mask; 

    // Buttons pressed status - when a bit is 1 it indicates a button is pressed 
    uint8_t pressed; 

    // Buttons released status - when a bit is 1 it indicates a button is released 
    uint8_t released; 
}
switch_debounce_t; 


// Data record instance 
static switch_debounce_t debouncer; 

//=======================================================================================


//=======================================================================================
// Initialization 

// Switch debounce initialization 
void debounce_init(
    uint8_t pull_mask)
{
    // Clear the buffer 
    memset((void *)debouncer.state, CLEAR, sizeof(debouncer.state)); 

    // Set the index to zero 
    debouncer.index = CLEAR; 

    // Record the pull mask 
    debouncer.pull_mask = pull_mask; 

    // Clear the button statuses 
    debouncer.pressed = CLEAR; 
    debouncer.released = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// Debounce 

// Switch debounce calculation 
void debounce(
    uint8_t button_status)
{
    // Local variables 
    uint8_t press_pull_up = CLEAR; 
    uint8_t press_pull_down = CLEAR; 
    uint8_t release_pull_up = CLEAR; 
    uint8_t release_pull_down = CLEAR; 
    uint8_t and_results = BYTE_HIGH; 
    uint8_t or_results = CLEAR; 

    // Record the button status 
    debouncer.state[debouncer.index++] = button_status; 

    // Check the index 
    if (debouncer.index >= DEBOUNCE_NUM_SAMPLES) 
    {
        debouncer.index = CLEAR; 
    }

    // OR the button states for press pull-up and release pull-down status 
    // AND the button states for press pull-down and release pull-up status 
    for (uint8_t i = 0; i < DEBOUNCE_NUM_SAMPLES; i++)
    {
        or_results |= debouncer.state[i]; 
        and_results &= debouncer.state[i]; 
    }

    // To isolate the pressed pull-up buttons we XOR the OR results with logic high (0xFF) 
    // and AND the XOR results with the pull mask 
    press_pull_up = (BYTE_HIGH ^ or_results) & debouncer.pull_mask; 

    // To isolate the pressed pull-down buttons we NOT the pull mask and AND this with 
    // the AND results 
    press_pull_down = ~(debouncer.pull_mask) & and_results; 

    // To isolate the released pull-up buttons we AND the pull mask with the AND results 
    release_pull_up = debouncer.pull_mask & and_results; 

    // To isolate the released pull-down buttons we OR the OR results with the pull mask 
    // NOT the outcome 
    release_pull_down = ~(or_results | debouncer.pull_mask); 

    // Combine the results the get the status of pressed and released buttons irrespective 
    // of pull-up or pull-down configuration 
    debouncer.pressed = press_pull_up | press_pull_down; 
    debouncer.released = release_pull_up | release_pull_down; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get pressed button status 
uint8_t debounce_pressed(
    uint8_t button_select)
{
    return (debouncer.pressed & button_select); 
}


// Get released button status 
uint8_t debounce_released(
    uint8_t button_select)
{
    return (debouncer.released & button_select); 
}

//=======================================================================================
