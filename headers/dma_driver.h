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
    DMA_CHNL_0,   // 
    DMA_CHNL_1,   // 
    DMA_CHNL_2,   // 
    DMA_CHNL_3,   // 
    DMA_CHNL_4,   // 
    DMA_CHNL_5,   // 
    DMA_CHNL_6,   // 
    DMA_CHNL_7    // 
} dma_channel_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_STREAM_0,    // 
    DMA_STREAM_1,    // 
    DMA_STREAM_2,    // 
    DMA_STREAM_3,    // 
    DMA_STREAM_4,    // 
    DMA_STREAM_5,    // 
    DMA_STREAM_6,    // 
    DMA_STREAM_7     // 
} dma_stream_t; 


/**
 * @brief Priority level 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_PRIOR_LOW,    // Low priority 
    DMA_PRIOR_MED,    // Medium priority 
    DMA_PRIOR_HI,     // High priority
    DMA_PRIOR_VHI,    // Very high priority 
} dma_priority_t; 


/**
 * @brief Data transfer direction 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_DIR_PM,   // Peripheral-to-memory 
    DMA_DIR_MP,   // Memory-to-peripheral 
    DMA_DIR_MM    // Memory-to-memory 
} dma_direction_t; 


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


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_DATA_SIZE_BYTE,  // Byte (8-bits) 
    DMA_DATA_SIZE_HALF,  // Half-word (16-bits) 
    DMA_DATA_SIZE_WORD   // Word (32-bits) 
} dma_data_size_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_ADDR_FIXED,      // Address pointer is fixed 
    DMA_ADDR_INCREMENT   // Address pointer is incremented after each data transfer 
} dma_addr_inc_mode_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_FIFO_STAT_0,       // 
    DMA_FIFO_STAT_1,       // 
    DMA_FIFO_STAT_2,       // 
    DMA_FIFO_STAT_3,       // 
    DMA_FIFO_STAT_EMPTY,   // 
    DMA_FIFO_STAT_FULL,    // 
} dma_fifo_status_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_FTH_0,       // 0 <= FIFO Level < 1/4 
    DMA_FTH_1,       // 1/4 <= FIFO Level < 1/2 
    DMA_FTH_2,       // 1/2 <= FIFO Level < 3/4 
    DMA_FTH_3,       // 3/4 <= FIFO Level < FULL 
    DMA_FTH_EMPTY,   // Empty 
    DMA_FTH_FULL,    // Full 
    DMA_FTH_NA       // Non-applicable --> Used for direct mode config 
} dma_fifo_threshold_t; 



typedef enum {
    DMA_DIRECT_MODE,  // Direct mode (no FIFO threshold used) 
    DMA_FIFO_MODE     // FIFO mode 
} dma_fifo_mode_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_DBM_DISABLE,   // Disable double buffer mode 
    DMA_DBM_ENABLE     // Enable double buffer mode 
} dma_dbm_t; 


/**
 * @brief 
 * 
 * @detaiils 
 * 
 */
typedef enum {
    DMA_CM_DISABLE,   // Circular mode disabled 
    DMA_CM_ENABLE     // Circualr mode enabled 
} dma_cm_t; 


/**
 * @brief 
 * 
 * @details 
 * 
 */
typedef enum {
    DMA_FLOW_CTL_DMA,   // The DMA is the flow controller 
    DMA_FLOW_CTL_PER    // The peripheral is the flow controller 
} dma_flow_ctl_t; 

//================================================================================


//================================================================================
// Data Types 

typedef dma_fifo_status_t FIFO_STATUS; 

//================================================================================


//================================================================================
// Initialization 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param dma 
 */
void dma_port_init(
    DMA_TypeDef *dma); 



/**
 * @brief 
 * 
 * @details 
 * 
 * @param dam : 
 * @param dma_stream : 
 * @param per_addr : 
 * @param dbm : 
 * @param m0_addr : 
 * @param m1_addr : 
 * @param data_items : 
 * @param channel : 
 * @param flow_ctl : 
 * @param priority : 
 * @param fifo_thresh : 
 * @param dir : 
 * @param minc : 
 * @param pinc : 
 * @param mburst : 
 * @param pburst : 
 * @param msize : 
 * @param psize : 
 * @param cm : 
 * 
 */
void dma_stream_init(
    DMA_TypeDef *dma, 
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr, 
    dma_dbm_t dbm, 
    uint32_t m0_addr, 
    uint32_t m1_addr, 
    uint16_t data_items, 
    dma_channel_t channel, 
    dma_flow_ctl_t flow_ctl, 
    dma_priority_t priority, 
    dma_fifo_threshold_t fifo_thresh, 
    dma_fifo_mode_t fifo_mode, 
    dma_direction_t dir, 
    dma_addr_inc_mode_t minc, 
    dma_addr_inc_mode_t pinc, 
    dma_burst_config_t mburst, 
    dma_burst_config_t pburst, 
    dma_data_size_t msize, 
    dma_data_size_t psize, 
    dma_cm_t cm); 

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
 * @param stream 
 * @return uint32_t 
 */
uint32_t dma_read_int_flags(
    DMA_TypeDef *dma, 
    dma_stream_t stream); 

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
 * 
 * @param dma_stream 
 * @param dbm 
 */
void dma_dbm(
    DMA_Stream_TypeDef *dma_stream, 
    dma_dbm_t dbm);


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
 * @brief Peripheral flow control 
 * 
 * @details 
 * 
 * @param dma_stream 
 * @param flow_ctl 
 */
void dma_flow_ctl(
    DMA_Stream_TypeDef *dma_stream, 
    dma_flow_ctl_t flow_ctl); 


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

//================================================================================


//================================================================================
// DMA Stream x Number of Data Register 

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

//================================================================================


//================================================================================
// DMA Stream x Peripheral Address Register 

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

//================================================================================


//================================================================================
// DMA Stream x Memory Address Register 

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
 * @brief Set memory 1 address 
 * 
 * @details 
 *          Base address of memory area 1 from/to which the data will be read/written. 
 *          This register is used only in double buffer mode. 
 *          These bits can only be written when the stream is disabled. 
 * 
 * @param dma_stream 
 * @param m1ar 
 */
void dma_m1ar(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t m1ar); 

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


/**
 * @brief Direct mode disable 
 * 
 * @details 
 *          These bits can only be written when the stream is disabled. 
 *          This is set by hardware if memory-to-memory mode is selected and the stream is 
 *          enabled because the direct mode is not allowed in the memory-to-memory 
 *          configuration. 
 * 
 * @param dma_stream 
 */
void dma_dm_disable(
    DMA_Stream_TypeDef *dma_stream); 


/**
 * @brief Direct mode enable 
 * 
 * @details 
 *          These bits can only be written when the stream is disabled. 
 * 
 * @param dma_stream 
 */
void dma_dm_enable(
    DMA_Stream_TypeDef *dma_stream); 


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

#endif   // _DMA_DRIVER_H_
