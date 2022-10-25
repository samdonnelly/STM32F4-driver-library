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
void adc_port_init(void)
{
    // Enable the ADC1 clock 
    RCC->APB2ENR |= (SET_BIT << SHIFT_8); 
}


// ADC init 
void adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp, 
    adc_seq_num_t seq_num)
{
    // Notes: 
    //  - This function gets called for each ADC channel being initialized so don't put 
    //    code that only needs to be called once in here. 

    // Configure the GPIO for analog mode 
    gpio->MODER |= (SET_3 << (2*adc_pin)); 
    // gpio->OSPEEDR |= (SET_3 << (2*adc_pin)); 

    // Configure the ADC 

    // Set the channel resolution 

    // Set the data alignment 

    // Select end of conversion 

    // Set DMA settings 

    // Set continuous conversion 
    // Is this it's own function? 

    // Set scan conversion 
    // Is this it's own function? 

    // Set the sample time for the channel 
    if (adc_channel > ADC_CHANNEL_9) 
        adc->SMPR1 |= (smp << 3*(adc_channel - ADC_CHANNEL_10)); 
    else 
        adc->SMPR2 |= (smp << 3*adc_channel); 
    
    // Set the watchdog thresholds 

    // Select the channel that the watchdog watches 
    // Maybe make this it's own function so it can be changed on the go 

    // Set the channel sequence number 
    if (seq_num > ADC_SEQ_12) 
        adc->SQR1 |= (adc_channel << 5*(seq_num - ADC_SEQ_13)); 
    else if (seq_num > ADC_SEQ_6) 
        adc->SQR2 |= (adc_channel << 5*(seq_num - ADC_SEQ_7)); 
    else 
        adc->SQR3 |= (adc_channel << 5*(seq_num - ADC_SEQ_1)); 

    // Set the ADC pre-scalar 
    // ADC_CCR --> ADCPRE
}


// Regular channel sequence length setter 
void adc_seq_len_set(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len) 
{
    adc->SQR1 |= (seq_len << SHIFT_20); 
}

//================================================================================


//================================================================================
// Read 

// Start an ADC conversion 
void adc_start(
    ADC_TypeDef *adc)
{
    // 
}


// ADC data register read 
uint16_t adc_dr(
    ADC_TypeDef *adc)
{
    return (uint16_t)(adc->DR); 
}

//================================================================================


//================================================================================
// Control 

// Turn ADC on and off 
void adc_on_off(
    ADC_TypeDef *adc)
{
    // 
}

//================================================================================


//================================================================================
// Status 

// Wait for the start bit to set 
void adc_start_wait(void)
{
    //  
}

// ADC end of conversion 
void adc_eoc(void)
{
    // 
}

//================================================================================
