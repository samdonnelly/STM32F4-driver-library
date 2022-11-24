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

//================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//================================================================================


//================================================================================
// Macros 

#define RESET_COUNT 0        // Used to reset count registers 
#define PREFIX_SCALAR 1000   // Scalar between microseconds and milliseconds 

// Standard delays 
#define TIM9_1MS    1    // 1 ms delay 
#define TIM9_10MS   10   // 10 ms delay 
#define TIM9_100MS  100  // 100 ms delay 

#define TIM9_1US    1    // 1 us delay 
#define TIM9_2US    2    // 2 us delay 
#define TIM9_10US   10   // 10 us delay 
#define TIM9_100US  100  // 100 us delay 

//================================================================================


//================================================================================
// Enums 

/**
 * @brief Timer channel 
 */
typedef enum {
    TIM_CHANNEL_1, 
    TIM_CHANNEL_2, 
    TIM_CHANNEL_3, 
    TIM_CHANNEL_4 
} tim_channel_t; 


/**
 * @brief Timer prescalars 
 * 
 * @details these prescalars are for generating a 1us count depending on the 
 *          frequency. The clock frequency of the timer gets divided by this prescalar 
 *          so that the clock counter only increments once per timer clock cycle. 
 *          See the reference manual for more information on calculating this value. 
 */
typedef enum {
    TIMERS_APB2_84MHZ_1US_PRESCALAR = 83, 
    TIMERS_APB2_84MHZ_100US_PRESCALAR = 8399 
} timer_us_prescalars_t;


/**
 * @brief Counter configuration 
 */
typedef enum {
    TIM_CEN_DISABLE,    // Counter disable 
    TIM_CEN_ENABLE      // Counter enable 
} tim_cen_t; 


/**
 * @brief Auto-reload preload configuration 
 */
typedef enum {
    TIM_ARPE_DISABLE,    // Auto-reload preload disable 
    TIM_ARPE_ENABLE      // Auto-reload preload enable 
} tim_arpe_t; 


/**
 * @brief Update interrupt configuration 
 */
typedef enum {
    TIM_UP_INT_DISABLE,    // Update interrupt disable 
    TIM_UP_INT_ENABLE      // Update interrupt enable 
} tim_up_int_t; 


/**
 * @brief Output compare mode configuration 
 */
typedef enum {
    TIM_OCM_FROZEN,    // CNT and CCR comparision has no effect on the outputs 
    TIM_OCM_AM,        // Set output to active (high) on CNT and CCR match 
    TIM_OCM_IM,        // Set output to inactive (low) on CNT and CCR match 
    TIM_OCM_TOGGLE,    // Toggle the output on CNT and CCR match 
    TIM_OCM_FI,        // Force the output inactive (low) 
    TIM_OCM_FA,        // Force the output active (high) 
    TIM_OCM_PWM1,      // Output is active (high) when CNT<CCR 
    TIM_OCM_PWM2,      // Output is inactive (low) when CNT<CCR 
} tim_ocm_t; 


/**
 * @brief Output compare preload configuration 
 */
typedef enum {
    TIM_OCPE_DISABLE,    // Output compare preload disable 
    TIM_OCPE_ENABLE      // Output compare preload enable 
} tim_ocpe_t; 


/**
 * @brief Compare output polarity configuration 
 */
typedef enum {
    TIM_CCP_AH,    // Output compare active high 
    TIM_CCP_AL     // Output compare active low 
} tim_ccp_t; 


/**
 * @brief Compare output on/off configuration 
 */
typedef enum {
    TIM_CCE_OFF,    // Output compare not active 
    TIM_CCE_ON      // Output compare signal is output on the corresponding output pin 
} tim_cce_t; 

//================================================================================


//================================================================================
// Datatypes 

typedef uint32_t TIM_COUNTER; 

//================================================================================


//================================================================================
// Initialization 

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
 * @brief Timer 1 setup 
 * 
 * @details 
 * 
 * @param prescalar 
 */
void tim1_init(
    timer_us_prescalars_t prescalar); 


/**
 * @brief Timer 2-5 setup 
 * 
 * @details 
 * 
 * @param timer 
 * @param prescalar 
 */
void tim_2_to_5_init(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar); 


/**
 * @brief Timer 9-11 setup 
 * 
 * @details 
 * 
 * @see tim_channel_t
 * @see timer_us_prescalars_t
 * @see tim_ocm_t
 * @see tim_ocpe_t
 * @see tim_arpe_t
 * @see tim_ccp_t
 * @see tim_cce_t
 * @see tim_up_int_t
 * 
 * @param timer : 
 * @param channel : 
 * @param prescalar : 
 * @param arr : 
 * @param ocm : 
 * @param ocpe : 
 * @param arpe : 
 * @param ccp : 
 * @param cce : 
 * @param uie : 
 */
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
    tim_up_int_t uie); 

//================================================================================


//================================================================================
// Timer enable 

/**
 * @brief Enable a timer 
 * 
 * @details 
 * 
 * @param timer 
 */
void tim_enable(
    TIM_TypeDef *timer); 


