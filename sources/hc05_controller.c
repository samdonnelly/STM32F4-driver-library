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
 * @brief HC05 initialization state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_init_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 not connected state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_not_connected_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 connected state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_connected_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 send state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_send_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 read state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_read_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 low power state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_low_power_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 low power exit state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_low_power_exit_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 fault state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_fault_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 reset state 
 * 
 * @details 
 * 
 * @param hc05_device : pointer to device tracker 
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

    hc05_device_trackers.startup = SET_BIT; 
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
            if (!hc05_device_trackers.startup)
            {
                next_state = HC05_NOT_CONNECTED_STATE; 
            }
            break; 

        case HC05_NOT_CONNECTED_STATE: 
            if (hc05_device_trackers.fault_code)
            {
                next_state = HC05_FAULT_STATE; 
            }
            else if (hc05_device_trackers.reset)
            {
                next_state = HC05_RESET_STATE; 
            }
            else if (hc05_device_trackers.low_pwr)
            {
                next_state = HC05_LOW_POWER_STATE; 
            }
            else if (hc05_device_trackers.connect)
            {
                next_state = HC05_CONNECTED_STATE; 
            }
            break; 

        case HC05_CONNECTED_STATE: 
            if (hc05_device_trackers.fault_code)
            {
                next_state = HC05_FAULT_STATE; 
            }
            else if (hc05_device_trackers.reset)
            {
                next_state = HC05_RESET_STATE; 
            }
            else if (hc05_device_trackers.low_pwr)
            {
                next_state = HC05_LOW_POWER_STATE; 
            }
            else if (!hc05_device_trackers.connect)
            {
                next_state = HC05_NOT_CONNECTED_STATE; 
            }
            else if (hc05_device_trackers.send)
            {
                next_state = HC05_SEND_STATE; 
            }
            else if (hc05_device_trackers.read)
            {
                next_state = HC05_READ_STATE; 
            }
            break; 

        case HC05_SEND_STATE: 
            next_state = HC05_CONNECTED_STATE; 

        case HC05_READ_STATE: 
            if (hc05_device_trackers.fault_code)
            {
                next_state = HC05_FAULT_STATE; 
            }
            else if (hc05_device_trackers.reset)
            {
                next_state = HC05_RESET_STATE; 
            }
            else if (!hc05_device_trackers.connect)
            {
                next_state = HC05_NOT_CONNECTED_STATE; 
            }
            else if (!hc05_device_trackers.read)
            {
                next_state = HC05_CONNECTED_STATE; 
            }
            break; 

        case HC05_LOW_POWER_STATE: 
            if (hc05_device_trackers.fault_code || 
                hc05_device_trackers.reset  || 
               !hc05_device_trackers.low_pwr)
            {
                next_state = HC05_LOW_POWER_EXIT_STATE; 
            }
            break; 

        case HC05_LOW_POWER_EXIT_STATE: 
            if (hc05_device_trackers.fault_code)
            {
                next_state = HC05_FAULT_STATE; 
            }
            else if (hc05_device_trackers.reset)
            {
                next_state = HC05_RESET_STATE; 
            }
            else 
            {
                next_state = HC05_NOT_CONNECTED_STATE; 
            }
            break; 

        case HC05_FAULT_STATE: 
            if (hc05_device_trackers.reset)
            {
                next_state = HC05_RESET_STATE; 
            }
            else if (!hc05_device_trackers.fault_code)
            {
                next_state = HC05_NOT_CONNECTED_STATE; 
            }
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

// Initialization state 
void hc05_init_state(hc05_device_trackers_t *hc05_device) 
{
    // Ensures the init state runs once on startup 
    hc05_device->startup = CLEAR_BIT; 

    // Clear device tracking data 
    hc05_device->reset = CLEAR_BIT; 
    hc05_device->connect = CLEAR_BIT; 
    hc05_device->send = CLEAR_BIT; 
    hc05_device->read = CLEAR_BIT; 
    hc05_device->read_status = CLEAR_BIT; 
    hc05_device->low_pwr = CLEAR_BIT; 

    // Clear the data buffers 
    memset(hc05_device->send_data, CLEAR, HC05_BUFF_SIZE); 
    memset(hc05_device->read_data, CLEAR, HC05_BUFF_SIZE); 
}


// Not connected state 
void hc05_not_connected_state(hc05_device_trackers_t *hc05_device) 
{
    // Check for a connection 
    hc05_device->connect = hc05_status(); 

    // Clear send and read flag triggers 
    hc05_device->send = CLEAR_BIT; 
    hc05_device->read = CLEAR_BIT; 
}


// Connected state 
void hc05_connected_state(hc05_device_trackers_t *hc05_device) 
{
    // Check for a connection 
    hc05_device->connect = hc05_status(); 

    // Clear the devices UART data register of misc data 
    hc05_clear(); 
}


// Send state 
void hc05_send_state(hc05_device_trackers_t *hc05_device) 
{
    // 
}


// Read state 
void hc05_read_state(hc05_device_trackers_t *hc05_device) 
{
    // Pole for data at the UART port 
    // Set the read status flag 
    // If we lose connection and the read flag is set we can set a fault 
}


// Low power state 
void hc05_low_power_state(hc05_device_trackers_t *hc05_device) 
{
    hc05_off(); 
}


// Low power exit state 
void hc05_low_power_exit_state(hc05_device_trackers_t *hc05_device) 
{
    hc05_on(); 

    // Clear device tracker data 
    hc05_device->connect = CLEAR_BIT; 
    hc05_device->send = CLEAR_BIT; 
    hc05_device->read = CLEAR_BIT; 
    hc05_device->read_status = CLEAR_BIT; 
    hc05_device->low_pwr = CLEAR_BIT;
}


// Fault state 
void hc05_fault_state(hc05_device_trackers_t *hc05_device) 
{
    // Wait for fault code to clear or reset flag to set 
}


// Reset state 
void hc05_reset_state(hc05_device_trackers_t *hc05_device) 
{
    hc05_device->fault_code = CLEAR; 

    // Should we power cycle the device? 
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
