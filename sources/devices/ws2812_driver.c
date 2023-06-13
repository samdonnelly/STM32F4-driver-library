/**
 * @file ws2812_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 (Neopixel) driver 
 * 
 * @version 0.1
 * @date 2023-03-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "ws2812_driver.h"

//=======================================================================================


//=======================================================================================
// Global variables 

// Device data record 
typedef struct ws2812_driver_data_s
{
    // Linked list tracking 
    struct ws2812_driver_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripheral information 
    TIM_TypeDef *timer; 
    tim_channel_t tim_channel; 

    // Data 
    // uint32_t colour_data[WS2812_LED_NUM]; 
    // uint8_t pwm_duty[WS2812_LED_NUM * WS2812_BITS_PER_LED]; 
}
ws2812_driver_data_t; 


// Device driver first data pointer 
static ws2812_driver_data_t *ws2812_driver_data_ptr = NULL; 

//=======================================================================================


//=======================================================================================
// Initialization 

// WS2812 initialization 
void ws2812_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin)
{
    // Create a data record for the device 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)create_linked_list_entry(
            device_num, 
            (void *)&ws2812_driver_data_ptr, 
            sizeof(ws2812_driver_data_t)); 

    // Initialize the PWM timer 
    tim_2_to_5_output_init(
        timer, 
        tim_channel, 
        gpio, 
        pin, 
        TIM_DIR_UP, 
        WS2812_84MHZ_PWM_ARR, 
        TIM_OCM_PWM1, 
        TIM_OCPE_ENABLE, 
        TIM_ARPE_ENABLE, 
        TIM_CCP_AH, 
        TIM_UP_DMA_DISABLE); 

    // Initialize data record 
    driver_data_ptr->timer = timer; 
    driver_data_ptr->tim_channel = tim_channel; 
    // memset((void *)driver_data_ptr->colour_data, CLEAR, sizeof(driver_data_ptr->colour_data)); 
    // memset((void *)driver_data_ptr->pwm_duty, CLEAR, sizeof(driver_data_ptr->pwm_duty)); 

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Write 

// Write data to the device 
void ws2812_send(
    device_number_t device_num, 
    const uint32_t *colour_data)
{
    // Get the device data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL) return; 

    // Local variables 
    uint8_t led_index; 
    uint8_t colour_index; 
    uint8_t pwm_duty[WS2812_LED_NUM * WS2812_BITS_PER_LED]; 
    uint16_t pwm_duty_index = CLEAR; 
    uint16_t pwm_duty_size = WS2812_LED_NUM * WS2812_BITS_PER_LED; 

    //===================================================
    // Update the pwm data using the colour data 

    for (led_index = CLEAR; led_index < WS2812_LED_NUM; led_index++)
    {
        for (colour_index = WS2812_BITS_PER_LED; colour_index > 0; colour_index--)
        {
            // if ((driver_data_ptr->colour_data[led_index] >> (colour_index - 1) & 0x01))
            if ((*colour_data++ >> (colour_index - 1) & WS2812_PWM_BIT_MASK))
            {
                // driver_data_ptr->pwm_duty[pwm_duty_index++] = WS2812_1_CODE_DUTY; 
                pwm_duty[pwm_duty_index++] = WS2812_1_CODE_DUTY; 
            }
            else 
            {
                // driver_data_ptr->pwm_duty[pwm_duty_index++] = WS2812_0_CODE_DUTY; 
                pwm_duty[pwm_duty_index++] = WS2812_0_CODE_DUTY; 
            }
        }
    }

    //===================================================

    //===================================================
    // Update the duty cycle every counter update event with the pwm data 
    
    tim_enable(driver_data_ptr->timer); 

    for (pwm_duty_index = CLEAR; pwm_duty_index < pwm_duty_size; pwm_duty_index++)
    {
        // Wait for a counter update event 
        while (!tim_uif_read(driver_data_ptr->timer)); 

        // Update the duty cycle to send the right colour code 
        tim_ccr(
            driver_data_ptr->timer, 
            // (uint32_t)driver_data_ptr->pwm_duty[pwm_duty_index], 
            (uint32_t)pwm_duty[pwm_duty_index], 
            driver_data_ptr->tim_channel); 

        // Clear the update interrupt flag 
        tim_uif_clear(driver_data_ptr->timer); 
    }

    // Wait for the last data to be sent 
    while (!tim_uif_read(driver_data_ptr->timer)); 

    // Set the duty cycle to zero to prevent further colour changes 
    tim_ccr(driver_data_ptr->timer, CLEAR, driver_data_ptr->tim_channel); 

    tim_uif_clear(driver_data_ptr->timer); 
    tim_disable(driver_data_ptr->timer); 

    // NOTE: the device requires a 50us delay between write operations to distinguish 
    //       between data transfers. It is assumed that the time between send calls 
    //       will accumulate this time. If applications arise where this delay is not being 
    //       then an internal driver timer/delay will be added to ensure the delay. 
    
    //===================================================
}

//=======================================================================================


//=======================================================================================
// Setters 

// // Colour set 
// void ws2812_colour_set(
//     device_number_t device_num, 
//     const uint8_t *colour_data, 
//     uint8_t led_num)
// {
//     // Get the device data record 
//     ws2812_driver_data_t *driver_data_ptr = 
//         (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

//     // Check for valid data 
//     if (driver_data_ptr == NULL ||     // No data record 
//         colour_data == NULL ||         // No colour data 
//         led_num > WS2812_LED_NUM)      // LED doesn't exist 
//     {
//         return; 
//     }

//     // Set data 
//     driver_data_ptr->colour_data[led_num] = (*(colour_data + WS2812_GREEN) << SHIFT_16) + 
//                                             (*(colour_data + WS2812_RED)   << SHIFT_8) + 
//                                              *(colour_data + WS2812_BLUE); 
// }

//=======================================================================================
