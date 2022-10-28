/**
 * @file dma_driver.h
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

#ifndef _DMA_DRIVER_H_ 
#define _DMA_DRIVER_H_ 

//================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 

// Other drivers 

//================================================================================


//================================================================================
// Macros 
//================================================================================


//================================================================================
// Enums 

/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_CHANNEL_0,   // 
    DMA_CHANNEL_1,   // 
    DMA_CHANNEL_2,   // 
    DMA_CHANNEL_3,   // 
    DMA_CHANNEL_4,   // 
    DMA_CHANNEL_5,   // 
    DMA_CHANNEL_6,   // 
    DMA_CHANNEL_7    // 
} dma_channel_t; 


/**
 * @brief Priority level 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_PRIORITY_LOW,    // Low priority 
    DMA_PRIORITY_MED,    // Medium priority 
    DMA_PRIORITY_HI,     // High priority
    DMA_PRIORITY_VHI,    // Very high priority 
} dma_priority_t; 


/**
 * @brief 
 * 
 * @detaiils 
 * 
 */
typedef enum {
    DMA_BURST_1,    // Single transfer 
    DMA_BURST_4,    // Increment burst of 4 beats 
    DMA_BURST_8,    // Increment burst of 8 beats 
    DMA_BURST_16,   // Increment burst of 16 beats 
} dma_burst_config_t; 

//================================================================================


//================================================================================
// DMA Stream x Configuration Register 

/**
 * @brief 
 * 
 * @details 
 *          Once enabled, a stream is unable to be configured so don't enable the stream 
 *          until this is complete. 
 *          Before enabling to start a new transfer the event flag corresponding to the stream 
 *          in DMA_LISR and DMA_HISR register must be cleared. 
 * 
 * @param dma_stream 
 */
void dma_stream_enable(
    DMA_Stream_TypeDef *dma_stream); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 */
void dma_stream_disable(
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
 * @brief Memory burst transfer configuration 
 * 
 * @details 
 *          These bits can only be written if EN=0. 
 *          In direct mode, these bits are forced to 0x0 by hardware when EN=1. 
 * 
 * @param dma_stream 
 * @param burst_config 
 */
void dma_mburst(
    DMA_Stream_TypeDef *dma_stream, 
    dma_burst_config_t burst_config); 


/**
 * @brief Peripheral burst transfer configuration 
 * 
 * @details 
 *          These bits can only be written if EN=0. 
 *          In direct mode, these bits are forced to 0x0 by hardware. 
 * 
 * @param dma_stream 
 * @param burst_config 
 */
void dma_pburst(
    DMA_Stream_TypeDef *dma_stream, 
    dma_burst_config_t burst_config); 


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

//================================================================================

#endif   // _DMA_DRIVER_H_
