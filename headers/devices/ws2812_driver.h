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
#define WS2812_PWM_BIT_MASK 0x01       // Masks colour data to get bit 0 for the PWM setpoint 
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

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief WS2812 initialization 
 * 
 * @details Creates a data record for the device so multiple devices can easily use this 
 *          driver. Records the timer info in the data record and sets up the PWM output 
 *          for a specified timer and pin. Must be called during setup for each device. 
 *          
 *          NOTE: This driver/function is equipped for TIM2-TIM5. Other timers cannot be 
 *                 used at this time. 
 * 
 * @param device_num : number used to fetch the device data record 
 * @param timer : timer port used for the PWM output 
 * @param tim_channel : timer channel used for the pwm output 
 * @param gpio : GPIO port of PWM pin 
 * @param pin : number of PWM pin 
 */
void ws2812_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin); 

//=======================================================================================


//=======================================================================================
// Write 

/**
 * @brief WS2812 send data to device 
 * 
 * @details Updates the colour of the LEDs on the device. This function takes the color 
 *          data as input and generates a PWM waveform that gets sent to the device to
 *          change the LED colour. 
 *          
 *          The WS2812 device reads the length of time the data bus is high to distinguish 
 *          between 0 and 1 colour data bits (see the datasheet for more info). To achieve 
 *          this timing, a variable PWM waveform is sent. These devices can also be strung 
 *          together one after the other and they will pass info sequentially from one to 
 *          the next making them addressable, however you must send colour data to all 
 *          LEDs leading up to the desired LED. This particular device contains 8 LEDs 
 *          and colour data passed to this function must contain 8 sets of data. 
 *          
 *          Colour data for a single LED consists of 24-bits - 8 bits for each of green, 
 *          red and blue: 
 *          - Bits 0-7: Blue 
 *          - Bits 8-15: Red 
 *          - Bits 16-23: Green 
 *          Colour data should be passed to this function in the above format to achieve 
 *          the desired colour. The highest bit gets sent first (i.e. bit 23 overall or 
 *          green bit 7). 
 * 
 * @param device_num : number used to fetch the device data record 
 * @param colour_data : pointer to colour data array that gets sent to the device 
 */
void ws2812_send(
    device_number_t device_num, 
    const uint32_t *colour_data); 

//=======================================================================================

#endif   // _WS2812_DRIVER_H_ 
