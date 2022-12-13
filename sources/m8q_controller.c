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
// - Run the m8q_read function only when TX_READY is set by the receiver 
// - How do we translate faults in drivers to the controller? 

//===================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_init_state(m8q_trackers_t m8q_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_no_fix_state(m8q_trackers_t m8q_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_fix_state(m8q_trackers_t m8q_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_low_pwr_state(m8q_trackers_t m8q_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_fault_state(m8q_trackers_t m8q_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_reset_state(m8q_trackers_t m8q_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Instance of the device tracker record 
static m8q_trackers_t m8q_device_trackers; 


// Function pointers to controller states 
static m8q_state_functions_t state_table[M8q_NUM_STATES] =
{
    &m8q_init_state, 
    &m8q_no_fix_state, 
    &m8q_fix_state, 
    &m8q_low_pwr_state, 
    &m8q_fault_state, 
    &m8q_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control functions 

// Initialization 
void m8q_controller_init(void)
{
    m8q_device_trackers.state = M8Q_INIT_STATE; 

    m8q_device_trackers.fault_code = CLEAR; 

    m8q_device_trackers.low_pwr = CLEAR_BIT; 

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
    //==================================================

    // Record the state 
    m8q_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// Initialization state 
void m8q_init_state(m8q_trackers_t m8q_device)
{
    // CLear the reset flag 
    m8q_device_trackers.reset = CLEAR_BIT; 

    // Clear the startup bit 
    m8q_device_trackers.startup = CLEAR_BIT; 
}


// 
void m8q_no_fix_state(m8q_trackers_t m8q_device)
{
    // 
}


// 
void m8q_fix_state(m8q_trackers_t m8q_device)
{
    // 
}


// 
void m8q_low_pwr_state(m8q_trackers_t m8q_device)
{
    // 
}


// 
void m8q_fault_state(m8q_trackers_t m8q_device)
{
    // 
}


// 
void m8q_reset_state(m8q_trackers_t m8q_device)
{
    // 
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
    // 
}


// Get fault code 
M8Q_FAULT_CODE m8q_get_fault_code(void)
{
    // 
}

//=======================================================================================
