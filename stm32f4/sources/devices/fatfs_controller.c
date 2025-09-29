/**
 * @file fatfs_controller.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FATFS controller 
 * 
 * @version 0.1
 * @date 2023-01-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "fatfs_controller.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief FATFS initialization state 
 * 
 * @details Attempts to mount the volume. If successful, the project directory saved in 
 *          fatfs_trackers_t will be made if it does not already exist and the state 
 *          machine will move to the "access" state. If unsuccessful, the "not ready" state
 *          will be entered. The init state is called on startup and controller reset as 
 *          well as from the "not ready" state once a device is seen. If there is a fault 
 *          during one of the volume read operations, excluding the mounting procedure, 
 *          then the fault flag will be set and the "fault" state will be entered. 
 * 
 * @param fatfs_device : device tracker that defines control characteristics 
 */
void fatfs_init_state(fatfs_trackers_t *fatfs_device); 


/**
 * @brief FATFS not ready state 
 * 
 * @details Continuously checks for the presence of the volume to see if it shows up. This 
 *          state indicates that the volume is not seen by the system (not ready flag set) 
 *          or the user has intentions to remove the volume (eject flag set). This state
 *          is entered from the "init" state if volume mounting is unsuccessful or from
 *          the "eject" state. This state is left when both the not ready flag (which 
 *          indicates a device is present) and the eject flag (which indicates the user is 
 *          ready to use the volume) are cleared at which point the "init" state is 
 *          entered. This state can also be left if the reset flag gets set. The fault 
 *          flag cannot be set here as it assumes the volume is not present so no volume 
 *          faults can occur. 
 *          
 *          Note that being in this state will render the use of the read/write getters/
 *          setters useless. If you're in this state then either the disk is not present 
 *          so these reads/writes won't work or the eject flag is set which will unmount 
 *          the device and therefore make the reads/writes not possible. 
 * 
 * @param fatfs_device : device tracker that defines control characteristics 
 */
void fatfs_not_ready_state(fatfs_trackers_t *fatfs_device); 


/**
 * @brief FATFS access state 
 * 
 * @param fatfs_device : data record 
 */
void fatfs_access_state(fatfs_trackers_t *fatfs_device); 


/**
 * @brief FATFS access check state 
 * 
 * @details Continuously checks for the presence of the volume to see if it goes missing. If 
 *          it is missing the not ready flag will be set. This state indicates that the volume 
 *          is seen by the system and normal read/write operations can be performed (using the 
 *          getters and setters). This state is entered from the "init" state when mounting is 
 *          successful. It is left when the fault or reset flags are set, or if either of the 
 *          not ready or eject flags are set. 
 * 
 * @param fatfs_device : device tracker that defines control characteristics 
 */
void fatfs_access_check_state(fatfs_trackers_t *fatfs_device); 


/**
 * @brief FATFS eject state 
 * 
 * @details If there is an open file this state closes it then unmounts the volume. This state 
 *          is triggered by the eject flag being set which is set by the application through 
 *          the fatfs_set_eject_flag setter. This can be used to safely remove the disk while 
 *          the system is still powered on. After this state is run it immediately goes to 
 *          the "not ready" state. The eject flag is cleared through the application code using 
 *          the fatfs_clear_eject_flag setter. 
 * 
 * @see fatfs_set_eject_flag 
 * @see fatfs_clear_eject_flag 
 * 
 * @param fatfs_device : device tracker that defines control characteristics 
 */
void fatfs_eject_state(fatfs_trackers_t *fatfs_device); 


/**
 * @brief FATFS fault state 
 * 
 * @details Currently this state does nothing until the reset flag is set. This state can be 
 *          entered from the "init" or "access" states after the fault flag has been set. The 
 *          fault flag can be set in the getter/setter functions when a FATFS file operation 
 *          is performed if one of these operations is not successful. To leave this state, the 
 *          reset flag has to be set using the fatfs_set_reset_flag setter at which point the 
 *          "reset" state will be entered. 
 * 
 * @see fatfs_set_reset_flag 
 * 
 * @param fatfs_device : device tracker that defines control characteristics 
 */
void fatfs_fault_state(fatfs_trackers_t *fatfs_device); 


