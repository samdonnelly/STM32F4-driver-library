/**
 * @file esc_readytosky_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Readytosky bidirectional 40A 2-6S ESC driver 
 * 
 * @version 0.1
 * @date 2023-09-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "esc_readytosky_driver.h" 

//=======================================================================================


//=======================================================================================
// Global variables 

// Device data record 
typedef struct esc_readytosky_driver_data_s
{
    // Linked list tracking 
    struct esc_readytosky_driver_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripheral information 
    TIM_TypeDef *timer; 
    tim_channel_t tim_channel; 

    // Speed information 
    uint16_t fwd_speed_lim; 
    uint16_t rev_speed_lim; 
}
esc_readytosky_driver_data_t; 


// Device driver first data pointer 
static esc_readytosky_driver_data_t *esc_driver_data_ptr = NULL; 

//=======================================================================================


//=======================================================================================
// Initialization 

// ESC initialization 
void esc_readytosky_init(
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
    esc_readytosky_driver_data_t *driver_data_ptr = 
        (esc_readytosky_driver_data_t *)create_linked_list_entry(
            device_num, 
            (void *)&esc_driver_data_ptr, 
            sizeof(esc_readytosky_driver_data_t)); 

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

    // Make sure the speed limits are within the limitations of the device. 

    if (fwd_speed_lim > ESC_FWD_MAX_TIME)
    {
        driver_data_ptr->fwd_speed_lim = ESC_FWD_MAX_TIME; 
    }
    else if (fwd_speed_lim < ESC_FWD_START_TIME)
    {
        driver_data_ptr->fwd_speed_lim = ESC_FWD_START_TIME; 
    }
    else 
    {
        driver_data_ptr->fwd_speed_lim = fwd_speed_lim; 
    }

    if (rev_speed_lim < ESC_REV_MAX_TIME)
    {
        driver_data_ptr->rev_speed_lim = ESC_REV_MAX_TIME; 
    }
    else if (rev_speed_lim > ESC_REV_START_TIME)
    {
        driver_data_ptr->rev_speed_lim = ESC_REV_START_TIME; 
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
void esc_readytosky_send(
    device_number_t device_num, 
    int16_t throttle_cmd)
{
    // Get the device data record 
    esc_readytosky_driver_data_t *driver_data_ptr = 
        (esc_readytosky_driver_data_t *)get_linked_list_entry(device_num, esc_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL) return; 

    // Local variables 
    uint32_t pwm_cmd = ESC_NEUTRAL_TIME; 
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
        pwm_cmd = (uint32_t)(ESC_FWD_START_TIME + 
            ((driver_data_ptr->fwd_speed_lim - ESC_FWD_START_TIME)*throttle) / ESC_MAX_THROTTLE); 
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
            ((ESC_REV_START_TIME - driver_data_ptr->rev_speed_lim)*throttle) / ESC_MAX_THROTTLE); 
    }

    // Write the PWM command 
    tim_ccr(driver_data_ptr->timer, pwm_cmd, driver_data_ptr->tim_channel); 
}

//=======================================================================================