/**
 * @brief Disable a timer 
 * 
 * @details 
 * 
 * @param timer 
 */
void tim_disable(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Delay functions 

// Note that all these functions produce blocking delays 

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
 * @brief Microsecond delay function (blocking)
 * 
 * @details 
 * 
 * @param timer 
 * @param delay_us 
 */
void tim_delay_us(
    TIM_TypeDef *timer, 
    uint16_t delay_us); 


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


/**
 * @brief Millisecond delay function (blocking)
 * 
 * @details 
 * 
 * @param timer 
 * @param delay_ms 
 */
void tim_delay_ms(
    TIM_TypeDef *timer, 
    uint16_t delay_ms); 

//================================================================================


//================================================================================
// Control register 

/**
 * @brief Counter enable 
 * 
 * @details 
 *          This function is available for all timers. 
 * 
 * @see tim_cen_t
 * 
 * @param timer : 
 * @param cen :  
 */
void tim_cen(
    TIM_TypeDef *timer, 
    tim_cen_t cen); 


/**
 * @brief Auto-reload preload enable 
 * 
 * @details 
 * 
 * @see tim_arpe_t
 * 
 * @param timer 
 * @param arpe 
 */
void tim_arpe(
    TIM_TypeDef *timer, 
    tim_arpe_t arpe); 

//================================================================================


//================================================================================
// Interrupt register 

/**
 * @brief Update interrupt 
 * 
 * @details 
 *          This function is available for all timers. 
 * 
 * @see tim_up_int_t
 * 
 * @param timer 
 * @param uie 
 */
void tim_uie(
    TIM_TypeDef *timer, 
    tim_up_int_t uie); 

//================================================================================


//================================================================================
// Status register 

/**
 * @brief Update interrupt flag clear 
 * 
 * @details 
 *          This function is available for all timers. 
 * 
 * @param timer 
 */
void tim_uif_clear(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Event generation register 

/**
 * @brief Update generation 
 * 
 * @details generates an update event via software. Automatically cleared by hardware. 
 *          This function is available for all timers. 
 * 
 * @param timer 
 */
void tim_ug_set(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Capture/compare mode registers 

/**
 * @brief Output compare mode selection 
 * 
 * @details 
 * 
 * @param timer 
 * @param ocm 
 * @param channel 
 */
void tim_ocm(
    TIM_TypeDef *timer, 
    tim_ocm_t ocm, 
    tim_channel_t channel); 


/**
 * @brief Output compare preload enable 
 * 
 * @details 
 * 
 * @param timer 
 * @param ocpe 
 * @param channel 
 */
void tim_ocpe(
    TIM_TypeDef *timer, 
    tim_ocpe_t ocpe, 
    tim_channel_t channel); 

//================================================================================


//================================================================================
// Capture/compare enable registers 

/**
 * @brief Compare output polarity 
 * 
 * @details 
 * 
 * @param timer 
 * @param ccp 
 * @param channel 
 */
void tim_ccp(
    TIM_TypeDef *timer, 
    tim_ccp_t ccp, 
    tim_channel_t channel); 


/**
 * @brief Compare output enable 
 * 
 * @details 
 * 
 * @param timer 
 * @param cce 
 * @param channel 
 */
void tim_cce(
    TIM_TypeDef *timer, 
    tim_cce_t cce, 
    tim_channel_t channel); 

//================================================================================


//================================================================================
// Counter register 

/**
 * @brief 
 * 
 * @details 
 *          This function is available for all timers. 
 *          
 *          Note that only TIM2 and TIM5 are 32-bit values. All other timers are 16 bits. 
 * 
 * @param timer 
 * @param counter 
 */
void tim_cnt_set(
    TIM_TypeDef *timer, 
    uint32_t counter); 


/**
 * @brief 
 * 
 * @details 
 *          This function is available for all timers. 
 *          
 *          Note that only TIM2 and TIM5 are 32-bit values. All other timers are 16 bits. 
 * 
 * @param timer 
 * @return TIM_COUNTER 
 */
TIM_COUNTER tim_cnt_read(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Prescaler register 

/**
 * @brief Set the counter clock prescalar 
 * 
 * @details 
 *          This function is available for all timers. 
 * 
 * @param timer 
 * @param prescalar 
 */
void tim_psc_set(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar); 

//================================================================================


//================================================================================
// Auto-reload register 

/**
 * @brief Auto-reload register (ARR) set 
 * 
 * @details 
 *          This function is available for all timers. 
 *          
 *          Note that only TIM2 and TIM5 are 32-bit values. All other timers are 16 bits. 
 * 
 * @param timer 
 * @param arr 
 */
void tim_arr_set(
    TIM_TypeDef *timer, 
    uint32_t arr); 

//================================================================================


//================================================================================
// Capture/compare register 

/**
 * @brief Capture/compare value 
 * 
 * @details 
 * 
 * @param timer 
 * @param ccr 
 * @param channel 
 */
void tim_ccr(
    TIM_TypeDef *timer, 
    uint32_t ccr, 
    tim_channel_t channel); 

//================================================================================

#endif   // _TIMERS_H_
