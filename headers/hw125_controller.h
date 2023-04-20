/**
 * @file hw125_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 controller 
 * 
 * @version 0.1
 * @date 2023-01-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _HW125_CONTROLLER_H_ 
#define _HW125_CONTROLLER_H_ 

//=======================================================================================
// Includes 

// Drivers 
#include "hw125_driver.h" 

// STM drivers 
#include "fatfs.h"

//=======================================================================================


//=======================================================================================
// Macros 

// State machine 
#define HW125_NUM_STATES 6             // Number of possible states for the controller 

// Controller tracker 
#define HW125_PATH_SIZE 50             // Volume path max length 
#define HW125_INFO_SIZE 30             // Device info buffer size 
#define HW125_FREE_THRESH 0x0000C350   // Free space threshold before disk full fault (KB) 

// Volume numbers 
#define HW125_VOL_NUM_0 0              // Logical drive number 0 (default number) 
#define HW125_VOL_NUM_1 1              // Logical drive number 1 
#define HW125_VOL_NUM_2 2              // Logical drive number 2 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HW125 controller states 
 */
typedef enum {
    HW125_INIT_STATE, 
    HW125_NOT_READY_STATE, 
    HW125_ACCESS_STATE, 
    HW125_EJECT_STATE, 
    HW125_FAULT_STATE, 
    HW125_RESET_STATE 
} hw125_states_t; 


/**
 * @brief 
 */
typedef enum {
    HW125_FAULT_MKDIR = 0x01,            // Make directory 
    HW125_FAULT_OPEN  = 0x02,            // Open 
    HW125_FAULT_CLOSE = 0x04,            // Close 
    HW125_FAULT_WRITE = 0x08,            // Write 
    HW125_FAULT_READ  = 0x10,            // Read 
    HW125_FAULT_SEEK  = 0x20,            // Seek 
    HW125_FAULT_FREE  = 0x40,            // Free space 
    HW125_FAULT_COMMS = 0x80             // Comms 
} hw125_fault_codes_t; 

//=======================================================================================


//=======================================================================================
// Structures 

// HW125 controller trackers 
typedef struct hw125_trackers_s 
{
    // Controller information 
    hw125_states_t state;                        // State of the controller 
    hw125_fault_codes_t fault_code;              // Fault code 
    DWORD fault_mode;                            // Fault mode - based on FRESULT 

    // File system information 
    FATFS file_sys;                              // File system object 
    FIL file;                                    // File object 
    FRESULT fresult;                             // Store result of FatFs operation 
    UINT br, bw;                                 // Read and write counters 
    TCHAR path[HW125_PATH_SIZE];                 // Path to project directory 
    TCHAR dir[HW125_PATH_SIZE];                  // Sub-directory in project directory 

    // Card capacity 
    FATFS *pfs;                                  // Pointer to file system object 
    DWORD fre_clust;                             // Free clusters 
    DWORD total, free_space;                     // Volume total and free space 
    
    // Volume tracking 
    TCHAR vol_label[HW125_INFO_SIZE];            // Volume label 
    DWORD serial_num;                            // Volume serial number 

    // State trackers 
    uint8_t mount      : 1;                      // Volume mount flag 
    uint8_t not_ready  : 1;                      // Not ready flag 
    uint8_t eject      : 1;                      // Eject flag - set by setters 
    uint8_t open_file  : 1;                      // Open file flag 
    uint8_t reset      : 1;                      // Reset state trigger 
    uint8_t startup    : 1;                      // Ensures the init state is run 
}
hw125_trackers_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef hw125_states_t HW125_STATE; 
typedef hw125_fault_codes_t HW125_FAULT_CODE; 
typedef DWORD HW125_FAULT_MODE; 
typedef uint8_t HW125_FILE_STATUS; 
typedef int8_t HW125_EOF; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief HW125 state machine function pointer 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 */
typedef void (*hw125_state_functions_t)(
    hw125_trackers_t *hw125_device); 

//=======================================================================================


//=======================================================================================
// Control functions 

/**
 * @brief HW125 controller initialization 
 * 
 * @details Initializes the controller tracker information. The 'path' argument specifies 
 *          the directory where files will be saved on the volume. This directory is the 
 *          applications root directory and all subsequent folders and files will be saved 
 *          here. This allows for files from different applications to be easily separated. 
 *          Note that the path length must be less than HW125_PATH_SIZE to prevent overrun. 
 *          If the path length is too long then the project/applications directory will not 
 *          be set. 
 * 
 * @param path : path to directory to use on the volume 
 */
void hw125_controller_init(
    char *path); 


/**
 * @brief HW125 controller 
 * 
 * @details Contains the state machine to control the flow of the controller. Should be 
 *          called continuously by the application while the device is in use. 
 */
void hw125_controller(void); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief Set the eject flag 
 * 
 * @details The eject flag is intended to put the controller in the "eject" state which 
 *          preps the volume for removal. This flag is set by the application if the user 
 *          wants to remove the volume while the system still has power. 
 */
void hw125_set_eject_flag(void); 


/**
 * @brief Clear the eject flag 
 * 
 * @details The eject flag must be cleared in order for the volume to be properly mounted 
 *          and used. This setter is only needed after hw125_set_eject_flag has been called. 
 */
void hw125_clear_eject_flag(void); 


