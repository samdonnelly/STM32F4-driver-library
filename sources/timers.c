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

//================================================================================
// Includes 

#include "timers.h"

//================================================================================

// TODO see if you can read the clock speed during tim9_init so you can adjust the counter 
// This will make maintain an accurate (blocking) delay 

// TODO replace timer specific functions with general purpose functions with pointers to 
//      timer ports 

// TODO add a PWM init function that just calls the other init functions with predefined values. 

//================================================================================
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

    // 2. Set the prescalar and the auto-reload register (ARR) 
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
    RCC->APB2ENR |= (SET_BIT << SHIFT_0);

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
    // Get the timer port index 
    uint32_t index = (uint32_t)(&timer - TIM9_BASE) >> SHIFT_10; 

    // Enable the timer clock 
    RCC->APB2ENR |= (SET_BIT << (index + SHIFT_16));

    // Set the clock prescalar 

    // Enable the timer and wait for confirmation 
}


//==================================================
// PWM setup 
// 1. Set the ARR register to set the frequency 
// 2. Set the CCRx register to set the duty cycle 
// 3. Set the PWM mode (OCxM bits) in the CCMRx register 
// 4. Enable the preload register by setting the OCxPE bit in the CCMRx register 
// 5. Enable the auto-reload preload register by setting the ARPE bit in the CR1 register 
// 6. Set the UG bit in the EGR register to initialize all the registers 
// 7. Set the output capture polarity using the CCxP bit in the CCER register 
// 8. The OCx output is enabled by the CCxE bit in the CCER register 
// 9. Start the counter (when it's ready to be enabled) 
//==================================================


// PWM setup 
void pwm_init(void)
{
    // 
}

//================================================================================


//================================================================================
// Delay functions 

// Microsecond delay function (blocking)
void tim9_delay_us(uint16_t delay_us)
{
    // Reset the counter 
    TIM9->CNT = RESET_COUNT; 

    // Count up to specified value in blocking mode to produce delay 
    while((TIM9->CNT) < delay_us); 
}


// Microsecond delay function (blocking)
void tim_delay_us(
    TIM_TypeDef *timer, 
    uint16_t delay_us)
{
    // Reset the counter 
    timer->CNT = RESET_COUNT; 

    // Count up to specified value in blocking mode to produce delay 
    while((timer->CNT) < delay_us); 
}


// Millisecond delay function (blocking)
void tim9_delay_ms(uint16_t delay_ms)
{
    // Repeatedly call tim9_delay_us
    for (uint16_t i = 0; i < delay_ms; i++)
    {
        tim9_delay_us(PREFIX_SCALAR);
    }
}


// Millisecond delay function (blocking)
void tim_delay_ms(
    TIM_TypeDef *timer, 
    uint16_t delay_ms)
{
    // Repeatedly call the microsecond delay function 
    for (uint16_t i = 0; i < delay_ms; i++)
    {
        tim_delay_us(timer, PREFIX_SCALAR); 
    }
}

//================================================================================


//================================================================================
// Register functions 

// Counter enable 
void tim_cen(
    TIM_TypeDef *timer, 
    tim_cen_t cen)
{
    timer->CR1 &= ~(SET_BIT << SHIFT_0); 
    timer->CR1 |= (cen << SHIFT_0); 
}


// Update interrupt 
void tim_uie(
    TIM_TypeDef *timer, 
    tim_up_int_t uie)
{
    timer->DIER &= ~(SET_BIT >> SHIFT_0); 
    timer->DIER |= (uie >> SHIFT_0); 
}


// Update interrupt flag clear 
void tim_uif_clear(
    TIM_TypeDef *timer)
{
    timer->SR &= ~(SET_BIT << SHIFT_0); 
}


// Update generation 
void tim_ug_set(
    TIM_TypeDef *timer)
{
    timer->EGR |= (SET_BIT << SHIFT_0); 
}


// Timer counter set 
void tim_cnt_set(
    TIM_TypeDef *timer, 
    uint32_t counter)
{
    timer->CNT = counter; 
}


// Timer counter read 
TIM_COUNTER tim_cnt_read(
    TIM_TypeDef *timer)
{
    return timer->CNT; 
}


// Set the counter clock prescalar 
void tim_psc_set(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar)
{
    timer->PSC = prescalar; 
}


// Auto-reload register (ARR) set 
void tim_arr_set(
    TIM_TypeDef *timer, 
    uint32_t arr)
{
    timer->ARR = arr; 
}

//================================================================================
