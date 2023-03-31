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
#include "gpio_driver.h" 
#include "timers.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define WS2812_BITS_PER_LED 24            // Data bits per LED - 1 byte per colour 
#define WS2812_COUNTS_PER_BIT 4           // Clock counts per 1 byte of data transfer 
#define WS2812_COLOUR_PER_LED 3           // Colours in each LED - Green, Red, Blue 
#define WS2812_LED_NUM 8                  // Max number of LEDs available 

//=======================================================================================


//=======================================================================================
// Enums 
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
 * @param gpio 
 * @param pin 
 */
void ws2812_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin); 


/**
 * @brief WS2812 update data 
 * 
 * @details 
 * 
 * @param device_num : 
 */
void ws2812_update(
    device_number_t device_num); 


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
 * @brief WS2812 write 
 * 
 * @details Set, update and write data to the device 
 * 
 * @param device_num 
 * @param timer 
 * @param colour_data 
 * @param led_num 
 */
void ws2812_write(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    const uint8_t *colour_data, 
    uint8_t led_num); 


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
