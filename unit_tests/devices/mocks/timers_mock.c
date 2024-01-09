/**
 * @file timer_mock.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Mock timers driver implementation - for unit testing 
 * 
 * @version 0.1
 * @date 2024-01-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "timers.h"

// Unit testing 
#include "timers_mock.h"

//=======================================================================================


//=======================================================================================
// User functions 

// Timer 1 setup 
void tim1_output_init(
    tim_channel_t channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    tim_dir_t dir, 
    uint16_t arr, 
    tim_ocm_t ocm, 
    tim_ocpe_t ocpe, 
    tim_arpe_t arpe, 
    tim_ccp_t ccp, 
    tim_up_dma_t ude)
{
    // 
}


// Timer 2-5 output mode setup 
void tim_2_to_5_output_init(
    TIM_TypeDef *timer, 
    tim_channel_t channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    tim_dir_t dir, 
    timer_us_prescalars_t prescalar, 
    uint16_t arr, 
    tim_ocm_t ocm, 
    tim_ocpe_t ocpe, 
    tim_arpe_t arpe, 
    tim_ccp_t ccp, 
    tim_up_dma_t ude)
{
    // 
}


// Timer 2-5 DMA setup 
void tim_2_5_dma_init(
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    uint8_t burst_len)
{
    // 
}


// Timer 9-11 counter mode setup 
void tim_9_to_11_counter_init(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar, 
    uint16_t arr, 
    tim_up_int_t uie)
{
    // 
}


// Enable a timer 
void tim_enable(
    TIM_TypeDef *timer)
{
    // 
}


// Disable a timer 
void tim_disable(
    TIM_TypeDef *timer)
{
    // 
}


// Microsecond delay function (blocking)
void tim_delay_us(
    TIM_TypeDef *timer, 
    uint16_t delay_us)
{
    // 
}


// Millisecond delay function (blocking)
void tim_delay_ms(
    TIM_TypeDef *timer, 
    uint16_t delay_ms)
{
    // 
}


// Elapsed time calculation (non-blocking delay) 
uint8_t tim_compare(
    TIM_TypeDef *timer, 
    uint32_t clk_freq, 
    uint32_t time_compare, 
    uint32_t *count_total, 
    uint32_t *count_compare, 
    uint8_t  *count_start)
{
    return 0; 
}


// Update interrupt flag read 
uint8_t tim_uif_read(
    TIM_TypeDef *timer)
{
    return 0; 
}


// Update interrupt flag clear 
void tim_uif_clear(
    TIM_TypeDef *timer)
{
    // 
}


// Update generation 
void tim_ug_set(
    TIM_TypeDef *timer)
{
    // 
}


// Timer counter set 
void tim_cnt_set(
    TIM_TypeDef *timer, 
    uint32_t counter)
{
    // 
}


// Timer counter read 
TIM_COUNTER tim_cnt_read(
    TIM_TypeDef *timer)
{
    return 0; 
}


// Capture/compare value 
void tim_ccr(
    TIM_TypeDef *timer, 
    uint32_t ccr, 
    tim_channel_t channel)
{
    // 
}


// Get the timer clock frequency 
uint32_t tim_get_pclk_freq(
    TIM_TypeDef *timer)
{
    return 0; 
}

//=======================================================================================


//=======================================================================================
// Mock functions 
//=======================================================================================
