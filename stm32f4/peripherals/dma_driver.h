/**
 * @file dma_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Direct Memory Access (DMA) driver interface 
 * 
 * @version 0.1
 * @date 2022-10-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _DMA_DRIVER_H_ 
#define _DMA_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
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


// Double buffer mode configuration 
typedef enum {
    DMA_DBM_DISABLE,   // Disable - No buffer switching 
    DMA_DBM_ENABLE     // Enable - memory target swicthed at end of DMA transfer 
} dma_dbm_t; 


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

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief DMA transfer indexing 
 * 
 * @details This is a useful info to record if the data transfer size by DMA is of an 
 *          unknown length. For example, if using a circular buffer to store UART data 
 *          and the received data size is unknown, then this can help update the circular 
 *          buffer index for parsing data. 
 */
typedef struct dma_index_s
{
    uint16_t data_size;   // Size of data transferred 
    uint16_t ndt_old;     // Previous remaining data items to be transferred 
    uint16_t ndt_new;     // Current remaining data items to be transferred 
}
dma_index_t; 

//=======================================================================================


//=======================================================================================
// Data Types 

typedef dma_fifo_status_t FIFO_STATUS; 

//=======================================================================================


//=======================================================================================
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
 * @param dbm : double buffer mode configuration 
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
    dma_dbm_t dbm, 
    dma_addr_inc_mode_t minc, 
    dma_addr_inc_mode_t pinc, 
    dma_data_size_t msize, 
    dma_data_size_t psize); 


/**
 * @brief Configure the DMA stream 
 * 
 * @details This functions configures the DMA data characteristics such as the number 
 *          of data items in a transfer and the source and destination addresses. At the 
 *          end of the function the stream is enabled. This function is separate from the 
 *          stream initialization function because these data characteristics need to be 
 *          reconfigured in the event of a transfer fault. 
 *          
 *          'data_items' is the max number of transfers the DMA does. However, if using 
 *          circular mode then this value automatically resets after the number is 
 *          reached and continues going. This value should match the size of the buffer 
 *          used to store the data transferred by the DMA. In circular mode this value 
 *          still matters because it tells the DMA when to go to the start of the buffer 
 *          again and therefore not exceed allocated memory. If using DMA for something 
 *          such as UART RX, then the buffer (this value) should be large enough to 
 *          accommodate the max data transfer size that will be seen, but a transfer 
 *          smaller than this will stop the memory address increment and be the starting 
 *          point for the next transfer. 
 * 
 * @see dma_stream_init
 * 
 * @param dma_stream : pointer to DMA port stream to configure 
 * @param per_addr : peripheral address 
 * @param mem0_addr : memory 0 address 
 * @param mem1_addr : memory 1 address (used in double buffer mode) 
 * @param data_items : number of items involved in the DMA transfer - see notes above 
 */
void dma_stream_config(
    DMA_Stream_TypeDef *dma_stream, 
    uint32_t per_addr, 
    uint32_t mem0_addr, 
    uint32_t mem1_addr, 
    uint16_t data_items); 


/**
 * @brief Configure FIFO mode 
 * 
 * @details This function allows for configuring the FIFO for each stream. It is separate from 
 *          the other initialization/configuration functions so that it can be ignored if not 
 *          needed. FIFO mode allows for deciding how full to fill the FIFO with DMA data 
 *          from the source before draining the FIFO buffer to the destination. An interrupt 
 *          can also be configured for FIFO mode to indicate when the FIFO has reached it's 
 *          threshold. <br>
 *          
 *          When FIFO mode is not used the system is in direct mode which means data will 
 *          be sent to the destination as soon as it enters the FIFO from the source. 
 * 
 * @see dma_fifo_mode_t
 * @see dma_fifo_threshold_t
 * @see dma_feie_t
 * 
 * @param dma_stream : pointer to the DMA port stream to configure 
 * @param mode : FIFO mode configuration 
 * @param fth : FIFO threshold 
 * @param feie : FIFO interrupt configuration 
 */
void dma_fifo_config(
    DMA_Stream_TypeDef *dma_stream, 
    dma_fifo_mode_t mode, 
    dma_fifo_threshold_t fth, 
    dma_feie_t feie); 


/**
 * @brief Configure DMA interrupts 
 * 
 * @details Allows for configuring what interrupts to use for each DMA stream. Each DMA stream 
 *          will has it's own interrupt handler that it triggers. Interrupts can be enabled 
 *          for full and half transfers as well as transfer errors. 
 * 
 * @see dma_tcie_t
 * @see dma_htie_t
 * @see dma_teie_t
 * @see dma_dmeie_t
 * 
 * @param dma_stream : pointer to the DMA port stream to configure 
 * @param tcie : transfer complete interrupt configuration 
 * @param htie : half transfer interrupt configuration 
 * @param teie : transfer error interrupt configuration 
 * @param dmeie : direct mode error interrupt configuration 
 */
