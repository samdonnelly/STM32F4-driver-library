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
    MPU6050_FAULT_CODE driver_fault)
{
    // Controller information 
    mpu6050_device_trackers.state = MPU6050_INIT_STATE; 
    mpu6050_device_trackers.fault_code = CLEAR; 

    // State trackers 
    mpu6050_device_trackers.startup = SET_BIT; 
    mpu6050_device_trackers.low_power = CLEAR_BIT; 

    // Check for driver faults 
    mpu6050_device_trackers.fault_code |= driver_fault; 
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
            // Run the init state at least once 
            if (!mpu6050_device_trackers.startup)
            {
                // Fault code set 
                if (mpu6050_device_trackers.fault_code)
                {
                    next_state = MPU6050_FAULT_STATE; 
                }
                
                // Default to the run state if no faults 
                else 
                {
                    next_state = MPU6050_RUN_STATE; 
                }
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
            if (mpu6050_device_trackers.low_power)
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
    // Clear device trackers 
    mpu6050_device->startup = CLEAR_BIT; 
    mpu6050_device->reset = CLEAR_BIT; 

    // Run self-test, run calibration (and save results), set fault codes as needed 
}


// MPU6050 run state 
void mpu6050_run_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Checks for data (?) and reads from the device. 

    // mpu6050_temp_read(); 
    // mpu6050_accel_read(); 
    // mpu6050_gyro_read(); 
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
    // Flag set when a low power set or clear setter is called which triggers this state. 
    // This state calls the power management register(s) to write the state of the 
    // low power flag. 

    // Pass the low power flag as an argument to the low power function in the driver 
}


// MPU6050 fault state 
void mpu6050_fault_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Triggered by failed driver init, failed self-test, or failed I2C comms. Idles until 
    // reset is called. 
}


// MPU6050 reset state 
void mpu6050_reset_state(
    mpu6050_trackers_t *mpu6050_device)
{
    // Reset registers and re-call driver init? 
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
    mpu6050_device_trackers.low_power = SET_BIT; 
}


// Clear low power flag 
void mpu6050_clear_low_power(void)
{
    mpu6050_device_trackers.low_power = CLEAR_BIT; 
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
