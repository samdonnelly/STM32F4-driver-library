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
    // Record the button status 
    debouncer->state[debouncer->index++] = button_status; 

    // Check the index 
    if (debouncer->index >= DEBOUNCE_NUM_BUTTONS) 
    {
        debouncer->index = CLEAR; 
    }

    // OR (for pull-up) and AND (for pull-down) all the button states 
    for (uint8_t i = 0; i < DEBOUNCE_NUM_BUTTONS; i++)
    {
        // OR 
        // AND 
    }

    // XOR the combined button state (for pull-ups) to identify all the buttons reading 
    // a debounced low 

    // AND the results to see which pull-up buttons and pull-down buttons are pressed 

    // OR the pull-up and pull-down results to get final button pressed results, 
    // irrespective of pull-up or pull-down configuration 
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
