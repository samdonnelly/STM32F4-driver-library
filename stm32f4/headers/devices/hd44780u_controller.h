/**
 * @file hd44780u_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U + PCF8574 20x4 LCD screen controller interface 
 * 
 * @version 0.1
 * @date 2022-12-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HD44780U_CONTROLLER_H_ 
#define _HD44780U_CONTROLLER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "hd44780u_driver.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define HD44780U_NUM_STATES 10           // Number of controller states 
#define HD44780U_LINE_UPDATE_MASK 0x01   // Mask to identify which line contents to send 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HD44780U controller states 
 */
typedef enum {
    HD44780U_INIT_STATE,                 // State 0: Initialization state 
    HD44780U_IDLE_STATE,                 // State 1: Idle state 
    HD44780U_PWR_SAVE_STATE,             // State 2: Power save state 
    HD44780U_WRITE_STATE,                // State 3: Write state 
    HD44780U_CLEAR_STATE,                // State 4: Clear screen state 
    HD44780U_LOW_PWR_ENTER_STATE,        // State 5: Low power mode enter state 
    HD44780U_LOW_PWR_STATE,              // State 6: Low power state 
    HD44780U_LOW_PWR_EXIT_STATE,         // State 7: Low power mode exit state 
    HD44780U_FAULT_STATE,                // State 8: Fault state 
    HD44780U_RESET_STATE                 // State 9: Reset state 
} hd44780u_states_t; 

//=======================================================================================


//=======================================================================================
// Structures 

// HD44780U controller trackers 
typedef struct hd44780u_trackers_s 
{
    // Device and controller information 
    hd44780u_states_t state;                // State of the controller 
    uint8_t fault_code;                     // Fault code of the device/controller 

    // Screen sleep (backlight off) timer 
    TIM_TypeDef *timer;                     // Timer used for non-blocking delays 
    uint32_t sleep_time;                    // Time (us) until screen sleeps 
    tim_compare_t sleep_timer;              // Screen sleep timing info 

    // State flags 
    uint8_t startup   : 1;                  // Ensures the init state is run 
    uint8_t pwr_save  : 1;                  // Power save state flag 
    uint8_t write     : 1;                  // Write state flag 
    uint8_t clear     : 1;                  // Clear screen state flag 
    uint8_t low_power : 1;                  // Low power state flag 
    uint8_t reset     : 1;                  // Reset state flag 
}
hd44780u_trackers_t; 


// Screen line information 
typedef struct hd44780u_msgs_s 
{
    hd44780u_lines_t line; 
    char msg[HD44780U_LINE_LEN]; 
    uint8_t offset; 
}
hd44780u_msgs_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef uint8_t HD44780U_FAULT_CODE; 
typedef hd44780u_states_t HD44780U_STATE; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief HD44780U state machine function pointer 
 * 
 * @param hd44780u_device : device tracker that defines control characteristics 
 */
typedef void (*hd44780u_state_functions_t)(hd44780u_trackers_t *hd44780u_device); 

//=======================================================================================


//=======================================================================================
// Control functions 

/**
 * @brief HD44780U controller initialization 
 * 
 * @details Initializes the parameters of the device tracker. This function is called 
 *          during initialization in the application code. 
 * 
 * @see hd44780u_trackers_t
 * 
 * @param timer : timer used screen sleep in power save mode 
 */
void hd44780u_controller_init(TIM_TypeDef *timer); 


/**
 * @brief HD44780U controller 
 * 
 * @details This function runs the devices state machine. It reads the status of control 
 *          flags and determine which state to go to. This is repeatedly called by the  
 *          application code. 
 */
void hd44780u_controller(void); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief Set power save mode 
 * 
 * @details Triggers the power save state in the controller. When set, the controller will 
 *          default back to the power save state instead of the idle state when not being 
 *          used. In the power save state the screens backlight will turn off after a 
 *          specified period of time to save power. This is a non blocking state. The 
 *          time it takes to turn the backlight off can be set using hd44780u_set_sleep_time. 
 *          The backlight can be turned back on temporarily (sleep timer reset) using 
 *          hd44780u_wake_up. Note that writing to the screen will not turn the backlight 
 *          back on. 
 * 
 * @see hd44780u_set_sleep_time
 * @see hd44780u_wake_up 
 */
void hd44780u_set_pwr_save_flag(void); 


/**
 * @brief Clear power save mode 
 * 
 * @details Takes the controller out of power save mode at which point the controller will 
 *          default back to the idle state when not being used instead of the power save 
 *          state. 
 */
void hd44780u_clear_pwr_save_flag(void); 


/**
 * @brief Reset the power save state 
 * 
 * @details When in power save mode and the screen backlight goes off, a call to this 
 *          function will turn the backlight on and reset the timer used in the power save 
 *          state to turn the backlight off. 
 */
void hd44780u_wake_up(void); 


/**
 * @brief Set screen sleep time 
 * 
 * @details Sets the time it takes for the screen backlight to shut off while in the power 
 *          save state. The time is in microseconds. When the screen is woken up while in the 
 *          power save state, the timer will be reset and the time set using this function 
 *          is how long will be before the screen sleeps. 
 * 
 * @param sleep_time : time (us) until screen backlight turns off in power save mode 
 */
void hd44780u_set_sleep_time(uint32_t sleep_time); 


/**
 * @brief Message set 
 * 
 * @details Takes a whole screen message as an argument, updates the drivers line data with 
 *          the message, then triggers a write to the screen so the new message is displayed. 
 *          Screen messages can be defined by creating an array of type hd44780u_msgs_t and 
 *          passing that array to this function. 
 * 
 * @see hd44780u_msgs_t 
 * 
 * @param msg : pointer to screen message data 
 * @param msg_len : number of lines used in the message 
 */
void hd44780u_set_msg(
    hd44780u_msgs_t *msg, 
    uint8_t msg_len); 


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
 * @brief Set the clear screen flag 
 * 
 * @details Triggers the clear screen state which clears the screen and data record of 
 *          message content. This flag is cleared automatically after the clear state is 
 *          run. 
 */
void hd44780u_set_clear_flag(void); 


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

//=======================================================================================


//=======================================================================================
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

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _HD44780U_CONTROLLER_H_ 
