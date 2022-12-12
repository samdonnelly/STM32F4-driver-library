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
//=======================================================================================


//=======================================================================================
// Control functions 

// Initialization 
void m8q_controller_init(void)
{
    // 
}


// Controller 
void m8q_controller(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// State functions 

// 
void m8q_init_state(m8q_trackers_t m8q_device)
{
    // 
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

// 
void m8q_set_low_pwr_flag(void)
{
    // 
}


// 
void m8q_clear_low_pwr_flag(void)
{
    // 
}


// 
void m8q_set_reset_flag(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Getters 

// 
M8Q_STATE m8q_get_state(void)
{
    // 
}


// 
M8Q_FAULT_CODE m8q_get_fault_code(void)
{
    // 
}

//=======================================================================================
