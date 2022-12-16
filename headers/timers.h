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

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "gpio_driver.h"

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

#define TIM_APB_CLK_FILTER 0x10000   // Filter to determine APB1 or APB2 frequency 

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
 * @details These are predefined prescalers for the timer counters. The name of each prescaler
 *          defines the time taken for the timer counter to increment at a given clock frequency. 
 *          For example, the prescaler TIM_84MHZ_1US_PSC is what you'd use if your timer clock 
 *          is running at 84MHz and you wanted the timer counter to increment ever 1us. When 
 *          choosing a prescaler it's important to know which timers run on which clock: <br> 
 *          - Timers 1 and 9-11 run on APB2 clock frequency. <br> 
 *          - Timers 2-5 run on APB1 clock frequency. <br> 
 */
typedef enum {
    TIM_84MHZ_1US_PSC = 83, 
    TIM_84MHZ_100US_PSC = 8399 
} timer_us_prescalars_t;


/**
 * @brief Counter configuration 
 */
typedef enum {
    TIM_CEN_DISABLE,    // Counter disable 
    TIM_CEN_ENABLE      // Counter enable 
} tim_cen_t; 


/**
 * @brief Counter direction configuration 
 */
typedef enum {
    TIM_DIR_UP,       // Upcounting 
    TIM_DIR_DOWN      // Downcounting 
} tim_dir_t; 


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
 * @brief Timer 1 initialization 
 * 
 * @details This function is currently not implemented. 
 * 
 * @param prescalar : timer clock prescaler 
 */
void tim1_init(
    timer_us_prescalars_t prescalar); 


/**
 * @brief Timer 2-5 output mode initialization 
 * 
 * @details Initializes timer 2, 3, 4 or 5 to a desired output mode. Note that if a timer port 
 *          that is not timer 2-5 is passed as an argument then no initialization will happen. 
 *          This function must be called once for each timer that is to be initialized. <br> 
 *          
 *          Timers set up using this function can be used to control a pin output based on 
 *          a timers sequence. An example is PWM output. 
 * 
 * @see tim_channel_t
 * @see pin_selector_t
 * @see tim_dir_t
 * @see tim_ocm_t
 * @see tim_ocpe_t
 * @see tim_arpe_t
 * @see tim_ccp_t
 * 
 * @param timer : pointer to timer port (2-5) being initialized 
 * @param channel : timer port channel to use 
 * @param gpio : pointer to GPIO port of chosen output pin 
 * @param pin : output pin chosen 
 * @param dir : counter direction 
 * @param arr : auto-reload register value 
 * @param ocm : output compare mode configuration 
 * @param ocpe : output compare preload configuration 
 * @param arpe : auto-reload preload register configuration 
 * @param ccp : output compare polarity configuration 
 */
void tim_2_to_5_output_init(
    TIM_TypeDef *timer, 
    tim_channel_t channel, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin, 
    tim_dir_t dir, 
    uint16_t arr, 
    tim_ocm_t ocm, 
    tim_ocpe_t ocpe, 
    tim_arpe_t arpe, 
    tim_ccp_t ccp); 


/**
 * @brief Timer 9-11 counter mode setup 
 * 
 * @details Initializes timer 9, 10 or 11 in upcounting mode. Note that if a timer port 
 *          that is not timer 9-11 is passed as an argument then no initialization will happen. 
 *          This function must be called once for each timer that is to be initialized. <br> 
 *          
 *          Timers set up using this function can be used to create precise timing sequences. 
 *          Examples include blocking delays or periodic interrupts. 
 * 
 * @see timer_us_prescalars_t
 * @see tim_up_int_t
 * 
 * @param timer : pointer to timer port (9-11) being initialized 
 * @param prescalar : counter clock prescaler (clock divider) 
 * @param arr : auto-reload register value 
 * @param uie : update interrupt configuration 
 */
void tim_9_to_11_counter_init(
    TIM_TypeDef *timer, 
    timer_us_prescalars_t prescalar, 
    uint16_t arr, 
    tim_up_int_t uie); 

//================================================================================


//================================================================================
// Timer enable 

