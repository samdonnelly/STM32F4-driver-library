/**
 * @file m8q_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS controller 
 * 
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _M8Q_CONTROLLER_H_ 
#define _M8Q_CONTROLLER_H_ 

//=======================================================================================
// Includes 

// Device drivers 
#include "m8q_driver.h"

// Libraries 

//=======================================================================================


//=======================================================================================
// Macros 

#define M8q_NUM_STATES 6   // Number of controller states 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q controller states 
 */
typedef enum {
    M8Q_INIT_STATE,        // Initialization state 
    M8Q_NO_FIX_STATE,      // No fix state 
    M8Q_FIX_STATE,         // Fix state 
    M8Q_LOW_PWR_STATE,     // Low power state 
    M8Q_FAULT_STATE,       // Fault state 
    M8Q_RESET_STATE        // Reset state 
} m8q_states_t; 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief M8Q controller trackers 
 */
typedef struct m8q_trackers_s 
{
    // Device and controller information 
    m8q_states_t state;                        // Controller state 
    uint8_t fault_code;                        // Fault code for the device/controller 

    // State flags 
    uint8_t low_pwr : 1;                       // Low power state trigger 
    uint8_t reset   : 1;                       // Reset state trigger 
}
m8q_trackers_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t M8Q_FAULT_CODE; 
typedef m8q_states_t M8Q_STATE; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief M8Q state machine function pointer 
 * 
 * @param m8q_device : 
 */
typedef void (*m8q_state_functions_t)(
    m8q_trackers_t m8q_device); 

//=======================================================================================


//=======================================================================================
// Control functions 

/**
 * @brief M8Q controller initialization 
 * 
 * @details 
 * 
 */
void m8q_controller_init(void); 


/**
 * @brief M8Q controller 
 * 
 * @details 
 * 
 */
void m8q_controller(void); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief 
 * 
 * @details 
 */
void m8q_set_low_pwr_flag(void); 


/**
 * @brief 
 * 
 * @details 
 */
void m8q_clear_low_pwr_flag(void); 


/**
 * @brief 
 * 
 * @details 
 */
void m8q_set_reset_flag(void); 

//=======================================================================================


//=======================================================================================
// Getters 

/**
 * @brief 
 * 
 * @details 
 * 
 * @return M8Q_STATE 
 */
M8Q_STATE m8q_get_state(void); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @return M8Q_FAULT_CODE 
 */
M8Q_FAULT_CODE m8q_get_fault_code(void); 


// TODO add GPS data getters as needed 

//=======================================================================================

#endif  // _M8Q_CONTROLLER_H_ 
