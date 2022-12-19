/**
 * @file m8q_controller.c
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

//=======================================================================================
// Includes 

#include "m8q_controller.h"

//=======================================================================================

//===================================================
// Notes 

// TODO 
// - How do we translate faults in drivers to the controller? 

//===================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief M8Q initialization state 
 * 
 * @details Initializes the controller parameters as needed. This state is run once upon startup 
 *          and is only entered again after the reset state. 
 *          The init state shouldn't need to delay to allow the device to set up before reading 
 *          because the TX ready pin will be checked before attempting a read. But test this. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_init_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q no fix state 
 * 
 * @details State for when the device is in use but does not have a valid position fix. During 
 *          this state, the data read from getters will not be valid. The fix state is checked 
 *          repeatedly to see if a position has been found, and if so it will move to the 
 *          fix state. This state is enetered directly after the init state and can also be 
 *          entered from the fix and low power exit states. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_no_fix_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q fix state 
 * 
 * @details State for when the device is in use and has a valid position fix. During this state, 
 *          valid data will be available through getters. The fix state is checked repeatedly 
 *          to see if a position fix has been lost, and if so it will revert to the no fix state. 
 *          This state can only be entered through the no fix state. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_fix_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q low power state 
 * 
 * @details Sets the EXTINT pin low to trigger low power mode in the receiver. This state idles 
 *          until the low power flag is cleared, a fault flag is set or the reset flag is set 
 *          at which point the low power mode exit state is entered. When the device is in low 
 *          power mode, it doesn't retreive or send updated position data in order to save power. 
 *          In other words, the device is not usable until it exits low power mode. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_low_pwr_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q low power exit state 
 * 
 * @details This state is used to make sure the receiver properly returns to a normal state from 
 *          a low power state. The receiver requires some specific steps (and time) in order to 
 *          return to proper functionaility. Once these steps are complete the controller will 
 *          go directly into it's next state without further trigger needed. This state is only 
 *          entered from the low power mode state and it can enter either the no fix, fault or 
 *          reset state. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_low_pwr_exit_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q fault state 
 * 
 * @details The controller enters this state when the fault code is set and idles here until the 
 *          rest flag is set or the fault code gets cleared. There are currently no mechanisms 
 *          in place to set a fault code so this state is a placeholder for when that 
 *          functionaility becomes available. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_fault_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q reset state 
 * 
 * @details Resets the controller and the device as if the system was to restart. In this state 
 *          the fault code is cleared and the device init function is called again. This state is 
 *          triggered by setting the reset flag and will immediately go to the init state once 
 *          done. A reset can be needed in the event of a fault of any kind. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_reset_state(m8q_trackers_t *m8q_device); 


/**
 * @brief Get the navigation status 
 * 
 * @details This function is used to read new message data from the receiver and check the 
 *          fix status. Messages will only be read if the receiver indicates that messages 
 *          are ready. The no fix and fix states use this function repeatedly. 
 *          
 *          The navstat tracker gets updated and configured here and not in the getter. It 
 *          is used to know when to enter and exit the fix/no-fix states. 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
void m8q_check_msgs(m8q_trackers_t *m8q_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Instance of the device tracker record 
static m8q_trackers_t m8q_device_trackers; 


// Function pointers to controller states 
static m8q_state_functions_t state_table[M8Q_NUM_STATES] =
{
    &m8q_init_state, 
    &m8q_no_fix_state, 
    &m8q_fix_state, 
    &m8q_low_pwr_state, 
    &m8q_low_pwr_exit_state, 
    &m8q_fault_state, 
    &m8q_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control functions 

// Initialization 
void m8q_controller_init(
    TIM_TypeDef *timer)
{
    m8q_device_trackers.timer = timer; 

    m8q_device_trackers.state = M8Q_INIT_STATE; 

    m8q_device_trackers.fault_code = CLEAR; 

    m8q_device_trackers.navstat = M8Q_NAVSTAT_NF; 

    m8q_device_trackers.time_cnt_total = CLEAR; 

    m8q_device_trackers.time_cnt = CLEAR; 

    m8q_device_trackers.time_start = SET_BIT; 

    m8q_device_trackers.fix = CLEAR_BIT; 

    m8q_device_trackers.low_pwr = CLEAR_BIT; 

    m8q_device_trackers.low_pwr_exit = CLEAR_BIT; 

    m8q_device_trackers.reset = CLEAR_BIT; 

    m8q_device_trackers.startup = SET_BIT; 
}


// Controller 
void m8q_controller(void)
{
    // Local variables 
    m8q_states_t next_state = m8q_device_trackers.state; 

    //==================================================
    // State machine 

    switch (next_state)
    {
        case M8Q_INIT_STATE: 
            // Startup flag cleared 
            if (!(m8q_device_trackers.startup))
            {
                next_state = M8Q_NO_FIX_STATE; 
            }

            break; 

        case M8Q_NO_FIX_STATE: 
            // Fault code set 
            if (m8q_device_trackers.fault_code)
            {
                next_state = M8Q_FAULT_STATE; 
            }
            
            // Reset flag set 
            else if (m8q_device_trackers.reset)
            {
                next_state = M8Q_RESET_STATE; 
            }
            
            // Low power flag set 
            else if (m8q_device_trackers.low_pwr)
            {
                next_state = M8Q_LOW_PWR_STATE; 
            }
            
            // Position fix detected 
            else if (m8q_device_trackers.fix)
            {
                next_state = M8Q_FIX_STATE; 
            }
            
            break; 

        case M8Q_FIX_STATE: 
            // Fault code set 
            if (m8q_device_trackers.fault_code)
            {
                next_state = M8Q_FAULT_STATE; 
            }

            // Reset flag set 
            else if (m8q_device_trackers.reset)
            {
                next_state = M8Q_RESET_STATE; 
            }

            // Low power flag set 
            else if (m8q_device_trackers.low_pwr)
            {
                next_state = M8Q_LOW_PWR_STATE; 
            }

            // Position fix not detected 
            else if (!(m8q_device_trackers.fix))
            {
                next_state = M8Q_NO_FIX_STATE; 
            }

            break; 

        case M8Q_LOW_PWR_STATE: 
            // Fault code set, reset flag set, or low power flag cleared 
            if (m8q_device_trackers.fault_code || 
                m8q_device_trackers.reset || 
              !(m8q_device_trackers.low_pwr)) 
            {
                next_state = M8Q_LOW_PWR_EXIT_STATE; 
            }
            
            break; 

        case M8Q_LOW_PWR_EXIT_STATE: 
            // If the low power state is complete then exit 
            if (m8q_device_trackers.low_pwr_exit)
            {
                // Fault code set 
                if (m8q_device_trackers.fault_code)
                {
                    next_state = M8Q_FAULT_STATE; 
                }

                // Reset flag set 
                else if (m8q_device_trackers.reset)
                {
                    next_state = M8Q_RESET_STATE; 
                }

                // Default back to the no fix state 
                else 
                {
                    next_state = M8Q_NO_FIX_STATE; 
                }

                m8q_device_trackers.low_pwr_exit = CLEAR_BIT; 
            }
            
            break; 

        case M8Q_FAULT_STATE: 
            // Reset flag set 
            if (m8q_device_trackers.reset)
            {
                next_state = M8Q_RESET_STATE; 
            }

            // Fault code cleared 
            else if (!(m8q_device_trackers.fault_code))
            {
                next_state = M8Q_INIT_STATE; 
            }
            
            break; 

        case M8Q_RESET_STATE: 
            next_state = M8Q_INIT_STATE; 
            break; 

        default: 
            next_state = M8Q_INIT_STATE; 
            break; 
    }

    //==================================================

    // Go to state function 
    (state_table[next_state])(&m8q_device_trackers); 

    // Record the state 
    m8q_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// Initialization state 
void m8q_init_state(m8q_trackers_t *m8q_device)
{
    // Clear the fix flag 
    m8q_device->fix = CLEAR_BIT; 

    // Clear the reset flag 
    m8q_device->reset = CLEAR_BIT; 

    // Clear the startup bit 
    m8q_device->startup = CLEAR_BIT; 

    // Indicate state presence 
    uart_sendstring(USART2, "init state\r\n"); 
}


// No fix state 
void m8q_no_fix_state(m8q_trackers_t *m8q_device)
{
    m8q_check_msgs(m8q_device); 

    if (m8q_device->navstat != M8Q_NAVSTAT_NF)
    {
        m8q_device->fix = SET_BIT; 
    }
}


// Fix state 
void m8q_fix_state(m8q_trackers_t *m8q_device)
{
    m8q_check_msgs(m8q_device); 

    if (m8q_device->navstat == M8Q_NAVSTAT_NF)
    {
        m8q_device->fix = CLEAR_BIT; 
    }
}


// Low power state 
void m8q_low_pwr_state(m8q_trackers_t *m8q_device)
{
    // Idle until a flag triggers an exit 

    // Set the EXTINT pin low to enter low power state 
    m8q_set_low_power(GPIO_LOW); 
}


// Low power exit state 
void m8q_low_pwr_exit_state(m8q_trackers_t *m8q_device)
{
    // Set the EXTINT pin high to exit the low power state 
    m8q_set_low_power(GPIO_HIGH); 

    // Wait for a specified period of time before exiting the state 
    if (tim_time_compare(m8q_device->timer, 
                         M8Q_LOW_PWR_EXIT_DELAY, 
                         &m8q_device->time_cnt_total, 
                         &m8q_device->time_cnt, 
                         &m8q_device->time_start))
    {
        // Set exit flag 
        m8q_device->low_pwr_exit = SET_BIT; 

        // Clear the low power flag 
        m8q_device->low_pwr = CLEAR_BIT; 

        // Clear fix flag 
        m8q_device->fix = CLEAR_BIT; 

        // Reset the start flag 
        m8q_device->time_start = SET_BIT; 
    }
}


// Fault state 
void m8q_fault_state(m8q_trackers_t *m8q_device)
{
    // Wait for the reset flag to be set or for the fault code to be cleared 
}


// Reset state 
void m8q_reset_state(m8q_trackers_t *m8q_device)
{
    // Clear the fault codes 
    m8q_device->fault_code = CLEAR; 

    // Re-initialize the device 
}

//=======================================================================================


//=======================================================================================
// Data functions 

// Get the navigation status 
void m8q_check_msgs(m8q_trackers_t *m8q_device)
{
    // Check if data is available 
    if (m8q_get_tx_ready())
    {
        // Read the available GPS message data 
        m8q_read(); 

        // Check the fix status 
        m8q_device->navstat = (m8q_get_navstat()) % REMAINDER_100; 
    }
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set the low power flag 
void m8q_set_low_pwr_flag(void)
{
    m8q_device_trackers.low_pwr = SET_BIT; 
}


// Clear the low power flag 
void m8q_clear_low_pwr_flag(void)
{
    m8q_device_trackers.low_pwr = CLEAR_BIT; 
}


// Set the reset flag 
void m8q_set_reset_flag(void)
{
    m8q_device_trackers.reset = SET_BIT; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get current controller state 
M8Q_STATE m8q_get_state(void)
{
    return m8q_device_trackers.state; 
}


// Get fault code 
M8Q_FAULT_CODE m8q_get_fault_code(void)
{
    return m8q_device_trackers.fault_code; 
}


// Get the navigation status 
M8Q_NAV_STATE m8q_get_nav_state(void)
{
    return m8q_device_trackers.navstat; 
}

//=======================================================================================
