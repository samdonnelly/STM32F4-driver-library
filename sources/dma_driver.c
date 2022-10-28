/**
 * @file dma_driver.c
 * 
 * @author your name (you@domain.com)
 * 
 * @brief Direct Memory Access (DMA) functions 
 * 
 * @version 0.1
 * @date 2022-10-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//================================================================================
// Includes 

#include "dma_driver.h"

//================================================================================


//================================================================================
// Initialization 
//================================================================================


//================================================================================
// DMA Stream x Configuration Register 

// This register is used to configure the concerned stream 

// DMA channel select 
void dma_chsel(
    DMA_Stream_TypeDef *dma_stream, 
    dma_channel_t channel) 
{
    // Clear the channel then set the desired channel 
    dma_stream->CR &= ~(SET_7 << SHIFT_25); 
    dma_stream->CR |= (channel << SHIFT_25);  
}


// Memory burst transfer configuration 
void dma_mburst(
    DMA_Stream_TypeDef *dma_stream, 
    dma_burst_config_t burst_config)
{
    // Clear the burst setting then set the desired burst configuration 
    dma_stream->CR &= ~(SET_3 << SHIFT_23); 
    dma_stream->CR |= (burst_config << SHIFT_23); 
}


// Peripheral burst transfer configuration 
void dma_pburst(
    DMA_Stream_TypeDef *dma_stream, 
    dma_burst_config_t burst_config)
{
    // Clear the burst setting then set the desired burst configuration 
    dma_stream->CR &= ~(SET_3 << SHIFT_21); 
    dma_stream->CR |= (burst_config << SHIFT_21); 
}


// Priority level 
void dma_priority(
    DMA_Stream_TypeDef *dma_stream, 
    dma_priority_t priority)
{
    // Clear the priority then set the desired priority level 
    dma_stream->CR &= ~(SET_3 << SHIFT_16); 
    dma_stream->CR |= (priority << SHIFT_16); 
}


// Stream enable 
void dma_stream_enable(
    DMA_Stream_TypeDef *dma_stream)
{
    dma_stream->CR |= (SET_BIT << SHIFT_0); 
}


// Stream disable  
void dma_stream_disable(
    DMA_Stream_TypeDef *dma_stream)
{
    dma_stream->CR &= ~(SET_BIT << SHIFT_0); 
}

//================================================================================
