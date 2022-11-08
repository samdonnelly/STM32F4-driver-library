/**
 * @file dma_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
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
// Function Prototypes 

/**
 * @brief Stream status 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @return uint8_t 
 */
uint8_t dma_stream_status(
    DMA_Stream_TypeDef *dma_stream); 


/**
 * @brief DMA channel select 
 * 
 * @details 
 *          Note: The channel can only be selected when EN=0. 
 * 
 * @param dma_stream 
 * @param channel 
 */
void dma_chsel(
    DMA_Stream_TypeDef *dma_stream, 
    dma_channel_t channel); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param dir 
 */
void dma_dir(
    DMA_Stream_TypeDef *dma_stream, 
    dma_direction_t dir); 


/**
 * @brief Circular mode 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param cm 
 */
void dma_cm(
    DMA_Stream_TypeDef *dma_stream, 
    dma_cm_t cm); 


/**
 * @brief 
 * 
 * @details 
 *          These bits can only be written when EN=0. 
 * 
 * @param dma_stream 
 * @param priority 
 */
void dma_priority(
    DMA_Stream_TypeDef *dma_stream, 
    dma_priority_t priority); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param msize 
 */
void dma_msize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t msize); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param minc 
 */
void dma_minc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t minc); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param psize 
 */
void dma_psize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t psize); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param pinc 
 */
void dma_pinc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t pinc); 


/**
 * @brief Transfer complete interrupt 
 * 
 * @details 
 * 
 * @param tcie : 
 */
void dma_tcie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_tcie_t tcie); 


/**
 * @brief Half transfer interrupt 
 * 
 * @details 
 * 
 * @param htie : 
 */
void dma_htie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_htie_t htie); 


/**
 * @brief Transfer error interrupt 
 * 
 * @details 
 * 
 * @param teie : 
 */
void dma_teie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_teie_t teie); 


/**
 * @brief Direct mode error interrupt 
 * 
 * @details 
 * 
 * @param dmeie : 
 */
void dma_dmeie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_dmeie_t dmeie); 


/**
 * @brief 
 * 
 * @details 
 *          This register can only be written when the stream is disabled. 
 *          When the stream is enabled this register is read only which indicates the 
 *          remaining items to be transmitted. The rgister decrements after each DMA 
 *          transfer. 
 *          Once the transfer is complete, this register can either stay at zero (normal 
 *          mode) or be reloaded automatically with the previosuly programmed value if 
 *          the stream is in circular mode or the stream is enabled again. 
 * 
 * @param dma_stream 
 * @param data_items 
 */
void dma_ndt(
    DMA_Stream_TypeDef *dma_stream, 
    uint16_t data_items); 


/**
 * @brief 
 * 
 * @details 
 *          Base address of the peripheral data register from/to which the data will be 
 *          read/written. 
 *          This register can only be written to when the stream is disabled. 
 * 
 * @param dma_stream 
 * @param per_addr 
 */
void dma_par(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr); 


/**
 * @brief Set memory 0 address 
 * 
 * @details 
 *          Base address of memory area 0 from/to which the data will be read/written. 
 *          These bits can only be written when the stream is disabled. 
 * 
 * @param dma_stream 
 * @param m0ar 
 */
void dma_m0ar(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t m0ar); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param feie 
 */
void dma_feie(
    DMA_Stream_TypeDef *dma_stream, 
    dma_feie_t feie); 


/**
 * @brief Direct/FIFO mode selection 
 * 
 * @details 
 *          These bits can only be written when the stream is disabled. 
 *          This is set by hardware if memory-to-memory mode is selected and the stream is 
 *          enabled because the direct mode is not allowed in the memory-to-memory 
 *          configuration. 
 * 
 * @param dma_stream 
 * @param mode 
 */
void dma_dmdis(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_mode_t mode); 


/**
 * @brief FIFO threshold selection 
 * 
 * @details 
 *          These bits are not used in direct mode. 
 *          These bits can only be written when the stream is disabled. 
 * 
 * @param dma_stream 
 * @param fth 
 */
void dma_fth(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_threshold_t fth); 

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
    dma_data_size_t psize)
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


// Configure the FIFO usage 
void dma_fifo_config(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_mode_t mode, 
    dma_fifo_threshold_t fth, 
    dma_feie_t feie)
{
    // FIFO mode 
    dma_dmdis(dma_stream, mode); 

    // FIFO threshold 
    dma_fth(dma_stream, fth); 

    // FIFO interrupt enable 
    dma_feie(dma_stream, feie); 
}


// Configure the DMA interrupts 
void dma_int_config(
    DMA_Stream_TypeDef *dma_stream,  
    dma_tcie_t tcie, 
    dma_htie_t htie, 
    dma_teie_t teie, 
    dma_dmeie_t dmeie)
{
    // Transfer complete interrupt 
    dma_tcie(dma_stream, tcie); 

    // Half transfer interrupt 
    dma_htie(dma_stream, htie); 

    // Transfer error interrupt 
    dma_teie(dma_stream, teie); 

    // Direct mode error interrupt 
    dma_dmeie(dma_stream, dmeie); 
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
void dma_int_flags(
    DMA_TypeDef *dma, 
    uint32_t lo_streams, 
    uint32_t hi_streams)
{
    // Record all stream interrupt flags 
    lo_streams = dma->LISR; 
    hi_streams = dma->HISR; 

    // Clear all flags 
    dma_clear_int_flags(dma); 
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
    dma_stream->CR &= ~(SET_BIT << SHIFT_9); 
    dma_stream->CR |= (pinc << SHIFT_9); 
}


// Transfer complete interrupt 
void dma_tcie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_tcie_t tcie)
{
    dma_stream->CR &= ~(SET_BIT << SHIFT_4); 
    dma_stream->CR |= (tcie << SHIFT_4);
}


// Half transfer interrupt 
void dma_htie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_htie_t htie)
{
    dma_stream->CR &= ~(SET_BIT << SHIFT_3); 
    dma_stream->CR |= (htie << SHIFT_3); 
}


// Transfer error interrupt 
void dma_teie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_teie_t teie)
{
    dma_stream->CR &= ~(SET_BIT << SHIFT_2); 
    dma_stream->CR |= (teie << SHIFT_2); 
}


// Direct mode error interrupt 
void dma_dmeie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_dmeie_t dmeie)
{
    dma_stream->CR &= ~(SET_BIT << SHIFT_1); 
    dma_stream->CR |= (dmeie << SHIFT_1); 
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

// FIFO error interrupt enable 
void dma_feie(
    DMA_Stream_TypeDef *dma_stream, 
    dma_feie_t feie)
{
    dma_stream->FCR &= ~(SET_BIT << SHIFT_7); 
    dma_stream->FCR |= (feie << SHIFT_7); 
}


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
    dma_stream->FCR &= ~(SET_BIT << SHIFT_2); 
    dma_stream->FCR |= (mode << SHIFT_2); 
}


// FIFO threshold selection 
void dma_fth(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_threshold_t fth)
{
    dma_stream->FCR &= ~(SET_3 << SHIFT_0); 
    dma_stream->FCR |= (fth << SHIFT_0); 
}

//================================================================================
