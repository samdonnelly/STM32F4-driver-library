/**
 * @file mpu6050_controller.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 IMU controller 
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
 * @details Resets various controller data record trackers and runs self-test and 
 *          calibration. This is the first state to be run on power-up and is called by the 
 *          reset state. After this state the controller goes directly to the run state if 
 *          it completed successfully or the fault state otherwise. 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_init_state(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief Read continuous state 
 * 
 * @details Reads device data at the rate specified in the controller init function. Checks 
 *          for faults after getting the new data. This state is called after the init 
 *          state and when exiting the low power state if no faults have occured. This state 
 *          will be left if the fault code is set, the reset flag is set or if the low power 
 *          flag is set. 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_read_cont_state(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief Read ready state 
 * 
 * @details Read from the device when the read bit is set. 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_read_ready_state(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 low power state 
 * 
 * @details Idle state that waits for the low power flag to clear. This state is triggered 
 *          by setting the low power flag. It is left by clearing the low power flag or when 
 *          the fault code or reset flag is set. This state is only entered and exited 
 *          from the low power transition state. The purpose of this state is to have the 
 *          device set in sleep mode to consume less power. 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_low_power_state(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 low power transition state 
 * 
 * @details Calls the low power configuration register to set the status of the low power 
 *          flag. This state is used an intermediate step between the low power state and 
 *          all other states. It is triggered and exited in the same way as the low power 
 *          state. 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_low_power_trans_state(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 fault state 
 * 
 * @details Idle state that waits for the reset flag to set. This state is triggered when 
 *          the fault code is set and is meant to inhibit the device from doing anything until 
 *          the fault has been addressed. This state is exited by setting the reset flag to 
 *          trigger the reset state. 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_fault_state(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief MPU6050 reset state 
 * 
 * @details Resets the controller and driver fault codes and ensures the device exits sleep 
 *          mode. This state is triggered by setting the reset flag. It exists after running 
 *          once and goes to the init state. 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_reset_state(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief Check for over temperature 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
void mpu6050_temp_check(mpu6050_cntrl_data_t *mpu6050_device); 

//=======================================================================================


//=======================================================================================
// Variables 

// Controller data record first pointer 
static mpu6050_cntrl_data_t *mpu6050_cntrl_data_ptr = NULL; 

// Function pointers to controller states 
static mpu6050_state_functions_t state_table[MPU6050_NUM_STATES] = 
{
    &mpu6050_init_state, 
    &mpu6050_read_cont_state, 
    &mpu6050_read_ready_state, 
    &mpu6050_low_power_trans_state, 
    &mpu6050_low_power_state, 
    &mpu6050_fault_state, 
    &mpu6050_reset_state 
}; 

// Function pointers to read functions 
static mpu6050_read_functions_t read_table[MPU6050_NUM_READS] = 
{
    &mpu6050_update 
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

    // Check for NULL pointers 
    if ((cntrl_data_ptr == NULL) || (timer == NULL)) return; 

    // Linked list tracking 
    cntrl_data_ptr->device_num = device_num; 

    // Peripherals 
    cntrl_data_ptr->timer = timer; 

    // Controller information 
    cntrl_data_ptr->state = MPU6050_INIT_STATE; 
    cntrl_data_ptr->fault_code = CLEAR; 
    cntrl_data_ptr->clk_freq = tim_get_pclk_freq(cntrl_data_ptr->timer); 
    cntrl_data_ptr->sample_period = sample_period; 
    cntrl_data_ptr->time_cnt_total = CLEAR; 
    cntrl_data_ptr->time_cnt = CLEAR; 
    cntrl_data_ptr->time_start = SET_BIT; 

    // Trackers 
    cntrl_data_ptr->low_power = CLEAR_BIT; 
    cntrl_data_ptr->reset = CLEAR_BIT; 
    cntrl_data_ptr->startup = SET_BIT; 
    cntrl_data_ptr->read = CLEAR_BIT; 
    cntrl_data_ptr->read_state = MPU6050_READ_CONT; 
    cntrl_data_ptr->smpl_type = MPU6050_READ_ALL; 
}


// MPU6050 controller 
void mpu6050_controller(device_number_t device_num)
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
            
            // Run the init state at least once 
            else if (cntrl_data_ptr->startup)
            {
                next_state = MPU6050_INIT_STATE; 
            }

            // Read mode set to ready 
            else if (cntrl_data_ptr->read_state)
            {
                next_state = MPU6050_READ_READY_STATE; 
            }

            // Read mode set to continuous 
            else 
            {
                next_state = MPU6050_READ_CONT_STATE; 
            }

            break;

        case MPU6050_READ_CONT_STATE: 
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

            // Read mode set to ready 
            else if (cntrl_data_ptr->read_state)
            {
                next_state = MPU6050_READ_READY_STATE; 
            }

            break;

        case MPU6050_READ_READY_STATE: 
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

            // Read mode set to continuous 
            else if (!cntrl_data_ptr->read_state)
            {
                next_state = MPU6050_READ_CONT_STATE; 
            }

            break; 

        case MPU6050_LOW_POWER_TRANS_STATE: 
            // Low power flag is set 
            if (cntrl_data_ptr->low_power)
            {
                next_state = MPU6050_LOW_POWER_STATE; 
            }

            // Read mode set to ready 
            else if (cntrl_data_ptr->read_state)
            {
                next_state = MPU6050_READ_READY_STATE; 
            }

            // Read mode set to continuous 
            else 
            {
                next_state = MPU6050_READ_CONT_STATE; 
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

        case MPU6050_FAULT_STATE: 
            // Reset flag set 
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
void mpu6050_init_state(mpu6050_cntrl_data_t *mpu6050_device)
{
    // Clear device trackers 
    mpu6050_device->startup = CLEAR_BIT; 
    mpu6050_device->reset = CLEAR_BIT; 

    // Run self-test 
    uint8_t st_result = CLEAR; 
    mpu6050_self_test(mpu6050_device->device_num, &st_result);

    // Provide time for device data to update so self-test data is not used for calibration 
    tim_delay_ms(mpu6050_device->timer, MPU6050_ST_DELAY); 
}


// Read continuous state 
void mpu6050_read_cont_state(mpu6050_cntrl_data_t *mpu6050_device)
{
    // Wait for a specified period of time before reading new data 
    if (tim_compare(mpu6050_device->timer, 
                    mpu6050_device->clk_freq, 
                    mpu6050_device->sample_period, 
                    &mpu6050_device->time_cnt_total, 
                    &mpu6050_device->time_cnt, 
                    &mpu6050_device->time_start))
    {
        read_table[mpu6050_device->smpl_type](mpu6050_device->device_num); 
        mpu6050_temp_check(mpu6050_device); 
    }
}


// Read ready state 
void mpu6050_read_ready_state(mpu6050_cntrl_data_t *mpu6050_device)
{
    // Read from the device on request 
    if (mpu6050_device->read)
    {
        read_table[mpu6050_device->smpl_type](mpu6050_device->device_num); 
        mpu6050_device->read = CLEAR_BIT; 
        mpu6050_temp_check(mpu6050_device); 
    }
}


// MPU6050 low power transition state 
void mpu6050_low_power_trans_state(mpu6050_cntrl_data_t *mpu6050_device)
{
    // Write the low power flag status to the power management register 
    mpu6050_low_pwr_config(
        mpu6050_device->device_num, 
        mpu6050_device->low_power); 

    // Reset the non-blocking delay 
    mpu6050_device->time_start = SET_BIT; 
}


// MPU6050 low power state 
void mpu6050_low_power_state(mpu6050_cntrl_data_t *mpu6050_device)
{
    // Waits for low power flag to be cleared 
}


// MPU6050 fault state 
void mpu6050_fault_state(mpu6050_cntrl_data_t *mpu6050_device)
{
    // Idle until the reset flag is set 
}


// MPU6050 reset state 
void mpu6050_reset_state(mpu6050_cntrl_data_t *mpu6050_device)
{
    // Reset the fault code in both the controller and driver 
    mpu6050_device->fault_code = CLEAR; 

    // Reset the low power flag and make sure to exit sleep mode 
    mpu6050_device->low_power = MPU6050_SLEEP_MODE_DISABLE; 
    mpu6050_low_pwr_config(
        mpu6050_device->device_num, 
        mpu6050_device->low_power); 

    // Reset the non-blocking delay 
    mpu6050_device->time_start = SET_BIT; 
}

//=======================================================================================


//=======================================================================================
// Data functions 

// Check for over temperature 
void mpu6050_temp_check(mpu6050_cntrl_data_t *mpu6050_device)
{
    if (mpu6050_get_temp_raw(mpu6050_device->device_num) > 
        (MPU6050_RAW_TEMP_MAX-MPU6050_RAW_TEMP_OFST))
    {
        mpu6050_device->fault_code |= (SET_BIT << SHIFT_8); 
    }
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set low power flag 
void mpu6050_set_low_power(device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    cntrl_data_ptr->low_power = MPU6050_SLEEP_MODE_ENABLE; 
}


// Clear low power flag 
void mpu6050_clear_low_power(device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    cntrl_data_ptr->low_power = MPU6050_SLEEP_MODE_DISABLE; 
}


// Set the data sample type 
void mpu6050_set_smpl_type(
    device_number_t device_num, 
    mpu6050_sample_type_t sample_type)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    // Update the sampling type 
    cntrl_data_ptr->smpl_type = sample_type; 
}


// Set the read state 
void mpu6050_set_read_state(
    device_number_t device_num, 
    mpu6050_read_state_t read_type)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    // Update the data record 
    cntrl_data_ptr->read_state = read_type; 
    cntrl_data_ptr->time_start = SET_BIT; 
}


// Set the read flag 
void mpu6050_set_read_flag(device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    // Set the read flag 
    cntrl_data_ptr->read = SET_BIT; 
}


// Set reset flag 
void mpu6050_set_reset_flag(device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) return; 

    cntrl_data_ptr->reset = SET_BIT; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get the controller state 
MPU6050_STATE mpu6050_get_state(device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) 
    {
        return MPU6050_FAULT_STATE; 
    }

    return cntrl_data_ptr->state; 
}


// Get the controller fault code 
MPU6050_FAULT_CODE mpu6050_get_fault_code(device_number_t device_num)
{
    // Get the controller data record 
    mpu6050_cntrl_data_t *cntrl_data_ptr = 
        (mpu6050_cntrl_data_t *)get_linked_list_entry(device_num, mpu6050_cntrl_data_ptr); 
    
    // Check that the data record is valid 
    if (cntrl_data_ptr == NULL) 
    {
        return 0; 
    }

    return cntrl_data_ptr->fault_code; 
}

//=======================================================================================