/**
 * @brief Set reset flag 
 * 
 * @details The reset flag triggers a controller reset. This flag will be cleared 
 *          automatically after being set. 
 */
void hw125_set_reset_flag(void); 


/**
 * @brief Make a new directory in project directory 
 * 
 * @details Wrapper function for the FATFS function f_mkdir. 
 *          
 *          Concatenates the argument 'dir' onto the project/application 'path' (defined in 
 *          the controller init function), checks for the existance of the new directory and 
 *          creates the directory if it does not already exist. If there is an error creating 
 *          the directory then the fault code will be set accordingly. 
 *          
 *          Since 'dir' is added onto the project path with each function call, passing 
 *          different directories will create new folders in parallel to one another. If 
 *          further sub-directories are desired then 'dir' must contain an existing directory 
 *          with the sub-directory added to the end and separated by a "/". 
 *          
 *          The length of 'dir' and 'path' together should be less than twice the length of 
 *          HW125_PATH_SIZE. 
 *          
 *          If 'dir' is an invalid pointer then the function will return before attempting to 
 *          create a directory. 
 * 
 * @param dir : sub directory to creae within the project directory 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT hw125_mkdir(
    const TCHAR *dir); 


/**
 * @brief Open file 
 * 
 * @details Wrapper function for the FATFS function f_open. 
 *          
 *          Concatenates the file name ('file_name') onto the project directory and attempts to 
 *          open the specified file. If there is an error opening the file then the fault code 
 *          will be updated accordingly. Note that if a subdirectory for the project has been 
 *          created using hw125_mkdir then the file will be made in that directory. If you 
 *          want the file in a different directory then use hw125_mkdir to update the 
 *          subdirectory accordingly (can specify 'dir' as "" in hw125_mkdir to go to the 
 *          project root directory). 
 *          
 *          When the function attempts to open the specified file it will use the method 
 *          specified by 'mode' to do so. For example, if you specify HW125_MODE_W as the mode 
 *          then the function will create a file if it does not already exist and open it in 
 *          write mode. See the HW125 driver header for possible modes. 
 *           
 *          If a file is already open then there will be no attempt to open another. The result 
 *          can be observed in the return value. 
 * 
 * @param file_name : name of the file to open 
 * @param mode : mode to open the file in (read, write, etc.) 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT hw125_open(
    const TCHAR *file_name, 
    uint8_t mode); 


/**
 * @brief Close the open file 
 * 
 * @details Wrapper function for the FATFS function f_close. 
 * 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT hw125_close(void); 


/**
 * @brief Write to the open file 
 * 
 * @details Wrapper function for the FATFS function f_write. 
 * 
 * @param buff 
 * @param btw 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT hw125_f_write(
    const void *buff, 
    UINT btw); 


/**
 * @brief Write a string to the open file 
 * 
 * @details Wrapper function for the FATFS function f_puts. 
 *          
 *          If there is a fault the fault mode will always read FR_DISK_ERR. f_puts is a 
 *          wrapper of f_write and if there is an error of any kind in f_write then the 
 *          return of f_puts is negative. There is no distinguishing (that is know) of 
 *          fault/error types. 
 * 
 * @param str 
 * @return int16_t 
 */
int8_t hw125_puts(
    const TCHAR *str); 


/**
 * @brief Write a formatted string to the open file 
 * 
 * @details Wrapper function for the FATFS function f_printf. 
 * 
 * @param fmt_str : 
 * @param fmt_value : 
 * @return int8_t 
 */
int8_t hw125_printf(
    const TCHAR *fmt_str, 
    uint16_t fmt_value); 


/**
 * @brief Navigate within the open file 
 * 
 * @details Wrapper function for the FATFS function f_lseek. 
 * 
 * @param offset 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT hw125_lseek(
    FSIZE_t offset); 

//=======================================================================================


//=======================================================================================
// Getters 

/**
 * @brief Get state 
 * 
 * @return HW125_STATE 
 */
HW125_STATE hw125_get_state(void); 


/**
 * @brief Get fault code 
 * 
 * @return HW125_FAULT_CODE 
 */
HW125_FAULT_CODE hw125_get_fault_code(void); 


/**
 * @brief Get fault mode 
 * 
 * @return HW125_FAULT_MODE 
 */
HW125_FAULT_MODE hw125_get_fault_mode(void); 


/**
 * @brief Get open file flag 
 * 
 * @return HW125_FILE_STATUS 
 */
HW125_FILE_STATUS hw125_get_file_status(void); 


/**
 * @brief Read data from open file 
 * 
 * @details Wrapper function for the FATFS function f_read. 
 * 
 * @param buff 
 * @param btr 
 * @return FRESULT : FATFS file function return code 
 */
FRESULT hw125_f_read(
    void *buff, 
    UINT btr); 


/**
 * @brief Reads a string from open file 
 * 
 * @details Wrapper function for the FATFS function f_gets. 
 * 
 * @param buff : 
 * @param len : 
 * @return TCHAR : 
 */
TCHAR* hw125_gets(
    TCHAR *buff, 
    uint16_t len); 


/**
 * @brief Test for end of file on open file 
 * 
 * @details Wrapper function for the FATFS macro function f_eof. 
 * 
 * @return HW125_EOF 
 */
HW125_EOF hw125_eof(void); 

//=======================================================================================

#endif   // _HW125_CONTROLLER_H_
