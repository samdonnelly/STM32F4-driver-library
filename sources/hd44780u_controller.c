/**
 * @file hd44780u_controller.c
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

//================================================================================
// Includes 

#include "hd44780u_controller.h"

//================================================================================


//================================================================================
// Notes 
//================================================================================


//================================================================================
// Function prototypes 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_init_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_idle_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_write_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_read_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_low_power_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_fault_state(
    hd44780u_trackers_t hd44780u_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hd44780u_device 
 */
void hd44780u_reset_state(
    hd44780u_trackers_t hd44780u_device); 

//================================================================================


//================================================================================
// Variables 

// Instance of the device tracker record 
static hd44780u_trackers_t hd44780u_device_trackers; 

// Function pointers 
static hd44780u_state_functions_t state_table[HD44780U_NUM_STATES] = 
{
    &hd44780u_init_state, 
    &hd44780u_idle_state, 
    &hd44780u_write_state, 
    &hd44780u_read_state, 
    &hd44780u_low_power_state, 
    &hd44780u_fault_state, 
    &hd44780u_reset_state 
}; 

//================================================================================


//================================================================================
// Control Functions 

// Initialization 
void hd44780u_controller_init(void)
{
    hd44780u_device_trackers.state = HD44780U_INIT_STATE; 

    hd44780u_device_trackers.fault_code = CLEAR; 

    hd44780u_device_trackers.write = CLEAR_BIT; 

    hd44780u_device_trackers.read = CLEAR_BIT; 

    hd44780u_device_trackers.low_power = CLEAR_BIT; 

    hd44780u_device_trackers.reset = CLEAR_BIT; 

    hd44780u_device_trackers.startup = SET_BIT; 
}


// Controller 
void hd44780u_controller(void)
{
    // Local variables 
    hd44780u_states_t next_state = hd44780u_device_trackers.state; 

    //==================================================
    // State machine 

    // Fault state 
    if (hd44780u_device_trackers.fault_code)
    {
        // Check the current state status 
        if (next_state == HD44780U_FAULT_STATE)
        {
            if (hd44780u_device_trackers.reset)
                next_state = HD44780U_RESET_STATE; 
        }
        else 
            next_state = HD44780U_FAULT_STATE; 
    }

    // Reset state 
    else if (hd44780u_device_trackers.reset)
    {
        if (next_state == HD44780U_RESET_STATE)
            next_state = HD44780U_INIT_STATE; 
        else 
            next_state = HD44780U_RESET_STATE; 
    }

    // Other states 
    else 
    {
        switch (next_state)
        {
            case HD44780U_INIT_STATE: 
                if (!(hd44780u_device_trackers.startup))
                    next_state = HD44780U_IDLE_STATE; 
                break; 

            case HD44780U_IDLE_STATE: 
                if (hd44780u_device_trackers.write)
                    next_state = HD44780U_WRITE_STATE; 
                
                else if (hd44780u_device_trackers.read)
                    next_state = HD44780U_READ_STATE; 

                else if (hd44780u_device_trackers.low_power)
                    next_state = HD44780U_LOW_PWR_STATE; 
                
                break; 

            case HD44780U_WRITE_STATE: 
                if (hd44780u_device_trackers.read)
                    next_state = HD44780U_READ_STATE; 

                else if (!(hd44780u_device_trackers.write))
                    next_state = HD44780U_IDLE_STATE; 
                
                break; 

            case HD44780U_READ_STATE: 
                if (hd44780u_device_trackers.write)
                    next_state = HD44780U_WRITE_STATE; 

                else if (!(hd44780u_device_trackers.read))
                    next_state = HD44780U_IDLE_STATE; 
                
                break; 

            case HD44780U_LOW_PWR_STATE: 
                if (!(hd44780u_device_trackers.low_power))
                    next_state = HD44780U_IDLE_STATE; 
                break; 

            default: 
                break; 
        }
    }

    //==================================================

    // Go to state function 
    (state_table[next_state])(hd44780u_device_trackers); 

    // Update the state 
    hd44780u_device_trackers.state = next_state; 
}

//================================================================================
// State functions 

// 
void hd44780u_init_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear reset flag 
    // Clear startup flag 
}


// 
void hd44780u_idle_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Do basically nothing 
}


// 
void hd44780u_write_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear the write flag 
}


// 
void hd44780u_read_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear the read flag 
}


// 
void hd44780u_low_power_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Put the screen in low power mode. 
    // Should this just be a transition state? You'd call this then return to idle 
    // then go to a different state to bring the screen out of low power. 
}


// 
void hd44780u_fault_state(
    hd44780u_trackers_t hd44780u_device)
{
    // 
}


// 
void hd44780u_reset_state(
    hd44780u_trackers_t hd44780u_device)
{
    // Clear the fault code 
}

//================================================================================


//================================================================================
// Setters 
//================================================================================


//================================================================================
// Getters 
//================================================================================

//================================================================================
