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
    CMD0  = INDEX_OFFSET + 0x00,  // GO_IDLE_STATE
    CMD1  = INDEX_OFFSET + 0x01,  // SEND_OP_COND
    CMD8  = INDEX_OFFSET + 0x08,  // SEND_IF_COND
    CMD9  = INDEX_OFFSET + 0x09,  // SEND_CSD
    CMD10 = INDEX_OFFSET + 0x0A,  // SEND_CID
    CMD12 = INDEX_OFFSET + 0x0C,  // STOP_TRANSMISSION
    CMD16 = INDEX_OFFSET + 0x10,  // SET_BLOCKLEN
    CMD17 = INDEX_OFFSET + 0x11,  // READ_SINGLE_BLOCK
    CMD18 = INDEX_OFFSET + 0x12,  // READ_MULTIPLE_BLOCK
    CMD23 = INDEX_OFFSET + 0x17,  // SET_BLOCK_COUNT
    CMD24 = INDEX_OFFSET + 0x18,  // WRITE_BLOCK
    CMD25 = INDEX_OFFSET + 0x19,  // WRITE_MULTIPLE_BLOCK
    CMD41 = INDEX_OFFSET + 0x29,  // APP_SEND_OP_COND
    CMD55 = INDEX_OFFSET + 0x37,  // APP_CMD
    CMD58 = INDEX_OFFSET + 0x3A   // READ_OCR
} hw125_command_index_t;


/**
 * @brief HW125 CRC commands 
 * 
 * @details 
 * 
 */
typedef enum {
    CRC_CMDX = 1,
    CRC_CMD8 = 0x87,
    CRC_CMD0 = 0x95
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
