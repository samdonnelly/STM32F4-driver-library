/**
 * @file analog_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog (ADC and DAQ) data functions 
 * 
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _ANALOG_DRIVER_H_
#define _ANALOG_DRIVER_H_

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 

// Other drivers 

//=======================================================================================


//================================================================================
// Enums 

/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_CHANNEL_0,    // 
    ADC_CHANNEL_1,    // 
    ADC_CHANNEL_2,    // 
    ADC_CHANNEL_3,    // 
    ADC_CHANNEL_4,    // 
    ADC_CHANNEL_5,    // 
    ADC_CHANNEL_6,    // 
    ADC_CHANNEL_7,    // 
    ADC_CHANNEL_8,    // 
    ADC_CHANNEL_9,    // 
    ADC_CHANNEL_10,    // 
    ADC_CHANNEL_11,    // 
    ADC_CHANNEL_12,    // 
    ADC_CHANNEL_13,    // 
    ADC_CHANNEL_14,    // 
    ADC_CHANNEL_15     // 
} adc_channel_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_SMP_3,     // 3 cycles 
    ADC_SMP_15,    // 15 cycles
    ADC_SMP_28,    // 28 cycles
    ADC_SMP_56,    // 56 cycles
    ADC_SMP_84,    // 84 cycles
    ADC_SMP_112,   // 112 cycles
    ADC_SMP_144,   // 144 cycles
    ADC_SMP_480    // 480 cycles
} adc_smp_cycles_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_SEQ_1 = 1,    // 
    ADC_SEQ_2,        // 
    ADC_SEQ_3,        // 
    ADC_SEQ_4,        // 
    ADC_SEQ_5,        // 
    ADC_SEQ_6,        // 
    ADC_SEQ_7,        // 
    ADC_SEQ_8,        // 
    ADC_SEQ_9,        // 
    ADC_SEQ_10,       // 
    ADC_SEQ_11,       // 
    ADC_SEQ_12,       // 
    ADC_SEQ_13,       // 
    ADC_SEQ_14,       // 
    ADC_SEQ_15,       // 
    ADC_SEQ_16        // 
} adc_seq_num_t; 

//================================================================================


//================================================================================
// Function prototypes 

/**
 * @brief ADC port initialization 
 * 
 */
void adc_port_init(void); 


/**
 * @brief ADC pin initialization 
 * 
 * @details 
 * 
 * @param adc 
 * @param gpio 
 * @param adc_pin 
 * @param adc_channel 
 * @param adc_smp 
 * @param adc_seq_num : conversion number of channel in the regular sequence 
 */
void adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp, 
    adc_seq_num_t seq_num); 


/**
 * @brief Regular channel sequence length setter 
 * 
 * @details 
 * 
 * @param adc : 
 * @param seq_len : number of conversions in the regular channel conversion sequence 
 */
void adc_seq_len_set(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len); 

//================================================================================


//================================================================================
// Read 

/**
 * @brief ADC data register read 
 * 
 * @details 
 * 
 * @param adc 
 * @return uint16_t 
 */
uint16_t adc_dr(
    ADC_TypeDef *adc); 

//================================================================================

#endif  // _ANALOG_DRIVER_H_
