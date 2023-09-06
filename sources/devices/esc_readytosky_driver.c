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

    // Check and set speed bounds 
    driver_data_ptr->fwd_speed_lim = fwd_speed_lim; 
    driver_data_ptr->rev_speed_lim = rev_speed_lim; 

    // Set PWM to "neutral" 
}

//=======================================================================================


//=======================================================================================
// Write 

// ESC PWM command send 
void esc_readytosky_send(
    device_number_t device_num, 
    int8_t throttle_cmd)
{
    // 
}

//=======================================================================================