void dma_int_config(
    DMA_Stream_TypeDef *dma_stream,  
    dma_tcie_t tcie, 
    dma_htie_t htie, 
    dma_teie_t teie, 
    dma_dmeie_t dmeie); 

//=======================================================================================


//=======================================================================================
// DMA interrupt status registers 

/**
 * @brief Clear all interrupt flags in all streams 
 * 
 * @details Interrupt flags for all streams are grouped together in a few registers. This 
 *          function clears the interrupts flags for all those registers and subsequently 
 *          all the streams. This function is needed by interrupt handlers in order to exit 
 *          the handler. 
 * 
 * @param dma : pointer to DMA port of which to clear flags 
 */
void dma_clear_int_flags(DMA_TypeDef *dma); 


/**
 * @brief Read the stream interrupt flags 
 * 
 * @details This function reads all the DMA interrupt flags from all streams. This is done 
 *          by reading the high and low stream number interrupt flag registers and storing 
 *          the contents in the arguments passed to the function. It is left to the application 
 *          to parse the register data as needed. 
 * 
 * @param dma : pointer to the DMA port of which to read the interrupt flags 
 * @param lo_streams : buffer to store interrupt flags for streams 0-3 
 * @param hi_streams : buffer to store interrupt flags for streams 4-7 
 */
void dma_int_flags(
    DMA_TypeDef *dma, 
    uint32_t *lo_streams, 
    uint32_t *hi_streams); 


/**
 * @brief Get the transfer complete status 
 * 
 * @param dma : DMA port to check 
 * @param dma_stream : DMA stream to check 
 * @return uint8_t : completion status 
 */
uint8_t dma_get_tc_status(
    DMA_TypeDef *dma, 
    DMA_Stream_TypeDef* dma_stream); 

//=======================================================================================


//=======================================================================================
// DMA Stream x Configuration Register 

/**
 * @brief Stream enable 
 * 
 * @details This functions enables a DMA stream which is required for it to start operating. 
 *          Once enabled, a stream is unable to be configured until it is disabled again. <br>
 *          
 *          Before enabling, in order to start a new transfer, the interrupt flags for the 
 *          given stream must be cleared. This is handled during stream initialization but 
 *          may be require manual clearing if the DMA gets disabled for whatever reason. 
 * 
 * @see dma_stream_disable 
 * @see dma_clear_int_flags
 * 
 * @param dma_stream : pointer to the DMA port stream to enable 
 */
void dma_stream_enable(DMA_Stream_TypeDef *dma_stream); 


/**
 * @brief Stream disable 
 * 
 * @details This function disables the specified DMA stream. This function will wait until 
 *          register feedback that the stream has been disabled before exiting. <br>
 *          
 *          Streams must be disabled to stop their operation and to configure their behavior. 
 * 
 * @param dma_stream : pointer to the DMA port stream to disable 
 */
void dma_stream_disable(DMA_Stream_TypeDef *dma_stream); 


/**
 * @brief Stream status 
 * 
 * @details Reads the stream enable flag. This is used by the stream disable function 
 *          to know when a stream is disabled (bit cleared). The enable flag can be cleared 
 *          by hardware on the DMA end of transfer or if an error occurs. 
 * 
 * @param dma_stream : pointer to DMA port stream in question 
 * @return uint8_t : stream status - 0 = disabled, 1 = enabled 
 */
uint8_t dma_stream_status(DMA_Stream_TypeDef *dma_stream); 

//=======================================================================================


//=======================================================================================
// DMA Stream x Number of Data Register 

/**
 * @brief NDT register read - remaining data items to be transmitted 
 * 
 * @param dma_stream : DMA port to read from 
 * @return uint16_t : NDT register contents 
 */
uint16_t dma_ndt_read(const DMA_Stream_TypeDef *dma_stream); 

//=======================================================================================


//=======================================================================================
// DMA Stream x FIFO Control Register 

/**
 * @brief Read the FIFO status 
 * 
 * @details Reads the current state of the FIFO buffer - i.e. how full the buffer is. 
 *          These bits are not relevant in direct mode. 
 * 
 * @see dma_fifo_status_t
 * 
 * @param dma_stream : pointer to the DMA port stream of the FIFO in question 
 * @return FIFO_STATUS : FIFO buffer level 
 */
FIFO_STATUS dma_fs(DMA_Stream_TypeDef *dma_stream); 

//=======================================================================================


//=======================================================================================
// Data handling 

/**
 * @brief DMA circular buffer indexing 
 * 
 * @details Finds the number of data items transferred by DMA and updates a circular 
 *          buffer head index. This is useful when a circular buffer is being populated 
 *          by the DMA and the DMA data transfer size is unknown. If using this method 
 *          then this should be called after each data transfer or else data may be lost. 
 * 
 * @param dma_stream : DMA port to use 
 * @param dma_index : DMA buffer indexing info 
 * @param cb_index : circular buffer indexing info 
 */
void dma_cb_index(
    const DMA_Stream_TypeDef *dma_stream, 
    dma_index_t *dma_index, 
    cb_index_t *cb_index); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _DMA_DRIVER_H_
