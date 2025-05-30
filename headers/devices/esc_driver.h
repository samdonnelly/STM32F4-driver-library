/**
 * @file esc_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief ESC driver interface 
 * 
 * @version 0.1
 * @date 2023-09-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ESC_DRIVER_H_ 
#define ESC_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "linked_list_driver.h" 
#include "timers_driver.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Output 
#define ESC_MIN_PWM 1000       // Min PWM pulse duration (us) 
#define ESC_NEUTRAL_PWM 1500   // Neutral (zero motor output) PWM pulse duration (us) 
#define ESC_MAX_PWM 2000       // Max PWM pulse duration (us) 

// Calculation 
#define ESC_MAX_THROTTLE 100      // Max throttle command 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief ESC initialization 
 * 
 * @details Configures the specified pin for PWM output and calculates the forward and 
 *          reverse throttle limits that are used in the send function. These ESCs require 
 *          a "neutral" command to be sent before they will start to spin. This can be done 
 *          by passing a zero as the throttle command in the send function. While in neutral 
 *          the ESC will not actuate the motors. This function does NOT put the ESCs into 
 *          neutral as to prevent unexpected actuation if the PWM command is not a neutral 
 *          command to start. It is the responsibility of the application code to ensure 
 *          the ESCs enter neutral when the application is ready. 
 *          
 *          The forward and reverse speed limit arguments specify the PWM range the driver 
 *          will operate in. The ESC operates on a 50Hz or 20ms period and the full range 
 *          is from roughly 1ms/1000us to 2ms/2000us of PWM on-time. Roughly in the middle 
 *          is the Neutral command (1520us), below this is reverse and above is forward. 
 *          The speed limit arguments can be within this range. Outside of this range and 
 *          the limit will default to the edge of the range. These arguments are in 
 *          microseconds. The exact operating values of the ESC are listed as macros above. 
 *          
 *          NOTE: The prescalar and auto reload register must combine to make a counter 
 *                reload period of 20ms or 20000us. 
 * 
 * @param device_num : number used to fetch the device data record 
 * @param timer : timer port used for the PWM output 
 * @param tim_channel : timer channel used for the pwm output 
 * @param gpio : GPIO port of PWM pin 
 * @param pin : number of PWM pin 
 * @param prescalar : timer clock divider to set counter rate 
 * @param arr : counter auto-reload register value 
 * @param fwd_speed_lim : max PWM output (max forward speed) to cap the ESC at 
 * @param rev_speed_lim : min PWM output (max reverse speed) to cap the ESC at 
 */
void esc_init(
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
 * @details Send a throttle command to the ESC. The throttle command is anywhere between 
 *          -100 and 100 where: 
 *           - -100 --> -100% reverse throttle 
 *           - 0 -----> 0% throttle - neutral 
 *           - 100 ---> 100% forward throttle 
 *          
 *          Throttle commands outside of this range will be capped at +-/100%. 
 *          
 *          The limits of the ESC are defined in the macros above. In the init function 
 *          there are arguments specifying the speed limit for a particular ESC. If these 
 *          limits are less than full reverse/forward throttle then that becomes the max
 *          speed for a particular direction and throttle commands that exceed this limit 
 *          will be capped. Note that the init function specifys speed limits in the form 
 *          of PWM time whereas this function takes a 0 to +/-100% throttle command so 
 *          the mapping between these two metrics should be accounted for. 
 *          
 *          This function must be used to put the ESCs in neutral (0 command) on start-up 
 *          before they will start to spin with any other throttle command. 
 * 
 * @param device_num : number used to fetch the device data record 
 * @param throttle_cmd : -100% to 100% throttle command 
 */
void esc_send(
    device_number_t device_num, 
    int16_t throttle_cmd); 


/**
 * @brief ESC PWM command set 
 * 
 * @details Update the PWM command provided to the ESC. PWM commands follow standard 
 *          ESC PWM values of: 
 *          - 1000 --> Full speed one direction 
 *          - 1500 --> neutral/off 
 *          - 2000 --> Full speed the other direction 
 *          
 *          The provided PWM command will be capped if it exceeds the limits set in the 
 *          init function. 
 * 
 * @param device_num : number used to fetch the device data record 
 * @param pwm_command : PWM command to supply to the ESC 
 */
void esc_pwm_set(
    device_number_t device_num, 
    uint16_t pwm_command); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // ESC_DRIVER_H_ 
