/**
 * @file mpu6050_controller.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU6050 controller 
 * 
 * @version 0.1
 * @date 2023-01-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "mpu6050_controller.h"

//=======================================================================================


//=======================================================================================
// TODO 
// - Do we need to re-call the mpu6050 driver init function on controller reset so we 
//   can collect the fault code and/or reset registers? 
// - Does calibrate need to be a separate state? Calibration requires the device to be 
//   sitting still. 
//=======================================================================================


// extern uint8_t mpu6050_action; 


//=======================================================================================
// Function prototypes 

/**
 * @brief MPU6050 initialization state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_init_state(
    mpu6050_trackers_t *mpu6050_device); 


/**
 * @brief MPU6050 run state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_run_state(
    mpu6050_trackers_t *mpu6050_device); 


/**
 * @brief MPU6050 low power state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_low_power_state(
    mpu6050_trackers_t *mpu6050_device); 


/**
 * @brief MPU6050 low power transition state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_low_power_trans_state(
    mpu6050_trackers_t *mpu6050_device); 


/**
 * @brief MPU6050 fault state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_fault_state(
    mpu6050_trackers_t *mpu6050_device); 


/**
 * @brief MPU6050 reset state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_reset_state(
    mpu6050_trackers_t *mpu6050_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Device tracker record instance 
static mpu6050_trackers_t mpu6050_device_trackers; 

// Function pointer to controller states 
static mpu6050_state_functions_t state_table[MPU6050_NUM_STATES] = 
{
    &mpu6050_init_state, 
    &mpu6050_run_state, 
    &mpu6050_low_power_state, 
    &mpu6050_low_power_trans_state, 
    &mpu6050_fault_state, 
    &mpu6050_reset_state 
}; 

//=======================================================================================


//=======================================================================================
// Control functions 

// MPU6050 controller initialization 
void mpu6050_controller_init(
    TIM_TypeDef *timer, 
    uint32_t sample_period)
{
    // Peripherals 
    mpu6050_device_trackers.timer = timer; 

    // Controller information 
    mpu6050_device_trackers.state = MPU6050_INIT_STATE; 
    mpu6050_device_trackers.fault_code = CLEAR; 
    mpu6050_device_trackers.time_cnt_total = CLEAR; 
    mpu6050_device_trackers.time_cnt = CLEAR; 
    mpu6050_device_trackers.time_start = SET_BIT; 
    mpu6050_device_trackers.sample_period = sample_period; 

    // State trackers 
    mpu6050_device_trackers.startup = SET_BIT; 
    mpu6050_device_trackers.low_power = CLEAR_BIT; 

    // Check for driver faults 
    mpu6050_device_trackers.fault_code |= (uint16_t)mpu6050_get_fault_flag(); 
}


// MPU6050 controller 
void mpu6050_controller(void)
{
    // Record the controller state 
    MPU6050_STATE next_state = mpu6050_device_trackers.state; 

    //===================================================
    // State machine 

    switch (next_state)
    {
        case MPU6050_INIT_STATE: 
            // Fault code set 
            if (mpu6050_device_trackers.fault_code)
            {
                next_state = MPU6050_FAULT_STATE; 
            }
            
            // Default to the run state if no faults 
            else if (!mpu6050_device_trackers.startup)
            {
                next_state = MPU6050_RUN_STATE; 
            }

            break;

        case MPU6050_RUN_STATE: 
            // Fault code set 
            if (mpu6050_device_trackers.fault_code)
            {
                next_state = MPU6050_FAULT_STATE; 
            }
            
            // Reset flag set 
            else if (mpu6050_device_trackers.reset)
            {
                next_state = MPU6050_RESET_STATE; 
            }

            // Low power flag set 
            else if (mpu6050_device_trackers.low_power)
            {
                next_state = MPU6050_LOW_POWER_TRANS_STATE; 
            }

            break;

        case MPU6050_LOW_POWER_STATE: 
            // Low power flag cleared, fault code set or reset flag set 
            if (!mpu6050_device_trackers.low_power || 
                mpu6050_device_trackers.fault_code ||
                mpu6050_device_trackers.reset)
            {
                next_state = MPU6050_LOW_POWER_TRANS_STATE; 
            }

            break;

        case MPU6050_LOW_POWER_TRANS_STATE: 
            // Fault code set 
            if (mpu6050_device_trackers.fault_code)
            {
                next_state = MPU6050_FAULT_STATE; 
            }

            // Reset flag set 
            else if (mpu6050_device_trackers.reset)
            {
                next_state = MPU6050_RESET_STATE; 
            }

            // Low power flag is set 
            else if (mpu6050_device_trackers.low_power)
            {
                next_state = MPU6050_LOW_POWER_STATE; 
            }

            // No flags set - default back to the run state 
            else 
            {
                next_state = MPU6050_RUN_STATE; 
            }

            break;

        case MPU6050_FAULT_STATE: 
            if (mpu6050_device_trackers.reset)
            {
                next_state = MPU6050_RESET_STATE; 
            }

            break;

        case MPU6050_RESET_STATE: 
            next_state = MPU6050_INIT_STATE; 
            break;
        
        default:
            next_state = MPU6050_INIT_STATE; 
            break;
    }

    //===================================================

    // Go to state function 
    (state_table[next_state])(&mpu6050_device_trackers); 

    // Update the state 
    mpu6050_device_trackers.state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// MPU6050 initialization state 
void mpu6050_init_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Test 
    uart_sendstring(USART2, "\r\ninit state\r\n"); 
    // mpu6050_action = SET_BIT; 

    // Clear device trackers 
    mpu6050_device->startup = CLEAR_BIT; 
    mpu6050_device->reset = CLEAR_BIT; 

    // Run self-test and record any faults (failed tests) 
    mpu6050_self_test();
    mpu6050_device->fault_code |= (uint16_t)mpu6050_get_fault_flag(); 
    
    // run calibration to zero the gyroscope values 
    mpu6050_calibrate(); 
}


// MPU6050 run state 
void mpu6050_run_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Wait for a specified period of time before reading new data 
    if (tim_time_compare(mpu6050_device->timer, 
                         mpu6050_device->sample_period, 
                         &mpu6050_device->time_cnt_total, 
                         &mpu6050_device->time_cnt, 
                         &mpu6050_device->time_start))
    {
        // Reset the start flag 
        mpu6050_device->time_start = SET_BIT; 

        // Sample the data 
        mpu6050_read_all(); 
        // mpu6050_temp_read(); 
        // mpu6050_accel_read(); 
        // mpu6050_gyro_read(); 

        // Check for faults 
        // mpu6050_device->fault_code |= (uint16_t)mpu6050_get_fault_flag(); 

        // if (mpu6050_get_temp_raw() > MPU6050_MAX_TEMP)
        // {
        //     mpu6050_device->fault_code |= (SET_BIT << SHIFT_8); 
        // }
    }
}


// MPU6050 low power state 
void mpu6050_low_power_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Does nothing. Waits for low power flag to be cleared. 
}


// MPU6050 low power transition state 
void mpu6050_low_power_trans_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Test 
    uart_sendstring(USART2, "\r\nLP trans state\r\n"); 
    // mpu6050_action = SET_BIT; 

    // Write the low power flag status to the power management register 
    mpu6050_low_pwr_config(mpu6050_device->low_power); 
}


// MPU6050 fault state 
void mpu6050_fault_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Triggered by failed driver init, failed self-test or failed I2C comms. Idles until 
    // reset is called. 
}


// MPU6050 reset state 
void mpu6050_reset_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Test 
    uart_sendstring(USART2, "\r\nreset state\r\n"); 
    // mpu6050_action = SET_BIT; 

    // Reset registers and re-call driver init? 

    // Reset the fault code 
    mpu6050_device->fault_code = CLEAR; 
    mpu6050_clear_fault_flag(); 

    // Reset the low power flag 
    mpu6050_device->low_power = SLEEP_MODE_DISABLE; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set reset flag 
void mpu6050_set_reset_flag(void)
{
    mpu6050_device_trackers.reset = SET_BIT; 
}


// Set low power flag 
void mpu6050_set_low_power(void)
{
    mpu6050_device_trackers.low_power = SLEEP_MODE_ENABLE; 
}


// Clear low power flag 
void mpu6050_clear_low_power(void)
{
    mpu6050_device_trackers.low_power = SLEEP_MODE_DISABLE; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get the controller state 
MPU6050_STATE mpu6050_get_state(void)
{
    return mpu6050_device_trackers.state; 
}


// Get the controller fault code 
MPU6050_FAULT_CODE mpu6050_get_fault_code(void)
{
    return mpu6050_device_trackers.fault_code; 
}

//=======================================================================================