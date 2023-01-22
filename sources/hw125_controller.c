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
void hw125_not_ready_state(
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
    &hw125_not_ready_state, 
    &hw125_standby_state, 
    &hw125_access_state, 
    &hw125_fault_state, 
    &hw125_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control functions 

// HW125 controller initialization 
void hw125_controller_init(
    char *path)
{
    hw125_device_trackers.state = HW125_INIT_STATE; 

    hw125_device_trackers.fault_code = CLEAR; 

    strcpy(hw125_device_trackers.path, path); 

    memset((void *)hw125_device_trackers.data_buff, CLEAR, HW125_BUFF_SIZE); 

    memset((void *)hw125_device_trackers.vol_label, CLEAR, HW125_INFO_SIZE); 

    hw125_device_trackers.serial_num = CLEAR; 

    hw125_device_trackers.mount = CLEAR_BIT; 

    hw125_device_trackers.not_ready = CLEAR_BIT; 
    
    hw125_device_trackers.reset = CLEAR_BIT; 

    hw125_device_trackers.startup = SET_BIT; 
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
            // Make sure the init state runs at least once 
            if (!hw125_device_trackers.startup)
            {
                // Fault during mounting 
                if (hw125_device_trackers.fault_code) 
                {
                    next_state = HW125_FAULT_STATE; 
                }

                // Device not mounted but no fault 
                else if (!hw125_device_trackers.mount) 
                {
                    next_state = HW125_NOT_READY_STATE; 
                }

                // Device successfully mounted 
                else 
                {
                    next_state = HW125_STANDBY_STATE; 
                }
            }

            break; 

        case HW125_NOT_READY_STATE: 
            // Waiting period over 
            if (!hw125_device_trackers.not_ready) 
            {
                next_state = HW125_INIT_STATE; 
            }
            break; 

        case HW125_STANDBY_STATE: 
            // File access fault 
            if (hw125_device_trackers.fault_code) 
            {
                next_state = HW125_FAULT_STATE; 
            }

            // Reset flag set 
            else if (hw125_device_trackers.reset) 
            {
                next_state = HW125_RESET_STATE; 
            }

            // File opened 
            else if (hw125_device_trackers.open_file) 
            {
                next_state = HW125_ACCESS_STATE; 
            }

            break; 

        case HW125_ACCESS_STATE: 
            // File operation fault 
            if (hw125_device_trackers.fault_code) 
            {
                next_state = HW125_FAULT_STATE; 
            }

            // Reset flag set 
            if (hw125_device_trackers.reset) 
            {
                next_state = HW125_RESET_STATE; 
            }

            // File closed 
            if (!hw125_device_trackers.open_file) 
            {
                next_state = HW125_STANDBY_STATE; 
            }

            break; 

        case HW125_FAULT_STATE: 
            // Wait for reset flag to set 
            if (hw125_device_trackers.reset) 
            {
                next_state = HW125_RESET_STATE; 
            }

            break; 

        case HW125_RESET_STATE: 
            next_state = HW125_INIT_STATE; 
            break; 

        default: 
            // Default back to the init state 
            next_state = HW125_INIT_STATE; 
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
    hw125_device->startup = CLEAR_BIT; 

    // Clear reset bit 
    hw125_device->reset = CLEAR_BIT; 

    // Clear the open file flag 
    hw125_device_trackers.open_file = CLEAR_BIT; 

    // Mount the drive 

    // Check for free space - set fault code if insuficient 

    // Get and record the volume label 

    // Check for the specified directory - if it does not exist then create it 
}


// HW125 not ready state 
void hw125_not_ready_state(
    hw125_trackers_t *hw125_device)
{
    // Check for disk to be inserted using hw125_power_on 
    // --> If it is not inserted there should be a failed SPI communication (no volume 
    //     feedback) 
    // --> Once inserted (and assuming the drive works properly) the hw125_power_on 
    //     function will indicate the drive is in IDLE state and we can go back to the 
    //     init function to mount the drive. 
}


// HW125 standby state 
void hw125_standby_state(
    hw125_trackers_t *hw125_device) 
{
    // Check the existance of the card using hw125_ready_rec 
    // If the function indicates a timeout (card removed) --> go to not ready state 
    // If function indicates neither ready or timeout then go to fault 
}


// HW125 access file state 
void hw125_access_state(
    hw125_trackers_t *hw125_device) 
{
    // Check the existance of the card using hw125_ready_rec 
    // If the function indicates a timeout (card removed) or not ready (comms fault) --> 
    // close the file and go to fault state 
}


// HW125 fault state 
void hw125_fault_state(
    hw125_trackers_t *hw125_device) 
{
    // Idle until the reset flag is set 
}


// HW125 reset state 
void hw125_reset_state(
    hw125_trackers_t *hw125_device) 
{
    // If open_file set 
    // --> Close the file 
    // --> Clear the open_file flag 
    
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
    // Check that a file is not already open 
    // Checks for the existance of the specified file 
    // If the file doesn't exist and write mode is requested then create the file 
    // Set the open file flag 
}


// Close the open file 
FRESULT hw125_close(void) 
{
    // Close the already open file 
    // Update the remaining volume free space? 
    // Clear the open file flag 
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
