/**
 * @file dma_driver.h
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

#ifndef _DMA_DRIVER_H_ 
#define _DMA_DRIVER_H_ 

//================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

//================================================================================


//================================================================================
// Enums 

/**
 * @brief DMA channel number 
 * 
 * @details DMA ports have streams that they control and each stream can be assigned a 
 *          channel. Each stream can only have one channel assigned but there are up to 7 
 *          channels for the stream to use and the channel dictates what peripheral the 
 *          stream working with. 
 */
typedef enum {
    DMA_CHNL_0, 
    DMA_CHNL_1, 
    DMA_CHNL_2, 
    DMA_CHNL_3, 
    DMA_CHNL_4, 
    DMA_CHNL_5, 
    DMA_CHNL_6, 
    DMA_CHNL_7 
} dma_channel_t; 


/**
 * @brief DMA stream number 
 * 
 * @details Each DMA port has up to 7 possible streams that can be implemented simultaneously. 
 *          The streams can be assigned to certain peripherals using their available channels. 
 */
typedef enum {
    DMA_STREAM_0, 
    DMA_STREAM_1, 
    DMA_STREAM_2, 
    DMA_STREAM_3, 
    DMA_STREAM_4, 
    DMA_STREAM_5, 
    DMA_STREAM_6, 
    DMA_STREAM_7 
} dma_stream_t; 


/**
 * @brief Data transfer direction 
 */
typedef enum {
    DMA_DIR_PM,      // Peripheral-to-memory 
    DMA_DIR_MP,      // Memory-to-peripheral 
    DMA_DIR_MM       // Memory-to-memory 
} dma_direction_t; 


/**
 * @brief DMA circular mode configuration 
 */
typedef enum {
    DMA_CM_DISABLE,   // Circular mode disabled 
    DMA_CM_ENABLE     // Circualr mode enabled 
} dma_cm_t; 


/**
 * @brief Stream priority level 
 */
typedef enum {
    DMA_PRIOR_LOW,    // Low priority 
    DMA_PRIOR_MED,    // Medium priority 
    DMA_PRIOR_HI,     // High priority
    DMA_PRIOR_VHI,    // Very high priority 
} dma_priority_t; 


/**
 * @brief Size of individual piece of data being transferred by the DMA 
 */
typedef enum {
    DMA_DATA_SIZE_BYTE,      // Byte (8-bits) 
    DMA_DATA_SIZE_HALF,      // Half-word (16-bits) 
    DMA_DATA_SIZE_WORD       // Word (32-bits) 
} dma_data_size_t; 


/**
 * @brief Source and destination buffer address behavior 
 */
typedef enum {
    DMA_ADDR_FIXED,      // Address pointer is fixed 
    DMA_ADDR_INCREMENT   // Address pointer is incremented after each data transfer 
} dma_addr_inc_mode_t; 


/**
 * @brief Transfer complete interrupt configuration 
 */
typedef enum {
    DMA_TCIE_DISABLE,   // Transfer complete interrupt disabled 
    DMA_TCIE_ENABLE     // Transfer complete interrupt enabled 
} dma_tcie_t; 


/**
 * @brief Half transfer interrupt configuration 
 */
typedef enum {
    DMA_HTIE_DISABLE,   // Data half transfer interrupt disable 
    DMA_HTIE_ENABLE     // Data half transfer interrupt enable 
} dma_htie_t; 


/**
 * @brief Transfer error interrupt configuration 
 */
typedef enum {
    DMA_TEIE_DISABLE,   // Transfer error interrupt disable 
    DMA_TEIE_ENABLE     // Transfer error interrupt enable 
} dma_teie_t; 


/**
 * @brief Direct mode error interrupt configuration 
 */
typedef enum {
    DMA_DMEIE_DISABLE,   // Direct mode transfer error interrupt disable 
    DMA_DMEIE_ENABLE     // Direct mode transfer error interrupt enable 
} dma_dmeie_t; 


/**
 * @brief FIFO error interrupt configuration 
 */
typedef enum {
    DMA_FEIE_DISABLE,   // Disable the FIFO error interrupt 
    DMA_FEIE_ENABLE     // Enable the FIFO error interrupt 
} dma_feie_t; 


/**
 * @brief FIFO status 
 */
