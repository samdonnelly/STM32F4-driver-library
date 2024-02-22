/**
 * @file timers_driver.c
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

#include "timers_driver.h"

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief Counter enable 
 * 
 * @details Enable the counter for the timer. Enabling the counter enables the timer itself. 
 *          The counter is controlled using the timer enable and disable functions. 
 *          
 *          This function is available for all timers. 
 * 
 * @see tim_cen_t
 * @see tim_enable
 * @see tim_disable
 * 
 * @param timer : pointer to timer to configure 
 * @param cen : counter configuration 
 */
void tim_cen(
    TIM_TypeDef *timer, 
    tim_cen_t cen); 


/**
 * @brief Counter direction configuration 
 * 
 * @details Controls the direction that the counter counts. The counter can either be 
 *          upcounting or downcounting. 
 * 
 * @see tim_dir_t
 * 
 * @param timer : pointer to timer to configure 
 * @param dir : counter direction configuration 
 */
void tim_dir(
    TIM_TypeDef *timer, 
    tim_dir_t dir); 


/**
 * @brief Auto-reload preload enable 
 * 
 * @details Determines whether the auto-reload register (ARR) is buffered or not. 
 * 
 * @see tim_arpe_t
 * 
 * @param timer : pointer to timer to configure 
 * @param arpe : auto-reload preload configuration 
 */
void tim_arpe(
    TIM_TypeDef *timer, 
    tim_arpe_t arpe); 


/**
 * @brief Update DMA request 
 * 
 * @details Configures the update DMA request bit. This bit, if set, will generate a DMA request 
 *          on a counter update event. Note that the DMA must also be configured for this to do 
 *          anything. 
 * 
 * @param timer : pointer to timer to configure 
 * @param ude : update DMA request configuration 
 */
void tim_ude(
    TIM_TypeDef *timer, 
    tim_up_dma_t ude); 


/**
 * @brief Update interrupt 
 * 
 * @details Configures the update event interrupt. If enabled, an interrupt will be generated 
 *          for the specified timer when an update event occurs. An update event occurs when 
 *          a counter overflow occurs (counter reaches ARR value) or the UG bit is set 
 *          manually. 
 *          
 *          This function is available for all timers. 
 * 
 * @see tim_up_int_t
 * 
 * @param timer : pointer to timer to configure 
 * @param uie : update event interrupt configuration 
 */
void tim_uie(
    TIM_TypeDef *timer, 
    tim_up_int_t uie); 


/**
 * @brief Set the counter clock prescalar 
 * 
 * @details Configures the prescaler for the timer selected. The prescaler effectively 
 *          divides the clock used for the timer which determines the rate at which the 
 *          counter for the clock increments. 
 *          
 *          This function is available for all timers. 
 * 
 * @see timer_us_prescalars_t
 * 
 * @param timer : pointer to timer to configure 
 * @param prescalar : clock prescalar that determines the counter rate 
 */
void tim_psc_set(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar); 


/**
 * @brief Auto-reload register (ARR) set 
 * 
 * @details Configures the auto-relaod value of the timer counter. In upcounting mode the 
 *          counter will count to this reload value before resetting back to zero. In 
 *          downcounting mode the counter will reset to the reload value. A counter reset 
 *          is considered a counter overflow. The auto-reload value determines the signal 
 *          frequency in PWM mode. 
 *          
 *          This function is available for all timers. 
 *          
 *          Note that only TIM2 and TIM5 are 32-bit values. All other timers are 16 bits. 
 * 
 * @param timer : pointer to timer to configure 
 * @param arr : auto-relaod register value 
 */
void tim_arr_set(
    TIM_TypeDef *timer, 
    uint32_t arr); 


