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
 * @brief HW125 no disk file 
 * 
 * @details 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
void hw125_not_ready_state(
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
 * @brief HW125 e-stop state 
 * 
 * @details 
 * 
 * @param hw125_device 
 */
void hw125_estop_state(
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
    &hw125_estop_state, 
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
    // Controller information 
    hw125_device_trackers.state = HW125_INIT_STATE; 
    hw125_device_trackers.fault_code = CLEAR; 
    hw125_device_trackers.fault_code_check = CLEAR; 

    // File system information 
    strcpy(hw125_device_trackers.path, path); 

    // Volume tracking - is this needed? 
    memset((void *)hw125_device_trackers.vol_label, CLEAR, HW125_INFO_SIZE); 
    hw125_device_trackers.serial_num = CLEAR; 

    // Data buffers 
    memset((void *)hw125_device_trackers.data_buff, CLEAR, HW125_BUFF_SIZE); 

    // State trackers 
    hw125_device_trackers.mount = CLEAR_BIT; 
    hw125_device_trackers.not_ready = CLEAR_BIT; 
    hw125_device_trackers.eject = CLEAR_BIT; 
    hw125_device_trackers.open_file = CLEAR_BIT; 
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
                // Fault during drive access 
                if (hw125_device_trackers.fault_code) 
                {
                    next_state = HW125_FAULT_STATE; 
                }

                // Device successfully mounted 
                else if (hw125_device_trackers.mount) 
                {
                    next_state = HW125_STANDBY_STATE; 
                }

                // Default to the not ready state if not mounted 
                else 
                {
                    next_state = HW125_NOT_READY_STATE; 
                }
            }

            break; 

        case HW125_NOT_READY_STATE: 
            // Drive accessible and app code clears eject 
            if (!hw125_device_trackers.not_ready && !hw125_device_trackers.eject) 
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

            // Volume not seen or eject flag set 
            else if (hw125_device_trackers.not_ready || hw125_device_trackers.eject)
            {
                next_state = HW125_NOT_READY_STATE; 
            }

            // File opened 
            else if (hw125_device_trackers.open_file) 
            {
                next_state = HW125_ACCESS_STATE; 
            }

            break; 

        case HW125_ACCESS_STATE: 
            // File operation fault, reset flag set or eject flag set 
            if (hw125_device_trackers.fault_code || 
                hw125_device_trackers.reset || 
                hw125_device_trackers.eject) 
            {
                next_state = HW125_ESTOP_STATE; 
            }

            // File closed 
            if (!hw125_device_trackers.open_file) 
            {
                next_state = HW125_STANDBY_STATE; 
            }

            break; 

        case HW125_ESTOP_STATE: 
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

            // Volume eject request 
            if (!hw125_device_trackers.eject) 
            {
                next_state = HW125_NOT_READY_STATE; 
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
    // Clear startup flag 
    hw125_device->startup = CLEAR_BIT; 

    // Clear reset flag  
    hw125_device->reset = CLEAR_BIT; 

    // Clear the fault code check 
    hw125_device->fault_code_check = CLEAR; 

    // Mount the drive 
    // - If this fails then set the not ready flag and skip the next steps 
    hw125_device->fresult = f_mount(&hw125_device->file_sys, "", HW125_MOUNT_NOW); 

    if (hw125_device->fresult == FR_OK) 
    {        
        // Check free space 
        hw125_device->fresult = f_getfree("", &hw125_device->fre_clust, &hw125_device->pfs); 

        if (hw125_device->fresult == FR_OK) 
        {
            // Read the volume label and serial number 

            // Calculate the total space - may be unnecessary to calculate this 
            hw125_device->total = (uint32_t)((hw125_device->pfs->n_fatent - 2) * 
                                              hw125_device->pfs->csize * 0.5);
            
            // Calculate the free space 
            hw125_device->free_space = (uint32_t)(hw125_device->fre_clust * 
                                                  hw125_device->pfs->csize * 0.5); 

            // Check the free space against a threshold - need to add a threshold 
            // If free space is less than threshold then set fault code 

            // Navigate to directory specified by 'path' and create folder for the day 
            // - Where do we get the new folder name from? 
            // - If the specified path doesn't exist then create it 
            // - If this fails then set the fault code 
        }
        else 
        {
            // Set the fault code 
        }
    }
    else 
    {
        hw125_device->not_ready = SET_BIT; 
    }
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
    // TODO need to add a return value and SPI timeout 
    hw125_power_on(); 

    // If inserted then clear not ready flag 

    // Clear the fault code check 
    hw125_device->fault_code_check = CLEAR; 
}


// HW125 standby state 
void hw125_standby_state(
    hw125_trackers_t *hw125_device) 
{
    // Check the existance of the card using hw125_ready_rec 
    // - If the function indicates a timeout (card removed) then set the not 
    //   ready flag 
    // - If function indicates neither ready or timeout then go to fault 
    // TODO need to add a return value to this function 
    hw125_ready_rec(); 
    
    // Check fault code check against an f_open and f_mkdir mask 
    if (hw125_device->fault_code_check & (HW125_FAULT_OPEN | HW125_FAULT_MKDIR)) 
    {
        hw125_device->fault_code = (HW125_FAULT_OPEN | HW125_FAULT_MKDIR); 
    }
}


// HW125 access file state 
void hw125_access_state(
    hw125_trackers_t *hw125_device) 
{
    // Check for the existance of the drive uisng hw125_ready_rec 
    // - If not present then set the not ready flag 
    hw125_ready_rec(); 

    // Check the fault code check 
    // - If true then set the fault code 
    if (hw125_device->fault_code_check) 
    {
        hw125_device->fault_code = hw125_device->fault_code_check; 
    }
}


// HW125 e-stop state 
void hw125_estop_state(
    hw125_trackers_t *hw125_device)
{
    // Attempt to close the open file 
    // Clear the open file flag 
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
    // Clear the fault code 
    hw125_device->fault_code = CLEAR; 
    
    // If open file flag set then attempt to close the file 
    
    // Clear the open file flag 
    hw125_device->open_file = CLEAR_BIT; 

    // Attempt to unmount the drive 

    // Clear the mount flag 

    // memset the data buffer 
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

    // Attempt f_open 
    // - If successful then set open file flag 
}


// Close the open file 
FRESULT hw125_close(void) 
{
    // Close the already open file 
    // Update the remaining volume free space? 
    // Clear the open file flag 

    // Attempt f_close 
    // - If successful then clear open file flag 
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
