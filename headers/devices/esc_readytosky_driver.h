/**
 * @file esc_readytosky_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Readytosky bidirectional 40A 2-6S ESC driver header 
 * 
 * @version 0.1
 * @date 2023-09-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ESC_READYTOSKY_DRIVER_H_ 
#define ESC_READYTOSKY_DRIVER_H_ 

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

// Output 
#define ESC_NEUTRAL_TIME 1500     // PWM pulse duration for "neutral" gear (us) 
#define ESC_REV_BASE_TIME 1480    // PWM pulse duration for start of reverse thrust (us) 
#define ESC_REV_MAX_TIME 1000     // PWM pulse duration for max reverse thrust (us) 
#define ESC_FWD_BASE_TIME 1520    // PWM pulse duration for start of forward thrust (us) 
#define ESC_FWD_MAX_TIME 2000     // PWM pulse duration for max forward thrust (us) 

//=======================================================================================


//=======================================================================================
// Enums 
//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief ESC initialization 
 * 
 * @details 
 *          NOTE: The prescalar and auto reload register must combine to make a counter 
 *                reload period of 3ms or 3000us. 
 * 
 * @param device_num : number used to fetch the device data record 
 * @param timer : timer port used for the PWM output 
 * @param tim_channel : timer channel used for the pwm output 
 * @param gpio : GPIO port of PWM pin 
 * @param pin : number of PWM pin 
 * @param prescalar 
 * @param arr 
 * @param fwd_speed_lim 
 * @param rev_speed_lim 
 */
void esc_readytosky_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    timer_us_prescalars_t prescalar, 
    uint16_t arr, 
    uint16_t fwd_speed_lim, 
    uint16_t rev_speed_lim); 

//=======================================================================================


//=======================================================================================
// Write 

/**
 * @brief ESC PWM command send 
 * 
 * @details 
 * 
 * @param device_num : number used to fetch the device data record 
 * @param throttle_cmd : -100% to 100% throttle command 
 */
void esc_readytosky_send(
    device_number_t device_num, 
    int8_t throttle_cmd); 

//=======================================================================================

#endif   // ESC_READYTOSKY_DRIVER_H_ 