/**
 * @brief Output compare mode selection 
 * 
 * @details Selects the output mode of the chosen timer. The channel for the timer must be 
 *          specified. See the tim_ocm_t enum for different output mode. 
 * 
 * @see tim_ocm_t
 * @see tim_channel_t
 * 
 * @param timer : pointer to timer to configure 
 * @param ocm : output compare mode configuration 
 * @param channel : timer channel 
 */
void tim_ocm(
    TIM_TypeDef *timer, 
    tim_ocm_t ocm, 
    tim_channel_t channel); 


/**
 * @brief Output compare preload enable 
 * 
 * @details Configures the output compare preload. If the preload register is disabled then 
 *          the CCR register can be written at any time and the new value is taken into 
 *          account immediately. If enabled then CCR read/write operations use a preload 
 *          register to hold the value which is then loaded on an update event. 
 * 
 * @see tim_ocpe_t
 * @see tim_channel_t
 * 
 * @param timer : pointer to timer to configure 
 * @param ocpe : output compare preload configuration 
 * @param channel : timer channel 
 */
void tim_ocpe(
    TIM_TypeDef *timer, 
    tim_ocpe_t ocpe, 
    tim_channel_t channel); 


/**
 * @brief Compare output polarity 
 * 
 * @details Configures the output polarity. In output compare mode this function sets the 
 *          polarity as either active high or active low. 
 * 
 * @see tim_ccp_t
 * @see tim_channel_t
 * 
 * @param timer : pointer to timer to configure 
 * @param ccp : compare output polarity 
 * @param channel : timer channel 
 */
void tim_ccp(
    TIM_TypeDef *timer, 
    tim_ccp_t ccp, 
    tim_channel_t channel); 


/**
 * @brief Compare output enable 
 * 
 * @details Allows for enabling and disabling of the output or input. If output mode is 
 *          enabled, compare output disabled means the OC is not active and compare output 
 *          enabled means the OC signal is output to the corresponding output pin. 
 * 
 * @see tim_cce_t
 * @see tim_channel_t
 * 
 * @param timer : pointer to timer to configure 
 * @param cce : compare output enable 
 * @param channel : timer channel 
 */
void tim_cce(
    TIM_TypeDef *timer, 
    tim_cce_t cce, 
    tim_channel_t channel); 

//=======================================================================================


//=======================================================================================
// Initialization 

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
    // Enable the timer clock 
    RCC->APB2ENR |= (SET_BIT << SHIFT_0);

    // Configure the output pin 
    gpio_pin_init(gpio, pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio, pin, SET_BIT); 

    // Set the counter direction 
    tim_dir(TIM1, dir); 

    // Set the capture/compare mode 
    tim_ocm(TIM1, ocm, channel); 

    // Set the auto-reload register (ARR) 
    tim_arr_set(TIM1, arr); 

    // Configure the output compare preload enable register 
    tim_ocpe(TIM1, ocpe, channel); 

    // Configure the auto-reload preload register 
    tim_arpe(TIM1, arpe); 

    // Set the output capture polarity 
    tim_ccp(TIM1, ccp, channel); 

    // Enable the OCx output 
    tim_cce(TIM1, TIM_CCE_ON, channel); 

    // Set the MOE (main output enable) bit 
    TIM1->BDTR |= (SET_BIT << SHIFT_15); 

    // Configure the update DMA request 
    tim_ude(TIM1, ude); 

    // Reset the counter 
    tim_cnt_set(TIM1, RESET_COUNT); 

    // Set the UG bit to initialize all registers 
    tim_ug_set(TIM1); 
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
    // Get the timer port index 
    uint32_t index = ((uint32_t)timer - (uint32_t)TIM2_BASE) >> SHIFT_10; 

    // Return if the timer port is out of the acceptable range (TIM2-TIM5) 
    if (index > SET_3) return; 

    // Enable the timer clock 
    RCC->APB1ENR |= (SET_BIT << index);

    // Configure the output pin 
    gpio_pin_init(gpio, pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    if (timer == TIM2) gpio_afr(gpio, pin, SET_BIT); 
    else gpio_afr(gpio, pin, SET_2); 

    // Set the counter direction 
    tim_dir(timer, dir); 

    // Set the capture/compare mode 
    tim_ocm(timer, ocm, channel); 

    // Set the clock prescalar 
    tim_psc_set(timer, prescalar); 

    // Set the auto-reload register (ARR) 
    tim_arr_set(timer, arr); 

    // Configure the output compare preload enable register 
    tim_ocpe(timer, ocpe, channel); 

    // Configure the auto-reload preload register 
    tim_arpe(timer, arpe); 

    // Set the output capture polarity 
    tim_ccp(timer, ccp, channel); 

    // Enable the OCx output 
    tim_cce(timer, TIM_CCE_ON, channel); 

    // Configure the update DMA request 
    tim_ude(timer, ude); 

    // Reset the counter 
    tim_cnt_set(timer, RESET_COUNT); 

    // Set the UG bit to initialize all registers 
    tim_ug_set(timer); 
}


