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

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "m8q_driver.h"
#include "timers_driver.h"

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
} m8q_states_t; 

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
 * @details Initializes the controller data record. This must be called once before using 
 *          the main controller function. The timer passed here is used to create a 
 *          non-blocking delay when exiting low power mode. This means the timer must be 
 *          compatible with the non-blocking delay function. 
 *          
 *          Note that in order to use this controller, the driver must be configured to 
 *          use the driver data record as well as the low power and TX ready pins. 
 * 
 * @see tim_compare 
 * 
 * @param timer : timer used for non-blocking delays 
 */
void m8q_controller_init(TIM_TypeDef *timer); 


/**
 * @brief M8Q controller 
 * 
 * @details Main control function. This function contains the controllers state machine 
 *          that dictates the flow of the controller. It also checks for faults that 
 *          occurred in the driver and controller. 
 *          
 *          The state of the controller can be changed using the flag setting functions. 
 *          This function should be called continuously to make sure the controller can 
 *          update it's state and keep operating the driver. The controller 
 *          initialization function must be called before starting to use this. 
 *          
 *          Note that in order to use this controller, the driver must be configured to 
 *          use the driver data record as well as the low power and TX ready pins. 
 */
void m8q_controller(void); 

//=======================================================================================


//=======================================================================================
// Setters and getters 

/**
 * @brief Set M8Q controller the read flag 
 * 
 * @details Puts the controller into the read state when in the idle state, or when 
 *          coming from the init and low power exit states. After running the controller 
 *          initialization function the controller will default to the read state from 
 *          the init state without needing to set this flag. In the read state the 
 *          controller checks for the presence of data in the data stream using the TX 
 *          ready pin. If there is data then it will be read and stored by the driver 
 *          as needed. 
 */
void m8q_set_read_flag(void); 


/**
 * @brief Set M8Q controller the idle flag 
 * 
 * @details Puts the controller into the idle state when in the read state, or when 
 *          coming from the init and low power exit states. In the idle state the 
 *          controller performs no actions. 
 */
void m8q_set_idle_flag(void); 


/**
 * @brief Set M8Q controller low power flag 
 * 
 * @details Puts the controller into the low power state when in either the read or 
 *          idle states. Note that when the controller is in the low power state the 
 *          state will read as the idle state, the only difference being the low power 
 *          flag will be set which can be read using the getter. The reason for this 
 *          is that low power mode and idle mode both perform no action. 
 *          
 *          Setting the low power flag will cause the driver to set the device 
 *          interrupt pin low. When the device is in low power mode it doesn't send 
 *          data and it consumes small amounts of power. 
 * 
 * @see m8q_get_lp_flag
 */
void m8q_set_low_pwr_flag(void); 


/**
 * @brief Clear M8Q controller the low power flag 
 * 
 * @details Removes the controller from the low power state. The controller will go to 
 *          either the read or idle state but first it passes through a low power exit 
 *          state which gives the device time to wake up and restart its data stream. 
 * 
 *          Clearing the low power flag will cause the driver to set the device 
 *          interrupt pin high which wakes the device up. 
 */
void m8q_clear_low_pwr_flag(void); 


/**
 * @brief Set M8Q controller reset flag 
 * 
 * @details Triggers the reset state if the controller is in the fault state. Resetting 
 *          the system clears the fault code and starts the controller over again. 
 */
void m8q_set_reset_flag(void); 


/**
 * @brief Get M8Q controller controller state 
 * 
 * @details Returns the current state of the controller. 
 * 
 * @see m8q_states_t
 * 
 * @return M8Q_STATE : controller state 
 */
M8Q_STATE m8q_get_state(void); 


/**
 * @brief Get M8Q controller low power flag 
 * 
 * @details Returns the state of the low power flag. The low power and idle states will 
 *          both read as the idle state so when this flag is set it means the controller 
 *          is in the low power state. Both the low power and idle states perform no 
 *          action so they share a state. 
 * 
 * @return uint8_t : low power flag state 
 */
uint8_t m8q_get_lp_flag(void); 


/**
 * @brief Get M8Q controller fault code 
 * 
 * @details Returns of the fault code of the device/controller. This code will only be 
 *          non-zero when in the fault state. The fault code is defined as follows: 
 *          - Bit 0: Not used 
 *          - Bit 1: Invalid pointer 
 *          - Bit 2: Not used 
 *          - Bit 3: Driver write fault 
 *          - Bit 4: Driver read fault 
 *          - Bits 5-15: Not currently used 
 * 
 * @see m8q_status_t 
 * 
 * @return M8Q_FAULT_CODE : device/controller fault code 
 */
M8Q_FAULT_CODE m8q_get_fault_code(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _M8Q_CONTROLLER_H_ 