/**
 * @brief Enable a timer 
 * 
 * @details This is used to start a specified timer. Timer initialization must be called 
 *          before calling this function but this function must be called in order for the 
 *          timer to start functioning. 
 * 
 * @param timer : pointer to timer port to enable 
 */
void tim_enable(
    TIM_TypeDef *timer); 


/**
 * @brief Disable a timer 
 * 
 * @details This is used to stop a specified timer. 
 * 
 * @param timer : pointer to timer port to disable 
 */
void tim_disable(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Delay functions 

/**
 * @brief Microsecond delay function (blocking)
 * 
 * @details This function produces a blocking delay of a specified number of microseconds. 
 *          Note that in order for this function to work properly, a timer must be set up to 
 *          count every millisecond using the correct prescaler in timer_us_prescalars_t. 
 *          See any of the timer counter initialization functions for details. 
 * 
 * @see timer_us_prescalars_t
 * 
 * @param timer : pointer to timer used to produce the delay 
 * @param delay_us : number of microseconds to delay 
 */
void tim_delay_us(
    TIM_TypeDef *timer, 
    uint16_t delay_us); 


/**
 * @brief Millisecond delay function (blocking)
 * 
 * @details This function produces a blocking delay of a specified number of milliseconds. 
 *          This function repeatedly uses the microsecond delay function to get to the 
 *          desired number of milliseconds. See tim_delay_us for more details. 
 * 
 * @see tim_delay_us
 * 
 * @param timer : pointer to timer used to produce the delay 
 * @param delay_ms : number of milliseconds to delay 
 */
void tim_delay_ms(
    TIM_TypeDef *timer, 
    uint16_t delay_ms); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param timer 
 * @param time_compare : minum delay time to compare against (in microseconds) 
 * @param total_count 
 * @param count_compare 
 * @return uint8_t 
 */
uint8_t tim_time_compare(
    TIM_TypeDef *timer, 
    uint32_t time_compare, 
    uint32_t *count_total, 
    uint32_t *count_compare); 

//================================================================================


//================================================================================
// Status register 

/**
 * @brief Update interrupt flag clear 
 * 
 * @details Clears the interrupt flag that is set on an update event. This function is required 
 *          in the interrupt handler functions in order to return from the ISR. <br> 
 *          
 *          This function is available for all timers. 
 * 
 * @param timer : pointer to timer that triggered the update event 
 */
void tim_uif_clear(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Event generation register 

/**
 * @brief Update generation 
 * 
 * @details Allows for generating an update event manually through software. After calling this 
 *          function the bit is automatically cleared by hardware. <br> 
 *          
 *          This function is available for all timers. 
 * 
 * @param timer : pointer to timer of which to trigger an update event 
 */
void tim_ug_set(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Counter register 

/**
 * @brief Counter read 
 * 
 * @details Reads the counter value of a specified timer. The counter counts are a rate 
 *          define by the prescaler and counts up to the auto-reload register value. <br> 
 *          
 *          This function is available for all timers. <br> 
 *          
 *          Note that only TIM2 and TIM5 are 32-bit values. All other timers are 16 bits. 
 * 
 * @param timer : pointer to timer port of which to read its counter 
 * @return TIM_COUNTER : current value of the counter 
 */
TIM_COUNTER tim_cnt_read(
    TIM_TypeDef *timer); 

//================================================================================


//================================================================================
// Capture/compare register 

/**
 * @brief Capture/compare value 
 * 
 * @details This functions sets the capture/compare register value for a specified timer. 
 *          The value in this register is used as a reference for both capture and compare 
 *          operations. For example, in PWM mode, this register value is compared to the 
 *          counter value to know when to set the output pin to high or low so this register 
 *          dictates the PWM duty cycle. 
 * 
 * @see tim_channel_t
 * 
 * @param timer : pointer to timer port of which to change the CCR value 
 * @param ccr : CCR value to set 
 * @param channel : timer channel 
 */
void tim_ccr(
    TIM_TypeDef *timer, 
    uint32_t ccr, 
    tim_channel_t channel); 

//================================================================================

#endif   // _TIMERS_H_
