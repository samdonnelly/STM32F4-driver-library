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
// Function Prototypes 
//=======================================================================================


//=======================================================================================
// Global variables 

// Data record to hold the debounce information 
typedef struct switch_debounce_s 
{
    // Circular buffer to store multiple button state instances 
    uint8_t state[DEBOUNCE_NUM_BUTTONS]; 

    // Circular buffer index 
    uint8_t index; 

    // Pull mask used to specify which buttons/bits are pull-up and pull-down 
    // Pull-down --> 0 
    // Pull-up ----> 1 
    uint8_t pull_mask; 
}
switch_debounce_t; 


// Data record instance 
static switch_debounce_t *debouncer; 

//=======================================================================================


//=======================================================================================
// Initialization 

// Switch debounce initialization 
void debounce_init(
    uint8_t pull_mask)
{
    // Clear the buffer 
    memset((void *)debouncer->state, CLEAR, sizeof(debouncer->state)); 

    // Set the index to zero 
    debouncer->index = CLEAR; 

    // Record the pull mask 
    debouncer->pull_mask = pull_mask; 
}

//=======================================================================================


//=======================================================================================
// Debounce 

// Switch debounce calculation 
void debounce(
    uint8_t button_status)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Switch debounce pressed buttons 
uint8_t debounce_pressed(
    uint8_t button_select)
{
    // 
}


// Switch debounce released buttons 
uint8_t debounce_released(
    uint8_t button_select)
{
    // 
}

//=======================================================================================
