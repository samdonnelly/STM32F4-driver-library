/**
 * @file ws2812_driver.h
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

#ifndef _WS2812_DRIVER_H_ 
#define _WS2812_DRIVER_H_ 

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "linked_list_driver.h" 
#include "timers.h" 
#include "dma_driver.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Device information 
#define WS2812_BITS_PER_LED 24         // Data bits per LED - 1 byte per colour 
#define WS2812_COLOUR_PER_LED 3        // Colours in each LED - Green, Red, Blue 
#define WS2812_LED_NUM 8               // Max number of LEDs available 
#define WS2812_MIN_SEND_DELAY 50       // Minimum delay between sends (microseconds) 

// PWM data 
#define WS2812_84MHZ_PWM_ARR 105       // PWM ARR to hit 1.25us period at 84MHz clock 
#define WS2812_0_CODE_DUTY 32          // Duty cycle to send 0 - max of WS2812_84MHZ_PWM_ARR 
#define WS2812_1_CODE_DUTY 64          // Duty cycle to send 1 - max of WS2812_84MHZ_PWM_ARR 

//=======================================================================================


//=======================================================================================
// Enums 

// LED index 
typedef enum {
    WS2812_LED_0, 
    WS2812_LED_1, 
    WS2812_LED_2, 
    WS2812_LED_3, 
    WS2812_LED_4, 
    WS2812_LED_5, 
    WS2812_LED_6, 
    WS2812_LED_7 
} ws2812_led_index_t; 


// LED base colours 
typedef enum {
    WS2812_GREEN, 
    WS2812_RED, 
    WS2812_BLUE
} ws2812_colours_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief WS2812 initialization 
 * 
 * @details 
 * 
 * @param device_num 
 * @param timer 
 * @param tim_channel 
 * @param gpio 
 * @param pin 
 * @param dma 
 * @param dma_stream 
 * @param dma_channel 
 */
void ws2812_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    DMA_TypeDef *dma, 
    DMA_Stream_TypeDef *dma_stream, 
    dma_channel_t dma_channel); 


/**
 * @brief WS2812 send data to device 
 * 
 * @details 
 * 
 * @param device_num : 
 */
void ws2812_send(
    device_number_t device_num); 


/**
 * @brief Colour set 
 * 
 * @details 
 * 
 * @param device_num 
 * @param colour_data 
 * @param led_num 
 */
void ws2812_colour_set(
    device_number_t device_num, 
    const uint8_t *colour_data, 
    uint8_t led_num); 

//=======================================================================================

#endif   // _WS2812_DRIVER_H_ 
