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
// TODO 
// - When about to send data (to Android) look for a prompt message to start 
// - Have a state that is called at the end of AT command mode state where the baud 
//   rate is read and used to set the baud rate for data mode 
// - Make sure device can read from a user device or another module (i.e. do 
//   the term chars on the received data match?) 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HC05 initialization state 
 * 
 * @details Initializes device tracker parameters. This state is run once on startup and 
 *          is only run again after the reset state. After running it goes to the not 
 *          connected state. 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_init_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 not connected state 
 * 
 * @details In this state the controller idles until a connection is detected at which 
 *          point it goes to the connected state. It is entered from the init state, the 
 *          connected state when a connection is lost, the fault state if the faulr code 
 *          clears and from the low power exit state when returning from low power mode. 
 *          Requests to read and write while in this state will be ignored. 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_not_connected_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 connected state 
 * 
 * @details This state is entered from the not connected state when a Bluetooth 
 *          connection to an external device is detected. From here the controller can 
 *          enter send and read states when requested. It will revert back to the not 
 *          connected state when the connection is lost. 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_connected_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 send state 
 * 
 * @details Sends the data specified using hc05_set_send. When that setter is called 
 *          this state is called but only if the controller in the connected state. If 
 *          in another state then calling the setter will have no affect. The controller 
 *          returns to the connected state immediately after sending the data. 
 * 
 * @see hc05_set_send 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_send_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 read state 
 * 
 * @details In this state the controller continuously checks for available incoming 
 *          data from external devices. If data is available then it reads and stores 
 *          it then sets the read status flag which can be checked using 
 *          hc05_get_read_status. The read data can be gotten using hc05_get_read_data 
 *          which in turn clears the read status flag when called. 
 *          
 *          This state can only be enetered from the connected state and is triggered 
 *          using the hc05_set_read setter. Calling the setter in any other state has 
 *          no affect. The read state exits back to the connected state if the 
 *          hc05_clear setter is called or it goes to the not connected state if a 
 *          connection is lost. 
 * 
 * @see hc05_set_read 
 * @see hc05_clear_read 
 * @see hc_get_read_status 
 * @see hc05_get_read_data 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_read_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 low power state 
 * 
 * @details Puts the device into low power mode by shutting the device off. This 
 *          state is used for low power applications. It is triggered using the 
 *          low power setter. This state can be enetered from the not connected, 
 *          connected or read states. This state is left only is the low power 
 *          flag is cleared, or a fault or reset flag is set. Attempting to read 
 *          or send in this state has no affect. 
 * 
 * @see hc05_set_low_power 
 * @see hc05_clear_low_power
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_low_power_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 low power exit state 
 * 
 * @details Returns the controller from the low power state. In this state the device 
 *          is turned back on and device tracking information is cleared as needed. 
 *          This state is run once and defaults back to the not connected state. 
 *          This state is only enetered from the low power state once a trigger has 
 *          been set to pull the controller out of low power mode. 
 * 
 * @see hc05_clear_low_power
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_low_power_exit_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 fault state 
 * 
 * @details This state is entered when the fault code is set. Here the controller 
 *          idles until the fault code is cleared, in which case it returns to the 
 *          not connected state, or if the reset flag is set at which point the 
 *          controller will enter the reset state. This is the highest priority state 
 *          can can be entered from most states. 
 * 
 * @param hc05_device : pointer to device tracker 
 */
void hc05_fault_state(hc05_device_trackers_t *hc05_device); 


/**
 * @brief HC05 reset state 
 * 
 * @details This state resets the contoller. It power cycles the device, resets any 
 *          fault codes and sends the controller to the init state where device 
 *          trackers are re-initialized. This state is the next highest priority 
 *          behind the fault state. It is triggered by setting the reset flag. 
 * 
 * @see hc05_set_reset
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
void hc05_controller_init(
    TIM_TypeDef *timer)
{
    hc05_device_trackers.timer = timer; 

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

    // Ensure the send and read states are not triggered immediately once the 
    // connected state is reached 
    hc05_device->send = CLEAR_BIT; 
    hc05_device->read = CLEAR_BIT; 
    hc05_device->read_status = CLEAR_BIT; 
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
    // Send data record 
    hc05_send((char *)hc05_device->send_data); 

    // Update fault code if send fails 

    hc05_device->send = CLEAR_BIT; 
}


// Read state 
void hc05_read_state(hc05_device_trackers_t *hc05_device) 
{
    // Pole for data at the UART port 
    if (hc05_data_status())
    {
        hc05_read((char *)hc05_device->read_data); 
        hc05_device_trackers.read_status = SET_BIT; 
    }

    // Update fault code if read fails 

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

    // Power cycle the device to ensure data mode 
    hc05_off(); 
    tim_delay_ms(hc05_device->timer, HC05_RESET_DELAY); 
    hc05_on(); 
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

// Get state 
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
