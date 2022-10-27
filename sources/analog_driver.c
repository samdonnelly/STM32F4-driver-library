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

// Libraries 

//=======================================================================================


//================================================================================
// Initialization 

// ADC port init 
void adc_port_init(
    adc_prescalar_t prescalar, 
    adc_res_t resolution, 
    adc_eoc_config_t eoc)
{
    // Enable the ADC1 clock 
    RCC->APB2ENR |= (SET_BIT << SHIFT_8); 

    // Set the ADC clock frequency 
    adc_prescalar(ADC1_COMMON, prescalar); 

    // Set the channel resolution 
    adc_res(ADC1, resolution); 

    // Set the EOC behavior 
    adc_eoc_select(ADC1, eoc); 
}


// ADC init 
void adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp)
{
    // Configure the GPIO pin for analog mode 
    gpio_moder(gpio, MODER_ANALOG, adc_pin); 

    
    // Configure the ADC 

    // Set the sample time for the channel 
    adc_smp(adc, adc_channel, smp);

    // Set the data alignment 

    // Set DMA settings 

    // Set continuous conversion 
    // Is this it's own function? 

    // Set scan conversion 
    // Is this it's own function?  
    
    // Set the watchdog thresholds 

    // Select the channel that the watchdog watches 
    // Maybe make this it's own function so it can be changed on the go 
}

//================================================================================


//================================================================================
// Read 

// TODO come up with different read functions for single, continuous and scan modes 

// Read process: (ADC must be on first) 
// - Trigger a read 
// - See if the start bit gets set 
// - Wait for indication that the read is complete 
// - Read the data register 

// ADC data register read 
uint16_t adc_dr(
    ADC_TypeDef *adc)
{
    return (uint16_t)(adc->DR); 
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


// Analog watchdog bit status 
uint8_t adc_wd_flag(ADC_TypeDef *adc)
{
    return (adc->SR & (SET_BIT << SHIFT_0)); 
}

//================================================================================


//================================================================================
// Control Registers

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
    // Clear the resolution 
    adc->CR1 &= ~(SET_3 << SHIFT_24); 

    // Set the new resolution 
    adc->CR1 |= (resolution << SHIFT_24); 
}


// End of Conversion (EOC) selection 
void adc_eoc_select(
    ADC_TypeDef *adc, 
    adc_eoc_config_t eoc_select)
{
    // Clear the previous selection 
    adc->CR2 &= ~(SET_BIT << SHIFT_10); 

    // Select the new config 
    adc->CR2 |= (eoc_select << SHIFT_10); 
}


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
    adc->SR = CLEAR;   // Clear the status register 
    adc->CR2 |= (SET_BIT << SHIFT_30); 
}


// Enable CONT mode 
void adc_cont_enable(
    ADC_TypeDef *adc) 
{
    adc->CR2 |= (SET_BIT << SHIFT_1); 
}


// Disable CONT mode 
void adc_cont_disable(
    ADC_TypeDef *adc) 
{
    adc->CR2 &= ~(SET_BIT << SHIFT_1); 
}


// Analog watchdog enable on regular channels 
void adc_wd_enable(
    ADC_TypeDef *adc) 
{
    adc->CR1 |= (SET_BIT << SHIFT_23); 
}


// Analog watchdog disable on regular channels 
void adc_wd_disable(
    ADC_TypeDef *adc) 
{
    adc->CR1 &= ~(SET_BIT << SHIFT_23); 
}


// Enable SCAN mode 
void adc_scan_enable(
    ADC_TypeDef *adc) 
{
    adc->CR1 |= (SET_BIT << SHIFT_8); 
}


// Disable SCAN mode 
void adc_scan_disable(
    ADC_TypeDef *adc) 
{
    adc->CR1 &= ~(SET_BIT << SHIFT_8); 
}


// Enable watchdog on a single channel in SCAN mode 
void adc_wd_chan_scan_enable(
    ADC_TypeDef *adc) 
{
    adc->CR1 |= (SET_BIT << SHIFT_9); 
}


// Disable watchdog on a single channel in SCAN mode 
void adc_wd_chan_scan_disable(
    ADC_TypeDef *adc) 
{
    adc->CR1 &= ~(SET_BIT << SHIFT_9); 
}


// Analog watchdog channel select 
void adc_wd_chan_select(
    ADC_TypeDef *adc, 
    adc_channel_t adc_channel) 
{
    // Clear the previous selection 
    adc->CR1 &= ~(SET_31 << SHIFT_0); 

    // Select the channel 
    adc->CR1 |= (adc_channel << SHIFT_0); 
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
    // TODO 
    // - This must be called independent of the init function as the same channel can 
    //   be in the sequence more than once 
    // - Add overwriting capabilities 
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

//===============================================================================
