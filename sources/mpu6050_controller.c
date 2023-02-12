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
//   sitting still and often during power/start up the device is not going to be 
//   perfectly still. 
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
    mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 run state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_run_state(
    mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 low power state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_low_power_state(
    mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 low power transition state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_low_power_trans_state(
    mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 fault state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_fault_state(
    mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 reset state 
 * 
 * @details 
 * 
 * @param mpu6050_device 
 */
void mpu6050_reset_state(
    mpu6050_cntrl_data_t *mpu6050_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Device tracker record instance 
static mpu6050_cntrl_data_t *mpu6050_cntrl_data_ptr = NULL; 

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
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    uint32_t sample_period)
{
    // Create a controller data record if it does not already exist 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)create_linked_list_entry(
                                        device_num, 
                                        (void *)&mpu6050_cntrl_data_ptr, 
                                        sizeof(mpu6050_cntrl_data_t)); 
    
    // Peripherals 
    cntrl_data_ptr->timer = timer; 

    // Controller information 
    cntrl_data_ptr->device_num = device_num; 
    cntrl_data_ptr->state = MPU6050_INIT_STATE; 
    cntrl_data_ptr->fault_code = CLEAR; 
    cntrl_data_ptr->clk_freq = tim_get_pclk_freq(cntrl_data_ptr->timer); 
    cntrl_data_ptr->sample_period = sample_period; 
    cntrl_data_ptr->time_cnt_total = CLEAR; 
    cntrl_data_ptr->time_cnt = CLEAR; 
    cntrl_data_ptr->time_start = SET_BIT; 

    // State trackers 
    cntrl_data_ptr->startup = SET_BIT; 
    cntrl_data_ptr->low_power = CLEAR_BIT; 

    // Check for driver faults 
    cntrl_data_ptr->fault_code |= 
        mpu6050_get_fault_flag(cntrl_data_ptr->device_num); 
}


// MPU6050 controller 
void mpu6050_controller(
    device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    // Record the controller state 
    MPU6050_STATE next_state = cntrl_data_ptr->state; 

    //===================================================
    // State machine 

    switch (next_state)
    {
        case MPU6050_INIT_STATE: 
            // Fault code set 
            if (cntrl_data_ptr->fault_code)
            {
                next_state = MPU6050_FAULT_STATE; 
            }
            
            // Default to the run state if no faults 
            else if (!cntrl_data_ptr->startup)
            {
                next_state = MPU6050_RUN_STATE; 
            }

            break;

        case MPU6050_RUN_STATE: 
            // Fault code set 
            if (cntrl_data_ptr->fault_code)
            {
                next_state = MPU6050_FAULT_STATE; 
            }
            
            // Reset flag set 
            else if (cntrl_data_ptr->reset)
            {
                next_state = MPU6050_RESET_STATE; 
            }

            // Low power flag set 
            else if (cntrl_data_ptr->low_power)
            {
                next_state = MPU6050_LOW_POWER_TRANS_STATE; 
            }

            break;

        case MPU6050_LOW_POWER_STATE: 
            // Low power flag cleared, fault code set or reset flag set 
            if (!cntrl_data_ptr->low_power || 
                cntrl_data_ptr->fault_code ||
                cntrl_data_ptr->reset)
            {
                next_state = MPU6050_LOW_POWER_TRANS_STATE; 
            }

            break;

        case MPU6050_LOW_POWER_TRANS_STATE: 
            // Fault code set 
            if (cntrl_data_ptr->fault_code)
            {
                next_state = MPU6050_FAULT_STATE; 
            }

            // Reset flag set 
            else if (cntrl_data_ptr->reset)
            {
                next_state = MPU6050_RESET_STATE; 
            }

            // Low power flag is set 
            else if (cntrl_data_ptr->low_power)
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
            if (cntrl_data_ptr->reset)
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
    (state_table[next_state])(cntrl_data_ptr); 

    // Update the state 
    cntrl_data_ptr->state = next_state; 
}

//=======================================================================================


//=======================================================================================
// State functions 

// MPU6050 initialization state 
void mpu6050_init_state(
    mpu6050_cntrl_data_t *mpu6050_device)
{
    // Clear device trackers 
    mpu6050_device->startup = CLEAR_BIT; 
    mpu6050_device->reset = CLEAR_BIT; 

    // Run self-test and record any faults (failed tests) 
    mpu6050_self_test(mpu6050_device->device_num);
    mpu6050_device->fault_code |= mpu6050_get_fault_flag(mpu6050_device->device_num); 

    // TODO the delay time changes based on the clock prescaler so this is not robust 
    // Provide time for data to update so self-test data is not used for calibration 
    tim_delay_ms(mpu6050_device->timer, MPU6050_ST_DELAY); 
    
    // run calibration to zero the gyroscope values 
    mpu6050_calibrate(mpu6050_device->device_num); 
}


// MPU6050 run state 
void mpu6050_run_state(
    mpu6050_cntrl_data_t *mpu6050_device)
{
    // Wait for a specified period of time before reading new data 
    if (tim_compare(mpu6050_device->timer, 
                    mpu6050_device->clk_freq, 
                    mpu6050_device->sample_period, 
                    &mpu6050_device->time_cnt_total, 
                    &mpu6050_device->time_cnt, 
                    &mpu6050_device->time_start))
    {
        // Sample the data 
        mpu6050_read_all(mpu6050_device->device_num); 

        // Check for faults 
        mpu6050_device->fault_code |= mpu6050_get_fault_flag(mpu6050_device->device_num); 

        if (mpu6050_get_temp_raw(mpu6050_device->device_num) > 
            (MPU6050_RAW_TEMP_MAX-MPU6050_RAW_TEMP_OFST))
        {
            mpu6050_device->fault_code |= (SET_BIT << SHIFT_9); 
        }
    }
}


// MPU6050 low power state 
void mpu6050_low_power_state(
    mpu6050_cntrl_data_t *mpu6050_device)
{
    // Does nothing. Waits for low power flag to be cleared. 
}


// MPU6050 low power transition state 
void mpu6050_low_power_trans_state(
    mpu6050_cntrl_data_t *mpu6050_device)
{
    // Write the low power flag status to the power management register 
    mpu6050_low_pwr_config(
        mpu6050_device->device_num, 
        mpu6050_device->low_power); 
}


// MPU6050 fault state 
void mpu6050_fault_state(
    mpu6050_cntrl_data_t *mpu6050_device)
{
    // Triggered by failed driver init, failed self-test or failed I2C comms. Idles until 
    // reset is called. 
}


// MPU6050 reset state 
void mpu6050_reset_state(
    mpu6050_cntrl_data_t *mpu6050_device)
{
    // Reset registers and re-call driver init? 

    // Reset the fault code 
    mpu6050_device->fault_code = CLEAR; 
    mpu6050_clear_fault_flag(mpu6050_device->device_num); 

    // Reset the low power flag and make sure to exit sleep mode 
    mpu6050_device->low_power = SLEEP_MODE_DISABLE; 
    mpu6050_low_pwr_config(
        mpu6050_device->device_num, 
        mpu6050_device->low_power); 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set reset flag 
void mpu6050_set_reset_flag(
    device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    cntrl_data_ptr->reset = SET_BIT; 
}


// Set low power flag 
void mpu6050_set_low_power(
    device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    cntrl_data_ptr->low_power = SLEEP_MODE_ENABLE; 
}


// Clear low power flag 
void mpu6050_clear_low_power(
    device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    cntrl_data_ptr->low_power = SLEEP_MODE_DISABLE; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get the controller state 
MPU6050_STATE mpu6050_get_state(
    device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return MPU6050_FAULT_STATE; 

    return cntrl_data_ptr->state; 
}


// Get the controller fault code 
MPU6050_FAULT_CODE mpu6050_get_fault_code(
    device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return MPU6050_NULL_PTR_RETURN; 

    return cntrl_data_ptr->fault_code; 
}

//=======================================================================================
