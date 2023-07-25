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
 * @brief DMA channel select 
 * 
 * @details Selects the channel for a given DMA stream. Each stream has up to 8 channels 
 *          to choose from, each with a different function. Only one channel can be assigned 
 *          to the stream. <br> 
 *          Note: The channel can only be selected when the stream is disabled 
 * 
 * @see dma_channel_t 
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param channel : channel to assign to the stream 
 */
void dma_chsel(
    DMA_Stream_TypeDef *dma_stream, 
    dma_channel_t channel); 


/**
 * @brief Data transfer direction 
 * 
 * @details This configures the direction that data is transferred by the DMA - i.e. 
 *          memory-to-memory, memory-to-peripheral or peripheral-to-memory. 
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param dir : data transfer direction 
 */
void dma_dir(
    DMA_Stream_TypeDef *dma_stream, 
    dma_direction_t dir); 


/**
 * @brief Circular mode 
 * 
 * @details Configures circular mode. Circular mode makes the DMA transfers start over 
 *          automatically after finishing. If circular mode is not enabled then DMA 
 *          transfers will stop once complete and will require the stream to be re-enabled 
 *          to start a new transfer. 
 * 
 * @see dma_cm_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param cm : circular mode configuration 
 */
void dma_cm(
    DMA_Stream_TypeDef *dma_stream, 
    dma_cm_t cm); 


/**
 * @brief Stream priority 
 * 
 * @details Sets the priority of the DMA stream. Higher priority streams will be served before 
 *          lower priority streams. These bits can only be written when the stream is disabled. 
 * 
 * @see dma_priority_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param priority : stream transfer priority 
 */
void dma_priority(
    DMA_Stream_TypeDef *dma_stream, 
    dma_priority_t priority); 


/**
 * @brief Memory data size 
 * 
 * @details Tells the stream the size of the data being transferred from memory. It is 
 *          recommended to make the memory and peripheral data sizes the same. The memory 
 *          can act as either the source or the destination. 
 * 
 * @see dma_data_size_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param msize : memory data size configuration 
 */
void dma_msize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t msize); 


/**
 * @brief Memory address increment 
 * 
 * @details Defines the behavior of the memory address after transfers. This can either be 
 *          defined as fixed, meaning the memory address doesn't change after a transfer, or 
 *          it can be defined as increment, meaning the address will be incremented after 
 *          each transfer. <br> 
 *          
 *          As an example, in peripheral-to-memory transfers such as ADC scan 
 *          mode using DMA, you want to store more than one conversion in memory to prevent 
 *          a loss of data. This can be done by getting the DMA to increment to the next address 
 *          in a buffer automatically using increment mode. 
 * 
 * @see dma_addr_inc_mode_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param minc : memory address increment configuration 
 */
void dma_minc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t minc); 


/**
 * @brief Peripheral data size 
 * 
 * @details Tells the stream the size of the data being transferred from the peripheral. It is 
 *          recommended to make the memory and peripheral data sizes the same. The peripheral 
 *          can act as either the source or the destination. In memory-to-memory transfers, this 
 *          peripheral configuration acts as the destination memory configuration. 
 * 
 * @see dma_data_size_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param psize : peripheral data size configuration 
 */
void dma_psize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t psize); 


/**
 * @brief Peripheral address increment 
 * 
 * @details Defines the behavior of the peripheral address after transfers. This can either be 
 *          defined as fixed, meaning the peripheral address doesn't change after a transfer, or 
 *          it can be defined as increment, meaning the address will be incremented after 
 *          each transfer. 
 * 
 * @see dma_addr_inc_mode_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param pinc : peripheral address increment configuration 
 */
void dma_pinc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t pinc); 


/**
 * @brief Transfer complete interrupt 
 * 
 * @see dma_tcie_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param tcie : transfer complete interrupt configuration 
 */
void dma_tcie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_tcie_t tcie); 


/**
 * @brief Half transfer interrupt 
 * 
 * @see dma_htie_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param htie : half transfer interrupt configuration 
 */
void dma_htie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_htie_t htie); 


/**
 * @brief Transfer error interrupt 
 * 
 * @see dma_teie_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param teie : transfer error interrupt configuration 
 */
void dma_teie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_teie_t teie); 


/**
 * @brief Direct mode error interrupt 
 * 
 * @see dma_dmeie_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param dmeie : direct mode error interrupt configuration 
 */
void dma_dmeie(
    DMA_Stream_TypeDef *dma_stream,  
    dma_dmeie_t dmeie); 


/**
 * @brief Number of data items to transfer 
 * 
 * @details Tells the stream the number of data items needed to complete the transfer. 
 *          This register can only be written when the stream is disabled. When the stream is 
 *          enabled, this register is read only but can indicate the remaining items to be 
 *          transmitted. The register decrements after each individual data transfer. Once 
 *          the transfer is complete, this register can either stay at zero if in non-circular 
 *          mode, or it can be reloaded automatically with the previosuly programmed value if 
 *          in circular mode. If in non-circular mode, this register gets reloaded by enabling 
 *          the stream again. 
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param data_items : number of data items in a full transfer 
 */
void dma_ndt(
    DMA_Stream_TypeDef *dma_stream, 
    uint16_t data_items); 


/**
 * @brief Peripheral address 
 * 
 * @details Tells the stream the base address of the peripheral data register from/to which the 
 *          data will be read/written if using a peripheral transfer mode. This register can 
 *          only be written to when the stream is disabled. 
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param per_addr : peripheral address 
 */
void dma_par(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr); 


