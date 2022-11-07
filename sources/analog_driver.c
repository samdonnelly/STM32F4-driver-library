/**
 * @file analog_driver.c
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

//=======================================================================================
// Includes 

// Drivers 
#include "analog_driver.h"

//=======================================================================================


//================================================================================
// Function Prototypes 

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


/**
 * @brief EOC interrupt 
 * 
 * @details 
 * 
 * @param adc 
 * @param eocie 
 */
void adc_eocie(
    ADC_TypeDef *adc, 
    adc_eoc_int_t eocie); 


/**
 * @brief SCAN mode configuration 
 * 
 * @details 
 * 
 * @param adc 
 * @param scan 
 */
void adc_scan(
    ADC_TypeDef *adc, 
    adc_scan_t scan); 


/**
 * @brief CONT mode 
 * 
 * @details 
 * 
 * @param adc 
 * @param cont 
 */
void adc_cont(
    ADC_TypeDef *adc, 
    adc_cont_t cont); 


/**
 * @brief DMA Mode 
 * 
 * @details 
 * 
 * @param adc 
 * @param dma 
 */
void adc_dma(
    ADC_TypeDef *adc, 
    adc_dma_t dma); 


/**
 * @brief DMA disable 
 * 
 * @details 
 * 
 * @param adc 
 * @param dds 
 */
void adc_dds(
    ADC_TypeDef *adc, 
    adc_dds_t dds); 


/**
 * @brief Analog watchdog enable on regular channels 
 * 
 * @details 
 * 
 * @param wd 
 */
void adc_awden(
    ADC_TypeDef *adc, 
    adc_wd_t wd); 


/**
 * @brief Enable the watchdog on a single channel in scan mode
 * 
 * @details 
 * 
 * @param wdsc 
 */
void adc_awdsgl(
    ADC_TypeDef *adc, 
    adc_wd_sc_t wdsc); 


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
 * @brief Analog watchdog interrupt 
 * 
 * @details 
 * 
 * @param adc 
 * @param awdie 
 */
void adc_awdie(
    ADC_TypeDef *adc, 
    adc_awdie_t awdie); 


/**
 * @brief Overrun interrupt 
 * 
 * @details 
 * 
 * @param adc 
 * @param ovrie 
 */
void adc_ovrie(
    ADC_TypeDef *adc, 
    adc_ovrie_t ovrie); 


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


/**
 * @brief ADC sequence clear 
 * 
 * @details Clears all data in the SQRx registers which includes sequence order and length. 
 * 
 * @param adc 
 */
void adc_seq_clear(
    ADC_TypeDef *adc); 

//================================================================================


//================================================================================
// Initialization 

// ADC port init 
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
    adc_ovrie_t ovrie)
{
    // Enable the ADC1 clock 
    if (adc == ADC1) RCC->APB2ENR |= (SET_BIT << SHIFT_8); 

    // Set the ADC clock frequency 
    adc_prescalar(adc_common, prescalar); 

    // Set the channel resolution 
    adc_res(adc, resolution); 

    // Set the EOC behavior 
    adc_eoc_select(adc, eoc); 

    // Set scan conversion 
    adc_scan(adc, scan);

    // Set continuous conversion 
    adc_cont(adc, cont); 

    // Set DMA settings 
    adc_dma(adc, dma); 
    adc_dds(adc, dds); 

    // Set overrun interrupt 
    adc_ovrie(adc, ovrie); 
}


// ADC pin init 
void adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp)
{
    // Configure the GPIO pin for analog mode 
    gpio_moder(gpio, MODER_ANALOG, adc_pin); 

    // Set the sample time for the channel 
    adc_smp(adc, adc_channel, smp);
}


// ADC watchdog setup 
void adc_wd_init(
    ADC_TypeDef *adc, 
    adc_wd_t wd, 
    adc_wd_sc_t wdsc, 
    adc_channel_t channel, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh, 
    adc_awdie_t awdie)
{
    // Analog watchdog enable on regular channels 
    adc_awden(adc, wd); 

    // Enable the watchdog on a single channel in scan mode
    adc_awdsgl(adc, wdsc); 

    // Select the channel that the watchdog watches 
    adc_wd_chan_select(adc, channel); 

    // Set the watchdog thresholds 
    adc_wd_thresh(adc, hi_thresh, lo_thresh); 

    // Analog watchdog interrupt 
    adc_awdie(adc, awdie); 
}

