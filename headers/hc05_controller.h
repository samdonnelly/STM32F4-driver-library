/**
 * @file hc05_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2023-01-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _HC05_CONTROLLER_H_ 
#define _HC05_CONTROLLER_H_ 

//================================================================================
// Includes 

// Device drivers 
#include "hc05_driver.h" 

// Libraries 

//================================================================================


//================================================================================
// Macros 

#define HC05_NUM_STATES 9     // 

//================================================================================


//================================================================================
// Enums 
/**
 * @brief HC05 controller states 
 */
typedef enum {
    HC05_INIT_STATE, 
    HC05_NOT_CONNECTED_STATE, 
    HC05_CONNECTED_STATE, 
    HC05_SEND_STATE, 
    HC05_READ_STATE, 
    HC05_LOW_POWER_STATE, 
    HC05_LOW_POWER_EXIT_STATE, 
    HC05_FAULT_STATE, 
    HC05_RESET_STATE 
} hc05_states_t; 

//================================================================================


//================================================================================
// Structures 

/**
 * @brief HC05 device trackers 
 */
typedef struct hc05_device_trackers_s 
{
    // Device and controller information 
    hc05_states_t state;                    // Controller state 
    uint8_t fault_code;                     // Controller fault code 

    // State flags 
    uint8_t connect : 1;                    // Device connection status 
    uint8_t low_pwr : 1;                    // Low power state flag 
    uint8_t reset   : 1;                    // Reset state flag 
}
hc05_device_trackers_t; 

//================================================================================


//================================================================================
// Data types 

typedef hc05_states_t HC05_STATE; 
typedef uint8_t HC05_FAULT_CODE; 

//================================================================================


//================================================================================
// Function pointers 

/**
 * @brief 
 * 
 */
typedef void (*hc05_state_functions_t)(
    hc05_device_trackers_t *hc05_device); 

//================================================================================


//================================================================================
// Control functions 

/**
 * @brief 
 * 
 * @details 
 * 
 */
void hc05_controller_init(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 */
void hc05_controller(void); 

//================================================================================


//================================================================================
// Setters 
//================================================================================


//================================================================================
// Getters 
//================================================================================

#endif   // _HC05_CONTROLLER_H_ 
