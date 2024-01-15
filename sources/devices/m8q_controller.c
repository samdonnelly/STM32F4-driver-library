/**
 * @file m8q_controller.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS controller implementation 
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


//=======================================================================================
// Macros 

#define M8Q_NUM_STATES 7                // Number of controller states 
#define M8Q_LOW_PWR_EXIT_DELAY 150000   // (us) time to wait when exiting low power mode 
#define M8Q_STATUS_FAULT_MASK 0x001A    // Identifies fault statuses - bitmask for m8q_status_t 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief M8Q state machine function pointer 
 * 
 * @param m8q_device : device tracker that defines controller characteristics 
 */
typedef void (*m8q_state_functions_t)(
    m8q_trackers_t *m8q_device); 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Check for driver and controller faults 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_fault_check(
    m8q_trackers_t *m8q_device); 

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
void m8q_init_state(
    m8q_trackers_t *m8q_device); 


/**
 * @brief Read state 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_read_state(
    m8q_trackers_t *m8q_device); 


/**
 * @brief Idle state 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_idle_state(
    m8q_trackers_t *m8q_device); 


/**
 * @brief Low power transition state 
 * 
 * @details 
 * 
 * @param m8q_device 
 */
void m8q_low_pwr_enter_state(
    m8q_trackers_t *m8q_device); 


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
void m8q_low_pwr_exit_state(
    m8q_trackers_t *m8q_device); 


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
void m8q_fault_state(
    m8q_trackers_t *m8q_device); 


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
void m8q_reset_state(
    m8q_trackers_t *m8q_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Instance of the device tracker record 
static m8q_trackers_t m8q_device_trackers; 


// Function pointers to controller states 
static m8q_state_functions_t state_table[M8Q_NUM_STATES] =
{
    &m8q_init_state, 
    &m8q_read_state, 
    &m8q_idle_state, 
    &m8q_low_pwr_enter_state, 
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
    if (timer == NULL)
    {
        m8q_device_trackers.device_status = (SET_BIT << M8Q_INVALID_PTR); 
        return; 
    }

    // Peripherals 
    m8q_device_trackers.timer = timer;

    // Device and controller information 
    m8q_device_trackers.state = M8Q_INIT_STATE; 
    m8q_device_trackers.device_status = CLEAR; 
    m8q_device_trackers.fault_code = CLEAR; 
    m8q_device_trackers.clk_freq = tim_get_pclk_freq(timer); 
    m8q_device_trackers.time_cnt_total = CLEAR; 
    m8q_device_trackers.time_cnt = CLEAR; 
    m8q_device_trackers.time_start = SET_BIT; 

    // State flags 
    m8q_device_trackers.init = SET_BIT; 
    m8q_device_trackers.read = SET_BIT; 
    m8q_device_trackers.idle = CLEAR_BIT; 
    m8q_device_trackers.low_pwr = CLEAR_BIT; 
    m8q_device_trackers.low_pwr_enter = CLEAR_BIT; 
    m8q_device_trackers.low_pwr_exit = CLEAR_BIT; 
    m8q_device_trackers.reset = CLEAR_BIT; 
}


// Controller 
void m8q_controller(void)
{
    m8q_states_t next_state = m8q_device_trackers.state; 

    // Check the driver and controller status for faults 
    if (m8q_device_trackers.device_status)
    {
        m8q_fault_check(&m8q_device_trackers); 
    }

    //==================================================
    // State machine 

    switch (next_state)
    {
        case M8Q_INIT_STATE: 
            if (m8q_device_trackers.fault_code)
            {
                next_state = M8Q_FAULT_STATE; 
            }
            else if (m8q_device_trackers.init)
            {
                next_state = M8Q_INIT_STATE; 
            }
            else if (m8q_device_trackers.read)
            {
                next_state = M8Q_READ_STATE; 
            }
            else if (m8q_device_trackers.idle)
            {
                next_state = M8Q_IDLE_STATE; 
            }

            break; 

        case M8Q_READ_STATE: 
            if (m8q_device_trackers.fault_code)
            {
                next_state = M8Q_FAULT_STATE; 
            }
            else if (m8q_device_trackers.low_pwr_enter)
            {
                next_state = M8Q_LOW_PWR_ENTER_STATE; 
            }
            else if (m8q_device_trackers.idle)
            {
                next_state = M8Q_IDLE_STATE; 
            }

            break; 

        case M8Q_IDLE_STATE: 
            if (m8q_device_trackers.fault_code)
            {
                next_state = M8Q_FAULT_STATE; 
            }
            else if (m8q_device_trackers.low_pwr)
            {
                if (m8q_device_trackers.low_pwr_exit)
                {
                    next_state = M8Q_LOW_PWR_EXIT_STATE; 
                }
            }
            else if (m8q_device_trackers.low_pwr_enter)
            {
                next_state = M8Q_LOW_PWR_ENTER_STATE; 
            }
            else if (m8q_device_trackers.read)
            {
                next_state = M8Q_READ_STATE; 
            }

            break; 

        case M8Q_LOW_PWR_ENTER_STATE: 
            next_state = M8Q_IDLE_STATE; 
            break; 

        case M8Q_LOW_PWR_EXIT_STATE: 
            if (m8q_device_trackers.fault_code)
            {
                next_state = M8Q_FAULT_STATE; 
            }
            else if (!m8q_device_trackers.low_pwr)
            {
                if (m8q_device_trackers.read)
                {
                    next_state = M8Q_READ_STATE; 
                }
                else
                {
                    next_state = M8Q_IDLE_STATE; 
                }
            }
            
            break; 

        case M8Q_FAULT_STATE: 
            if (m8q_device_trackers.reset || !m8q_device_trackers.fault_code)
            {
                next_state = M8Q_RESET_STATE; 
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

    // Go to state function and record the state 
    (state_table[next_state])(&m8q_device_trackers); 
    m8q_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// Controller functions 

// Check for driver and controller faults 
void m8q_fault_check(
    m8q_trackers_t *m8q_device)
{
    m8q_device->fault_code = m8q_device->device_status & M8Q_STATUS_FAULT_MASK; 
    m8q_device->device_status = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// Initialization state 
void m8q_init_state(
    m8q_trackers_t *m8q_device)
{
    m8q_device->init = CLEAR_BIT; 
    m8q_device->reset = CLEAR_BIT; 
}


// Read state 
void m8q_read_state(
    m8q_trackers_t *m8q_device)
{
    M8Q_STATUS read_status; 

    if (m8q_get_tx_ready())
    {
        read_status = m8q_read_data(); 
        m8q_device_trackers.device_status = (SET_BIT << read_status); 
    }
}


// Idle state 
void m8q_idle_state(
    m8q_trackers_t *m8q_device)
{
    // Do nothing - used in idle and low power modes 
}


// Low power transition state 
void m8q_low_pwr_enter_state(
    m8q_trackers_t *m8q_device)
{
    m8q_set_low_pwr(); 
    m8q_device->low_pwr = SET_BIT; 
    m8q_device->low_pwr_enter = CLEAR_BIT; 
}


// Low power exit state 
void m8q_low_pwr_exit_state(
    m8q_trackers_t *m8q_device)
{
    M8Q_STATUS read_status; 
    uint8_t dummy_buff; 
    static uint8_t exit_flag = CLEAR; 

    // Set the EXTINT pin high to exit the low power state 
    m8q_clear_low_pwr(); 

    // Give the device some time to wake up before accessing it. 
    if (tim_compare(m8q_device->timer, 
                    m8q_device->clk_freq, 
                    M8Q_LOW_PWR_EXIT_DELAY, 
                    &m8q_device->time_cnt_total, 
                    &m8q_device->time_cnt, 
                    &m8q_device->time_start))
    {
        exit_flag++; 
    }

    // Once the waiting period is over then flush the device data stream before exiting 
    // the state. 
    if (exit_flag)
    {
        read_status = m8q_read_ds(&dummy_buff, BYTE_1); 
        m8q_device_trackers.device_status = (SET_BIT << read_status); 
        exit_flag = CLEAR; 

        if (read_status == M8Q_DATA_BUFF_OVERFLOW)
        {
            m8q_device->low_pwr = CLEAR_BIT; 
            m8q_device->low_pwr_exit = CLEAR_BIT; 
            m8q_device->time_start = SET_BIT; 
        }
    }
}


// Fault state 
void m8q_fault_state(
    m8q_trackers_t *m8q_device)
{
    // Wait for the reset flag to be set or for the fault code to be cleared 

    m8q_device->low_pwr = CLEAR_BIT; 
    m8q_device->low_pwr_enter = CLEAR_BIT; 
    m8q_device->low_pwr_exit = CLEAR_BIT; 
    m8q_device->time_start = SET_BIT; 
}


// Reset state 
void m8q_reset_state(
    m8q_trackers_t *m8q_device)
{
    // Clear the fault codes 
    m8q_device->fault_code = CLEAR; 
    // m8q_clear_status(); 
}

//=======================================================================================


//=======================================================================================
// User functions 

// Set the read flag 
void m8q_set_read_flag(void)
{
    m8q_device_trackers.read = SET_BIT; 
    m8q_device_trackers.idle = CLEAR_BIT; 
}


// Set the idle flag 
void m8q_set_idle_flag(void)
{
    m8q_device_trackers.read = CLEAR_BIT; 
    m8q_device_trackers.idle = SET_BIT; 
}


// Enter the low power state 
void m8q_set_low_pwr_flag(void)
{
    m8q_device_trackers.low_pwr_enter = SET_BIT; 
}


// Exit the low power state 
void m8q_clear_low_pwr_flag(void)
{
    m8q_device_trackers.low_pwr_exit = SET_BIT; 
}


// Set the reset flag 
void m8q_set_reset_flag(void)
{
    m8q_device_trackers.reset = SET_BIT; 
}


// Get current controller state 
M8Q_STATE m8q_get_state(void)
{
    return m8q_device_trackers.state; 
}


// Get low power flag state 
uint8_t m8q_get_lp_flag(void)
{
    return m8q_device_trackers.low_pwr; 
}


// Get fault code 
M8Q_FAULT_CODE m8q_get_fault_code(void)
{
    return m8q_device_trackers.fault_code; 
}

//=======================================================================================