//================================================================================


//================================================================================
// Read 

// ADC data register read 
uint16_t adc_dr(
    ADC_TypeDef *adc)
{
    return (uint16_t)(adc->DR); 
}


// Read a select single ADC conversion 
uint16_t adc_read_single(
    ADC_TypeDef *adc, 
    adc_channel_t channel)
{
    // Clear the existing sequence 
    adc_seq_clear(adc); 

    // Set the select channel as the next in the sequence 
    adc_seq(adc, channel, ADC_SEQ_1); 

    // Set the sequence length 
    adc_seq_len_set(adc, ADC_SEQ_1); 

    // Start and ADC conversion 
    adc_start(adc); 

    // Wait for end of ADC conversion 
    adc_eoc_wait(adc); 

    // Read the ADC value 
    return adc_dr(adc); 
}


// Scan all ADC conversion in the sequence 
void adc_scan_seq(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len, 
    uint16_t *adc_data)
{
    // Start and ADC conversion 
    adc_start(adc); 

    // Read the ADC sequence 
    do 
    {
        adc_eoc_wait(adc);            // Wait for end of ADC conversion 
        *adc_data++ = adc_dr(adc);    // Read the data 
    } 
    while (--seq_len); 
}

//================================================================================


//================================================================================
// Status Registers 

// Wait for the start bit to set 
void adc_start_wait(ADC_TypeDef *adc)
{
    while (!((adc->SR) & (SET_BIT << SHIFT_4))); 
}


// Wait for end of ADC conversion 
void adc_eoc_wait(ADC_TypeDef *adc)
{
    while(!((adc->SR) & (SET_BIT << SHIFT_1))); 
}


// Overrun bit status 
uint8_t adc_overrun_status(ADC_TypeDef *adc)
{
    return ((adc->SR & (SET_BIT << SHIFT_5)) >> SHIFT_5); 
}


// Clear the overrun bit 
void adc_overrun_clear(ADC_TypeDef *adc)
{
    adc->SR &= ~(SET_BIT << SHIFT_5); 
}


// Analog watchdog flag 
uint8_t adc_wd_flag(ADC_TypeDef *adc)
{
    return (adc->SR & (SET_BIT << SHIFT_0)); 
}

//================================================================================


//================================================================================
// Control Registers

// Turn ADC on 
void adc_on(
    ADC_TypeDef *adc)
{
    adc->CR2 |= (SET_BIT << SHIFT_0); 
    tim9_delay_ms(ADC_STAB_TIME);       // Give ADC stabilization time 
}


// Turn ADC off 
void adc_off(
    ADC_TypeDef *adc)
{
    adc->CR2 &= ~(SET_BIT << SHIFT_0); 
}


// Start an ADC conversion 
void adc_start(
    ADC_TypeDef *adc)
{
    adc->SR = CLEAR;                      // Clear the status register 
    adc->CR2 |= (SET_BIT << SHIFT_30);    // Set the start bit 
    adc_start_wait(adc);                  // Wait for the start bit to set 
}


// ADC prescalar 
void adc_prescalar(
    ADC_Common_TypeDef *adc,
    adc_prescalar_t prescalar)
{
    adc->CCR |= (prescalar << SHIFT_16); 
}


// Set the ADC resolution 
void adc_res(
    ADC_TypeDef *adc, 
    adc_res_t resolution)
{
    adc->CR1 &= ~(SET_3 << SHIFT_24); 
    adc->CR1 |= (resolution << SHIFT_24); 
}


// End of Conversion (EOC) selection 
void adc_eoc_select(
    ADC_TypeDef *adc, 
    adc_eoc_config_t eoc_select)
{
    adc->CR2 &= ~(SET_BIT << SHIFT_10); 
    adc->CR2 |= (eoc_select << SHIFT_10); 
}


// EOC interrupt --> add to init 
void adc_eocie(
    ADC_TypeDef *adc, 
    adc_eoc_int_t eocie)
{
    adc->CR1 &= ~(SET_BIT << SHIFT_5); 
    adc->CR1 |= (eocie << SHIFT_5); 
}


