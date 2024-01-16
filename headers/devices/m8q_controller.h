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
void m8q_controller_init(
    TIM_TypeDef *timer); 


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
 *          initialization function must be called before calling this. 
 *          
 *          Note that in order to use this controller, the driver must be configured to 
 *          use the driver data record as well as the low power and TX ready pins. 
 */
void m8q_controller(void); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Set the read flag 
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
 * @brief Set the idle flag 
 * 
 * @details Puts the controller into the idle state when in the read state, or when 
 *          coming from the init and low power exit states. In the idle state the 
 *          controller performs no actions. 
 */
void m8q_set_idle_flag(void); 


/**
 * @brief Set low pwoer flag 
 * 
 * @details Puts the controller into the low power state when in either the read or 
 *          idle states. Note that when the controller is in the low power state, the 
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
 * @brief Clear the low power flag 
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
 * @brief Get low power flag state 
 * 
 * @return uint8_t 
 */
uint8_t m8q_get_lp_flag(void); 


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
