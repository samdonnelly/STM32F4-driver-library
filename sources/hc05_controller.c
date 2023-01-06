/**
 * @file hc05_controller.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC05 controller 
 * 
 * @version 0.1
 * @date 2023-01-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "hc05_controller.h" 

//=======================================================================================


//=======================================================================================
// Notes 
//=======================================================================================


//=======================================================================================
// TODO 
// - Make sure data transfer is complete before turning off the module 
// - Verify the state pin shows connected before any data transfer 
// - When about to send data (to Android) look for a prompt message to start 
// - Have a state that is called at the end of AT command mode state where the baud 
//   rate is read and used to set the baud rate for data mode 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_init_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_not_connected_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_connected_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_send_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_read_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_low_power_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_low_power_exit_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_fault_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param hc05_device 
 */
void hc05_reset_state(hc05_device_trackers_t *hc05_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Instance of device tracker 
static hc05_device_trackers_t hc05_device_trackers; 


// List of function pointers 
static hc05_state_functions_t state_table[HC05_NUM_STATES] = 
{
    &hc05_init_state, 
    &hc05_not_connected_state, 
    &hc05_connected_state, 
    &hc05_send_state, 
    &hc05_read_state, 
    &hc05_low_power_state, 
    &hc05_low_power_exit_state, 
    &hc05_fault_state, 
    &hc05_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control Functions 

// HC05 controller initialization 
void hc05_controller_init(void)
{
    hc05_device_trackers.state = HC05_INIT_STATE; 

    hc05_device_trackers.fault_code = CLEAR; 

    hc05_device_trackers.connect = CLEAR_BIT; 

    hc05_device_trackers.low_pwr = CLEAR_BIT; 

    hc05_device_trackers.reset = CLEAR_BIT; 
}


// HC05 controller 
void hc05_controller(void)
{
    // Record the state 
    hc05_states_t next_state = hc05_device_trackers.state; 

    //=================================================
    // State machine 

    switch (next_state) 
    {
        case HC05_INIT_STATE: 
            break; 

        case HC05_NOT_CONNECTED_STATE: 
            break; 

        case HC05_CONNECTED_STATE: 
            break; 

        case HC05_SEND_STATE: 
            break; 

        case HC05_READ_STATE: 
            break; 

        case HC05_LOW_POWER_STATE: 
            break; 

        case HC05_LOW_POWER_EXIT_STATE: 
            break; 

        case HC05_FAULT_STATE: 
            break; 

        case HC05_RESET_STATE: 
            next_state = HC05_INIT_STATE; 
            break; 

        default: 
            next_state = HC05_INIT_STATE; 
            break; 
    }
    //=================================================

    // State function call 
    (state_table[next_state])(&hc05_device_trackers); 

    // Record the state 
    hc05_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State Functions 

// 
void hc05_init_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_not_connected_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_connected_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_send_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_read_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_low_power_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_low_power_exit_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_fault_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// 
void hc05_reset_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set the send data 
void hc05_set_send(
    uint8_t *data, 
    uint8_t data_size)
{
    // Check that the buffers are suitable for memcpy 
    if ((!data) || (HC05_BUFF_SIZE < data_size)) 
    {
        return; 
    }

    memcpy(hc05_device_trackers.send_data, data, data_size); 

    hc05_device_trackers.send = SET_BIT; 
}


// Set the read flag 
void hc05_set_read(void)
{
    hc05_device_trackers.read = SET_BIT; 
}


// Clear the read flag 
void hc05_clear_read(void)
{
    hc05_device_trackers.read = CLEAR_BIT; 
}


// Set low power flag 
void hc05_set_low_power(void)
{
    hc05_device_trackers.low_pwr = SET_BIT; 
}


// Clear low power flag 
void hc05_clear_low_power(void)
{
    hc05_device_trackers.low_pwr = CLEAR_BIT; 
}


// Set reset flag 
void hc05_set_reset(void)
{
    hc05_device_trackers.reset = SET_BIT; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Read state 
HC05_STATE hc05_get_state(void)
{
    return hc05_device_trackers.state; 
}


// Get fault code 
HC05_FAULT_CODE hc05_get_fault_code(void)
{
    return hc05_device_trackers.fault_code; 
}


// Get the read status 
HC05_READ_STATUS hc05_get_read_status(void)
{
    return hc05_device_trackers.read_status; 
}


// Get the read data 
void hc05_get_read_data(
    uint8_t *buffer, 
    uint8_t buff_size)
{
    // Check that the buffer is suitable for memcpy 
    if ((!buffer) || (buff_size < HC05_BUFF_SIZE)) 
    {
        return; 
    }

    memcpy(buffer, hc05_device_trackers.read_data, HC05_BUFF_SIZE); 

    hc05_device_trackers.read_status = CLEAR_BIT; 
}

//=======================================================================================
