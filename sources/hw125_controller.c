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
 * @brief HW125 standby state 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_standby_state(
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
 * @brief HW125 no disk file 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_no_disk_state(
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
    &hw125_access_state, 
    &hw125_no_disk_state, 
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

        case HW125_ACCESS_STATE: 
            break; 

        case HW125_NO_DISK_STATE: 
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
    // Check that media has not been removed - if so then no disk state 
}


// HW125 access file state 
void hw125_access_state(
    hw125_trackers_t *hw125_device) 
{
    // Check that media has not been removed - if so then fault state 
}


// HW125 no disk state 
void hw125_no_disk_state(
    hw125_trackers_t *hw125_device)
{
    // Check for disk to be inserted (FR_INVALID_OBJECT ?) 
    // Wait for disk then go to init state when available 
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

// Make a new directory in project directory 
FRESULT hw125_mkdir(
    const TCHAR *dir) 
{
    // Create a new folder within the project directory 
    // How to we add the specified directory onto the path? 
    // Do we store this new directory in memory or are we already in the new folder? 
}


// Open file 
FRESULT hw125_open(
    const TCHAR *file_name, 
    uint8_t mode) 
{
    // - Check that a file is not already open 
    // - Checks for the existance of the specified file 
    // - If the file doesn't exist and write mode is requested then create the file 
}


// Close the open file 
FRESULT hw125_close(void) 
{
    // Close the already open file 
    // Update the remaining volume free space? 
}


// Write to the open file 
FRESULT hw125_f_write(
    const void *buff, 
    UINT btw) 
{
    // 
}


// Navigate within the open file 
FRESULT hw125_lseek(
    FSIZE_t offset) 
{
    // 
}


// Write a character to the open file 
int8_t hw125_putc(
    TCHAR character) 
{
    // 
}


// Write a string to the open file 
int16_t hw125_puts(
    const TCHAR *str) 
{
    // 
}


// Write a formatted string to the open file 
int16_t hw125_printf(
    const TCHAR *fmt_str) 
{
    // 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get state 
HW125_STATE hw125_get_state(void) 
{
    return hw125_device_trackers.state; 
}


// Get fault code 
HW125_FAULT_CODE hw125_get_fault_code(void) 
{
    return hw125_device_trackers.fault_code; 
}


// Get open file flag 
HW125_FILE_STATUS hw125_get_file_status(void)
{
    return hw125_device_trackers.open_file; 
}


// Read data from open file 
FRESULT hw125_f_read(
    void *buff, 
    UINT btr) 
{
    // 
}


// Reads a string from open file 
void hw125_gets(
    TCHAR *buff, 
    uint16_t len)
{
    // 
}


// Test for end of file on open file 
HW125_EOF hw125_eof(void) 
{
    // 
}

//=======================================================================================
