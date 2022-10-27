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
#include "gpio_driver.h"

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
    ADC_PCLK2_2,   // PLCK2 divided by 2 
    ADC_PCLK2_4,   // PLCK2 divided by 4 
    ADC_PCLK2_6,   // PLCK2 divided by 6 
    ADC_PCLK2_8    // PLCK2 divided by 8 
} adc_prescalar_t; 


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
    ADC_RES_12,   // 12-bit resolution (15 ADCCLK cycles) 
    ADC_RES_10,   // 10-bit resolution (13 ADCCLK cycles) 
    ADC_RES_8,    // 8-bit resolution (11 ADCCLK cycles) 
    ADC_RES_6     // 6-bit resolution (9 ADCCLK cycles) 
} adc_res_t;


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


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_EOC_SEQ,   // EOC bit set after each sequence of regular conversions 
    ADC_EOC_EACH   // EOC bit set after each regular conversion 
} adc_eoc_config_t; 

//================================================================================


//================================================================================
// Initialization 

/**
 * @brief ADC port initialization 
 * 
 * @details 
 *          One time initiialization code for the ADC 
 * 
 * @param prescalar : 
 * @param resolution : 
 */
void adc_port_init(
    adc_prescalar_t prescalar, 
    adc_res_t resolution, 
    adc_eoc_config_t eoc); 


/**
 * @brief ADC pin initialization 
 * 
 * @details 
 *          Init code for each ADC channel 
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
    adc_smp_cycles_t smp); 

//================================================================================


//================================================================================
// Read 

/**
 * @brief ADC data register read 
 * 
 * @details 
 *           // TODO move prototype to source file - only should be called once 
 * 
 * @param adc 
 * @return uint16_t 
 */
uint16_t adc_dr(
    ADC_TypeDef *adc); 

//================================================================================


//================================================================================
// Status Registers 

/**
 * @brief Wait for start bit to set 
 * 
 * @details 
 * 
 */
void adc_start_wait(ADC_TypeDef *adc); 

/**
 * @brief ADC end of conversion 
 * 
 * @details 
 *          Set when the conversion of a regular group of channels is complete. 
 *          Cleared by reading the data register. 
 *          
 *          0 --> conversion (EOCS=0) or sequence of conversions (EOCS=1) not complete 
 *          1 --> conversion (EOCS=0) or sequence of conversions (EOCS=1) complete 
 * 
 */
void adc_eoc_wait(ADC_TypeDef *adc); 


/**
 * @brief Overrun bit status 
 * 
 * @details 
 * 
 * @param adc 
 * @return uint8_t 
 */
uint8_t adc_overrun_status(ADC_TypeDef *adc); 


/**
 * @brief Clear the overrun bit 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_overrun_clear(ADC_TypeDef *adc); 


/**
 * @brief Analog watchdog bit status 
 * 
 * @details 
 * 
 * @param adc 
 * @return uint8_t 
 */
uint8_t adc_wd_flag(ADC_TypeDef *adc); 

//================================================================================


//================================================================================
// Control Registers 

/**
 * @brief ADC prescalar 
 * 
 * @details 
 * 
 * @param adc 
 * @param prescalar 
 */
void adc_prescalar(
    ADC_Common_TypeDef *adc,
    adc_prescalar_t prescalar); 


/**
 * @brief Turn ADC on 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_on(
    ADC_TypeDef *adc); 


/**
 * @brief Turn ADC off 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_off(
    ADC_TypeDef *adc); 


/**
 * @brief Start an ADC conversion 
 * 
 * @details Sets by software and and cleared by hardware once the conversion starts. Note that 
 *          no conversion will be launched unless the ADC is on. 
 * 
 * @see adc_on 
 * 
 * @param adc 
 */
void adc_start(
    ADC_TypeDef *adc); 


/**
 * @brief Enable CONT mode 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_cont_enable(
    ADC_TypeDef *adc); 


/**
 * @brief Disable CONT mode 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_cont_disable(
    ADC_TypeDef *adc); 


/**
 * @brief 
 * 
 * @details 
 *          // TODO move prototype to source - only to be used once during init 
 * 
 * @param adc 
 * @param resolution 
 */
void adc_res(
    ADC_TypeDef *adc, 
    adc_res_t resolution); 


/**
 * @brief Analog watchdog enable on regular channels 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_wd_enable(ADC_TypeDef *adc); 


/**
 * @brief Analog watchdog disable on regular channels 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_wd_disable(
    ADC_TypeDef *adc); 


/**
 * @brief Enable SCAN mode 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_scan_enable(
    ADC_TypeDef *adc); 


/**
 * @brief Disable SCAN mode 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_scan_disable(
    ADC_TypeDef *adc); 


/**
 * @brief Enable watchdog on a single channel in SCAN mode 
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_wd_chan_scan_enable(
    ADC_TypeDef *adc); 


/**
 * @brief Disable watchdog on a single channel in SCAN mode
 * 
 * @details 
 * 
 * @param adc 
 */
void adc_wd_chan_scan_disable(
    ADC_TypeDef *adc); 


/**
 * @brief Analog watchdog channel select 
 * 
 * @details Select the input channel to be guarded by the analog watchdog. 
 * 
 * @param adc 
 * @param adc_channel 
 */
void adc_wd_chan_select(
    ADC_TypeDef *adc, 
    adc_channel_t adc_channel); 


/**
 * @brief End of Conversion (EOC) selection 
 * 
 * @details 
 * 
 * @see adc_eoc_wait 
 * 
 * @param adc 
 * @param eoc_select 
 */
void adc_eoc_select(
    ADC_TypeDef *adc, 
    adc_eoc_config_t eoc_select); 

//================================================================================


//================================================================================
// Sample Registers 

/**
 * @brief Set the sample time for the channel 
 * 
 * @details 
 * 
 * @param adc 
 * @param channel 
 * @param smp 
 */
void adc_smp(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_smp_cycles_t smp); 

//================================================================================


//================================================================================
// Watchdog Registers 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param adc 
 * @param hi_thresh 
 * @param lo_thresh 
 */
void adc_wd_thresh(
    ADC_TypeDef *adc, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh); 

//================================================================================


//================================================================================
// Sequence Registers 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param adc 
 * @param channel 
 * @param seq_num 
 */
void adc_seq(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
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

#endif  // _ANALOG_DRIVER_H_
