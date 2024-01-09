/**
 * @file m8q_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS controller interface 
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

// Drivers 
#include "m8q_driver.h"
#include "timers.h"

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q controller states 
 */
typedef enum {
    M8Q_INIT_STATE,           // Initialization state 
    M8Q_READ_STATE,           // Read state 
    M8Q_IDLE_STATE,           // Idle state 
    M8Q_LOW_PWR_ENTER_STATE,  // Low power enter state 
    M8Q_LOW_PWR_EXIT_STATE,   // Low power exit state 
    M8Q_FAULT_STATE,          // Fault state 
    M8Q_RESET_STATE           // Reset state 
    // M8Q_READ_CONT_STATE,      // Read continuous state 
    // M8Q_READ_READY_STATE,     // Read ready state 
    // M8Q_LOW_PWR_STATE,        // Low power state 
} m8q_states_t; 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief M8Q controller trackers 
 */
typedef struct m8q_trackers_s 
{
    // Peripherals 
    TIM_TypeDef *timer;                        // Pointer to timer port used in controller 
    
    // Device and controller information 
    m8q_states_t state;                        // Controller state 
    uint16_t fault_code;                       // Fault code for the device/controller 
    uint32_t clk_freq;                         // Timer clock frquency 
    uint32_t time_cnt_total;                   // Time delay counter total count 
    uint32_t time_cnt;                         // Time delay counter instance 
    uint8_t  time_start;                       // Time delay counter start flag 

    // State flags 
    uint8_t init          : 1;                 // Ensures the init state is run 
    uint8_t read          : 1;                 // Read flag --> for read ready state 
    uint8_t idle          : 1;                 // Idle state trigger 
    uint8_t low_pwr       : 1;                 // Low power state trigger 
    uint8_t low_pwr_enter : 1;                 // Low power exit state trigger 
    uint8_t low_pwr_exit  : 1;                 // Low power exit state trigger 
    uint8_t reset         : 1;                 // Reset state trigger 
    // uint8_t read_ready    : 1;                 // Triggers read ready state 
}
m8q_trackers_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint16_t M8Q_FAULT_CODE; 
typedef m8q_states_t M8Q_STATE; 

//=======================================================================================


//=======================================================================================
// Control functions 

/**
 * @brief M8Q controller initialization 
 * 
 * @details Initializes device trackers characteristics. Note that the timer passed to 
 *          this function should be a counter timer that can handle any needed 
 *          non-blocking delays of the controller. 
 * 
 * @param timer : timer used by the controller for non-blocking state delays 
 */
void m8q_controller_init(
    TIM_TypeDef *timer); 


/**
 * @brief M8Q controller 
 * 
 * @details Main control function. This function contains the controllers state machine 
 *          that dictates the flow of the controller. 
 */
void m8q_controller(void); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief Set the read flag 
 * 
 * @details Clears the idle flag as well 
 */
void m8q_set_read_flag(void); 


/**
 * @brief Set the idle flag 
 * 
 * @details Clears the read flag as well 
 */
void m8q_set_idle_flag(void); 


/**
 * @brief M8Q set low power flag 
 * 
 * @details This flag will set the interrupt pin on the receiver low to enable low power 
 *          mode. In this mode the receiver does not send data and consumes very little 
 *          power. This flag will cause the state machine to enter the low power state. 
 *          Note that in order for this flag to work correctly, the proper pin needs to 
 *          be configires through the device initialization. 
 */
void m8q_set_low_pwr_flag(void); 


/**
 * @brief M8Q clear low power flag 
 * 
 * @details This flag is used to set the interrupt pin on the receiver high to bring the 
 *          receiver back to normal mode. When this flag is set the state machine will 
 *          enter the low power exit state then proceed to the no-fix state once done. 
 *          Note that in order for this flag to work correctly, the proper pin needs to 
 *          be configires through the device initialization. 
 */
void m8q_clear_low_pwr_flag(void); 


/**
 * @brief M8Q set reset flag 
 * 
 * @details Triggers the reset state for the controller which will re-initialize the 
 *          device and controller as needed. This flag will cause a reset regardless of 
 *          the controller state. This flag can be used whenever a reset is needed such 
 *          as in the event of a fault that cannot be cleared on its own. 
 */
void m8q_set_reset_flag(void); 


// /**
//  * @brief Set the read ready state flag 
//  * 
//  * @details 
//  */
// void m8q_set_read_ready(void); 


// /**
//  * @brief Clear the read ready state flag 
//  * 
//  * @details 
//  */
// void m8q_clear_read_ready(void); 


// /**
//  * @brief Clear the read flag 
//  * 
//  * @details 
//  */
// void m8q_clear_read_flag(void); 

//=======================================================================================


//=======================================================================================
// Getters 

/**
 * @brief M8Q get controller state 
 * 
 * @details Returns the state of the M8Q controller in the form of an integer defined by 
 *          the m8q_states_t enum. 
 * 
 * @see m8q_states_t
 * 
 * @return M8Q_STATE : controller state 
 */
M8Q_STATE m8q_get_state(void); 


/**
 * @brief M8Q get fault code 
 * 
 * @details Returns of the fault code of the device/controller. Note that there are 
 *          currently no mechanisms in place to set fault codes. This is here as a 
 *          placeholder for future development. 
 * 
 * @return M8Q_FAULT_CODE : device/controller fault code 
 */
M8Q_FAULT_CODE m8q_get_fault_code(void); 

//=======================================================================================

#endif  // _M8Q_CONTROLLER_H_ 
