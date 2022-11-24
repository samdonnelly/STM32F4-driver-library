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


//================================================================================
// Notes 
// - This driver does not yet support input modes 
//================================================================================


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

// TODO replace timer specific functions with general purpose functions with pointers to 
//      timer ports 

//================================================================================
// Initialization 

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
}


// Timer 9-11 setup 
void tim_9_to_11_init(
    TIM_TypeDef *timer, 
    tim_channel_t channel, 
    timer_us_prescalars_t prescalar, 
    uint16_t arr, 
    tim_ocm_t ocm, 
    tim_ocpe_t ocpe, 
    tim_arpe_t arpe, 
    tim_ccp_t ccp, 
    tim_cce_t cce, 
    tim_up_int_t uie)
{
    // Get the timer port index 
    uint32_t index = (uint32_t)timer - (uint32_t)TIM9_BASE >> SHIFT_10; 

    // Enable the timer clock 
    RCC->APB2ENR |= (SET_BIT << (index + SHIFT_16));

    // Set the clock prescalar 
    tim_psc_set(timer, prescalar); 

    // Set the auto-reload register (ARR) 
    tim_arr_set(timer, arr); 

    // Set the capture/compare mode 
    tim_ocm(timer, ocm, channel); 

    // Configure the preload register 
    tim_ocpe(timer, ocpe, channel); 

    // Configure the auto-reload preload register 
    tim_arpe(timer, arpe); 

    // Set the output capture polarity 
    tim_ccp(timer, ccp, channel); 

    // Enable the OCx output 
    tim_cce(timer, cce, channel); 

    // Configure the update interrupt 
    tim_uie(timer, uie); 

    // Reset the counter 
    tim_cnt_set(timer, RESET_COUNT); 

    // Set the UG bit to initialize all registers 
    tim_ug_set(timer); 
}

//================================================================================


//================================================================================
// Timer enable 

// Enable a timer 
void tim_enable(
    TIM_TypeDef *timer)
{
    tim_cen(timer, TIM_CEN_ENABLE); 
    // while(!(timer->SR & (SET_BIT << SHIFT_0)));
}


// Disable a timer 
void tim_disable(
    TIM_TypeDef *timer)
{
    tim_cen(timer, TIM_CEN_DISABLE); 
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
    tim_cnt_set(timer, RESET_COUNT); 

    // Count up to the specified value in blocking mode to produce a delay 
    while(tim_cnt_read(timer) < delay_us); 
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
    for (uint16_t i = 0; i < delay_ms; i++)
        tim_delay_us(timer, PREFIX_SCALAR); 
}

//================================================================================


//================================================================================
// Control register 

// Counter enable 
void tim_cen(
    TIM_TypeDef *timer, 
    tim_cen_t cen)
{
    timer->CR1 &= ~(SET_BIT << SHIFT_0); 
    timer->CR1 |= (cen << SHIFT_0); 
}

// Auto-reload preload enable 
void tim_arpe(
    TIM_TypeDef *timer, 
    tim_arpe_t arpe)
{
    timer->CR1 &= ~(SET_BIT << SHIFT_7); 
    timer->CR1 |= (arpe << SHIFT_7); 
}

//================================================================================


//================================================================================
// Interrupt register 

// Update interrupt 
void tim_uie(
    TIM_TypeDef *timer, 
    tim_up_int_t uie)
{
    timer->DIER &= ~(SET_BIT >> SHIFT_0); 
    timer->DIER |= (uie >> SHIFT_0); 
}

//================================================================================


//================================================================================
// Status register 

// Update interrupt flag clear 
void tim_uif_clear(
    TIM_TypeDef *timer)
{
    timer->SR &= ~(SET_BIT << SHIFT_0); 
}

//================================================================================


//================================================================================
// Event generation register 

// Update generation 
void tim_ug_set(
    TIM_TypeDef *timer)
{
    timer->EGR |= (SET_BIT << SHIFT_0); 
}

//================================================================================


//================================================================================
// Capture/compare mode registers 

// Output compare mode selection 
void tim_ocm(
    TIM_TypeDef *timer, 
    tim_ocm_t ocm, 
    tim_channel_t channel)
{
    if (channel < TIM_CHANNEL_3)
    {
        timer->CCMR1 &= ~(SET_7 << (SHIFT_4 + SHIFT_8*channel)); 
        timer->CCMR1 |= (ocm << (SHIFT_4 + SHIFT_8*channel)); 
    }
    else 
    {
        timer->CCMR2 &= ~(SET_7 << (SHIFT_4 + SHIFT_8*(channel-TIM_CHANNEL_3))); 
        timer->CCMR2 |= (ocm << (SHIFT_4 + SHIFT_8*(channel-TIM_CHANNEL_3))); 
    }
}


// Output compare preload enable 
void tim_ocpe(
    TIM_TypeDef *timer, 
    tim_ocpe_t ocpe, 
    tim_channel_t channel)
{
    if (channel < TIM_CHANNEL_3)
    {
        timer->CCMR1 &= ~(SET_BIT << (SHIFT_3 + SHIFT_8*channel)); 
        timer->CCMR1 |= (ocpe << (SHIFT_3 + SHIFT_8*channel)); 
    }
    else 
    {
        timer->CCMR2 &= ~(SET_BIT << (SHIFT_3 + SHIFT_8*(channel-TIM_CHANNEL_3))); 
        timer->CCMR2 |= (ocpe << (SHIFT_3 + SHIFT_8*(channel-TIM_CHANNEL_3))); 
    }
}

//================================================================================


//================================================================================
// Capture/compare enable registers 

// Compare output polarity 
void tim_ccp(
    TIM_TypeDef *timer, 
    tim_ccp_t ccp, 
    tim_channel_t channel)
{
    timer->CCER &= ~(SET_BIT << (SHIFT_1 + SHIFT_4*channel)); 
    timer->CCER |= (ccp << (SHIFT_1 + SHIFT_4*channel)); 
}


// Compare output enable 
void tim_cce(
    TIM_TypeDef *timer, 
    tim_cce_t cce, 
    tim_channel_t channel)
{
    timer->CCER &= ~(SET_BIT << (SHIFT_0 + SHIFT_4*channel)); 
    timer->CCER |= (cce << (SHIFT_0 + SHIFT_4*channel)); 
}

//================================================================================


//================================================================================
// Counter register 

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

//================================================================================


//================================================================================
// Prescaler register 

// Set the counter clock prescalar 
void tim_psc_set(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar)
{
    timer->PSC = prescalar; 
}

//================================================================================


//================================================================================
// Auto-reload register 

// Auto-reload register (ARR) set 
void tim_arr_set(
    TIM_TypeDef *timer, 
    uint32_t arr)
{
    timer->ARR = arr; 
}

//================================================================================


//================================================================================
// Capture/compare register 

// Capture/compare value 
void tim_ccr(
    TIM_TypeDef *timer, 
    uint32_t ccr, 
    tim_channel_t channel)
{
    switch (channel)
    {
        case TIM_CHANNEL_1:
            timer->CCR1 = ccr; 
            break;
        
        case TIM_CHANNEL_2:
            timer->CCR2 = ccr; 
            break;

        case TIM_CHANNEL_3:
            timer->CCR3 = ccr; 
            break;

        case TIM_CHANNEL_4:
            timer->CCR4 = ccr; 
            break;
        
        default:
            break;
    }
}

//================================================================================
