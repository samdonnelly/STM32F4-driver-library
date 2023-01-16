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
//=======================================================================================


//=======================================================================================
// Macros 

#define HW125_NUM_STATES 7           // Number of possible states for the controller 

#define HW125_BUFF_SIZE 100          // Read and write buffer size 
#define HW125_INFO_SIZE 30           // Device infor buffer size 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HW125 controller states 
 */
typedef enum {
    HW125_INIT_STATE, 
    HW125_STANDBY_STATE, 
    HW125_OPEN_STATE, 
    HW125_CLOSE_STATE, 
    HW125_ACCESS_STATE, 
    HW125_FAULT_STATE, 
    HW125_RESET_STATE 
} hw125_states_t; 

//=======================================================================================


//=======================================================================================
// Structures 

// HW125 controller trackers 
typedef struct hw125_trackers_s 
{
    // Controller information 
    hw125_states_t state;                        // State of the controller 
    uint8_t fault_code;                          // Fault code of the device/controller 

    // File system information 
    TCHAR path;                                  // Path to desired directory 
    FATFS *file_sys;                             // 
    FIL file;                                    // 
    FRESULT result;                              // 
    UINT br;                                     // Read counter 
    UINT bw;                                     // Write counter 
    char data_buff[HW125_BUFF_SIZE];             // Buffer to store read and write data 
    FATFS *pfs;                                  // 
    DWORD fre_clust;                             // 
    uint32_t total;                              // Volume total space 
    uint32_t free_space;                         // Volume free space 
    TCHAR vol_label[HW125_INFO_SIZE];            // Volume label 
    DWORD serial_num;                            // Volume serial number 

    // State trackers 
    uint8_t open_file : 1;                       // Open file flag 
    uint8_t reset     : 1;                       // Reset state trigger 
    uint8_t startup   : 1;                       // Ensures the init state is run 
}
hw125_trackers_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 
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
 */
void hw125_controller_init(void); 


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
//=======================================================================================


//=======================================================================================
// Getters 
//=======================================================================================

#endif   // _HW125_CONTROLLER_H_
