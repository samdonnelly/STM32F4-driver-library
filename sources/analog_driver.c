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
 * @details Reads the data register to get the convered ADC value. This function is only 
 *          used by the read functions that are called only when DMA is not used. 
 * 
 * @see adc_read_single
 * @see adc_scan_seq
 * 
 * @param adc : pointer to the ADC port used 
 * @return uint16_t : converted ADC value 
 */
uint16_t adc_dr(ADC_TypeDef *adc); 


/**
 * @brief Wait for start bit to set 
 * 
 * @details After triggering the start of an ADC conversion, the hardware will indicate 
 *          when the conversion has started. This function is called immediately after 
 *          adc_start and is used to wait until the conversion has started before proceeding. 
 * 
 * @see adc_start 
 * 
 * @param adc : pointer to the ADC port used 
 */
void adc_start_wait(ADC_TypeDef *adc); 


/**
 * @brief ADC end of conversion 
 * 
 * @details This function is used to wait for the end of conversion (EOC) flag to set. The 
 *          EOC flag can be set either at the end of each conversion or at the end of a 
 *          sequence of conversions depending on the configuration. This function is used 
 *          in the read functions to indicate when to read the data register. After reading 
 *          the data register then the EOC flag will clear. 
 * 
 * @see adc_read_single
 * @see adc_scan_seq
 * 
 * @param adc : pointer to the ADC port used 
 */
void adc_eoc_wait(ADC_TypeDef *adc); 


/**
 * @brief ADC prescalar 
 * 
 * @details Sets the prescalar for the ADC circuitry. The prescalar divides the APB2 clock 
 *          to set the ADC clock speed. The ADC has a maximum clock speed that it can operate 
 *          at so dividing the APB2 clock is necessary. See the device datasheet for the max 
 *          acceptable clock speed. 
 * 
 * @see adc_prescalar_t
 * @see adc_port_init
 * 
 * @param adc : pointer to the common ADC port used 
 * @param prescalar : clock divider that determines the ADC clock speed 
 */
void adc_prescalar(
    ADC_Common_TypeDef *adc,
    adc_prescalar_t prescalar); 


/**
 * @brief ADC resolution 
 * 
 * @details Sets the resolution of the ADC conversions. The resolution is the number of bits 
 *          used to represent the converted value meaning that a higher resolution will 
 *          produce a more granular or precise result than a lower resolution. 
 * 
 * @see adc_res_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param resolution : conversion resolution 
 */
void adc_res(
    ADC_TypeDef *adc, 
    adc_res_t resolution); 


/**
 * @brief End of Conversion (EOC) selection 
 * 
 * @details Selects the EOC behavior for the ADC port. The EOC flag can either be set at the 
 *          end of each conversion or at the end of a sequence of conversions. The EOC flag 
 *          indicates when the data register is ready to be read. 
 * 
 * @see adc_eoc_config_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param eoc_select : end of conversion behavior 
 */
void adc_eoc_select(
    ADC_TypeDef *adc, 
    adc_eoc_config_t eoc_select); 


/**
 * @brief End of Conversion (EOC) interrupt 
 * 
 * @details Selects the EOC interrupt configuration. This chooses whether to disable or enable 
 *          the interrupt triggered when the EOC flag is set. 
 * 
 * @see adc_eoc_int_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param eocie : EOC interrupt configuration 
 */
void adc_eocie(
    ADC_TypeDef *adc, 
    adc_eoc_int_t eocie); 


/**
 * @brief SCAN mode configuration 
 * 
 * @details Allows for enabling or disabling of scan mode. Scan mode reads all the ADC 
 *          conversions in the sequence one after the other automatically. 
 * 
 * @see adc_scan_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param scan : scan mode configuration 
 */
void adc_scan(
    ADC_TypeDef *adc, 
    adc_scan_t scan); 


/**
 * @brief CONT mode 
 * 
 * @details Allows for enabling and disabling of continuous mode. Continuous mode makes the 
 *          next ADC conversion happen automatically after one conversion finishes. If in scan 
 *          mode then after a scan is complete the scan will start over again. 
 * 
 * @see adc_cont_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param cont : continuous mode configuration 
 */
void adc_cont(
    ADC_TypeDef *adc, 
    adc_cont_t cont); 


/**
 * @brief DMA Mode 
 * 
 * @details Allows for enabling and disabling of the ADC DMA. The DMA will perform ADC 
 *          conversions in the background. 
 * 
 * @param adc : pointer to the ADC port used 
 * @param dma : DMA mode configuration 
 */
void adc_dma(
    ADC_TypeDef *adc, 
    adc_dma_t dma); 


/**
 * @brief DMA disable 
 * 
 * @details Allows for enabling and disabling of the DMA disable feature. If enabled then this 
 *          feature will disable the DMA after the last transfer. 
 * 
 * @see adc_dds_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param dds : DMA disable configuration 
 */
void adc_dds(
    ADC_TypeDef *adc, 
    adc_dds_t dds); 


/**
 * @brief Analog watchdog enable on regular channels 
 * 
 * @details Select the watchdog configuration. If enabled then the watchdog will monitor the 
 *          input voltage on the channels and trigger the watchdog flag if a voltage threshold 
 *          is surpassed. 
 * 
 * @see adc_wd_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param wd : watchdog configuration 
 */
void adc_awden(
    ADC_TypeDef *adc, 
    adc_wd_t wd); 


/**
 * @brief Enable the watchdog on a single channel in scan mode
 * 
 * @details 
 * 
 * @see adc_wd_sc_t
 * 
 * @param adc : pointer to the ADC port used 
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
 * @param adc : pointer to the ADC port used 
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
 * @param adc : pointer to the ADC port used 
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
 * @param adc : pointer to the ADC port used 
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
 * @param adc : pointer to the ADC port used 
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
 * @param adc : pointer to the ADC port used 
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
 * @param adc : pointer to the ADC port used 
 */
void adc_seq_clear(ADC_TypeDef *adc); 

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
    adc_eoc_int_t eocie, 
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

    // Set the EOC interrupt 
    adc_eocie(adc, eocie); 

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
    // TODO change this to the gpio_pin_init function and test it 
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
