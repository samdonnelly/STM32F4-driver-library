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
#include "timers.h"

//=======================================================================================


//================================================================================
// Macros 

#define ADC_STAB_TIME 10   // Stabilization time (ms) for ADC after being turned on 

//================================================================================


//================================================================================
// Enums 

/**
 * @brief ADC clock prescalar 
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
 * @brief ADC channel number 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_CHANNEL_0, 
    ADC_CHANNEL_1, 
    ADC_CHANNEL_2, 
    ADC_CHANNEL_3, 
    ADC_CHANNEL_4, 
    ADC_CHANNEL_5, 
    ADC_CHANNEL_6, 
    ADC_CHANNEL_7, 
    ADC_CHANNEL_8, 
    ADC_CHANNEL_9, 
    ADC_CHANNEL_10, 
    ADC_CHANNEL_11, 
    ADC_CHANNEL_12, 
    ADC_CHANNEL_13, 
    ADC_CHANNEL_14, 
    ADC_CHANNEL_15  
} adc_channel_t; 


/**
 * @brief Sampling cycles selection 
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
 * @brief Resolution selection 
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
 * @brief Sequence number 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_SEQ_1 = 1, 
    ADC_SEQ_2,  
    ADC_SEQ_3,  
    ADC_SEQ_4,  
    ADC_SEQ_5,  
    ADC_SEQ_6,  
    ADC_SEQ_7,  
    ADC_SEQ_8,  
    ADC_SEQ_9,  
    ADC_SEQ_10, 
    ADC_SEQ_11, 
    ADC_SEQ_12, 
    ADC_SEQ_13, 
    ADC_SEQ_14, 
    ADC_SEQ_15, 
    ADC_SEQ_16 
} adc_seq_num_t; 


/**
 * @brief EOC selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_EOC_SEQ,   // EOC bit set after each sequence of regular conversions 
    ADC_EOC_EACH   // EOC bit set after each regular conversion 
} adc_eoc_config_t; 


/**
 * @brief EOC interrupt selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_EOC_INT_DISABLE,   // Disable EOC interrupt 
    ADC_EOC_INT_ENABLE     // Enable EOC interrupt 
} adc_eoc_int_t; 


/**
 * @brief Scan mode selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_SCAN_DISABLE,   // Disable scan mode 
    ADC_SCAN_ENABLE     // Enable scan mode 
} adc_scan_t; 


/**
 * @brief Continuous mode selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_CONT_DISABLE,   // Disable continuous mode (single conversion mode) 
    ADC_CONT_ENABLE     // Enable continuous mode 
} adc_cont_t; 


/**
 * @brief DMA mode selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_DMA_DISABLE,   // Disable DMA for ADC 
    ADC_DMA_ENABLE     // Enable DMA for ADC 
} adc_dma_t; 


/**
 * @brief DMA Disable selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_DDS_DISABLE,    // No new DMA request is issued after the last transfer 
    ADC_DDS_ENABLE      // DMA requets are issued as long as data are coverted 
} adc_dds_t; 


/**
 * @brief Watchdog selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_WD_DISABLE,   // Disable the watchdog 
    ADC_WD_ENABLE     // Enable the watchdog 
} adc_wd_t; 


/**
 * @brief Watchdog single channel selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_WD_SC_DISABLE,   // Disable the watchdog single channel selection 
    ADC_WD_SC_ENABLE     // Enable the watchdog single channel selection 
} adc_wd_sc_t; 


/**
 * @brief Watchdog interrupt selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_WD_INT_DISABLE,   // Disable the watchdog interrupt 
    ADC_WD_INT_ENABLE     // Enable the watchdog interrupt 
} adc_awdie_t; 


/**
 * @brief Overrun interrupt selection 
 * 
 * @details 
 * 
 */
typedef enum {
    ADC_OVR_INT_DISABLE,   // Disable the overrun interrupt 
    ADC_OVR_INT_ENABLE     // Enable the overrun interrupt 
} adc_ovrie_t; 

//================================================================================


//================================================================================
// Initialization 

/**
 * @brief ADC port initialization 
 * 
 * @details 
 *          One time initiialization code for the ADC 
 * 
 * @param adc 
 * @param adc_common 
 * @param prescalar 
 * @param resolution 
 * @param eoc 
 * @param scan 
 * @param cont 
 * @param dma 
 * @param dds 
 * @param eocie 
 * @param ovrie 
 */
void adc_port_init(
    ADC_TypeDef *adc, 
    ADC_Common_TypeDef *adc_common, 
    adc_prescalar_t prescalar, 
    adc_res_t resolution, 
    adc_eoc_config_t eoc, 
    adc_scan_t scan, 
    adc_cont_t cont, 
    adc_dma_t dma, 
    adc_dds_t dds, 
    adc_eoc_int_t eocie, 
    adc_ovrie_t ovrie); 


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
 * @param smp 
 */
void adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param adc 
 * @param wd 
 * @param wdsc 
 * @param channel 
 * @param hi_thresh 
 * @param lo_thresh 
 * @param awdie 
 */
void adc_wd_init(
    ADC_TypeDef *adc, 
    adc_wd_t wd, 
    adc_wd_sc_t wdsc, 
    adc_channel_t channel, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh, 
    adc_awdie_t awdie); 

//================================================================================


//================================================================================
// Read 

/**
 * @brief 
 * 
 * @details 
 *          Note: this function clears any pre-existing sequences. This function is meant 
 *                to be used for systems where there are multiple ADC inputs but they only 
 *                read periodically in a random order. If there is a specific read sequence 
 *                then define the sequence during initialization and use adc_read_single_next
 *                or the scan and/or cont modes with DMA. 
 * 
 * @param adc 
 * @param channel 
 * @return uint16_t 
 */
uint16_t adc_read_single(
    ADC_TypeDef *adc, 
    adc_channel_t channel); 


/**
 * @brief Scan all ADC conversion in the sequence 
 * 
 * @details 
 *          Note: this function is only to be used when DMA is not used. 
 * 
 * @param adc 
 * @param seq_len 
 * @param adc_data 
 */
void adc_scan_seq(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len, 
    uint16_t *adc_data); 

//================================================================================


//================================================================================
// Status Registers 

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

//================================================================================


//================================================================================
// Sequence Registers 

/**
 * @brief 
 * 
 * @details 
 *          This is called independently of the adc init function because the same 
 *          pin/channel can be added to the sequence more than once. 
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
