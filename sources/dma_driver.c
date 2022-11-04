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

// Initialize the DMA stream 
void dma_stream_init(
    DMA_TypeDef *dma, 
    DMA_Stream_TypeDef *dma_stream, 
    dma_channel_t channel, 
    dma_direction_t dir, 
    dma_cm_t cm, 
    dma_priority_t priority, 
    dma_addr_inc_mode_t minc, 
    dma_addr_inc_mode_t pinc, 
    dma_data_size_t msize, 
    dma_data_size_t psize, 
    dma_fifo_threshold_t fifo_thresh, 
    dma_fifo_mode_t fifo_mode)
{
    // Enable the DMA clock
    if (dma == DMA1)
        RCC->AHB1ENR |= (SET_BIT << SHIFT_21); 
    else if (dma == DMA2)
        RCC->AHB1ENR |= (SET_BIT << SHIFT_22); 
    
    // Disable the stream 
    dma_stream_disable(dma_stream); 
    
    // Clear all the stream interrupt flags LISR and HISR registers 
    dma_clear_int_flags(dma); 
    
    // Select the DMA channel 
    dma_chsel(dma_stream, channel); 

    // Configure the direction 
    dma_dir(dma_stream, dir); 

    // Configure circular 
    dma_cm(dma_stream, cm); 
    
    // Configure the stream priority 
    dma_priority(dma_stream, priority); 
    
    // Configure increment/fixed memory mode 
    dma_minc(dma_stream, minc); 
    dma_pinc(dma_stream, pinc); 
    
    // Configure data widths 
    dma_msize(dma_stream, msize); 
    dma_psize(dma_stream, psize); 

    // Configure the FIFO usage 
    dma_fth(dma_stream, fifo_thresh); 
    dma_dmdis(dma_stream, fifo_mode); 
} 


// Configure the DMA stream 
void dma_stream_config(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr, 
    uint32_t mem_addr, 
    uint16_t data_items)
{
    // Configure the total number of data items to be transferred 
    dma_ndt(dma_stream, data_items); 

    // Set the peripheral port address 
    dma_par(dma_stream, per_addr); 

    // Set the memory address and subsequently double buffer mode if needed 
    dma_m0ar(dma_stream, mem_addr); 

    // Enable the DMA stream 
    dma_stream_enable(dma_stream); 
}

//================================================================================


//================================================================================
// DMA interrupt status registers 

// Clear all the stream interrupt flags LISR and HISR registers 
void dma_clear_int_flags(
    DMA_TypeDef *dma)
{
    dma->LIFCR = ~((uint32_t)CLEAR); 
    dma->HIFCR = ~((uint32_t)CLEAR); 
}


// Read the stream interrupt flags 
uint32_t dma_read_int_flags(
    DMA_TypeDef *dma, 
    dma_stream_t stream)
{
    if (stream < DMA_STREAM_4)
        return dma->LISR; 
    else 
        return dma->HISR; 
}

//================================================================================


//================================================================================
// DMA Stream x Configuration Register 

// This register is used to configure the concerned stream 

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
    // Disable the stream 
    dma_stream->CR &= ~(SET_BIT << SHIFT_0); 

    // Read the EN bit until the stream reads as disabled 
    while(dma_stream_status(dma_stream)); 
}


// Stream status 
uint8_t dma_stream_status(
    DMA_Stream_TypeDef *dma_stream)
{
    return (dma_stream->CR & (SET_BIT << SHIFT_0)); 
}


// DMA channel select 
void dma_chsel(
    DMA_Stream_TypeDef *dma_stream, 
    dma_channel_t channel) 
{
    // Clear the channel then set the desired channel 
    dma_stream->CR &= ~(SET_7 << SHIFT_25); 
    dma_stream->CR |= (channel << SHIFT_25);  
}


// Data transfer direction 
void dma_dir(
    DMA_Stream_TypeDef *dma_stream, 
    dma_direction_t dir)
{
    // Clear the data transfer direction then set the desired value 
    dma_stream->CR &= ~(SET_3 << SHIFT_6); 
    dma_stream->CR |= (dir << SHIFT_6); 
}


// Circular mode 
void dma_cm(
    DMA_Stream_TypeDef *dma_stream, 
    dma_cm_t cm)
{
    // Clear the circular mode setting then set the desired circular configuration 
    dma_stream->CR &= ~(SET_BIT << SHIFT_8); 
    dma_stream->CR |= (cm << SHIFT_8); 
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


// Memory data size 
void dma_msize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t msize)
{
    // Clear the memory data size then set the desired value 
    dma_stream->CR &= ~(SET_3 << SHIFT_13); 
    dma_stream->CR |= (msize << SHIFT_13); 
}


// Memory increment mode 
void dma_minc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t minc)
{
    // Clear the memory increment mode then set the desired value 
    dma_stream->CR &= ~(SET_BIT << SHIFT_10); 
    dma_stream->CR |= (minc << SHIFT_10); 
}


// Peripheral data size 
void dma_psize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t psize)
{
    // Clear the peripheral data size then set the desired value 
    dma_stream->CR &= ~(SET_3 << SHIFT_11); 
    dma_stream->CR |= (psize << SHIFT_11); 
}


// Peripheral increment mode 
void dma_pinc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t pinc)
{
    // Clear the peripheral increment mode then set the desired value 
    dma_stream->CR &= ~(SET_BIT << SHIFT_9); 
    dma_stream->CR |= (pinc << SHIFT_9); 
}

//================================================================================


//================================================================================
// DMA Stream x Number of Data Register 

// Number of data items to transfer 
void dma_ndt(
    DMA_Stream_TypeDef *dma_stream, 
    uint16_t data_items)
{
    dma_stream->NDTR = data_items; 
}

//================================================================================


//================================================================================
// DMA Stream x Peripheral Address Register 

// Peripheral address 
void dma_par(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr)
{
    dma_stream->PAR = per_addr; 
}
//================================================================================


//================================================================================
// DMA Stream x Memory Address Registers 

// Set memory 0 base address 
void dma_m0ar(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t m0ar)
{
    dma_stream->M0AR = m0ar; 
}

//================================================================================


//================================================================================
// DMA Stream x FIFO Control Register 

// Read the FIFO status 
FIFO_STATUS dma_fs(
    DMA_Stream_TypeDef *dma_stream)
{
    return ((dma_stream->FCR & (SET_7 << SHIFT_3)) >> SHIFT_3); 
}


// Direct/FIFO mode selection 
void dma_dmdis(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_mode_t mode)
{
    // Clear the mode then set the desired mode 
    dma_stream->FCR &= ~(SET_BIT << SHIFT_2); 
    dma_stream->FCR |= (mode << SHIFT_2); 
}


// FIFO threshold selection 
void dma_fth(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_threshold_t fth)
{
    // Clear the FIFO threshold selection then set the specified value 
    dma_stream->FCR &= ~(SET_3 << SHIFT_0); 
    dma_stream->FCR |= (fth << SHIFT_0); 
}

//================================================================================
