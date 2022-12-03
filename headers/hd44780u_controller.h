/**
 * @file hd44780u_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U + PCF8574 20x4 LCD screen controller 
 * 
 * @version 0.1
 * @date 2022-12-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HD44780U_CONTROLLER_H_ 
#define _HD44780U_CONTROLLER_H_ 

//================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "hd44780u_driver.h"

//================================================================================


//================================================================================
// Macros 

#define HD44780U_NUM_STATES 7    // Number of states for the device 

//================================================================================


//================================================================================
// Enums 

/**
 * @brief HD44780U controller states 
 */
typedef enum {
    HD44780U_INIT_STATE, 
    HD44780U_IDLE_STATE, 
    HD44780U_WRITE_STATE, 
    HD44780U_READ_STATE, 
    HD44780U_LOW_PWR_STATE, 
    HD44780U_FAULT_STATE, 
    HD44780U_RESET_STATE 
} hd44780u_states_t; 

//================================================================================


//================================================================================
// Structures 

/**
 * @brief HD44780U controller trackers 
 */
typedef struct hd44780u_trackers_s 
{
    // Device and controller information 
    hd44780u_states_t state;                // State of the controller 
    uint8_t fault_code;                     // Fault code of the device/controller 
    uint8_t read_msg[20];                   // Buffer to store read messages 

    // State flags 
    uint8_t write     : 1;                  // Write state trigger 
    uint8_t read      : 1;                  // Read state trigger 
    uint8_t low_power : 1;                  // Low power state trigger 
    uint8_t reset     : 1;                  // Reset state trigger 
    uint8_t startup   : 1;                  // Ensures the init state is run 
}
hd44780u_trackers_t; 

//================================================================================


//================================================================================
// Function pointers 

/**
 * @brief 
 * 
 * @details 
 */
typedef void (*hd44780u_state_functions_t)(
    hd44780u_trackers_t hd44780u_device); 

//================================================================================


//================================================================================
// Function prototypes 


void hd44780u_controller_init(void); 



void hd44780u_controller(void); 

//================================================================================

#endif   // _HD44780U_CONTROLLER_H_ 
