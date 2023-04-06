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
// Notes 
// - You could set multiple LED colours with one function call by including the LED 
//   index in the data pointer array passed to the driver. 
//=======================================================================================


//=======================================================================================
// Global variables 

// Device data record 
typedef struct ws2812_driver_data_s
{
    // Linked list tracking 
    struct ws2812_driver_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripherals 
    TIM_TypeDef *timer; 
    DMA_Stream_TypeDef *dma_stream; 

    // Data 
    uint32_t colour_data[WS2812_LED_NUM]; 
    uint16_t pwm_duty[WS2812_LED_NUM * WS2812_BITS_PER_LED]; 
}
ws2812_driver_data_t; 


// Device driver first data pointer 
static ws2812_driver_data_t *ws2812_driver_data_ptr; 

//=======================================================================================


//=======================================================================================
// Initialization 

// WS2812 initialization 
void ws2812_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    DMA_TypeDef *dma, 
    DMA_Stream_TypeDef *dma_stream, 
    dma_channel_t dma_channel)
{
    // Create data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)create_linked_list_entry(
            device_num, 
            (void *)&ws2812_driver_data_ptr, 
            sizeof(ws2812_driver_data_t)); 

    // // Local variables 
    volatile uint32_t tim_channel_addr; 

    //===================================================
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
        TIM_UP_DMA_ENABLE); 

    // tim_2_5_dma_init(
    //     timer, 
    //     tim_channel, 
    //     0x00); 

    //===================================================

    //===================================================
    // Initialize the DMA 

    dma_stream_init(
        dma, 
        dma_stream, 
        dma_channel, 
        DMA_DIR_MP, 
        DMA_CM_DISABLE,
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT, 
        DMA_ADDR_FIXED, 
        DMA_DATA_SIZE_HALF, 
        DMA_DATA_SIZE_HALF); 

    // Identify the timer channel 
    switch (tim_channel)
    {
        case TIM_CHANNEL_1:
            tim_channel_addr = (uint32_t)(&timer->CCR1); 
            break;
        
        case TIM_CHANNEL_2:
            tim_channel_addr = (uint32_t)(&timer->CCR2); 
            break;

        case TIM_CHANNEL_3:
            tim_channel_addr = (uint32_t)(&timer->CCR3); 
            break;

        default: 
            tim_channel_addr = (uint32_t)(&timer->CCR4); 
            break;
    }

    dma_stream_config(
        dma_stream, 
        (uint32_t)tim_channel_addr, 
        (uint32_t)driver_data_ptr->pwm_duty, 
        (uint16_t)(WS2812_LED_NUM * WS2812_BITS_PER_LED)); 

    // dma_stream_config(
    //     dma_stream, 
    //     (uint32_t)(&timer->DMAR), 
    //     (uint32_t)driver_data_ptr->pwm_duty, 
    //     (uint16_t)(WS2812_LED_NUM * WS2812_BITS_PER_LED)); 
    
    //===================================================

    //===================================================
    // Initialize data record 

    driver_data_ptr->timer = timer; 
    driver_data_ptr->dma_stream = dma_stream; 
    memset((void *)driver_data_ptr->colour_data, CLEAR, sizeof(driver_data_ptr->colour_data)); 
    memset((void *)driver_data_ptr->pwm_duty, CLEAR, sizeof(driver_data_ptr->pwm_duty)); 

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Write 

// Write to device 
void ws2812_send(
    device_number_t device_num)
{
    // Get the device data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL) return; 

    //===================================================
    // Update the write data 

    // Local variables 
    uint8_t led_index; 
    uint8_t colour_index; 
    uint16_t pwm_duty_index = CLEAR; 

    // Update data 
    for (led_index = CLEAR; led_index < WS2812_LED_NUM; led_index++)
    {
        for (colour_index = WS2812_BITS_PER_LED; colour_index > 0; colour_index--)
        {
            if ((driver_data_ptr->colour_data[led_index] >> (colour_index - 1) & 0x01))
            {
                driver_data_ptr->pwm_duty[pwm_duty_index++] = WS2812_1_CODE_DUTY; 
            }
            else 
            {
                driver_data_ptr->pwm_duty[pwm_duty_index++] = WS2812_0_CODE_DUTY; 
            }
        }
    }

    //===================================================

    //===================================================
    // Send the write data 

    // Enable the PWM timer and DMA stream 
    tim_enable(driver_data_ptr->timer); 
    dma_stream_enable(driver_data_ptr->dma_stream); 

    // Wait for the data transfer to be done 
    while (dma_stream_status(driver_data_ptr->dma_stream)); 

    // Set the duty cycle to zero? 

    // Disable the PWM timer 
    tim_disable(driver_data_ptr->timer); 

    // Delay between sends 
    
    //===================================================
}

//=======================================================================================


//=======================================================================================
// Setters 

// Colour set 
void ws2812_colour_set(
    device_number_t device_num, 
    const uint8_t *colour_data, 
    uint8_t led_num)
{
    // Get the device data record 
    ws2812_driver_data_t *driver_data_ptr = 
        (ws2812_driver_data_t *)get_linked_list_entry(device_num, ws2812_driver_data_ptr); 

    // Check for valid data 
    if (driver_data_ptr == NULL ||     // No data record 
        colour_data == NULL ||         // No colour data 
        led_num > WS2812_LED_NUM)      // LED doesn't exist 
    {
        return; 
    }

    // Set data 
    driver_data_ptr->colour_data[led_num] = (*(colour_data + WS2812_GREEN) << SHIFT_16) + 
                                            (*(colour_data + WS2812_RED)   << SHIFT_8) + 
                                             *(colour_data + WS2812_BLUE); 
}

//=======================================================================================
