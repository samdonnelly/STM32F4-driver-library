/**
 * @file esc_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief ESC driver 
 * 
 * @version 0.1
 * @date 2023-09-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "esc_driver.h" 

//=======================================================================================


//=======================================================================================
// Global variables 

// Device data record 
typedef struct esc_driver_data_s
{
    // Linked list tracking 
    struct esc_driver_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripheral information 
    TIM_TypeDef *timer; 
    tim_channel_t tim_channel; 

    // Speed information 
    uint16_t fwd_speed_lim; 
    uint16_t rev_speed_lim; 
}
esc_driver_data_t; 


// Device driver first data pointer 
static esc_driver_data_t *esc_driver_data_ptr = NULL; 

//=======================================================================================


//=======================================================================================
// Initialization 

// ESC initialization 
void esc_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    timer_us_prescalars_t prescalar, 
    uint16_t arr, 
    uint16_t fwd_speed_lim, 
    uint16_t rev_speed_lim)
{
    // Create a data record for the device 
    esc_driver_data_t *driver_data_ptr = 
        (esc_driver_data_t *)create_linked_list_entry(
            device_num, 
            (void *)&esc_driver_data_ptr, 
            sizeof(esc_driver_data_t)); 

    // Initialize the PWM timer 
    tim_2_to_5_output_init(
        timer, 
        tim_channel, 
        gpio, 
        pin, 
        TIM_DIR_UP, 
        prescalar, 
        arr, 
        TIM_OCM_PWM1, 
        TIM_OCPE_ENABLE, 
        TIM_ARPE_ENABLE, 
        TIM_CCP_AH, 
        TIM_UP_DMA_DISABLE); 

    // Initialize data record 
    driver_data_ptr->timer = timer; 
    driver_data_ptr->tim_channel = tim_channel; 

    // Make sure the speed limits don't exceed PWM boundaries. 
    if (fwd_speed_lim > ESC_MAX_PWM)
    {
        driver_data_ptr->fwd_speed_lim = ESC_MAX_PWM; 
    }
    else if (fwd_speed_lim < ESC_NEUTRAL_PWM)
    {
        driver_data_ptr->fwd_speed_lim = ESC_NEUTRAL_PWM; 
    }
    else
    {
        driver_data_ptr->fwd_speed_lim = fwd_speed_lim; 
    }

    if (rev_speed_lim > ESC_NEUTRAL_PWM)
    {
        driver_data_ptr->rev_speed_lim = ESC_NEUTRAL_PWM; 
    }
    else if (rev_speed_lim < ESC_MIN_PWM)
    {
        driver_data_ptr->rev_speed_lim = ESC_MIN_PWM; 
    }
    else
    {
        driver_data_ptr->rev_speed_lim = rev_speed_lim; 
    }
}

//=======================================================================================


//=======================================================================================
// Write 

// ESC PWM command send 
void esc_send(
    device_number_t device_num, 
    int16_t throttle_cmd)
{
    // Get the device data record 
    esc_driver_data_t *driver_data_ptr = 
        (esc_driver_data_t *)get_linked_list_entry(device_num, esc_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL) 
    {
        return; 
    }

    // uint32_t pwm_cmd = ESC_NEUTRAL_TIME; 
    uint32_t pwm_cmd = ESC_NEUTRAL_PWM; 
    uint16_t throttle = CLEAR; 

    // Determine the PWM output 
    if (throttle_cmd > 0)
    {
        // Cap the throttle if the request is above 100% 
        if (throttle_cmd > ESC_MAX_THROTTLE)
        {
            throttle_cmd = ESC_MAX_THROTTLE; 
        }

        // Change the throttle command to an unsigned value 
        throttle = (uint16_t)throttle_cmd; 

        // Calculate the forward throttle pwm command 
        pwm_cmd = (uint32_t)(ESC_NEUTRAL_PWM + 
            ((driver_data_ptr->fwd_speed_lim - ESC_NEUTRAL_PWM)*throttle) / ESC_MAX_THROTTLE); 
    }
    else if (throttle_cmd < 0)
    {
        // Cap the throttle if the request is below -100% 
        if (throttle_cmd < -ESC_MAX_THROTTLE)
        {
            throttle_cmd = -ESC_MAX_THROTTLE; 
        }

        // Invert the direction of the throttle command - make the command a positive number 
        throttle_cmd += ESC_MAX_THROTTLE; 

        // Change the throttle command to an unsigned value 
        throttle = (uint16_t)throttle_cmd; 

        // Calculate the reverse throttle pwm command  
        pwm_cmd = (uint32_t)(driver_data_ptr->rev_speed_lim + 
            ((ESC_NEUTRAL_PWM - driver_data_ptr->rev_speed_lim)*throttle) / ESC_MAX_THROTTLE); 
    }

    // Write the PWM command 
    tim_ccr(driver_data_ptr->timer, pwm_cmd, driver_data_ptr->tim_channel); 
}


// ESC PWM command set 
void esc_pwm_set(
    device_number_t device_num, 
    uint16_t pwm_command)
{
    // Get the device data record 
    esc_driver_data_t *esc_data = 
        (esc_driver_data_t *)get_linked_list_entry(device_num, esc_driver_data_ptr); 

    // Check for valid data 
    if (esc_data == NULL) 
    {
        return; 
    }

    // Cap the command if it exceeds the set limits 
    if (pwm_command > esc_data->fwd_speed_lim)
    {
        pwm_command = esc_data->fwd_speed_lim; 
    }
    else if (pwm_command < esc_data->rev_speed_lim)
    {
        pwm_command = esc_data->rev_speed_lim; 
    }

    // Set the PWM command 
    tim_ccr(esc_data->timer, (uint32_t)pwm_command, esc_data->tim_channel); 
}

//=======================================================================================
