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

#define HD44780U_NUM_STATES 8    // Current number of possible states for the controller 

//================================================================================


//================================================================================
// Enums 

/**
 * @brief HD44780U controller states 
 */
typedef enum {
    HD44780U_INIT_STATE,                    // Initialization state 
    HD44780U_RUN_STATE,                     // Run state 
    HD44780U_WRITE_STATE,                   // Write state 
    HD44780U_LOW_PWR_ENTER_STATE,           // Low power mode enter state 
    HD44780U_LOW_PWR_STATE,                 // Low power state 
    HD44780U_LOW_PWR_EXIT_STATE,            // Low power mode exit state 
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

    // State flags 
    uint8_t write     : 1;                  // Write state trigger 
    uint8_t low_power : 1;                  // Low power state trigger 
    uint8_t reset     : 1;                  // Reset state trigger 
    uint8_t startup   : 1;                  // Ensures the init state is run 
}
hd44780u_trackers_t; 

//================================================================================


//===============================================================================
// Datatypes 

typedef uint8_t HD44780U_FAULT_CODE; 
typedef hd44780u_states_t HD44780U_STATE; 

//===============================================================================


//================================================================================
// Function pointers 

/**
 * @brief HD44780U state machine function pointer 
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
typedef void (*hd44780u_state_functions_t)(
    hd44780u_trackers_t *hd44780u_device); 

//================================================================================


//================================================================================
// Control functions 

/**
 * @brief HD44780U controller initialization 
 * 
 * @details Initializes the parameters of the device tracker. This function is called 
 *          during initialization in the application code. 
 * 
 * @see hd44780u_trackers_t
 */
void hd44780u_controller_init(void); 


/**
 * @brief HD44780U controller 
 * 
 * @details This function runs the devices state machine. It reads the status of control 
 *          flags and determine which state to go to. This is repeatedly called by the  
 *          application code. 
 */
void hd44780u_controller(void); 

//================================================================================


//================================================================================
// Setters 

/**
 * @brief HD44780U set write flag 
 * 
 * @details Sets the write flag which is used to trigger the write state. In the write state
 *          the contents of the devices data record is written to the screen. This flag will 
 *          clear after the write state is complete. The write state will not occur if the 
 *          controller is in fault, reset or low power mode states. 
 */
void hd44780u_set_write_flag(void); 


/**
 * @brief HD44780U set low power mode flag 
 * 
 * @details Sets the low power mode flag which is used to enter low power mode. In low power
 *          mode the screen is shut off and cannot be written to or read from. To exit this 
 *          state the hd44780u_clear_low_pwr_flag setter must be called. 
 * 
 * @see hd44780u_clear_low_pwr_flag
 */
void hd44780u_set_low_pwr_flag(void); 


/**
 * @brief HD44780U clear low power mode flag 
 * 
 * @details Clears the low power mode flag which is used to bring the controller out of the 
 *          low power state. When the controller exits the low power state the screen is 
 *          turned back on. 
 * 
 * @see hd44780u_set_low_pwr_flag
 */
void hd44780u_clear_low_pwr_flag(void); 


/**
 * @brief HD44780U set reset flag 
 * 
 * @details Sets the reset flag which is used to trigger a controller and device reset. 
 *          When this flag is set then the controller will start the initialization 
 *          process over again. This flag can be called from any state but is meant 
 *          for when the screen encounters a fault and needs to reset. 
 */
void hd44780u_set_reset_flag(void); 

//================================================================================


//================================================================================
// Getters 

/**
 * @brief HD44780U state getter 
 * 
 * @details Returns the current state of the state machine. The states are numbered 
 *          according to hd44780u_states_t. 
 * 
 * @see hd44780u_states_t
 * 
 * @return HD44780U_STATE : current state machine state of the HD44780U controller 
 */
HD44780U_STATE hd44780u_get_state(void); 


/**
 * @brief HD44780U fault code getter 
 * 
 * @details Returns the current fault code of the controller. This information can be 
 *          used for logging. There is only one reset all possible fault codes. 
 * 
 * @return HD44780U_FAULT_CODE : fault code of the device 
 */
HD44780U_FAULT_CODE hd44780u_get_fault_code(void); 

//================================================================================

#endif   // _HD44780U_CONTROLLER_H_ 
