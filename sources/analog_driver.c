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

    // Set the sample time for the channel 
    if (adc_channel > ADC_CHANNEL_9) 
        adc->SMPR1 |= (smp << 3*(adc_channel - ADC_CHANNEL_10)); 
    else 
        adc->SMPR2 |= (smp << 3*adc_channel); 

    // Set the channel sequence number 
    if (seq_num > ADC_SEQ_12) 
        adc->SQR1 |= (adc_channel << 5*(seq_num - ADC_SEQ_13)); 
    else if (seq_num > ADC_SEQ_6) 
        adc->SQR2 |= (adc_channel << 5*(seq_num - ADC_SEQ_7)); 
    else 
        adc->SQR3 |= (adc_channel << 5*(seq_num - ADC_SEQ_1)); 
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

// ADC data register read 
uint16_t adc_dr(
    ADC_TypeDef *adc)
{
    return (uint16_t)(adc->DR); 
}

//================================================================================
