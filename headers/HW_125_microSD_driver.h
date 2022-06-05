/**
 * @file HW_125_microSD_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 SD card reader driver
 * 
 * @version 0.1
 * @date 2022-05-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HW_125_MICROSD_DRIVER_H_
#define _HW_125_MICROSD_DRIVER_H_


//=======================================================================================
// Includes

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define INDEX_OFFSET 0x40  // First two bits of command index 

#define SPI_TIMEOUT 1000   // Time before an error is thrown 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HW125 command index 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_CMD0  = INDEX_OFFSET + 0x00,  // GO_IDLE_STATE
    HW125_CMD1  = INDEX_OFFSET + 0x01,  // SEND_OP_COND
    HW125_CMD8  = INDEX_OFFSET + 0x08,  // SEND_IF_COND
    HW125_CMD9  = INDEX_OFFSET + 0x09,  // SEND_CSD
    HW125_CMD10 = INDEX_OFFSET + 0x0A,  // SEND_CID
    HW125_CMD12 = INDEX_OFFSET + 0x0C,  // STOP_TRANSMISSION
    HW125_CMD16 = INDEX_OFFSET + 0x10,  // SET_BLOCKLEN
    HW125_CMD17 = INDEX_OFFSET + 0x11,  // READ_SINGLE_BLOCK
    HW125_CMD18 = INDEX_OFFSET + 0x12,  // READ_MULTIPLE_BLOCK
    HW125_CMD23 = INDEX_OFFSET + 0x17,  // SET_BLOCK_COUNT
    HW125_CMD24 = INDEX_OFFSET + 0x18,  // WRITE_BLOCK
    HW125_CMD25 = INDEX_OFFSET + 0x19,  // WRITE_MULTIPLE_BLOCK
    HW125_CMD41 = INDEX_OFFSET + 0x29,  // APP_SEND_OP_COND
    HW125_CMD55 = INDEX_OFFSET + 0x37,  // APP_CMD
    HW125_CMD58 = INDEX_OFFSET + 0x3A   // READ_OCR
} hw125_command_index_t;


/**
 * @brief HW125 card types 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_CT_UNKNOWN = 0x00,   // Unknown card type - failed to read 
    HW125_CT_MMC     = 0x01,   // MMC version 3
    HW125_CT_SDC1    = 0x02,   // SDC version 1
    HW125_CT_SDC2    = 0x04    // SDC version 2
} hw125_card_type_t; 


/**
 * @brief HW125 CRC commands 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_CRC_CMDX = 1,
    HW125_CRC_CMD8 = 0x87,
    HW125_CRC_CMD0 = 0x95
} hw125_crc_cmd_t;

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief HW125 initialization 
 * 
 * @details 
 * 
 */
void hw125_init(void);

//=======================================================================================


#endif  // _HW_125_MICROSD_DRIVER_H_
