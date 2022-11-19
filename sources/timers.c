/**
 * @file timers.c
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

//=======================================================================================
// Includes 

#include "timers.h"

//=======================================================================================

// TODO see if you can read the clock speed during tim9_init so you can adjust the counter 
// This will make maintain an accurate (blocking) delay 

// TODO replace timer specific functions with general purpose functions with pointers to 
//      timer ports 

//=======================================================================================
// Initialization 

// ============================================================
// Timer setup 
// 1. Enable the timer clock
// 2. Set the prescalar and the ARR
// 3. Enable the timer, and wait for the update flag to set
// ============================================================


// Timer 9 setup 
void tim9_init(uint16_t prescalar)
{
    // 1. Enable the timer clock - TIM9 in the RCC_APB2ENR register 
    RCC->APB2ENR |= (SET_BIT << SHIFT_16);

    // 2. Set the prescalar and the ARR
    TIM9->PSC = prescalar;  // Set clock prescalar based on APB2 frequency
    TIM9->ARR = 0xFFFF;     // Max ARR value 

    // 3. Enable the timer and wait for the update flag to set
    TIM9->CR1 |= (SET_BIT << SHIFT_0);
    while(!(TIM9->SR & (SET_BIT << SHIFT_0)));
}


// Timer 1 setup 
void tim1_init(
    timer_us_prescalars_t prescalar)
{
    // Enable the timer clock 

    // Set the clock prescalar 

    // Enable the timer and wait for confirmation 
}


// Timer 2-5 setup 
void tim_2_to_5_init(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar)
{
    // Get the timer port index 
    uint32_t index = (uint32_t)(&timer - TIM2_BASE) >> SHIFT_10; 

    // Enable the timer clock 
    RCC->APB1ENR |= (SET_BIT << index);

    // Set the clock prescalar 

    // Enable the timer and wait for confirmation 
}


// Timer 9-11 setup 
void tim_9_to_11_init(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar)
{
    // Enable the timer clock 

    // Set the clock prescalar 

    // Enable the timer and wait for confirmation 
}

//=======================================================================================


//=======================================================================================
// Delays

// Microsecond delay function (blocking)
void tim9_delay_us(uint16_t delay_us)
// void tim_delay_us(
//     TIM_TypeDef *timer, 
//     uint16_t delay_us)
{
    // Reset the counter 
    TIM9->CNT = RESET_COUNT; 
    // timer->CNT = RESET_COUNT; 

    // Count up to specified value in blocking mode to produce delay 
    while((TIM9->CNT) < delay_us); 
    // while((timer->CNT) < delay_us); 
}


// Millisecond delay function (blocking)
void tim9_delay_ms(uint16_t delay_ms)
// void tim9_delay_ms(
//     TIM_TypeDef *timer, 
//     uint16_t delay_ms)
{
    // Repeatedly call tim9_delay_us
    for (uint16_t i = 0; i < delay_ms; i++)
    {
        tim9_delay_us(PREFIX_SCALAR);
        // tim_delay_us(timer, PREFIX_SCALAR);
    }
}

//=======================================================================================