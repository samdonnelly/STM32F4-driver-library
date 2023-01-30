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


extern uint8_t action; 


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
void hw125_eject_state(
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


/**
 * @brief Mount 
 * 
 * @return FRESULT 
 */
FRESULT hw125_mount(
    hw125_trackers_t *hw125_device); 


/**
 * @brief Unmount 
 * 
 * @return FRESULT 
 */
FRESULT hw125_unmount(
    hw125_trackers_t *hw125_device); 


/**
 * @brief Get the volume label 
 * 
 * @param hw125_device 
 * @return FRESULT 
 */
FRESULT hw125_getlabel(
    hw125_trackers_t *hw125_device); 


/**
 * @brief Get free space 
 * 
 * @return FRESULT 
 */
FRESULT hw125_getfree(
    hw125_trackers_t *hw125_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// FatFs volume tracker - included to ensure re-mounting is possible 
extern Disk_drvTypeDef disk; 

// Device tracker record instance 
static hw125_trackers_t hw125_device_trackers; 

// Function pointers to controller states 
static hw125_state_functions_t state_table[HW125_NUM_STATES] = 
{
    &hw125_init_state, 
    &hw125_not_ready_state, 
    &hw125_access_state, 
    &hw125_eject_state, 
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
    hw125_device_trackers.fault_mode = CLEAR; 

    // File system information 
    strcpy(hw125_device_trackers.path, path); 
    strcat(hw125_device_trackers.path, "/"); 
    memset((void *)hw125_device_trackers.dir, CLEAR, HW125_PATH_SIZE); 

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
                    next_state = HW125_ACCESS_STATE; 
                }

                // Default to the not ready state if not mounted 
                else 
                {
                    next_state = HW125_NOT_READY_STATE; 
                }
            }

            break; 

        case HW125_NOT_READY_STATE: 
            // Reset flag set 
            if (hw125_device_trackers.reset) 
            {
                next_state = HW125_RESET_STATE; 
            }

            // Drive accessible and application code clears eject flag 
            else if (!hw125_device_trackers.not_ready && !hw125_device_trackers.eject) 
            {
                next_state = HW125_INIT_STATE; 
            }

            break; 

        case HW125_ACCESS_STATE: 
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
                next_state = HW125_EJECT_STATE; 
            }

            break; 

        case HW125_EJECT_STATE: 
            next_state = HW125_NOT_READY_STATE; 
            break; 

        case HW125_FAULT_STATE: 
            // Wait for reset flag to set 
            if (hw125_device_trackers.reset) 
            {
                next_state = HW125_RESET_STATE; 
            }

            // Eject flag set 
            if (hw125_device_trackers.eject) 
            {
                next_state = HW125_EJECT_STATE; 
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
    // uart_sendstring(USART2, "\r\ninit state\r\n"); 
    // action = SET; 

    // Clear startup flag 
    hw125_device->startup = CLEAR_BIT; 

    // Clear reset flag  
    hw125_device->reset = CLEAR_BIT; 

    // Attempt to mount the drive 
    if (hw125_mount(hw125_device) == FR_OK) 
    {
        // Read the volume label and serial number 
        hw125_getlabel(hw125_device); 

        // Check free space 
        hw125_getfree(hw125_device); 
    }
    else   // Issue mounting the volume 
    {
        // Go to the not ready state 
        hw125_device->not_ready = SET_BIT; 

        // Unmount the volume 
        hw125_unmount(hw125_device); 
    }
}


// HW125 not ready state 
void hw125_not_ready_state(
    hw125_trackers_t *hw125_device)
{
    // uart_sendstring(USART2, "not ready state"); 

    // Check if the volume is present 
    if (hw125_get_existance() == HW125_RES_OK) 
    {
        // If exists then clear the not ready flag 
        hw125_device->not_ready = CLEAR_BIT; 
    }
}


// HW125 access file state 
void hw125_access_state(
    hw125_trackers_t *hw125_device) 
{
    // uart_sendstring(USART2, "access state"); 

    // Check for the presence of the volume 
    if (hw125_ready_rec()) 
    {
        // If not seen then set the not_ready flag 
        hw125_device->not_ready = SET_BIT; 
    }
}


// HW125 eject state 
void hw125_eject_state(
    hw125_trackers_t *hw125_device)
{
    // uart_sendstring(USART2, "\r\neject state\r\n"); 
    // action = SET; 

    // Attempt to close the open file 
    hw125_close(); 

    // Unmount the volume 
    hw125_unmount(hw125_device); 
}


// HW125 fault state 
void hw125_fault_state(
    hw125_trackers_t *hw125_device) 
{
    // uart_sendstring(USART2, "fault state"); 

    // Idle until the reset flag is set 
}