// Timer 2-5 DMA setup 
void tim_2_5_dma_init(
    TIM_TypeDef *timer, 
    tim_channel_t tim_channel, 
    uint8_t burst_len)
{
    // Configure the DMA control register 
    timer->DCR = CLEAR; 
    timer->DCR = (burst_len << SHIFT_8) | (TIM_CCR1_INDEX + tim_channel); 
}


// Timer 9-11 counter mode setup 
void tim_9_to_11_counter_init(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar, 
    uint16_t arr, 
    tim_up_int_t uie)
{
    // Get the timer port index 
    uint32_t index = ((uint32_t)timer - (uint32_t)TIM9_BASE) >> SHIFT_10; 

    // Return if the timer port is out of the acceptable range (TIM9-TIM11) 
    if (index > SET_2) return; 

    // Enable the timer clock 
    RCC->APB2ENR |= (SET_BIT << (index + SHIFT_16));

    // Set the clock prescalar 
    tim_psc_set(timer, prescalar); 

    // Set the auto-reload register (ARR) 
    tim_arr_set(timer, arr); 

    // Configure the update interrupt 
    tim_uie(timer, uie); 

    // Reset the counter 
    tim_cnt_set(timer, RESET_COUNT); 

    // Get the system clock so non-blocking delays can be used - only called in counter inits 
    get_sys_clk_init(); 
}

//=======================================================================================


//=======================================================================================
// Timer enable 

// Enable a timer 
void tim_enable(
    TIM_TypeDef *timer)
{
    tim_cen(timer, TIM_CEN_ENABLE); 
    
    if (!(timer->DIER & (SET_BIT << SHIFT_0)))
    {
        while(!(timer->SR & (SET_BIT << SHIFT_0)));
    }
}


// Disable a timer 
void tim_disable(
    TIM_TypeDef *timer)
{
    tim_cen(timer, TIM_CEN_DISABLE); 
}

//=======================================================================================


//=======================================================================================
// Delay functions 

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
void tim_delay_ms(
    TIM_TypeDef *timer, 
    uint16_t delay_ms)
{
    for (uint16_t i = 0; i < delay_ms; i++)
    {
        tim_delay_us(timer, PREFIX_SCALAR); 
    }
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
    // Local variables 
    uint32_t time_elapsed; 
    uint32_t count_tracker; 

    // Only update the clock counter if no delay has happened yet 
    if (*count_start)
    {
        *count_compare = tim_cnt_read(timer); 
        *count_total = CLEAR; 
        *count_start = CLEAR; 
        return FALSE; 
    }

    // Read the updated clock counter 
    count_tracker = tim_cnt_read(timer); 

    // Updated the total count 
    if (count_tracker >= *count_compare)
    {
        *count_total += (count_tracker - *count_compare); 
    }
    else 
    {
        *count_total += (count_tracker + (timer->ARR - *count_compare)); 
    }

    // Calculate the total elapsed time (in microseconds) 
    time_elapsed = *count_total * (timer->PSC + 1) / clk_freq; 

    // Update the counter reference - this is new so test it 
    *count_compare = count_tracker; 

    // Compare the times 
    if (time_elapsed >= time_compare)
    {
        // Enough time has elapsed so restart the timer 
        *count_total = CLEAR; 
        return TRUE; 
    }
    else 
    {
        // Elapsed time is less than desired delay time so updated the counter 
        return FALSE; 
    }
}

