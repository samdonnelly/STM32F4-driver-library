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
 * @brief 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_init_state(
    mpu6050_trackers_t *mpu6050_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Device tracker record instance 
static mpu6050_trackers_t mpu6050_device_trackers; 

// Function pointer to controller states 
static mpu6050_state_functions_t state_table[MPU6050_NUM_STATES] = 
{
    &mpu6050_init_state 
}; 

//=======================================================================================


//=======================================================================================
// Control functions 

// MPU6050 controller initialization 
void mpu6050_controller_init(void)
{
    // Controller information 
    mpu6050_device_trackers.state = MPU6050_INIT_STATE; 

    // State trackers 
    mpu6050_device_trackers.startup = SET_BIT; 
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
                // 
            }
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
    // Clear the startup bit 
    mpu6050_device->startup = CLEAR_BIT; 

    // Clear the reset bit 
    mpu6050_device->reset = CLEAR_BIT; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set reset flag 
void mpu6050_set_reset_flag(void)
{
    mpu6050_device_trackers.reset = SET_BIT; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get the controller state 
MPU6050_STATE mpu6050_get_state(void)
{
    return mpu6050_device_trackers.state; 
}

//=======================================================================================
