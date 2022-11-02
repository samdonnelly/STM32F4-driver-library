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

// Initialize a DMA peripheral 
void dma_port_init(
    DMA_TypeDef *dma)
{
    // Enable the DMA clock
    if (dma == DMA1)
        RCC->AHB1ENR |= (SET_BIT << SHIFT_21); 
    else if (dma == DMA2)
        RCC->AHB1ENR |= (SET_BIT << SHIFT_22); 
}

// Configure a DMA stream 
void dma_stream_init(
    DMA_Stream_TypeDef *dma_stream)
{
    // Disable the stream 
    
    // Read the EN bit until the stream reads as disabled 
    
    // Clear all the stream dedicated bits in the LISR and HISR registers 
    
    // Set the peripheral port address 
    
    // Set the memory address 
    
    // Configure the total number of data items to be transferred 
    
    // Select the DMA channel 
    
    // Configure flow control if necessary 
    
    // Configure the stream priority 
    
    // COnfigure the FIFO usage 
    
    // Configure the direction 
    
    // Configure increment/fixed memory mode 
    
    // Configure the burst 
    
    // Configure data widths 
    
    // Configure circular or double buffer mode if necessary 
}

//================================================================================


//================================================================================
// DMA interrupt status registers 

// Transfer complete interrupt flag 
uint8_t dma_tcif(
    DMA_TypeDef *dma, 
    dma_stream_t stream)
{
    // Local variables 
    uint8_t status = 0; 

    // Read the flag status then clear the flag 
    switch(stream)
    {
        case DMA_STREAM_0: 
            status = (dma->LISR & (SET_BIT << SHIFT_5)); 
            dma->LIFCR |= (SET_BIT << SHIFT_5); 
            break; 

        case DMA_STREAM_1: 
            status = (dma->LISR & (SET_BIT << SHIFT_11)); 
            dma->LIFCR |= (SET_BIT << SHIFT_11); 
            break;

        case DMA_STREAM_2:
            status = (dma->LISR & (SET_BIT << SHIFT_21));
            dma->LIFCR |= (SET_BIT << SHIFT_21);  
            break; 

        case DMA_STREAM_3:
            status = (dma->LISR & (SET_BIT << SHIFT_27));
            dma->LIFCR |= (SET_BIT << SHIFT_27);  
            break;

        case DMA_STREAM_4:
            status = (dma->HISR & (SET_BIT << SHIFT_5));
            dma->HIFCR |= (SET_BIT << SHIFT_5);  
            break; 

        case DMA_STREAM_5:
            status = (dma->HISR & (SET_BIT << SHIFT_11));
            dma->HIFCR |= (SET_BIT << SHIFT_11);  
            break;
        
        case DMA_STREAM_6:
            status = (dma->HISR & (SET_BIT << SHIFT_21));
            dma->HIFCR |= (SET_BIT << SHIFT_21);  
            break; 

        case DMA_STREAM_7:
            status = (dma->HISR & (SET_BIT << SHIFT_27));
            dma->HIFCR |= (SET_BIT << SHIFT_27);  
            break;

        default: 
            break;
    }

    return status; 
}


// Transfer error interrupt flag 
uint8_t dma_teif(
    DMA_TypeDef *dma, 
    dma_stream_t stream)
{
    // Local variables 
    uint8_t status = 0; 

    switch(stream)
    {
        case DMA_STREAM_0: 
            status = (dma->LISR & (SET_BIT << SHIFT_3)); 
            dma->LIFCR |= (SET_BIT << SHIFT_3); 
            break; 

        case DMA_STREAM_1: 
            status = (dma->LISR & (SET_BIT << SHIFT_9)); 
            dma->LIFCR |= (SET_BIT << SHIFT_9); 
            break;

        case DMA_STREAM_2:
            status = (dma->LISR & (SET_BIT << SHIFT_19)); 
            dma->LIFCR |= (SET_BIT << SHIFT_19); 
            break; 

        case DMA_STREAM_3:
            status = (dma->LISR & (SET_BIT << SHIFT_25)); 
            dma->LIFCR |= (SET_BIT << SHIFT_25); 
            break;

        case DMA_STREAM_4:
            status = (dma->HISR & (SET_BIT << SHIFT_3)); 
            dma->HIFCR |= (SET_BIT << SHIFT_3); 
            break; 

        case DMA_STREAM_5:
            status = (dma->HISR & (SET_BIT << SHIFT_9)); 
            dma->HIFCR |= (SET_BIT << SHIFT_9); 
            break;
        
        case DMA_STREAM_6:
            status = (dma->HISR & (SET_BIT << SHIFT_19)); 
            dma->HIFCR |= (SET_BIT << SHIFT_19); 
            break; 

        case DMA_STREAM_7:
            status = (dma->HISR & (SET_BIT << SHIFT_25)); 
            dma->HIFCR |= (SET_BIT << SHIFT_25); 
            break;

        default: 
            break;
    }

    return status; 
}


