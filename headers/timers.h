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
// Function Prototypes

/**
 * @brief 
 * 
 */
void timer6_init(void);

/**
 * @brief microsecond delay function 
 * 
 * @param delay_us 
 */
void tim6_delay_us(uint16_t delay_us);

/**
 * @brief millisecond delay function 
 * 
 * @param delay_ms 
 */
void tim6_delay_ms(uint16_t delay_ms);

//=======================================================================================


#endif   // _TIMERS_H_
