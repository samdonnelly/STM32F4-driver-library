/**
 * @file ws2812_driver_mock.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 driver mock 
 * 
 * @version 0.1
 * @date 2024-04-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "ws2812_driver_mock.h" 

//=======================================================================================


//=======================================================================================
// Variables 

static uint32_t led_colours_mock[WS2812_LED_NUM]; 

//=======================================================================================


//=======================================================================================
// Driver functions 

// WS2812 initialization 
void ws2812_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin)
{
    // 
}


// Write data to the device 
void ws2812_send(
    device_number_t device_num, 
    const uint32_t *colour_data)
{
    memcpy((void *)led_colours_mock, (void *)colour_data, sizeof(led_colours_mock)); 
}

//=======================================================================================


//=======================================================================================
// Mock functions 

// Initialize driver mock 
void ws2812_mock_init(void)
{
    memset((void *)led_colours_mock, ~CLEAR, sizeof(led_colours_mock)); 
}


// Get colour data 
void ws2812_mock_get_colour_data(uint32_t *led_colours)
{
    if (led_colours != NULL)
    {
        memcpy((void *)led_colours, (void *)led_colours_mock, sizeof(led_colours_mock)); 
    }
}

//=======================================================================================