// SCAN mode 
void adc_scan(
    ADC_TypeDef *adc, 
    adc_scan_t scan)
{
    adc->CR1 &= ~(SET_BIT << SHIFT_8); 
    adc->CR1 |= (scan << SHIFT_8); 
}


// CONT mode 
void adc_cont(
    ADC_TypeDef *adc, 
    adc_cont_t cont)
{
    adc->CR2 &= ~(SET_BIT << SHIFT_1); 
    adc->CR2 |= (cont << SHIFT_1); 
}


// DMA Mode 
void adc_dma(
    ADC_TypeDef *adc, 
    adc_dma_t dma)
{
    adc->CR2 &= ~(SET_BIT << SHIFT_8); 
    adc->CR2 |= (dma << SHIFT_8); 
}


// DMA disable 
void adc_dds(
    ADC_TypeDef *adc, 
    adc_dds_t dds)
{
    adc->CR2 &= ~(SET_BIT << SHIFT_9); 
    adc->CR2 |= (dds << SHIFT_9); 
}


// Analog watchdog enable on regular channels 
void adc_awden(
    ADC_TypeDef *adc, 
    adc_wd_t wd)
{
    adc->CR1 &= ~(SET_BIT << SHIFT_23); 
    adc->CR1 |= (wd << SHIFT_23); 
}


// Enable the watchdog on a single channel in scan mode
void adc_awdsgl(
    ADC_TypeDef *adc, 
    adc_wd_sc_t wdsc)
{
    adc->CR1 &= ~(SET_BIT << SHIFT_9); 
    adc->CR1 |= (wdsc << SHIFT_9); 
}


// Analog watchdog channel select 
void adc_wd_chan_select(
    ADC_TypeDef *adc, 
    adc_channel_t adc_channel) 
{
    adc->CR1 &= ~(SET_31 << SHIFT_0); 
    adc->CR1 |= (adc_channel << SHIFT_0); 
}


// Analog watchdog interrupt 
void adc_awdie(
    ADC_TypeDef *adc, 
    adc_awdie_t awdie)
{
    adc->CR1 &= ~(SET_BIT << SHIFT_6); 
    adc->CR1 |= (awdie << SHIFT_6); 
}


// Overrun interrupt --> add to init 
void adc_ovrie(
    ADC_TypeDef *adc, 
    adc_ovrie_t ovrie)
{
    adc->CR1 &= ~(SET_BIT << SHIFT_26); 
    adc->CR1 |= (ovrie << SHIFT_26); 
}

//================================================================================


//================================================================================
// Sample Registers 

// Set the sample time for the channel 
void adc_smp(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_smp_cycles_t smp)
{
    if (channel > ADC_CHANNEL_9) 
        adc->SMPR1 |= (smp << 3*(channel - ADC_CHANNEL_10)); 
    else 
        adc->SMPR2 |= (smp << 3*channel); 
}

//================================================================================


//===============================================================================
// Watchdog Registers 

// Analog watchdog thresholds 
void adc_wd_thresh(
    ADC_TypeDef *adc, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh)
{
    // TODO what is the format for these registers? Are they unsigned integers? 
    adc->HTR = hi_thresh; 
    adc->LTR = lo_thresh; 
}

//===============================================================================


//===============================================================================
// Sequence Registers 

// Channel sequence 
void adc_seq(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_seq_num_t seq_num)
{
    if (seq_num > ADC_SEQ_12) 
        adc->SQR1 |= (channel << (5*(seq_num - ADC_SEQ_13))); 
    else if (seq_num > ADC_SEQ_6) 
        adc->SQR2 |= (channel << (5*(seq_num - ADC_SEQ_7))); 
    else 
        adc->SQR3 |= (channel << (5*(seq_num - ADC_SEQ_1))); 
}


// Regular channel sequence length setter 
void adc_seq_len_set(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len) 
{
    adc->SQR1 |= ((seq_len - ADC_SEQ_1) << SHIFT_20); 
}


// ADC sequence clear 
void adc_seq_clear(
    ADC_TypeDef *adc)
{
    adc->SQR1 = CLEAR; 
    adc->SQR2 = CLEAR; 
    adc->SQR3 = CLEAR; 
}

//===============================================================================
