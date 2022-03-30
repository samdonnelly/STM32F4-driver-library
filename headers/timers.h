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

#define RESET_COUNT 0
#define PREFIX_SCALAR 1000

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Timer prescalars 
 * 
 * @details these prescalars are for generating a 1us count depending on the 
 *          frequency. 
 * 
 */
typedef enum {
    TIMERS_APB2_84MHZ_1US_PRESCALAR = 83
} timer_us_prescalars_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief 
 * 
 */
void tim9_init(uint16_t prescalar);

/**
 * @brief microsecond delay function 
 * 
 * @param delay_us 
 */
void tim9_delay_us(uint16_t delay_us);

/**
 * @brief millisecond delay function 
 * 
 * @param delay_ms 
 */
void tim9_delay_ms(uint16_t delay_ms);

//=======================================================================================


#endif   // _TIMERS_H_
