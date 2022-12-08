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

#define HD44780U_NUM_STATES 8    // Number of states for the device 

//================================================================================


//================================================================================
// Enums 

/**
 * @brief HD44780U controller states 
 */
typedef enum {
    HD44780U_INIT_STATE,                    // Initialization state 
    HD44780U_IDLE_STATE,                    // Idle state 
    HD44780U_WRITE_STATE,                   // Write state 
    HD44780U_READ_STATE,                    // Read state 
    HD44780U_LOW_PWR_TRANS_STATE,           // Low power mode transition state 
    HD44780U_LOW_PWR_STATE,                 // Low power state 
    HD44780U_FAULT_STATE,                   // Fault state 
    HD44780U_RESET_STATE                    // Reset state 
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


//===============================================================================
// Datatypes 

typedef uint8_t FAULT_CODE; 

//===============================================================================


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
// Control functions 

/**
 * @brief 
 * 
 */
void hd44780u_controller_init(void); 


/**
 * @brief 
 * 
 */
void hd44780u_controller(void); 

//================================================================================


//================================================================================
// Setters 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param display_data 
 * @param line_offset 
 */
void hd44780u_line1_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief 
 * 
 * @details 
 */
void hd44780u_line1_clear(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param display_data 
 * @param line_offset 
 */
void hd44780u_line2_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief 
 * 
 * @details 
 */
void hd44780u_line2_clear(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param display_data 
 * @param line_offset 
 */
void hd44780u_line3_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief 
 * 
 * @details 
 */
void hd44780u_line3_clear(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param display_data 
 * @param line_offset 
 */
void hd44780u_line4_set(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief 
 * 
 * @details 
 */
void hd44780u_line4_clear(void); 


/**
 * @brief Set write flag 
 * 
 * @details 
 */
void hd44780_set_write_flag(void); 


// Read flag 
/**
 * @brief Set read flag 
 * 
 * @details 
 */
void hd44780u_set_read_flag(void); 


/**
 * @brief Set low power mode flag 
 * 
 * @details 
 */
void hd44780u_set_low_pwr_flag(void); 


/**
 * @brief Clear low power mode flag 
 * 
 * @details 
 */
void hd44780u_clear_low_pwr_flag(void); 


/**
 * @brief Set reset flag 
 * 
 * @details 
 */
void hd44780u_set_reset_flag(void); 

//================================================================================


//================================================================================
// Getters 

/**
 * @brief 
 * 
 * @details 
 * 
 * @return hd44780u_states_t 
 */
hd44780u_states_t hd44780u_get_state(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
FAULT_CODE hd44780u_get_fault_code(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param read_msg 
 * @return code 
 */
void hd44780u_get_read_msg(char *read_msg); 

//================================================================================

#endif   // _HD44780U_CONTROLLER_H_ 