/**
 * @brief Set memory 0 address 
 * 
 * @details Tells the stream the base address of memory area 0 from/to which the data will be 
 *          read/written. These bits can only be written when the stream is disabled. This is 
 *          the first of two memory addresses available. THe second address is only used for 
 *          double buffer mode which is not implemented in this driver. 
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param m0ar : memory 0 address 
 */
void dma_m0ar(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t m0ar); 


/**
 * @brief FIFO error interrupt 
 * 
 * @see dma_feie_t
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param feie : FIFO error interrupt configuration 
 */
void dma_feie(
    DMA_Stream_TypeDef *dma_stream, 
    dma_feie_t feie); 


/**
 * @brief Direct/FIFO mode selection 
 * 
 * @details Configures FIFO mode. If FIFO mode is enabled then the FIFO buffer will be filled 
 *          with data from the source by DMA transfers up to the defined FIFO buffer threshold 
 *          at which point the FIFO will be drained to the destination. If not enabled, the 
 *          stream will be in direct mode where the FIFO buffer is drained immediately after 
 *          a data item enters it. <br> 
 *          
 *          If memory-to-memory mode is selected and the stream is enabled then FIFO mode is 
 *          automatically enabled as direct mode is not allowed in this circumstance. This mode 
 *          can only be written when the stream is disabled. 
 * 
 * @see dma_fifo_mode_t
 * @see dma_fth
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param mode : FIFO mode configuration 
 */
void dma_dmdis(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_mode_t mode); 


/**
 * @brief FIFO threshold selection 
 * 
 * @details Configures the threshold of the FIFO buffer. When the buffer reaches this 
 *          threshold then the data within it gets drained. This functionality is only used 
 *          in FIFO mode. These bits can only be written when the stream is disabled. 
 * 
 * @see dma_fifo_threshold_t
 * @see dma_dmdis
 * 
 * @param dma_stream : pointer to DMA port stream being configured 
 * @param fth : FIFO threshold configuration 
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
    {
        RCC->AHB1ENR |= (SET_BIT << SHIFT_21); 
    }
    else if (dma == DMA2)
    {
        RCC->AHB1ENR |= (SET_BIT << SHIFT_22); 
    }
    
    // Disable the stream 
    dma_stream_disable(dma_stream); 
    
    // Clear all the stream interrupt flags LISR and HISR registers 
    dma_clear_int_flags(dma); 
    
    // Select the DMA channel 
    dma_chsel(dma_stream, channel); 

    // Configure the data transfer direction 
    dma_dir(dma_stream, dir); 

    // Configure circular mode 
    dma_cm(dma_stream, cm); 
    
    // Configure the stream priority 
    dma_priority(dma_stream, priority); 
    
    // Configure increment/fixed memory mode 
    dma_minc(dma_stream, minc);     // Memory increment mode 
    dma_pinc(dma_stream, pinc);     // Peripheral increment mode 
    
    // Configure data widths 
    dma_msize(dma_stream, msize);   // Memory data width 
    dma_psize(dma_stream, psize);   // Peripheral data width 
} 


// Configure the DMA stream 
void dma_stream_config(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr, 
    uint32_t mem_addr, 
    uint16_t data_items)
{
    // Set the peripheral port address 
    dma_par(dma_stream, per_addr); 

    // Set the memory address and subsequently double buffer mode if needed 
    dma_m0ar(dma_stream, mem_addr);
    
    // Configure the total number of data items to be transferred 
    dma_ndt(dma_stream, data_items); 
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
    uint32_t *lo_streams, 
    uint32_t *hi_streams)
{
    // Record all stream interrupt flags 
    *lo_streams = dma->LISR; 
    *hi_streams = dma->HISR; 

    // Clear all flags 
    dma_clear_int_flags(dma); 
}

//================================================================================


//================================================================================
// DMA Stream x Configuration Register 

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
    dma_stream->CR &= ~(SET_7 << SHIFT_25); 
    dma_stream->CR |= (channel << SHIFT_25);  
}


// Data transfer direction 
void dma_dir(
    DMA_Stream_TypeDef *dma_stream, 
    dma_direction_t dir)
{
    dma_stream->CR &= ~(SET_3 << SHIFT_6); 
    dma_stream->CR |= (dir << SHIFT_6); 
}


// Circular mode 
void dma_cm(
    DMA_Stream_TypeDef *dma_stream, 
    dma_cm_t cm)
{
    dma_stream->CR &= ~(SET_BIT << SHIFT_8); 
    dma_stream->CR |= (cm << SHIFT_8); 
}


// Priority level 
void dma_priority(
    DMA_Stream_TypeDef *dma_stream, 
    dma_priority_t priority)
{
    dma_stream->CR &= ~(SET_3 << SHIFT_16); 
    dma_stream->CR |= (priority << SHIFT_16); 
}


// Memory data size 
void dma_msize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t msize)
{
    dma_stream->CR &= ~(SET_3 << SHIFT_13); 
    dma_stream->CR |= (msize << SHIFT_13); 
}


// Memory increment mode 
void dma_minc(
    DMA_Stream_TypeDef *dma_stream, 
    dma_addr_inc_mode_t minc)
{
    dma_stream->CR &= ~(SET_BIT << SHIFT_10); 
    dma_stream->CR |= (minc << SHIFT_10); 
}


// Peripheral data size 
void dma_psize(
    DMA_Stream_TypeDef *dma_stream, 
    dma_data_size_t psize)
{
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


// 
uint16_t dma_ndt_read(
    DMA_Stream_TypeDef *dma_stream)
{
    return dma_stream->NDTR; 
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
