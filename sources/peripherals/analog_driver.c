/**
 * @file analog_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog (ADC) data functions 
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


//=======================================================================================
// Macros 

// Data size 
#define ADC_CHNL_NUM_SIZE 5             // Channel number bit size 
#define ADC_SMPL_TIME_SIZE 3            // Sampling time bit size 
#define ADC_WD_THRESH_MASK 0x00000FFF   // Upper and lower watchdog threshold mask 

// Timing 
#define ADC_STABLE_TIME 10              // ADC stabilization time (ms) after being turned on 
#define ADC_WAIT_TIMEOUT 0xFFFF         // Timeout to prevent code from hanging 

//=======================================================================================


//=======================================================================================
// Prototypes 

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
uint16_t adc_dr(
    ADC_TypeDef *adc); 


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
#if DEV_CODE 
ADC_STATUS adc_start_wait(
    ADC_TypeDef *adc); 
#else   // DEV_CODE 
void adc_start_wait(
    ADC_TypeDef *adc); 
#endif   // DEV_CODE 


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
#if DEV_CODE 
ADC_STATUS adc_eoc_wait(
    ADC_TypeDef *adc); 
#else   // DEV_CODE 
void adc_eoc_wait(
    ADC_TypeDef *adc); 
#endif   // DEV_CODE 


/**
 * @brief Overrun bit status 
 * 
 * @details This returns the status of the ADC overrun which indicates if there has been 
 *          a loss of data. If the overrun interrupt is enabled then this function isn't 
 *          needed. Possible return values: <br> 
 *            - 0: no overrun <br> 
 *            - 1: overrun occured <br> 
 * 
 * @param adc : pointer to the ADC port used 
 * @return uint8_t : overrun status 
 */
uint8_t adc_overrun_status(
    ADC_TypeDef *adc); 


/**
 * @brief Clear the overrun flag 
 * 
 * @details Clears the overrun bit for a given ADC port. Note that if overrun interrupts are 
 *          enabled for the ADC then this function may be needed by the interrupt handler in 
 *          order to exit from the handler. This has yet to be tested. 
 * 
 * @param adc : pointer to the ADC port used 
 */
void adc_overrun_clear(
    ADC_TypeDef *adc); 


/**
 * @brief Analog watchdog bit status 
 * 
 * @details Returns the status of the ADC watchdog which indicates if a channel has exceeded 
 *          the defined voltage thresholds. If the watchdog interrupt is enabled then this 
 *          function is not needed. Possible return values: <br> 
 *            - 0: no watchdog flag <br> 
 *            - 1: watchdog flag - threshold exceeded <br> 
 * 
 * @param adc : pointer to the ADC port used 
 * @return uint8_t : watchdog status 
 */
uint8_t adc_wd_flag(
    ADC_TypeDef *adc); 


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
 * @details Enables the watchdog on the channel identified using the adc_wd_sc_t function. 
 * 
 * @see adc_wd_chan_select
 * @see adc_wd_sc_t
 * 
 * @see adc_wd_sc_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param wdsc : watchdog single channel configuration 
 */
void adc_awdsgl(
    ADC_TypeDef *adc, 
    adc_wd_sc_t wdsc); 


/**
 * @brief Analog watchdog channel select 
 * 
 * @details Select the input channel to be guarded by the analog watchdog. 
 * 
 * @see adc_channel_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param adc_channel : ADC channel guarded by watchdog 
 */
void adc_wd_chan_select(
    ADC_TypeDef *adc, 
    adc_channel_t adc_channel); 


/**
 * @brief Analog watchdog interrupt 
 * 
 * @details Configures the interrupt for the watchdog. If the watchdog exceeds either its upper 
 *          or lower voltage threshold (watchdog lfag set) then, if set, the watchdog interrupt 
 *          will trip. 
 * 
 * @see adc_awdie_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param awdie : watchdog interrupt configuration 
 */
void adc_awdie(
    ADC_TypeDef *adc, 
    adc_awdie_t awdie); 


/**
 * @brief Overrun interrupt 
 * 
 * @details Configures the interrupt for the data overrun. The overrun bit will set if there 
 *          has been a loss of data (data overwritten before being read in the data register) 
 *          and this interrupt can be triggered when the overrun bit sets. 
 * 
 * @see adc_ovrie_t 
 * 
 * @param adc : pointer to the ADC port used 
 * @param ovrie : overrun interrupt configuration 
 */