// Direct mode error interrupt flag 
uint8_t dma_dmeif(
    DMA_TypeDef *dma, 
    dma_stream_t stream)
{
    // Local variables 
    uint8_t status = 0; 

    switch(stream)
    {
        case DMA_STREAM_0: 
            status = (dma->LISR & (SET_BIT << SHIFT_2)); 
            dma->LIFCR |= (SET_BIT << SHIFT_2); 
            break; 

        case DMA_STREAM_1: 
            status = (dma->LISR & (SET_BIT << SHIFT_8)); 
            dma->LIFCR |= (SET_BIT << SHIFT_8); 
            break;

        case DMA_STREAM_2:
            status = (dma->LISR & (SET_BIT << SHIFT_18)); 
            dma->LIFCR |= (SET_BIT << SHIFT_18); 
            break; 

        case DMA_STREAM_3:
            status = (dma->LISR & (SET_BIT << SHIFT_24)); 
            dma->LIFCR |= (SET_BIT << SHIFT_24); 
            break;

        case DMA_STREAM_4:
            status = (dma->HISR & (SET_BIT << SHIFT_2)); 
            dma->HIFCR |= (SET_BIT << SHIFT_2); 
            break; 

        case DMA_STREAM_5:
            status = (dma->HISR & (SET_BIT << SHIFT_8)); 
            dma->HIFCR |= (SET_BIT << SHIFT_8); 
            break;
        
        case DMA_STREAM_6:
            status = (dma->HISR & (SET_BIT << SHIFT_18)); 
            dma->HIFCR |= (SET_BIT << SHIFT_18); 
            break; 

        case DMA_STREAM_7:
            status = (dma->HISR & (SET_BIT << SHIFT_24)); 
            dma->HIFCR |= (SET_BIT << SHIFT_24); 
            break;

        default: 
            break;
    }

    return status; 
}


// FIFO error interrupt flag 
uint8_t dma_feif(
    DMA_TypeDef *dma, 
    dma_stream_t stream)
{
    // Local variables 
    uint8_t status = 0; 

    switch(stream)
    {
        case DMA_STREAM_0: 
            status = (dma->LISR & (SET_BIT << SHIFT_0)); 
            dma->LIFCR |= (SET_BIT << SHIFT_0); 
            break; 

        case DMA_STREAM_1: 
            status = (dma->LISR & (SET_BIT << SHIFT_6)); 
            dma->LIFCR |= (SET_BIT << SHIFT_6); 
            break;

        case DMA_STREAM_2:
            status = (dma->LISR & (SET_BIT << SHIFT_16)); 
            dma->LIFCR |= (SET_BIT << SHIFT_16); 
            break; 

        case DMA_STREAM_3:
            status = (dma->LISR & (SET_BIT << SHIFT_22)); 
            dma->LIFCR |= (SET_BIT << SHIFT_22); 
            break;

        case DMA_STREAM_4:
            status = (dma->HISR & (SET_BIT << SHIFT_0)); 
            dma->HIFCR |= (SET_BIT << SHIFT_0); 
            break; 

        case DMA_STREAM_5:
            status = (dma->HISR & (SET_BIT << SHIFT_6)); 
            dma->HIFCR |= (SET_BIT << SHIFT_6); 
            break;
        
        case DMA_STREAM_6:
            status = (dma->HISR & (SET_BIT << SHIFT_16)); 
            dma->HIFCR |= (SET_BIT << SHIFT_16); 
            break; 

        case DMA_STREAM_7:
            status = (dma->HISR & (SET_BIT << SHIFT_22)); 
            dma->HIFCR |= (SET_BIT << SHIFT_22); 
            break;

        default: 
            break;
    }

    return status; 
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
    dma_stream->CR &= ~(SET_BIT << SHIFT_0); 
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
    dma_stream->CR |= ~(dir << SHIFT_6); 
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


// Memory data size 
void dma_msize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t msize)
{
    // Clear the memory data size then set the desired value 
    dma_stream->CR &= ~(SET_3 << SHIFT_13); 
    dma_stream->CR |= ~(msize << SHIFT_13); 
}


// Memory increment mode 
void dma_minc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t minc)
{
    // Clear the memory increment mode then set the desired value 
    dma_stream->CR &= ~(SET_BIT << SHIFT_10); 
    dma_stream->CR |= ~(minc << SHIFT_10); 
}


// Peripheral data size 
void dma_psize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t psize)
{
    // Clear the peripheral data size then set the desired value 
    dma_stream->CR &= ~(SET_3 << SHIFT_11); 
    dma_stream->CR |= ~(psize << SHIFT_11); 
}


// Peripheral increment mode 
void dma_pinc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t pinc)
{
    // Clear the peripheral increment mode then set the desired value 
    dma_stream->CR &= ~(SET_BIT << SHIFT_10); 
    dma_stream->CR |= ~(pinc << SHIFT_10); 
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


// Set memory 1 base address 
void dma_m1ar(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t m1ar)
{
    dma_stream->M1AR = m1ar; 
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


// Direct mode disable 
void dma_dm_disable(
    DMA_Stream_TypeDef *dma_stream)
{
    dma_stream->FCR |= (SET_BIT << SHIFT_2); 
}


// Direct mode enable 
void dma_dm_enable(
    DMA_Stream_TypeDef *dma_stream)
{
    dma_stream->FCR &= ~(SET_BIT << SHIFT_2); 
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