typedef enum {
    DMA_FIFO_STAT_0,         // 0 <= FIFO Level < 1/4 
    DMA_FIFO_STAT_1,         // 1/4 <= FIFO Level < 1/2 
    DMA_FIFO_STAT_2,         // 1/2 <= FIFO Level < 3/4 
    DMA_FIFO_STAT_3,         // 3/4 <= FIFO Level < FULL 
    DMA_FIFO_STAT_EMPTY,     // Empty 
    DMA_FIFO_STAT_FULL       // Full 
} dma_fifo_status_t; 


/**
 * @brief FIFO configuration 
 */
typedef enum {
    DMA_DIRECT_MODE,    // Direct mode (no FIFO threshold used) 
    DMA_FIFO_MODE       // FIFO mode 
} dma_fifo_mode_t; 


/**
 * @brief FIFO threshold configuration 
 */
typedef enum {
    DMA_FTH_1QTR,       // 1/4 full FIFO 
    DMA_FTH_HALF,       // 1/2 full FIFO 
    DMA_FTH_3QTR,       // 3/4 full FIFO 
    DMA_FTH_FULL        // Full FIFO 
} dma_fifo_threshold_t; 

//================================================================================


//================================================================================
// Data Types 

typedef dma_fifo_status_t FIFO_STATUS; 

//================================================================================


//================================================================================
// Initialization 

/**
 * @brief Initialize the DMA stream 
 * 
 * @details This function defines the characteristics of a specified stream in a specified 
 *          port. This function is called once for each stream being configured. 
 * 
 * @see dma_channel_t
 * @see dma_direction_t
 * @see dma_cm_t
 * @see dma_priority_t
 * @see dma_addr_inc_mode_t
 * @see dma_data_size_t
 * 
 * @param dma : pointer to DMA port to initialize  
 * @param dma_stream : pointer to DMA port stream to initialize  
 * @param channel : channel of stream to use 
 * @param dir : data transfer direction 
 * @param cm : circular mode configuration 
 * @param priority : stream priority configuration 
 * @param minc : memory increment configuration 
 * @param pinc : peripheral increment configuration 
 * @param msize : memory data size 
 * @param psize : peripheral data size 
 */
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
    dma_data_size_t psize); 


/**
 * @brief Configure the DMA stream 
 * 
 * @details This functions configures the DMA data characteristics such as the number 
 *          of data items in a transfer and the source and destination addresses. At the end 
 *          of the function the stream is enabled. This function is separate from the stream 
 *          initialization function because these data characteristics need to be reconfigured 
 *          in the event of a transfer fault. 
 * 
 * @see dma_stream_init
 * 
 * @param dma_stream : pointer to DMA port stream to configure 
 * @param per_addr : peripheral address 
 * @param mem_addr : memory address 
 * @param data_items : number of items involved in the DMA transfer 
 */
void dma_stream_config(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr, 
    uint32_t mem_addr, 
    uint16_t data_items); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param mode 
 * @param fth 
 * @param feie 
 */
void dma_fifo_config(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_mode_t mode, 
    dma_fifo_threshold_t fth, 
    dma_feie_t feie); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param tcie 
 * @param htie 
 * @param teie 
 * @param dmeie 
 */
void dma_int_config(
    DMA_Stream_TypeDef *dma_stream,  
    dma_tcie_t tcie, 
    dma_htie_t htie, 
    dma_teie_t teie, 
    dma_dmeie_t dmeie); 

//================================================================================


//================================================================================
// DMA interrupt status registers 

/**
 * @brief Clear all flags in all streams 
 * 
 * @details 
 * 
 * @param dma 
 */
void dma_clear_int_flags(
    DMA_TypeDef *dma); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma 
 * @param lo_streams 
 * @param hi_streams 
 */
void dma_int_flags(
    DMA_TypeDef *dma, 
    uint32_t lo_streams, 
    uint32_t hi_streams); 

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

//================================================================================


//================================================================================
// DMA Stream x FIFO Control Register 

/**
 * @brief Read the FIFO status 
 * 
 * @details 
 *          These bits are not relevant in direct mode. 
 * 
 * @param dma_stream 
 * @return FIFO_STATUS 
 */
FIFO_STATUS dma_fs(
    DMA_Stream_TypeDef *dma_stream); 

//================================================================================

#endif   // _DMA_DRIVER_H_