void adc_ovrie(
    ADC_TypeDef *adc, 
    adc_ovrie_t ovrie); 


/**
 * @brief Set the sample time for the channel 
 * 
 * @details Configure the sampling time of the ADC conversion on a specific channel. Longer 
 *          sampling times lead to more accurate readings but take more time and vice versa. 
 * 
 * @see adc_channel_t
 * @see adc_smp_cycles_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param channel : ADC channel being configured 
 * @param smp : sampling tie configuration 
 */
void adc_smp(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_smp_cycles_t smp); 


/**
 * @brief Set the watchdog thresholds 
 * 
 * @details Set the upper and lower voltage thresholds of the watchdog. If the ADC voltage 
 *          does not fall between the threshold boundaries then the watchdog flag will be set. 
 *          Note that the threshold values are up to 16 bit integers and are scaled to match the 
 *          ADC voltage range. 
 * 
 * @param adc : pointer to the ADC port used 
 * @param hi_thresh : upper watchdog threshold 
 * @param lo_thresh : lower watchdog threshold 
 */
void adc_wd_thresh(
    ADC_TypeDef *adc, 
    uint32_t hi_thresh, 
    uint32_t lo_thresh); 


/**
 * @brief ADC sequence clear 
 * 
 * @details Clears all data in the SQRx registers which includes sequence order and length. 
 * 
 * @param adc : pointer to the ADC port used 
 */
void adc_seq_clear(
    ADC_TypeDef *adc); 

//=======================================================================================


//=======================================================================================
// Initialization 

// ADC port init - called once for each ADC port used 
#if DEV_CODE 
void adc_port_init(
    ADC_TypeDef *adc, 
    ADC_Common_TypeDef *adc_common, 
    adc_prescalar_t prescalar, 
    adc_res_t resolution, 
    adc_param_config_t eoc, 
    adc_param_config_t eocie, 
    adc_param_config_t scan, 
    adc_param_config_t cont, 
    adc_param_config_t dma, 
    adc_param_config_t dds, 
    adc_param_config_t ovrie)