// HW125 reset state 
void hw125_reset_state(
    hw125_trackers_t *hw125_device) 
{
    // uart_sendstring(USART2, "\r\nreset state\r\n"); 
    // action = SET; 

    // Attempt to close a file 
    hw125_close(); 

    // Reset sub directory 
    memset((void *)hw125_device_trackers.dir, CLEAR, HW125_PATH_SIZE); 

    // Unmount the volume 
    hw125_unmount(hw125_device); 

    // Clear device trackers 
    hw125_device->fault_code = CLEAR; 
    hw125_device->fault_mode = CLEAR; 
    hw125_device->not_ready = CLEAR_BIT; 
    hw125_device->eject = CLEAR_BIT; 
}

//=======================================================================================


//=======================================================================================
// Controller volume access functions 

// Mount 
FRESULT hw125_mount(
    hw125_trackers_t *hw125_device) 
{
    hw125_device->fresult = f_mount(&hw125_device->file_sys, "", HW125_MOUNT_NOW); 

    if (hw125_device->fresult == FR_OK) 
    {
        hw125_device->mount = SET_BIT; 
    }

    return hw125_device_trackers.fresult; 
}

// Unmount 
FRESULT hw125_unmount(
    hw125_trackers_t *hw125_device) 
{
    // Unmount, clear the init status so it can be re-mounted, and clear the mount bit 
    f_unmount(""); 
    disk.is_initialized[0] = CLEAR;  // Logical drive number 0 - default number 
    hw125_device->mount = CLEAR_BIT; 

    return FR_OK; 
}


// Get the volume label 
FRESULT hw125_getlabel(
    hw125_trackers_t *hw125_device) 
{
    hw125_device->fresult = f_getlabel("", hw125_device->vol_label, &hw125_device->serial_num); 

    if (hw125_device->fresult) 
    {
        hw125_device->fault_mode |= (SET_BIT << hw125_device->fresult); 
        hw125_device->fault_code |= HW125_FAULT_COMMS; 
    }

    return hw125_device->fresult; 
}