//=======================================================================================


//=======================================================================================
// Control register 

// Counter enable 
void tim_cen(
    TIM_TypeDef *timer, 
    tim_cen_t cen)
{
    timer->CR1 &= ~(SET_BIT << SHIFT_0); 
    timer->CR1 |= (cen << SHIFT_0); 
}


// Direction configuration 
void tim_dir(
    TIM_TypeDef *timer, 
    tim_dir_t dir)
{
    timer->CR1 &= ~(SET_BIT << SHIFT_4); 
    timer->CR1 |= (dir << SHIFT_4); 
}


// Auto-reload preload enable 
void tim_arpe(
    TIM_TypeDef *timer, 
    tim_arpe_t arpe)
{
    timer->CR1 &= ~(SET_BIT << SHIFT_7); 
    timer->CR1 |= (arpe << SHIFT_7); 
}

//=======================================================================================


//=======================================================================================
// DMA / Interrupt Enable register 

// Update DMA request 
void tim_ude(
    TIM_TypeDef *timer, 
    tim_up_dma_t ude)
{
    timer->DIER &= ~(SET_BIT << SHIFT_8); 
    timer->DIER |= (ude << SHIFT_8); 
}


// Update interrupt 
void tim_uie(
    TIM_TypeDef *timer, 
    tim_up_int_t uie)
{
    timer->DIER &= ~(SET_BIT << SHIFT_0); 
    timer->DIER |= (uie << SHIFT_0); 
}

//=======================================================================================


//=======================================================================================
// Status register 

// Update interrupt flag read 
uint8_t tim_uif_read(
    TIM_TypeDef *timer)
{
    return (uint8_t)(timer->SR & (SET_BIT << SHIFT_0)); 
}


// Update interrupt flag clear 
void tim_uif_clear(
    TIM_TypeDef *timer)
{
    timer->SR &= ~(SET_BIT << SHIFT_0); 
}

//=======================================================================================


//=======================================================================================
// Event generation register 

// Update generation 
void tim_ug_set(
    TIM_TypeDef *timer)
{
    timer->EGR |= (SET_BIT << SHIFT_0); 
}

//=======================================================================================


//=======================================================================================
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

//=======================================================================================


//=======================================================================================
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

//=======================================================================================


//=======================================================================================
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

//=======================================================================================


//=======================================================================================
// Prescaler register 

// Set the counter clock prescalar 
void tim_psc_set(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar)
{
    timer->PSC = prescalar; 
}

//=======================================================================================


//=======================================================================================
// Auto-reload register 

// Auto-reload register (ARR) set 
void tim_arr_set(
    TIM_TypeDef *timer, 
    uint32_t arr)
{
    timer->ARR = arr; 
}

//=======================================================================================


//=======================================================================================
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

//=======================================================================================


//=======================================================================================
// Getters 

// Get the timer clock frequency 
uint32_t tim_get_pclk_freq(
    TIM_TypeDef *timer)
{
    if (((uint32_t)timer & TIM_APB_CLK_FILTER) >> SHIFT_4) 
    {
        // APB2 
        return (rcc_get_pclk2_frq() / DIVIDE_1000) / DIVIDE_1000; 
    }
    else 
    {
        // APB1 
        return (rcc_get_pclk1_frq() / DIVIDE_1000) / DIVIDE_1000; 
    }
}

//=======================================================================================