#else   // DEV_CODE 
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
#endif   // DEV_CODE 
{
    // Check for a valid ADC port 
    if (adc != ADC1) 
    {
        return; 
    }

    // Enable the ADC1 clock 
    RCC->APB2ENR |= (SET_BIT << SHIFT_8); 

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


// ADC pin init - called for each ADC pin used 
void adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp)
{
    // Configure the GPIO pin for analog mode 
    gpio_pin_init(gpio, adc_pin, MODER_ANALOG, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 

    // Set the sample time for the channel 
    adc_smp(adc, adc_channel, smp);
}


// ADC watchdog setup 
#if DEV_CODE 
void adc_wd_init(
    ADC_TypeDef *adc, 
    adc_param_config_t wd, 
    adc_param_config_t wdsc, 
    adc_channel_t channel, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh, 
    adc_param_config_t awdie)
#else   // DEV_CODE 
void adc_wd_init(
    ADC_TypeDef *adc, 
    adc_wd_t wd, 
    adc_wd_sc_t wdsc, 
    adc_channel_t channel, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh, 
    adc_awdie_t awdie)
#endif   // DEV_CODE 
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


// Channel sequence - called for each ADC input (pin) in the sequence 
void adc_seq(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_seq_num_t seq_num)
{
    uint32_t adc_channel = (uint32_t)channel; 

    if (seq_num > ADC_SEQ_12) 
    {
        adc->SQR1 |= (adc_channel << (uint32_t)(ADC_CHNL_NUM_SIZE*(seq_num - ADC_SEQ_13))); 
    }
    else if (seq_num > ADC_SEQ_6) 
    {
        adc->SQR2 |= (adc_channel << (uint32_t)(ADC_CHNL_NUM_SIZE*(seq_num - ADC_SEQ_7))); 
    }
    else 
    {
        adc->SQR3 |= (adc_channel << (uint32_t)(ADC_CHNL_NUM_SIZE*(seq_num - ADC_SEQ_1))); 
    }
}


// Regular channel sequence length setter - called once if a sequence is used 
void adc_seq_len_set(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len) 
{
    adc->SQR1 |= (uint32_t)((seq_len - ADC_SEQ_1) << SHIFT_20); 
}

//=======================================================================================


//=======================================================================================
// User functions 

// Turn ADC on 
void adc_on(
    ADC_TypeDef *adc)
{
    adc->CR2 |= (SET_BIT << SHIFT_0); 
}


// Turn ADC off 
void adc_off(
    ADC_TypeDef *adc)
{
    adc->CR2 &= ~(SET_BIT << SHIFT_0); 
}


// Start an ADC conversion 
#if DEV_CODE 
ADC_STATUS adc_start(
    ADC_TypeDef *adc)
{
    // Clear the status register, set the start bit and wait to see that the start 
    // bit was successfully set. If it does not set (times out) for whatever reason 
    // then a fault status will be returned. 
    adc->SR = CLEAR; 
    adc->CR2 |= (SET_BIT << SHIFT_30); 
    return (ADC_STATUS)adc_start_wait(adc); 
}
#else   // DEV_CODE 
void adc_start(
    ADC_TypeDef *adc)
{
    adc->SR = CLEAR;                      // Clear the status register 
    adc->CR2 |= (SET_BIT << SHIFT_30);    // Set the start bit 
    adc_start_wait(adc);                  // Wait for the start bit to set 
}
#endif   // DEV_CODE 


// Read a select single ADC conversion 
uint16_t adc_read_single(
    ADC_TypeDef *adc, 
    adc_channel_t channel)
{
    // Clear the existing sequence 
    adc_seq_clear(adc); 

    // Set the selected channel as the next in the sequence 
    adc_seq(adc, channel, ADC_SEQ_1); 

    // Set the sequence length 
    adc_seq_len_set(adc, ADC_SEQ_1); 

    // Start the ADC conversion 
    adc_start(adc); 

    // Wait for the end of the ADC conversion. If the conversion doesn't finish 
    // (times out) for whatever reason, the function will return zero. 
#if DEV_CODE 
    if (adc_eoc_wait(adc)) 
    {
        return NONE; 
    }
#else   // DEV_CODE 
    adc_eoc_wait(adc); 
#endif   // DEV_CODE 

    // Read the ADC value 
    return adc_dr(adc); 
}


// Scan all ADC conversion in the sequence 
#if DEV_CODE 
ADC_STATUS adc_scan_seq(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len, 
    uint16_t *adc_data)
{
    uint8_t seq_count = (uint8_t)seq_len; 

    // Start the ADC conversions and read each of the ADC sequence values. Wait for the 
    // current conversion to be finished before reading. If the conversion doesn't 
    // finish (times out) for whatever reason, the sequence will be cut short. 
    adc_start(adc); 

    do 
    {
        if (adc_eoc_wait(adc)) 
        {
            return (ADC_STATUS)ADC_TIMEOUT; 
        }
        *adc_data++ = adc_dr(adc); 
    } 
    while (--seq_count); 

    return (ADC_STATUS)ADC_OK; 
}
#else   // DEV_CODE 
void adc_scan_seq(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len, 
    uint16_t *adc_data)
{
    // Start the ADC conversions and read each of the ADC sequence values. Wait for the 
    // current conversion to be finished before reading. 
    adc_start(adc); 

    do 
    {
        adc_eoc_wait(adc); 
        *adc_data++ = adc_dr(adc); 
    } 
    while (--seq_len); 
}
#endif   // DEV_CODE 

//=======================================================================================


//=======================================================================================
// Register functions 

// ADC data register read 
uint16_t adc_dr(
    ADC_TypeDef *adc)
{
    return (uint16_t)(adc->DR); 
}


// Wait for the start bit to set 
#if DEV_CODE 
ADC_STATUS adc_start_wait(
    ADC_TypeDef *adc)
{
    uint16_t timeout = ADC_WAIT_TIMEOUT; 
    while ( (!((adc->SR) & (uint32_t)(SET_BIT << SHIFT_4))) && (--timeout) ); 
    return timeout ? (ADC_STATUS)ADC_OK : (ADC_STATUS)ADC_NOT_OK; 
}
#else   // DEV_CODE 
void adc_start_wait(
    ADC_TypeDef *adc)
{
    while (!((adc->SR) & (uint32_t)(SET_BIT << SHIFT_4))); 
}
#endif   // DEV_CODE 


// Wait for end of ADC conversion 
#if DEV_CODE 
ADC_STATUS adc_eoc_wait(
    ADC_TypeDef *adc)
{
    uint16_t timeout = ADC_WAIT_TIMEOUT; 
    while( (!((adc->SR) & (uint32_t)(SET_BIT << SHIFT_1))) && (--timeout) ); 
    return timeout ? (ADC_STATUS)ADC_OK : (ADC_STATUS)ADC_NOT_OK; 
}
#else   // DEV_CODE 
void adc_eoc_wait(
    ADC_TypeDef *adc)
{
    while(!((adc->SR) & (uint32_t)(SET_BIT << SHIFT_1))); 
}
#endif   // DEV_CODE 


// Overrun bit status 
uint8_t adc_overrun_status(
    ADC_TypeDef *adc)
{
    return (uint8_t)((adc->SR & (uint32_t)(SET_BIT << SHIFT_5)) >> SHIFT_5); 
}


// Clear the overrun bit 
void adc_overrun_clear(
    ADC_TypeDef *adc)
{
    adc->SR &= ~((uint32_t)(SET_BIT << SHIFT_5)); 
}


// Analog watchdog flag 
uint8_t adc_wd_flag(ADC_TypeDef *adc)
{
    return (uint8_t)(adc->SR & (uint32_t)(SET_BIT << SHIFT_0)); 
}


// ADC prescalar 
void adc_prescalar(
    ADC_Common_TypeDef *adc,
    adc_prescalar_t prescalar)
{
    adc->CCR &= ~((uint32_t)(SET_3 << SHIFT_16)); 
    adc->CCR |= (uint32_t)(prescalar << SHIFT_16); 
}


// Set the ADC resolution 
void adc_res(
    ADC_TypeDef *adc, 
    adc_res_t resolution)
{
    adc->CR1 &= ~((uint32_t)(SET_3 << SHIFT_24)); 
    adc->CR1 |= (uint32_t)(resolution << SHIFT_24); 
}


// End of Conversion (EOC) selection 
#if DEV_CODE 
void adc_eoc_select(
    ADC_TypeDef *adc, 
    adc_param_config_t eoc_select)
#else   // DEV_CODE 
void adc_eoc_select(
    ADC_TypeDef *adc, 
    adc_eoc_config_t eoc_select)
#endif   // DEV_CODE 
{
    adc->CR2 &= ~((uint32_t)(SET_BIT << SHIFT_10)); 
    adc->CR2 |= (uint32_t)(eoc_select << SHIFT_10); 
}


// EOC interrupt --> add to init 
#if DEV_CODE 
void adc_eocie(
    ADC_TypeDef *adc, 
    adc_param_config_t eocie)
#else   // DEV_CODE 
void adc_eocie(
    ADC_TypeDef *adc, 
    adc_eoc_int_t eocie)
#endif   // DEV_CODE 
{
    adc->CR1 &= ~((uint32_t)(SET_BIT << SHIFT_5)); 
    adc->CR1 |= (uint32_t)(eocie << SHIFT_5); 
}


// SCAN mode 
#if DEV_CODE 
void adc_scan(
    ADC_TypeDef *adc, 
    adc_param_config_t scan)
#else   // DEV_CODE 
void adc_scan(
    ADC_TypeDef *adc, 
    adc_scan_t scan)
#endif   // DEV_CODE 
{
    adc->CR1 &= ~((uint32_t)(SET_BIT << SHIFT_8)); 
    adc->CR1 |= (uint32_t)(scan << SHIFT_8); 
}


// CONT mode 
#if DEV_CODE 
void adc_cont(
    ADC_TypeDef *adc, 
    adc_param_config_t cont)
#else   // DEV_CODE 
void adc_cont(
    ADC_TypeDef *adc, 
    adc_cont_t cont)
#endif   // DEV_CODE 
{
    adc->CR2 &= ~((uint32_t)(SET_BIT << SHIFT_1)); 
    adc->CR2 |= (uint32_t)(cont << SHIFT_1); 
}


// DMA Mode 
#if DEV_CODE 
void adc_dma(
    ADC_TypeDef *adc, 
    adc_param_config_t dma)
#else   // DEV_CODE 
void adc_dma(
    ADC_TypeDef *adc, 
    adc_dma_t dma)
#endif   // DEV_CODE 
{
    adc->CR2 &= ~((uint32_t)(SET_BIT << SHIFT_8)); 
    adc->CR2 |= (uint32_t)(dma << SHIFT_8); 
}


// DMA disable 
#if DEV_CODE 
void adc_dds(
    ADC_TypeDef *adc, 
    adc_param_config_t dds)
#else   // DEV_CODE 
void adc_dds(
    ADC_TypeDef *adc, 
    adc_dds_t dds)
#endif   // DEV_CODE 
{
    adc->CR2 &= ~((uint32_t)(SET_BIT << SHIFT_9)); 
    adc->CR2 |= (uint32_t)(dds << SHIFT_9); 
}


// Analog watchdog enable on regular channels 
#if DEV_CODE 
void adc_awden(
    ADC_TypeDef *adc, 
    adc_param_config_t wd)
#else   // DEV_CODE 
void adc_awden(
    ADC_TypeDef *adc, 
    adc_wd_t wd)
#endif   // DEV_CODE 
{
    adc->CR1 &= ~((uint32_t)(SET_BIT << SHIFT_23)); 
    adc->CR1 |= (uint32_t)(wd << SHIFT_23); 
}


// Enable the watchdog on a single channel in scan mode 
#if DEV_CODE 
void adc_awdsgl(
    ADC_TypeDef *adc, 
    adc_param_config_t wdsc)
#else   // DEV_CODE 
void adc_awdsgl(
    ADC_TypeDef *adc, 
    adc_wd_sc_t wdsc)
#endif   // DEV_CODE 
{
    adc->CR1 &= ~((uint32_t)(SET_BIT << SHIFT_9)); 
    adc->CR1 |= (uint32_t)(wdsc << SHIFT_9); 
}


// Analog watchdog channel select 
void adc_wd_chan_select(
    ADC_TypeDef *adc, 
    adc_channel_t adc_channel) 
{
    adc->CR1 &= ~((uint32_t)(SET_31 << SHIFT_0)); 
    adc->CR1 |= (uint32_t)(adc_channel << SHIFT_0); 
}


// Analog watchdog interrupt 
#if DEV_CODE 
void adc_awdie(
    ADC_TypeDef *adc, 
    adc_param_config_t awdie)
#else   // DEV_CODE 
void adc_awdie(
    ADC_TypeDef *adc, 
    adc_awdie_t awdie)
#endif   // DEV_CODE 
{
    adc->CR1 &= ~((uint32_t)(SET_BIT << SHIFT_6)); 
    adc->CR1 |= (uint32_t)(awdie << SHIFT_6); 
}


// Overrun interrupt --> add to init 
#if DEV_CODE 
void adc_ovrie(
    ADC_TypeDef *adc, 
    adc_param_config_t ovrie)
#else   // DEV_CODE 
void adc_ovrie(
    ADC_TypeDef *adc, 
    adc_ovrie_t ovrie)
#endif   // DEV_CODE 
{
    adc->CR1 &= ~((uint32_t)(SET_BIT << SHIFT_26)); 
    adc->CR1 |= (uint32_t)(ovrie << SHIFT_26); 
}


// Set the sample time for the channel 
void adc_smp(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_smp_cycles_t smp)
{
    uint32_t sample = (uint32_t)smp; 

    if (channel > ADC_CHANNEL_9) 
    {
        adc->SMPR1 |= (sample << (uint32_t)(ADC_SMPL_TIME_SIZE*(channel - ADC_CHANNEL_10))); 
    }
    else 
    {
        adc->SMPR2 |= (sample << (uint32_t)(ADC_SMPL_TIME_SIZE*channel)); 
    }
}


// Analog watchdog thresholds 
void adc_wd_thresh(
    ADC_TypeDef *adc, 
    uint32_t hi_thresh, 
    uint32_t lo_thresh)
{
    adc->HTR = hi_thresh & ADC_WD_THRESH_MASK; 
    adc->LTR = lo_thresh & ADC_WD_THRESH_MASK; 
}


// ADC sequence clear 
void adc_seq_clear(
    ADC_TypeDef *adc)
{
    adc->SQR1 = CLEAR; 
    adc->SQR2 = CLEAR; 
    adc->SQR3 = CLEAR; 
}

//=======================================================================================