// Get free space 
FRESULT hw125_getfree(
    hw125_trackers_t *hw125_device) 
{
    hw125_device->fresult = f_getfree("", &hw125_device->fre_clust, &hw125_device->pfs); 

    if (hw125_device->fresult == FR_OK) 
    {
        // Calculate the total space 
        hw125_device->total = (uint32_t)((hw125_device->pfs->n_fatent - 2) * 
                                          hw125_device->pfs->csize * 0.5);
        
        // Calculate the free space 
        hw125_device->free_space = (uint32_t)(hw125_device->fre_clust * 
                                              hw125_device->pfs->csize * 0.5); 

        // Check if there is sufficient disk space 
        if (hw125_device->free_space < HW125_FREE_THRESH) 
        {
            hw125_device->fault_mode |= (SET_BIT << FR_DENIED); 
            hw125_device->fault_code |= HW125_FAULT_FREE; 
        }

    }
    else   // Communication fault 
    {
        hw125_device->fault_mode |= (SET_BIT << hw125_device->fresult); 
        hw125_device->fault_code |= HW125_FAULT_COMMS; 
    }

    return hw125_device->fresult; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set the eject flag 
void hw125_set_eject_flag(void) 
{
    hw125_device_trackers.eject = SET_BIT; 
}


// Clear the eject flag 
void hw125_clear_eject_flag(void) 
{
    hw125_device_trackers.eject = CLEAR_BIT; 
}


// Set reset flag 
void hw125_set_reset_flag(void) 
{
    hw125_device_trackers.reset = SET_BIT; 
}


// Make a new directory in the project directory 
FRESULT hw125_mkdir(
    const TCHAR *dir) 
{
    // Local variables 
    TCHAR sub_dir[HW125_PATH_SIZE*2]; 

    // Record the sub directory for creating new files 
    strcpy(hw125_device_trackers.dir, dir); 
    strcat(hw125_device_trackers.dir, "/");   // Make sure paths are divided by separators 
    
    // Concatenate paths to create the sub directory 
    strcpy(sub_dir, hw125_device_trackers.path); 
    strcat(sub_dir, hw125_device_trackers.dir); 

    hw125_device_trackers.fresult = f_mkdir(sub_dir); 

    // Set fault code if there is an access error 
    if (hw125_device_trackers.fresult) 
    {
        hw125_device_trackers.fault_mode |= (SET_BIT << hw125_device_trackers.fresult); 
        hw125_device_trackers.fault_code |= HW125_FAULT_MKDIR; 
    }

    return hw125_device_trackers.fresult; 
}


// Open file 
FRESULT hw125_open(
    const TCHAR *file_name, 
    uint8_t mode) 
{
    // Local variables 
    TCHAR file_dir[HW125_PATH_SIZE*3]; 

    // Concatenate the sub directory and the file name 
    strcpy(file_dir, hw125_device_trackers.path); 
    strcat(file_dir, hw125_device_trackers.dir); 
    strcat(file_dir, file_name); 

    // Attempt to open file if a file is not already open 
    if (!hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fresult = f_open(&hw125_device_trackers.file, 
                                               file_dir, 
                                               mode); 

        if (hw125_device_trackers.fresult == FR_OK) 
        {
            // Open success 
            hw125_device_trackers.open_file = SET_BIT; 
        }
        else 
        {
            // Open fault - record the fault types 
            hw125_device_trackers.fault_mode |= (SET_BIT << hw125_device_trackers.fresult); 
            hw125_device_trackers.fault_code |= HW125_FAULT_OPEN; 
        }

        return hw125_device_trackers.fresult; 
    }

    return FR_TOO_MANY_OPEN_FILES; 
}


// Close the open file 
FRESULT hw125_close(void) 
{
    // Attempt to close a file if it's open 
    if (hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fresult = f_close(&hw125_device_trackers.file); 

        if (hw125_device_trackers.fresult) 
        {
            // Close file fault 
            hw125_device_trackers.fault_mode |= (SET_BIT << hw125_device_trackers.fresult); 
            hw125_device_trackers.fault_code |= HW125_FAULT_CLOSE; 
        }

        // Clear the open file flag regardless of the fault code 
        hw125_device_trackers.open_file = CLEAR_BIT; 

        // Update the free space 
        hw125_getfree(&hw125_device_trackers); 

        return hw125_device_trackers.fresult; 
    }

    return FR_OK; 
}


// Write to the open file 
FRESULT hw125_f_write(
    const void *buff, 
    UINT btw) 
{
    // Write to the file 
    hw125_device_trackers.fresult = f_write(&hw125_device_trackers.file, 
                                            buff, 
                                            btw, 
                                            &hw125_device_trackers.bw); 

    // Set fault code if there is an access error and a file is open 
    if (hw125_device_trackers.fresult && hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fault_mode |= (SET_BIT << hw125_device_trackers.fresult); 
        hw125_device_trackers.fault_code |= HW125_FAULT_WRITE; 
    }

    return hw125_device_trackers.fresult; 
}


// Write a string to the open file 
int8_t hw125_puts(
    const TCHAR *str) 
{
    // Writes a string to the file 
    int8_t puts_return = f_puts(str, &hw125_device_trackers.file); 

    // Set fault code if there is a function error and a file is open 
    if ((puts_return < 0) && hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fault_mode |= (SET_BIT << FR_DISK_ERR); 
        hw125_device_trackers.fault_code |= HW125_FAULT_WRITE; 
    }

    return puts_return; 
}


// Write a formatted string to the open file 
int8_t hw125_printf(
    const TCHAR *fmt_str, 
    uint16_t fmt_value) 
{
    // Writes formatted string to the file 
    int8_t printf_return = f_printf(&hw125_device_trackers.file, 
                                    fmt_str, 
                                    fmt_value); 

    // Set fault code if there is a function error and a file is open 
    if ((printf_return < 0) && hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fault_mode |= (SET_BIT << FR_DISK_ERR); 
        hw125_device_trackers.fault_code |= HW125_FAULT_WRITE; 
    }

    return printf_return; 
}


// Navigate within the open file 
FRESULT hw125_lseek(
    FSIZE_t offset) 
{
    // Move to the specified position in the file 
    hw125_device_trackers.fresult = f_lseek(&hw125_device_trackers.file, offset); 

    // Set fault code if there is an access error and a file is open 
    if (hw125_device_trackers.fresult && hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fault_mode |= (SET_BIT << hw125_device_trackers.fresult); 
        hw125_device_trackers.fault_code |= HW125_FAULT_SEEK; 
    }

    return hw125_device_trackers.fresult; 
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


// Get fault mode 
HW125_FAULT_MODE hw125_get_fault_mode(void)
{
    return hw125_device_trackers.fault_mode; 
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
    // Read from the file 
    hw125_device_trackers.fresult = f_read(&hw125_device_trackers.file, 
                                           buff, 
                                           btr, 
                                           &hw125_device_trackers.br); 
    
    // Set fault code if there is an access error and a file is open 
    if (hw125_device_trackers.fresult && hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fault_mode |= (SET_BIT << hw125_device_trackers.fresult); 
        hw125_device_trackers.fault_code |= HW125_FAULT_READ; 
    }

    return hw125_device_trackers.fresult; 
}


// Reads a string from open file 
TCHAR* hw125_gets(
    TCHAR *buff, 
    uint16_t len)
{
    // Read a string from the file 
    TCHAR *gets_return = f_gets(buff, len, &hw125_device_trackers.file); 

    // Set fault code if there was a read operation error and a file is open 
    if ((gets_return == NULL) && hw125_device_trackers.open_file) 
    {
        hw125_device_trackers.fault_mode |= (SET_BIT << FR_DISK_ERR); 
        hw125_device_trackers.fault_code |= HW125_FAULT_READ; 
    }

    return gets_return; 
}


// Test for end of file on open file 
HW125_EOF hw125_eof(void) 
{
    // Look for end of file on an open file 
    HW125_EOF eof_return = f_eof(&hw125_device_trackers.file); 

    return eof_return; 
}

//=======================================================================================
