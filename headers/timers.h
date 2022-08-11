/**
 * @file timers.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief General purpose timers
 * 
 * @version 0.1
 * @date 2022-03-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TIMERS_H_
#define _TIMERS_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define RESET_COUNT 0        // Used to reset count registers 
#define PREFIX_SCALAR 1000   // Scalar between microseconds and milliseconds 

// Standard delays 
#define TIM9_1MS    1    // 1 ms delay 
#define TIM9_10MS   10   // 10 ms delay 
#define TIM9_100MS  100  // 100 ms delay 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Timer prescalars 
 * 
 * @details these prescalars are for generating a 1us count depending on the 
 *          frequency. The clock frequency of the timer gets divided by this prescalar 
 *          so that the clock counter only increments once per timer clock cycle. 
 *          See the reference manual for more information on calculating this value. 
 * 
 */
typedef enum {
    TIMERS_APB2_84MHZ_1US_PRESCALAR = 83
} timer_us_prescalars_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief TIM9 initialization 
 * 
 * @details Initializes TIM9 using a specified prescalar to generate a specific counting
 *          frequency based on the clock frequency. This timer is used for generating 
 *          blocking delays. 
 * 
 * @see timer_us_prescalars_t
 * @see tim9_delay_us
 * @see tim9_delay_ms
 * 
 * @param prescalar : value to divide the clock by to get a certain counting frequency
 * 
 */
void tim9_init(uint16_t prescalar);


/**
 * @brief TIM9 microsecond delay function (blocking)
 * 
 * @details TIM9 is initialized with a 1us frequency using tim9_init. This frequency 
 *          allows this function to increment the TIM9 count register once every 
 *          microsecond. The counter is incremented up to the limit specified by 
 *          delay_us. Note that this delay only works in blocking mode, otherwise 
 *          the dealy won't be accurate. 
 * 
 * @see tim9_init
 * 
 * @param delay_us : number of microseconds to delay 
 */
void tim9_delay_us(uint16_t delay_us);


/**
 * @brief TIM9 millisecond delay function (blocking)
 * 
 * @details This function allows for millisecond delays. It works by repeatedly calling
 *          tim9_delay_us in 1000 microsecond (1 millisecond) increments. Note that this 
 *          delay only works in blocking mode, otherwise the delay won't be accurate. 
 * 
 * @see tim9_delay_us
 * 
 * @param delay_ms : number of milliseconds to delay
 */
void tim9_delay_ms(uint16_t delay_ms);

//=======================================================================================


#endif   // _TIMERS_H_
