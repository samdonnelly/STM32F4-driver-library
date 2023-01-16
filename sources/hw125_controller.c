/**
 * @file hw125_controller.c
 * 
 * @author Sam Donnelly (sdonnelly@eneon-es.com)
 * 
 * @brief HW125 controller 
 * 
 * @version 0.1
 * @date 2023-01-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "hw125_controller.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HW125 initialization state 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_init_state(
    hw125_trackers_t *hw125_device); 


/**
 * @brief HW125 idle state 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_standby_state(
    hw125_trackers_t *hw125_device); 


/**
 * @brief HW125 open file 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_open_state(
    hw125_trackers_t *hw125_device); 


/**
 * @brief HW125 close file state 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_close_state(
    hw125_trackers_t *hw125_device); 


/**
 * @brief HW125 access file state 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_access_state(
    hw125_trackers_t *hw125_device); 


/**
 * @brief HW125 fault state 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_fault_state(
    hw125_trackers_t *hw125_device); 


/**
 * @brief HW125 reset state 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_reset_state(
    hw125_trackers_t *hw125_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Device tracker record instance 
static hw125_trackers_t hw125_device_trackers; 

// Function pointers to controller states 
static hw125_state_functions_t state_table[HW125_NUM_STATES] = 
{
    &hw125_init_state, 
    &hw125_standby_state, 
    &hw125_open_state, 
    &hw125_close_state, 
    &hw125_access_state, 
    &hw125_fault_state, 
    &hw125_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control functions 

// HW125 controller initialization 
void hw125_controller_init(void)
{
    // 
}


// HW125 controller 
void hw125_controller(void)
{
    // Local variables 
    hw125_states_t next_state = hw125_device_trackers.state; 

    //==================================================
    // State machine 

    switch (next_state)
    {
        case HW125_INIT_STATE: 
            break; 

        case HW125_STANDBY_STATE: 
            break; 

        case HW125_OPEN_STATE: 
            break; 

        case HW125_CLOSE_STATE: 
            break; 

        case HW125_ACCESS_STATE: 
            break; 

        case HW125_FAULT_STATE: 
            break; 

        case HW125_RESET_STATE: 
            break; 

        default: 
            break; 
    }

    //==================================================

    // Go to state function 
    (state_table[next_state])(&hw125_device_trackers); 

    // Update the state 
    hw125_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// HW125 initialization state 
void hw125_init_state(
    hw125_trackers_t *hw125_device) 
{
    // Clear startup bit 

    // Clear reset bit 

    // Mount the drive 

    // Check for free space - set fault code if insuficient 

    // Get and record the volume label 

    // Check for the specified directory - if it does not exist then create it 
}


// HW125 idle state 
void hw125_standby_state(
    hw125_trackers_t *hw125_device) 
{
    // Do nothing and wait for a file to be opened 
}


// HW125 open file state - state may not be needed 
void hw125_open_state(
    hw125_trackers_t *hw125_device) 
{
    // 
}


// HW125 close file state - state may not be needed 
void hw125_close_state(
    hw125_trackers_t *hw125_device) 
{
    // 
}


// HW125 access file state 
void hw125_access_state(
    hw125_trackers_t *hw125_device) 
{
    // 
}


// HW125 fault state 
void hw125_fault_state(
    hw125_trackers_t *hw125_device) 
{
    // 
}


// HW125 reset state 
void hw125_reset_state(
    hw125_trackers_t *hw125_device) 
{
    // Unmount the drive 

    // Clear the fault code 
}

//=======================================================================================


//=======================================================================================
// Setters 

// hw125_mkdir 
// - Create a new folder within the project directory 

// hw125_open 
// - Check that a file is not already open 
// - Checks for the existance of the specified file 
// - If the file doesn't exist and write mode is requested then create the file 

//=======================================================================================


//=======================================================================================
// Getters 
//=======================================================================================
