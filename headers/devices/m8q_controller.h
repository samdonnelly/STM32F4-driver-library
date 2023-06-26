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

// Drivers 
#include "m8q_driver.h"
#include "timers.h"

// Libraries 

//=======================================================================================


//=======================================================================================
// Macros 

#define M8Q_NUM_STATES 6                // Number of controller states 
#define M8Q_LOW_PWR_EXIT_DELAY 150000   // (us) time to wait when exiting low power mode 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q controller states 
 */
typedef enum {
    M8Q_INIT_STATE,           // Initialization state 
    M8Q_RUN_STATE,            // Run state 
    // M8Q_NO_FIX_STATE,         // No fix state 
    // M8Q_FIX_STATE,            // Fix state 
    M8Q_LOW_PWR_STATE,        // Low power state 
    M8Q_LOW_PWR_EXIT_STATE,   // Low power mode exit state 
    M8Q_FAULT_STATE,          // Fault state 
    M8Q_RESET_STATE           // Reset state 
} m8q_states_t; 


/**
 * @brief M8Q navigation status states 
 * 
 * @details The device provides the state of its navigation status which can be used to know 
 *          when the device has a fix, and if so what kind of fix. The status is provided in 
 *          the form of a two character string. This enum represents each state by using 
 *          the lowest two digits of the strings decimal number equivalent. For more 
 *          information on the states see the description of m8q_get_navstat. 
 * 
 * @see m8q_get_navstat
 */
typedef enum {
    M8Q_NAVSTAT_G2 = 26, 
    M8Q_NAVSTAT_G3 = 27, 
    M8Q_NAVSTAT_NF = 38, 
    M8Q_NAVSTAT_D2 = 58, 
    M8Q_NAVSTAT_D3 = 59, 
    M8Q_NAVSTAT_RK = 67, 
    M8Q_NAVSTAT_TT = 88, 
    M8Q_NAVSTAT_DR = 90 
} m8q_navstat_state_t; 

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
    uint8_t fault_code;                        // Fault code for the device/controller 
    m8q_navstat_state_t navstat;               // Navigation status of device 
    uint32_t clk_freq;                         // Timer clock frquency 
    uint32_t time_cnt_total;                   // Time delay counter total count 
    uint32_t time_cnt;                         // Time delay counter instance 
    uint8_t  time_start;                       // Time delay counter start flag 

    // State flags 
    uint8_t fix          : 1;                  // Position fix status - fix state trigger 
    uint8_t low_pwr      : 1;                  // Low power state trigger 
    uint8_t low_pwr_exit : 1;                  // Low power exit state trigger 
    uint8_t reset        : 1;                  // Reset state trigger 
    uint8_t startup      : 1;                  // Ensures the init state is run 
}
m8q_trackers_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t M8Q_FAULT_CODE; 
typedef m8q_states_t M8Q_STATE; 
typedef m8q_navstat_state_t M8Q_NAV_STATE; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief M8Q state machine function pointer 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
typedef void (*m8q_state_functions_t)(
    m8q_trackers_t *m8q_device); 

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


/**
 * @brief Get the navigation status 
 * 
 * @details Returns the navigation status of the device based on m8q_navstat_state_t. The 
 *          status is updated and configured through the m8q_check_msgs function. This 
 *          return value is left for interpretation by the application code - different 
 *          applications will have different standards for an acceptable fix. This 
 *          controller will be in the no-fix state if navstats reads as no fix and will 
 *          otherwise reside in the fix state. 
 * 
 * @see m8q_navstat_state_t
 * 
 * @return M8Q_NAV_STATE : device navigation (fix) statis 
 */
M8Q_NAV_STATE m8q_get_nav_state(void); 


// TODO add GPS data getters as needed 

//=======================================================================================

#endif  // _M8Q_CONTROLLER_H_ 
