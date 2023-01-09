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

// Drivers 
#include "hc05_driver.h" 
#include "timers.h" 

// Libraries 
#include <stdio.h> 

//================================================================================


//================================================================================
// Macros 

#define HC05_NUM_STATES 9        // Number of controller states 
#define HC05_BUFF_SIZE 15        // Read buffer size in bytes 
#define HC05_RESET_DELAY 100     // Reset state delay (ms) 

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
    // Peripherals 
    TIM_TypeDef *timer;                       // Timer port used in the controller 

    // Device and controller information 
    hc05_states_t state;                      // Controller state 
    uint8_t fault_code;                       // Controller fault code 
    uint8_t send_data[HC05_BUFF_SIZE];        // Send data buffer 
    uint8_t read_data[HC05_BUFF_SIZE];        // Read data buffer 

    // State flags 
    uint8_t connect     : 1;                  // Device connection status 
    uint8_t send        : 1;                  // Send state flag 
    uint8_t read        : 1;                  // Read state flag 
    uint8_t read_status : 1;                  // Read data status 
    uint8_t low_pwr     : 1;                  // Low power state flag 
    uint8_t reset       : 1;                  // Reset state flag 
    uint8_t startup     : 1;                  // Ensures the init state is run 
}
hc05_device_trackers_t; 

//================================================================================


//================================================================================
// Data types 

typedef hc05_states_t HC05_STATE; 
typedef uint8_t HC05_FAULT_CODE; 
typedef uint8_t HC05_READ_STATUS; 

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
void hc05_controller_init(
    TIM_TypeDef *timer); 


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

/**
 * @brief 
 * 
 * @details When this setter is used, the controller will send the data passed to it. 
 *          
 *          data_size must be less than HC05_BUFF_SIZE or else the data will not be 
 *          sent. 
 * 
 * @param data 
 * @param data_size 
 */
void hc05_set_send(
    uint8_t *data, 
    uint8_t data_size); 


/**
 * @brief Set the read flag 
 * 
 * @details 
 * 
 */
void hc05_set_read(void); 


/**
 * @brief Clear the read flag 
 * 
 * @details 
 * 
 */
void hc05_clear_read(void); 


/**
 * @brief Set low power flag 
 * 
 * @details 
 * 
 */
void hc05_set_low_power(void); 


/**
 * @brief Clear low power flag 
 * 
 * @details 
 * 
 */
void hc05_clear_low_power(void); 


/**
 * @brief Set reset flag 
 * 
 * @details 
 * 
 */
void hc05_set_reset(void); 

//================================================================================


//================================================================================
// Getters 

/**
 * @brief Read state 
 * 
 * @details 
 * 
 * @return HC05_STATE 
 */
HC05_STATE hc05_get_state(void); 


/**
 * @brief Get fault code 
 * 
 * @details 
 * 
 * @return HC05_FAULT_CODE 
 */
HC05_FAULT_CODE hc05_get_fault_code(void); 


/**
 * @brief Get the read status 
 * 
 * @details 
 * 
 * @return HC05_READ_STATUS 
 */
HC05_READ_STATUS hc05_get_read_status(void); 


/**
 * @brief Get the read data 
 * 
 * @details 
 * 
 * @param buffer 
 * @param buff_size 
 */
void hc05_get_read_data(
    uint8_t *buffer, 
    uint8_t buff_size); 

//================================================================================

#endif   // _HC05_CONTROLLER_H_ 