/**
 * @brief FATFS reset state 
 * 
 * @details Closes any open file, resets the projects subdirectory, unmounts the volumes and 
 *          resets controller trackers as needed. This state is triggered by setting the reset 
 *          flag using fatfs_set_reset_flag and can be entered from the "not ready", "access" or 
 *          "fault" states. This state is called typically when there is an issue in the system 
 *          and it needs to re-initialize itself. After this state is run it will go directly to 
 *          the "init" state and the reset flag will be automatically cleared. 
 * 
 * @see fatfs_set_reset_flag 
 * 
 * @param fatfs_device : device tracker that defines control characteristics 
 */
void fatfs_reset_state(fatfs_trackers_t *fatfs_device); 


/**
 * @brief Mount the volume 
 * 
 * @details Attempts to mount the volume and returns the file operation status. If the mount 
 *          is successful then the mount flag will be set. This function is called by the "init" 
 *          state and is needed before read/write operations can be performed on the volume. 
 * 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT fatfs_mount(fatfs_trackers_t *fatfs_device); 


/**
 * @brief Unmount the volume 
 * 
 * @details This function unmounts the volume. It also clears the init status flag in the FATFS 
 *          code memory (which is why 'disk' is included as an extern below) which is needed in 
 *          order to be able to re-mount the volume without a power cycle. The mount flag is 
 *          also cleared. This function is called buy the "init" state if mounting fails, and 
 *          also by the "eject" and "reset" states. If power remains on in the system, then 
 *          unmounting should be done before removing the volume. 
 *          
 *          Note that this controller/driver only support a single volume right now so this 
 *          function is hard coded to unmount logical drive 0 which is the default volume 
 *          number and number assigned to the volumes used. 
 * 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT fatfs_unmount(fatfs_trackers_t *fatfs_device); 


/**
 * @brief Get the volume label 
 * 
 * @details Reads the volume label and stores it in the controller tracker data structure if 
 *          needed. This function is called during the "init" state if mounting was 
 *          successful and will update the fault code in the process if something goes wrong 
 *          while reading the label. 
 * 
 * @param fatfs_device : data record 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT fatfs_getlabel(fatfs_trackers_t *fatfs_device); 


/**
 * @brief Get free space 
 * 
 * @details Checks the free space of the volume. This function is called after successful 
 *          mounting of the volume in the "init" state. The free space is checked against 
 *          a threshold to ensure there is sufficient space for the system to record data. 
 *          If the free space is below the threshold then the fault flag is set. 
 * 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT fatfs_getfree(fatfs_trackers_t *fatfs_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// FatFs volume tracker - included to ensure re-mounting is possible 
extern Disk_drvTypeDef disk; 

// Device tracker record instance 
static fatfs_trackers_t fatfs_device_trackers; 

// Function pointers to controller states 
static fatfs_state_functions_t state_table[FATFS_NUM_STATES] = 
{
    &fatfs_init_state, 
    &fatfs_not_ready_state, 
    &fatfs_access_state, 
    &fatfs_access_check_state, 
    &fatfs_eject_state, 
    &fatfs_fault_state, 
    &fatfs_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control functions 

// FATFS controller initialization 
void fatfs_controller_init(const char *path)
{
    // Check that the path length is not too long 

    // Controller information 
    fatfs_device_trackers.state = FATFS_INIT_STATE; 
    fatfs_device_trackers.fault_code = CLEAR; 
    fatfs_device_trackers.fault_mode = CLEAR; 

    // File system information 
    memset((void *)fatfs_device_trackers.path, CLEAR, FATFS_PATH_SIZE); 
    strcpy(fatfs_device_trackers.path, path); 
    memset((void *)fatfs_device_trackers.dir, CLEAR, FATFS_PATH_SIZE); 
    
    // State trackers 
    fatfs_device_trackers.mount = CLEAR_BIT; 
    fatfs_device_trackers.not_ready = CLEAR_BIT; 
    fatfs_device_trackers.check = CLEAR_BIT; 
    fatfs_device_trackers.eject = CLEAR_BIT; 
    fatfs_device_trackers.open_file = CLEAR_BIT; 
    fatfs_device_trackers.startup = SET_BIT; 
}


// FATFS controller 
void fatfs_controller(void)
{
    fatfs_states_t next_state = fatfs_device_trackers.state; 

    //==================================================
    // State machine 

    switch (next_state)
    {
        case FATFS_INIT_STATE: 
            // Make sure the init state runs at least once 
            if (!fatfs_device_trackers.startup)
            {
                // Fault during drive access 
                if (fatfs_device_trackers.fault_code) 
                {
                    next_state = FATFS_FAULT_STATE; 
                }

                // Device successfully mounted and access check requested 
                else if (fatfs_device_trackers.mount && fatfs_device_trackers.check) 
                {
                    next_state = FATFS_ACCESS_CHECK_STATE; 
                }

                // Device successfully mounted 
                else if (fatfs_device_trackers.mount) 
                {
                    next_state = FATFS_ACCESS_STATE; 
                }

                // Default to the not ready state if not mounted 
                else 
                {
                    next_state = FATFS_NOT_READY_STATE; 
                }
            }

            break; 

        case FATFS_NOT_READY_STATE: 
            // Reset flag set 
            if (fatfs_device_trackers.reset) 
            {
                next_state = FATFS_RESET_STATE; 
            }

            // Drive accessible and application code clears eject flag 
            else if (!fatfs_device_trackers.not_ready && !fatfs_device_trackers.eject) 
            {
                next_state = FATFS_INIT_STATE; 
            }

            break; 

        case FATFS_ACCESS_STATE: 
            // File access fault 
            if (fatfs_device_trackers.fault_code) 
            {
                next_state = FATFS_FAULT_STATE; 
            }

            // Reset flag set 
            else if (fatfs_device_trackers.reset) 
            {
                next_state = FATFS_RESET_STATE; 
            }

            // Eject flag set 
            else if (fatfs_device_trackers.eject)
            {
                next_state = FATFS_EJECT_STATE; 
            }

            // Check flag set 
            else if (fatfs_device_trackers.check)
            {
                next_state = FATFS_ACCESS_CHECK_STATE; 
            }

            break; 

        case FATFS_ACCESS_CHECK_STATE: 
            // File access fault 
            if (fatfs_device_trackers.fault_code) 
            {
                next_state = FATFS_FAULT_STATE; 
            }

            // Reset flag set 
            else if (fatfs_device_trackers.reset) 
            {
                next_state = FATFS_RESET_STATE; 
            }

            // Volume not seen or eject flag set 
            else if (fatfs_device_trackers.not_ready || fatfs_device_trackers.eject)
            {
                next_state = FATFS_EJECT_STATE; 
            }

            // Check flag cleared 
            else if (!fatfs_device_trackers.check)
            {
                next_state = FATFS_ACCESS_STATE; 
            }

            break; 

        case FATFS_EJECT_STATE: 
            // Default to the not ready state if the eject flag has been set 
            next_state = FATFS_NOT_READY_STATE; 
            break; 

        case FATFS_FAULT_STATE: 
            // Wait for reset flag to set 
            if (fatfs_device_trackers.reset) 
            {
                next_state = FATFS_RESET_STATE; 
            }

            // Eject flag set 
            if (fatfs_device_trackers.eject) 
            {
                next_state = FATFS_EJECT_STATE; 
            }

            break; 

        case FATFS_RESET_STATE: 
            next_state = FATFS_INIT_STATE; 
            break; 

        default: 
            // Default back to the init state 
            next_state = FATFS_INIT_STATE; 
            break; 
    }

    //==================================================

    // Go to state function 
    (state_table[next_state])(&fatfs_device_trackers); 

    // Update the state 
    fatfs_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// FATFS initialization state 
void fatfs_init_state(fatfs_trackers_t *fatfs_device) 
{
    // Clear startup flag 
    fatfs_device->startup = CLEAR_BIT; 

    // Clear reset flag 
    fatfs_device->reset = CLEAR_BIT; 

    // Attempt to mount the volume 
    if (fatfs_mount(fatfs_device) == FR_OK) 
    {
        // Mounting successful 
        // Read the volume, serial number, free space and make the directory specified by 
        // "path" if it does not exist 
        fatfs_getlabel(fatfs_device); 
        fatfs_getfree(fatfs_device); 
        fatfs_mkdir(""); 
    }
    else 
    {
        // Mounting unsuccessful 
        // Go to the not ready state and unmount the volume 
        fatfs_device->not_ready = SET_BIT; 
        fatfs_unmount(fatfs_device); 
    }
}


// FATFS not ready state 
void fatfs_not_ready_state(fatfs_trackers_t *fatfs_device)
{
    // Check if the volume is present 
    if (fatfs_get_existance() == FATFS_RES_OK) 
    {
        // Present - clear the not ready flag so we can try remounting 
        fatfs_device->not_ready = CLEAR_BIT; 
    }
}


// FATFS access state 
void fatfs_access_state(fatfs_trackers_t *fatfs_device)
{
    // Do nothing while the volume is accessed 
}


// FATFS access file state 
void fatfs_access_check_state(fatfs_trackers_t *fatfs_device) 
{
    // Check for the presence of the volume 
    if (fatfs_ready_rec()) 
    {
        // If not seen then set the not_ready flag 
        fatfs_device->not_ready = SET_BIT; 
    }
}


// FATFS eject state 
void fatfs_eject_state(fatfs_trackers_t *fatfs_device)
{
    // Attempt to close the open file 
    fatfs_close(); 

    // Unmount the volume 
    fatfs_unmount(fatfs_device); 
}


// FATFS fault state 
void fatfs_fault_state(fatfs_trackers_t *fatfs_device) 
{
    // Idle until the reset flag is set 
}


// FATFS reset state 
void fatfs_reset_state(fatfs_trackers_t *fatfs_device) 
{
    // Attempt to close a file 
    fatfs_close(); 

    // Reset sub directory 
    memset((void *)fatfs_device_trackers.dir, CLEAR, FATFS_PATH_SIZE); 

    // Unmount the volume 
    fatfs_unmount(fatfs_device); 

    // Clear device trackers 
    fatfs_device->fault_code = CLEAR; 
    fatfs_device->fault_mode = CLEAR; 
    fatfs_device->not_ready = CLEAR_BIT; 
    fatfs_device->eject = CLEAR_BIT; 
}

//=======================================================================================


//=======================================================================================
// Controller volume access functions 

// Mount the volume 
FRESULT fatfs_mount(fatfs_trackers_t *fatfs_device) 
{
    fatfs_device->fresult = f_mount(&fatfs_device->file_sys, "", FATFS_MOUNT_NOW); 

    if (fatfs_device->fresult == FR_OK) 
    {
        fatfs_device->mount = SET_BIT; 
    }

    return fatfs_device_trackers.fresult; 
}


// Unmount the volume 
FRESULT fatfs_unmount(fatfs_trackers_t *fatfs_device) 
{
    // Unmount, clear the init status so it can be re-mounted, and clear the mount bit 
    f_unmount(""); 
    disk.is_initialized[FATFS_VOL_NUM_0] = CLEAR;  // Logical drive number 0 - default number 
    fatfs_device->mount = CLEAR_BIT; 

    return FR_OK; 
}


// Get the volume label 
FRESULT fatfs_getlabel(fatfs_trackers_t *fatfs_device) 
{
    fatfs_device->fresult = f_getlabel("", fatfs_device->vol_label, &fatfs_device->serial_num); 

    if (fatfs_device->fresult) 
    {
        fatfs_device->fault_mode |= (SET_BIT << fatfs_device->fresult); 
        fatfs_device->fault_code |= (SET_BIT << FATFS_FAULT_COMMS); 
    }

    return fatfs_device->fresult; 
}


// Get free space 
FRESULT fatfs_getfree(fatfs_trackers_t *fatfs_device) 
{
    fatfs_device->fresult = f_getfree("", &fatfs_device->fre_clust, &fatfs_device->pfs); 

    if (fatfs_device->fresult == FR_OK) 
    {
        // Calculate the total space 
        // (n_fatent - 2) * csize / 2
        fatfs_device->total = (uint32_t)(((fatfs_device->pfs->n_fatent - 2) * 
                                           fatfs_device->pfs->csize) >> SHIFT_1);
        
        // Calculate the free space 
        // fre_clust * csize / 2 
        fatfs_device->free_space = (uint32_t)((fatfs_device->fre_clust * 
                                               fatfs_device->pfs->csize) >> SHIFT_1); 

        // Check if there is sufficient disk space 
        if (fatfs_device->free_space < FATFS_FREE_THRESH) 
        {
            fatfs_device->fault_mode |= (SET_BIT << FR_DENIED); 
            fatfs_device->fault_code |= (SET_BIT << FATFS_FAULT_FREE); 
        }

    }
    else   // Communication fault 
    {
        fatfs_device->fault_mode |= (SET_BIT << fatfs_device->fresult); 
        fatfs_device->fault_code |= (SET_BIT << FATFS_FAULT_COMMS); 
    }

    return fatfs_device->fresult; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set the check flag 
void fatfs_set_check_flag(void)
{
    fatfs_device_trackers.check = SET_BIT; 
}


// Clear the check flag 
void fatfs_clear_check_flag(void)
{
    fatfs_device_trackers.check = CLEAR_BIT; 
}


// Set the eject flag 
void fatfs_set_eject_flag(void) 
{
    fatfs_device_trackers.eject = SET_BIT; 
}


// Clear the eject flag 
void fatfs_clear_eject_flag(void) 
{
    fatfs_device_trackers.eject = CLEAR_BIT; 
}


// Set reset flag 
void fatfs_set_reset_flag(void) 
{
    fatfs_device_trackers.reset = SET_BIT; 
}


// Set directory 
void fatfs_set_dir(const TCHAR *dir)
{
    // Reset the saved directory and set the new directory 
    memset((void *)fatfs_device_trackers.dir, CLEAR, FATFS_PATH_SIZE); 
    strcpy(fatfs_device_trackers.dir, dir); 
}


// Make a new directory in the project directory 
FRESULT fatfs_mkdir(const TCHAR *dir) 
{
    // Check for NULL pointer 
    if (dir == NULL) 
    {
        return FR_INVALID_OBJECT; 
    }
    
    TCHAR sub_dir[FATFS_PATH_SIZE*2]; 

    // Record 'dir' for future use and establish 'path' as the base of the sub directory 
    fatfs_set_dir(dir); 
    strcpy(sub_dir, fatfs_device_trackers.path); 

    // If 'dir' is not a null character then prepare the sub-directory to be concatenated. 
    // 'dir' will be a null character when it's empty such as in the "init" state. 
    if (*fatfs_device_trackers.dir != NULL_CHAR)
    {
        strcat(sub_dir, "/"); 
    }

    // Concatenate 'dir' to complete the sub directory string 
    strcat(sub_dir, fatfs_device_trackers.dir); 

    // Check for the existance of the directory 
    fatfs_device_trackers.fresult = f_stat(sub_dir, (FILINFO *)NULL); 

    // Only proceed to make the directory if it does not exist 
    if (fatfs_device_trackers.fresult)
    {
        fatfs_device_trackers.fresult = f_mkdir(sub_dir); 

        // Set fault code if there is an access error 
        if (fatfs_device_trackers.fresult) 
        {
            fatfs_device_trackers.fault_mode |= (SET_BIT << fatfs_device_trackers.fresult); 
            fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_DIR); 
        }
    }

    return fatfs_device_trackers.fresult; 
}


// Open file 
FRESULT fatfs_open(
    const TCHAR *file_name, 
    uint8_t mode) 
{
    // Check for NULL pointers and strings 
    if ((file_name == NULL) || (*file_name == NULL_CHAR)) 
    {
        return FR_INVALID_OBJECT; 
    }

    TCHAR file_dir[FATFS_PATH_SIZE*3]; 

    // Attempt to open file if a file is not already open 
    if (!fatfs_device_trackers.open_file) 
    {
        // Establish 'path' as the root of the file directory 
        strcpy(file_dir, fatfs_device_trackers.path); 

        // If 'dir' is not a null character then concatenate it to the file directory 
        if (*fatfs_device_trackers.dir != NULL_CHAR)
        {
            strcat(file_dir, "/"); 
            strcat(file_dir, fatfs_device_trackers.dir); 
        }

        strcat(file_dir, "/"); 
        strcat(file_dir, file_name); 
        
        fatfs_device_trackers.fresult = f_open(&fatfs_device_trackers.file, 
                                               file_dir, 
                                               mode); 

        if (fatfs_device_trackers.fresult == FR_OK) 
        {
            fatfs_device_trackers.open_file = SET_BIT; 
        }
        else   // Open fault - record the fault types 
        {
            fatfs_device_trackers.fault_mode |= (SET_BIT << fatfs_device_trackers.fresult); 
            fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_OPEN); 
        }

        return fatfs_device_trackers.fresult; 
    }

    return FR_TOO_MANY_OPEN_FILES; 
}


// Close the open file 
FRESULT fatfs_close(void) 
{
    // Attempt to close a file if it's open 
    if (fatfs_device_trackers.open_file) 
    {
        fatfs_device_trackers.fresult = f_close(&fatfs_device_trackers.file); 

        if (fatfs_device_trackers.fresult) 
        {
            // Close file fault 
            fatfs_device_trackers.fault_mode |= (SET_BIT << fatfs_device_trackers.fresult); 
            fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_CLOSE); 
        }

        // Clear the open file flag regardless of the fault code 
        fatfs_device_trackers.open_file = CLEAR_BIT; 

        // Update the free space 
        fatfs_getfree(&fatfs_device_trackers); 

        return fatfs_device_trackers.fresult; 
    }

    return FR_OK; 
}


// Write to the open file 
FRESULT fatfs_f_write(
    const void *buff, 
    UINT btw) 
{
    // Check for void pointer? 
    // Check for open file? 

    // Write to the file 
    fatfs_device_trackers.fresult = f_write(&fatfs_device_trackers.file, 
                                            buff, 
                                            btw, 
                                            &fatfs_device_trackers.bw); 

    // Set fault code if there is an access error and a file is open 
    if (fatfs_device_trackers.fresult && fatfs_device_trackers.open_file) 
    {
        fatfs_device_trackers.fault_mode |= (SET_BIT << fatfs_device_trackers.fresult); 
        fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_WRITE); 
    }

    return fatfs_device_trackers.fresult; 
}


// Write a string to the open file 
int16_t fatfs_puts(const TCHAR *str) 
{
    // Check for void pointer? 
    // Check for open file? 

    // Writes a string to the file 
    int16_t puts_return = f_puts(str, &fatfs_device_trackers.file); 

    // Set fault code if there is a function error and a file is open 
    if ((puts_return < 0) && fatfs_device_trackers.open_file) 
    {
        fatfs_device_trackers.fault_mode |= (SET_BIT << FR_DISK_ERR); 
        fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_WRITE); 
    }

    return puts_return; 
}


// Write a formatted string to the open file 
int8_t fatfs_printf(
    const TCHAR *fmt_str, 
    uint16_t fmt_value) 
{
    // Check for void pointer? 
    // Check for open file? 

    // Writes a formatted string to the file 
    int8_t printf_return = f_printf(&fatfs_device_trackers.file, 
                                    fmt_str, 
                                    fmt_value); 

    // Set fault code if there is a function error and a file is open 
    if ((printf_return < 0) && fatfs_device_trackers.open_file) 
    {
        fatfs_device_trackers.fault_mode |= (SET_BIT << FR_DISK_ERR); 
        fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_WRITE); 
    }

    return printf_return; 
}


// Navigate within the open file 
FRESULT fatfs_lseek(FSIZE_t offset) 
{
    // Move to the specified position in the file 
    fatfs_device_trackers.fresult = f_lseek(&fatfs_device_trackers.file, offset); 

    // Set fault code if there is an access error and a file is open 
    if (fatfs_device_trackers.fresult && fatfs_device_trackers.open_file) 
    {
        fatfs_device_trackers.fault_mode |= (SET_BIT << fatfs_device_trackers.fresult); 
        fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_SEEK); 
    }

    return fatfs_device_trackers.fresult; 
}


// Delete a file 
FRESULT fatfs_unlink(const TCHAR* filename)
{
    // Check that path validity 
    if (filename == NULL) 
    {
        return FR_INVALID_OBJECT; 
    }

    TCHAR file_dir[FATFS_PATH_SIZE*3]; 

    // Establish 'path' as the root of the file directory 
    strcpy(file_dir, fatfs_device_trackers.path); 

    // If 'dir' is not a null character then concatenate it to the file directory 
    if (*fatfs_device_trackers.dir != NULL_CHAR)
    {
        strcat(file_dir, "/"); 
        strcat(file_dir, fatfs_device_trackers.dir); 
    }

    strcat(file_dir, "/"); 
    strcat(file_dir, filename); 

    // Attempt to delete the specified file 
    fatfs_device_trackers.fresult = f_unlink(file_dir); 

    // Set the fault code if the file failed to be deleted 
    if (fatfs_device_trackers.fresult)
    {
        fatfs_device_trackers.fault_mode |= (SET_BIT << fatfs_device_trackers.fresult); 
        fatfs_device_trackers.fault_code |= (SET_BIT << FATFS_FAULT_DIR); 
    }

    return fatfs_device_trackers.fresult; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get state 
FATFS_STATE fatfs_get_state(void) 
{
    return fatfs_device_trackers.state; 
}


// Get fault code 
FATFS_FAULT_CODE fatfs_get_fault_code(void) 
{
    return fatfs_device_trackers.fault_code; 
}


// Get fault mode 
FATFS_FAULT_MODE fatfs_get_fault_mode(void)
{
    return fatfs_device_trackers.fault_mode; 
}


// Get open file flag 
FATFS_FILE_STATUS fatfs_get_file_status(void)
{
    return fatfs_device_trackers.open_file; 
}


// Check for the existance of a file or directory 
FRESULT fatfs_get_exists(const TCHAR *str)
{
    // Check for a valid file name 
    if ((str == NULL) || (*str == NULL_CHAR)) 
    {
        return FR_INVALID_OBJECT; 
    }

    // Local variables 
    TCHAR directory[FATFS_PATH_SIZE*3]; 

    // Establish 'path' as the root of the file directory 
    strcpy(directory, fatfs_device_trackers.path); 

    // If 'dir' is not a null character then concatenate it to the file directory 
    if (*fatfs_device_trackers.dir != NULL_CHAR)
    {
        strcat(directory, "/"); 
        strcat(directory, fatfs_device_trackers.dir); 
    }

    strcat(directory, "/"); 
    strcat(directory, str); 

    // Check for the existance of the directory 
    return f_stat(directory, (FILINFO *)NULL); 
}


// Read data from open file 
FRESULT fatfs_f_read(
    void *buff, 
    UINT btr) 
{
    // Read from the file 
    fatfs_device_trackers.fresult = f_read(&fatfs_device_trackers.file, 
                                           buff, 
                                           btr, 
                                           &fatfs_device_trackers.br); 
    
    // Set fault code if there is an access error and a file is open 
    if (fatfs_device_trackers.fresult && fatfs_device_trackers.open_file) 
    {
        fatfs_device_trackers.fault_mode |= (SET_BIT << fatfs_device_trackers.fresult); 
        fatfs_device_trackers.fault_code |= FATFS_FAULT_READ; 
    }

    return fatfs_device_trackers.fresult; 
}


// Reads a string from open file 
TCHAR* fatfs_gets(
    TCHAR *buff, 
    uint16_t len)
{
    // Read a string from the file 
    TCHAR *gets_return = f_gets(buff, len, &fatfs_device_trackers.file); 

    // Set fault code if there was a read operation error and a file is open 
    if ((gets_return == NULL) && (!fatfs_eof()) && fatfs_device_trackers.open_file) 
    {
        fatfs_device_trackers.fault_mode |= (SET_BIT << FR_DISK_ERR); 
        fatfs_device_trackers.fault_code |= FATFS_FAULT_READ; 
    }

    return gets_return; 
}


// Test for end of file on open file 
FATFS_EOF fatfs_eof(void) 
{
    return (FATFS_EOF)f_eof(&fatfs_device_trackers.file); 
}

//=======================================================================================
