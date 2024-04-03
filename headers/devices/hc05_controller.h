/**
 * @file hc05_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC05 controller 
 * 
 * @version 0.1
 * @date 2023-01-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _HC05_CONTROLLER_H_ 
#define _HC05_CONTROLLER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//================================================================================
// Includes 

// Drivers 
#include "hc05_driver.h" 
#include "timers_driver.h" 

// Libraries 
#include <stdio.h> 

//================================================================================


//================================================================================
// Macros 

#define HC05_NUM_STATES 9        // Number of controller states 
#define HC05_BUFF_SIZE 30        // Read buffer size in bytes 
#define HC05_RESET_DELAY 100     // Reset state delay (ms) 

//================================================================================


//================================================================================
// Enums 
/**
 * @brief HC05 controller states 
 */
typedef enum {
    HC05_INIT_STATE,                   // Initialization state 
    HC05_NOT_CONNECTED_STATE,          // No Bluetooth connection state 
    HC05_CONNECTED_STATE,              // Bluetooth connected state 
    HC05_SEND_STATE,                   // Send data state 
    HC05_READ_STATE,                   // Read data state 
    HC05_LOW_POWER_STATE,              // Low power mode state 
    HC05_LOW_POWER_EXIT_STATE,         // Low power mode exit state 
    HC05_FAULT_STATE,                  // Fault state 
    HC05_RESET_STATE                   // Reset state 
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
 * @brief HC05 state function pointer 
 */
typedef void (*hc05_state_functions_t)(
    hc05_device_trackers_t *hc05_device); 

//================================================================================


//================================================================================
// Control functions 

/**
 * @brief HC05 controller initialization 
 * 
 * @details Configures device trackers used in the controller. This function must be 
 *          called during setup in order for the controller to work properly. The 
 *          timer passed as an argument is used for creating short delays in the 
 *          controller. The timer used should be equiped as a general purpose timer 
 *          that can be used in the timer driver delay functions. The delays are 
 *          used in states of the controller that are not time sensitive. 
 * 
 * @param timer : timer port used for the driver 
 */
void hc05_controller_init(
    TIM_TypeDef *timer); 


/**
 * @brief HC05 controller 
 * 
 * @details Main control scheme for the device. This function contains the state 
 *          machine used to dictate flow/control of the code/device. State 
 *          functions are called from here. 
 */
void hc05_controller(void); 

//================================================================================


//================================================================================
// Setters 

/**
 * @brief HC05 send data setter 
 * 
 * @details Used to specify the data to be sent by the device over Bluetooth. After 
 *          this setter is called the send state will be triggered but only if the 
 *          controller is in the connected state. If this setter is called while not 
 *          in the connected state then it will have to be called again to trigger 
 *          the send state. After sending the data the controller will return from 
 *          from the send state. This change of state can be used to know when you 
 *          can call this setter again with updated data to send. 
 *          
 *          NOTE: data_size must be less than HC05_BUFF_SIZE or else the data will not be 
 *                sent. This condition prevents overrun errors. 
 *          
 *          NOTE: The data passed to this setter must be NUL terminated. 
 * 
 * @param data : buffer holding the data to be sent 
 * @param data_size : size of data in the data buffer 
 */
void hc05_set_send(
    uint8_t *data, 
    uint8_t data_size); 


/**
 * @brief Set the read flag 
 * 
 * @details This setter is used to trigger the read state. The read state can only 
 *          be entered from the connected state. If this setter is called while not 
 *          in the read or connected state then it will have to be called again. 
 */
void hc05_set_read(void); 


/**
 * @brief Clear the read flag 
 * 
 * @details This setter is used to exit the read state. This setter is only useful 
 *          when the controller is already in the read state. 
 */
void hc05_clear_read(void); 


/**
 * @brief Set low power flag 
 * 
 * @details Used to trigger the low power state. This is used for power saving modes. 
 */
void hc05_set_low_power(void); 


/**
 * @brief Clear low power flag 
 * 
 * @details Used to trigger the exit of the low power state. 
 */
void hc05_clear_low_power(void); 


/**
 * @brief Set reset flag 
 * 
 * @details Triggers a controller reset. During a reset the device will be restarted 
 *          and device tracking information will be reset. This is useful in the event 
 *          of a system fault. 
 */
void hc05_set_reset(void); 

//================================================================================


//================================================================================
// Getters 

/**
 * @brief Get controller state 
 * 
 * @details Retrieves and returns the current state of the controller. 
 * 
 * @see hc05_states_t 
 * 
 * @return HC05_STATE : controller state 
 */
HC05_STATE hc05_get_state(void); 


/**
 * @brief Get fault code 
 * 
 * @details Returns the fault code of the controller. 
 * 
 * @return HC05_FAULT_CODE : controller fault code 
 */
HC05_FAULT_CODE hc05_get_fault_code(void); 


/**
 * @brief Get the read status 
 * 
 * @details Indicates if data is available in the controller for reading. This flag 
 *          will only be set if the controller is in the read state and data has been 
 *          read from an external source. 
 * 
 * @see hc05_get_read_data 
 * 
 * @return HC05_READ_STATUS : available read data status 
 */
HC05_READ_STATUS hc05_get_read_status(void); 


/**
 * @brief Get the read data 
 * 
 * @details Copies the most recent read data into the buffer passed to the function. 
 *          When new data is available, the read status flag will be set which can 
 *          be used to know when to call this getter. If this getter is called 
 *          without checking the read status flag, there is no guarentee the returned 
 *          data is different or updated from the previous read. 
 * 
 * @see hc05_get_read_status 
 * 
 * @param buffer : buffer to store read data 
 * @param buff_size : size of storage buffer passed to the getter 
 */
void hc05_get_read_data(
    uint8_t *buffer, 
    uint8_t buff_size); 

//================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _HC05_CONTROLLER_H_ 
