/**
 * @file hw125_controller.h
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

#ifndef _HW125_CONTROLLER_H_ 
#define _HW125_CONTROLLER_H_ 

//=======================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "hw125_driver.h" 

// STM drivers 
#include "fatfs.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define HW125_NUM_STATES 7             // Number of possible states for the controller 

#define HW125_BUFF_SIZE 100            // Read and write buffer size 
#define HW125_PATH_SIZE 50             // Volume path max length 
#define HW125_INFO_SIZE 30             // Device infor buffer size 

#define HW125_ERROR_MASK 0x00000001    // 
// Use a bit shift with the predefined FatFs FRESULT values then use this mask to 
// filter out the ones that don't matter 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HW125 controller states 
 */
typedef enum {
    HW125_INIT_STATE, 
    HW125_NOT_READY_STATE, 
    HW125_STANDBY_STATE, 
    HW125_ACCESS_STATE, 
    HW125_ESTOP_STATE, 
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
    BYTE fault_code;                             // Fault code of the device/controller 
    DWORD fault_code_check;                      // Fault code checker 

    // File system information 
    FATFS file_sys;                              // File system object 
    FIL file;                                    // File object 
    FRESULT fresult;                             // Store result of FatFs operation 
    UINT br, bw;                                 // Read and write counters 
    TCHAR path[HW125_PATH_SIZE];                 // Path to desired directory 

    // Card capacity 
    FATFS *pfs;                                  // Pointer to file system object 
    DWORD fre_clust, total, free_space;          // Free clusters, total and free space 
    
    // Volume tracking 
    TCHAR vol_label[HW125_INFO_SIZE];            // Volume label 
    DWORD serial_num;                            // Volume serial number 

    // Data buffers 
    TCHAR data_buff[HW125_BUFF_SIZE];            // Buffer to store read and write data 

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
typedef uint8_t HW125_FAULT_CODE; 
typedef uint8_t HW125_FILE_STATUS; 
typedef int8_t HW125_EOF; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief HW125 state machine function pointer 
 * 
 * @param hw125_device : device tracker that defines control characteristics 
 * 
 */
typedef void (*hw125_state_functions_t)(
    hw125_trackers_t *hw125_device); 

//=======================================================================================


//=======================================================================================
// Control functions 

/**
 * @brief HW125 controller initialization 
 * 
 * @details 
 * 
 * @param path : path to directory to use on the volume 
 */
void hw125_controller_init(
    char *path); 


/**
 * @brief HW125 controller 
 * 
 * @details 
 * 
 */
void hw125_controller(void); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief Make a new directory in project directory 
 * 
 * @param dir 
 * @return FRESULT 
 */
FRESULT hw125_mkdir(
    const TCHAR *dir); 


/**
 * @brief Open file 
 * 
 * @param file_name 
 * @param mode 
 * @return FRESULT 
 */
FRESULT hw125_open(
    const TCHAR *file_name, 
    uint8_t mode); 


/**
 * @brief Close the open file 
 * 
 * @return FRESULT 
 */
FRESULT hw125_close(void); 


/**
 * @brief Write to the open file 
 * 
 * @param buff 
 * @param btw 
 * @return FRESULT 
 */
FRESULT hw125_f_write(
    const void *buff, 
    UINT btw); 


/**
 * @brief Write a character to the open file 
 * 
 * @param character 
 * @return int8_t 
 */
int8_t hw125_putc(
    TCHAR character); 


/**
 * @brief Write a string to the open file 
 * 
 * @param str 
 * @return int16_t 
 */
int8_t hw125_puts(
    const TCHAR *str); 


/**
 * @brief Write a formatted string to the open file 
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
 * @param offset 
 * @return FRESULT 
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
 * @brief Get open file flag 
 * 
 * @return HW125_FILE_STATUS 
 */
HW125_FILE_STATUS hw125_get_file_status(void); 


/**
 * @brief Read data from open file 
 * 
 * @param buff 
 * @param btr 
 * @return FRESULT 
 */
FRESULT hw125_f_read(
    void *buff, 
    UINT btr); 


/**
 * @brief Reads a string from open file 
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
 * @return HW125_EOF 
 */
HW125_EOF hw125_eof(void); 

//=======================================================================================

#endif   // _HW125_CONTROLLER_H_
