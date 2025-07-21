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


//=======================================================================================
// Macros 

#define M8Q_NUM_STATES 7                // Number of controller states - see m8q_states_t 
#define M8Q_LOW_PWR_EXIT_DELAY 150000   // time to wait when exiting low power mode (us) 
#define M8Q_STATUS_FAULT_MASK 0x001A    // Identifies fault statuses - bitmask for m8q_status_t 

//=======================================================================================


//=======================================================================================
// Variables 

// M8Q controller trackers 
typedef struct m8q_trackers_s 
{
    // Peripherals 
    TIM_TypeDef *timer;                     // Non-blocking delay timer 
    
    // Device and controller information 
    m8q_states_t state;                     // Controller state 
    uint16_t device_status;                 // Device status based on m8q_status_t 
    uint16_t fault_code;                    // Fault code for the device/controller 
    uint32_t clk_freq;                      // Timer clock frquency 
    uint32_t time_cnt_total;                // Time delay counter total count 
    uint32_t time_cnt;                      // Time delay counter instance 
    uint8_t  time_start;                    // Time delay counter start flag 

    // State flags 
    uint8_t init          : 1;              // Init state trigger 
    uint8_t read          : 1;              // Read state trigger 
    uint8_t idle          : 1;              // Idle state trigger 
    uint8_t low_pwr       : 1;              // Low power state flag 
    uint8_t low_pwr_enter : 1;              // Low power mode enter trigger 
    uint8_t low_pwr_exit  : 1;              // Low power mode exit trigger 
    uint8_t reset         : 1;              // Reset state trigger 
}
m8q_trackers_t; 


// Instance of the device tracker record 
static m8q_trackers_t m8q_device_trackers; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Check for M8Q driver and controller faults 
 * 
 * @details If the driver status (checked in the main control function) is not M8Q_OK 
 *          then this function is called. Here the status is filtered to find critical 
 *          faults/statuses which get assigned to the controller fault code. If filtering 
 *          produces no code then the status is ignored and the controller continues. If 
 *          the code is set then the controller will enter the fault state. 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_fault_check(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q controller initialization state 
 * 
 * @details This state is dedicated to any process that needs to be run once when the 
 *          controller starts. This state is called when the main control function is 
 *          first called and after the reset state. Note that the controller 
 *          initialization function (not to be confused with this state) should be 
 *          called once at the beginning of the code before starting to use the 
 *          main control function. 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_init_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q controller read state 
 * 
 * @details Checks the TX ready pin to see if the device has data waiting. If so then 
 *          the device driver read function is called to read and sort the data. This 
 *          state can be entered from the init, idle and low power exit states if the 
 *          read flag is set. 
 * 
 * @see m8q_set_read_flag 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_read_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q controller idle state 
 * 
 * @details Performs no actions. This state acts as the idle and low power state since 
 *          they both do nothing. The idle state is meant to keep the device on without 
 *          using it. The low power state is meant to keep the device in low power mode 
 *          where it consumes small amounts of power. This state can be entered from 
 *          the read and init states if the idle flag is set and from the low power 
 *          enter state if the low power flag is set. 
 * 
 * @see m8q_set_idle_flag 
 * @see m8q_set_low_pwr_flag 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_idle_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q controller low power enter state 
 * 
 * @details Sets the device interrupt pin low to put the device into low power mode. 
 *          After doing so it immediately goes to the idle (low power) state. This 
 *          state can be entered from the read or idle states after the low power 
 *          enter flag is set. 
 * 
 * @see m8q_set_low_pwr_flag 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_low_pwr_enter_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q controller low power exit state 
 * 
 * @details Sets the device interrupt pin high to wake the device up from low power 
 *          mode. After this state is entered, a non-blocking timer will start to 
 *          give the device a chance to wake up. Once the timer is up then the state 
 *          attempts to read and flush the device data buffer. This process repeats 
 *          until the data buffer is successfully cleared at which point the state 
 *          will exit to either the read or idle states. This state can be entered 
 *          from the idle (low power) state after the low power exit flag is set. 
 * 
 * @see m8q_clear_low_pwr_flag 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_low_pwr_exit_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q controller fault state 
 * 
 * @details If the driver/controller fault code is set then this state will be 
 *          entered. Note that in order for the fault code to be set, the 
 *          controller has to be in a state that's performing actions (i.e. not the 
 *          idle / low power states). In the state the controller will idle until 
 *          the fault code is cleared or the reset flag is set. A fault is a 
 *          critical status that affects the devices ability to function properly. 
 * 
 * @see m8q_fault_check 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_fault_state(m8q_trackers_t *m8q_device); 


/**
 * @brief M8Q controller reset state 
 * 
 * @details Clears the fault code and performs any other needed actions to restart 
 *          the controller. This state is entered from the fault state after the 
 *          reset flag is set and will proceed directly to the init state after 
 *          it's done. 
 * 
 * @see m8q_set_reset_flag 
 * 
 * @param m8q_device : controller tracking information 
 */
void m8q_reset_state(m8q_trackers_t *m8q_device); 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief M8Q controller state machine function pointer 
 * 
 * @param m8q_device : controller tracking information 
 */
typedef void (*m8q_state_functions_t)(m8q_trackers_t *m8q_device); 


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
void m8q_controller_init(TIM_TypeDef *timer)
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
void m8q_fault_check(m8q_trackers_t *m8q_device)
{
    m8q_device->fault_code = m8q_device->device_status & M8Q_STATUS_FAULT_MASK; 
    m8q_device->device_status = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// Initialization state 
void m8q_init_state(m8q_trackers_t *m8q_device)
{
    m8q_device->init = CLEAR_BIT; 
    m8q_device->reset = CLEAR_BIT; 
}


// Read state 
void m8q_read_state(m8q_trackers_t *m8q_device)
{
    M8Q_STATUS read_status; 

    if (m8q_get_tx_ready())
    {
        read_status = m8q_read_data(); 
        m8q_device_trackers.device_status = (SET_BIT << read_status); 
    }
}


// Idle state 
void m8q_idle_state(m8q_trackers_t *m8q_device)
{
    // Do nothing - used in idle and low power modes 
}


// Low power enter state 
void m8q_low_pwr_enter_state(m8q_trackers_t *m8q_device)
{
    m8q_set_low_pwr(); 
    m8q_device->low_pwr = SET_BIT; 
    m8q_device->low_pwr_enter = CLEAR_BIT; 
}


// Low power exit state 
void m8q_low_pwr_exit_state(m8q_trackers_t *m8q_device)
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
void m8q_fault_state(m8q_trackers_t *m8q_device)
{
    // Wait for the reset flag to be set or for the fault code to be cleared 

    m8q_device->low_pwr = CLEAR_BIT; 
    m8q_device->low_pwr_enter = CLEAR_BIT; 
    m8q_device->low_pwr_exit = CLEAR_BIT; 
    m8q_device->time_start = SET_BIT; 
}


// Reset state 
void m8q_reset_state(m8q_trackers_t *m8q_device)
{
    // Clear the fault codes 
    m8q_device->fault_code = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// Setters and getters 

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
