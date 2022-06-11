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

// Command values 
#define HW125_INDEX_OFFSET 0x40  // First two bits of command index 

// Timers 
#define HW125_INIT_TIMER     1000  // Initiate initialization counter 
#define HW125_INIT_DELAY     1     // time delay in ms for initiate initialization sequence
#define HW125_POWER_ON_TIMER 10    // Power on sequence counter 
#define HW125_POWER_ON_DELAY 1     // time delay in ms for power on sequence

// Data information 
#define HW125_DATA_HIGH 0xFF       // DI/MOSI setpoint and DO/MISO response value 
#define HW125_TRAIL_RESP_BYTES 4   // Number of bytes in an R3/R7 response after receiving R1 

// Command response values
#define HW125_INIT_STATE   0x00   // SD card has initiated initialization 
#define HW125_IDLE_STATE   0x01   // SD card is in the idle state 
#define HW125_CCS_SET      0x40   // CCS bit location in OCR 
#define HW125_CMD8_R7_RESP 0x1AA  // SDCV2 return value from CMD8 

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
    HW125_CMD0  = HW125_INDEX_OFFSET + 0x00,  // GO_IDLE_STATE
    HW125_CMD1  = HW125_INDEX_OFFSET + 0x01,  // SEND_OP_COND
    HW125_CMD8  = HW125_INDEX_OFFSET + 0x08,  // SEND_IF_COND
    HW125_CMD9  = HW125_INDEX_OFFSET + 0x09,  // SEND_CSD
    HW125_CMD10 = HW125_INDEX_OFFSET + 0x0A,  // SEND_CID
    HW125_CMD12 = HW125_INDEX_OFFSET + 0x0C,  // STOP_TRANSMISSION
    HW125_CMD16 = HW125_INDEX_OFFSET + 0x10,  // SET_BLOCKLEN
    HW125_CMD17 = HW125_INDEX_OFFSET + 0x11,  // READ_SINGLE_BLOCK
    HW125_CMD18 = HW125_INDEX_OFFSET + 0x12,  // READ_MULTIPLE_BLOCK
    HW125_CMD23 = HW125_INDEX_OFFSET + 0x17,  // SET_BLOCK_COUNT
    HW125_CMD24 = HW125_INDEX_OFFSET + 0x18,  // WRITE_BLOCK
    HW125_CMD25 = HW125_INDEX_OFFSET + 0x19,  // WRITE_MULTIPLE_BLOCK
    HW125_CMD41 = HW125_INDEX_OFFSET + 0x29,  // APP_SEND_OP_COND
    HW125_CMD55 = HW125_INDEX_OFFSET + 0x37,  // APP_CMD
    HW125_CMD58 = HW125_INDEX_OFFSET + 0x3A   // READ_OCR
} hw125_command_index_t;


/**
 * @brief HW125 card types 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_CT_UNKNOWN     = 0x00,   // Unknown card type - failed to read 
    HW125_CT_MMC         = 0x01,   // MMC version 3
    HW125_CT_SDC1        = 0x02,   // SDC version 1
    HW125_CT_SDC2_BLOCK  = 0x04,   // SDC version 2 - block address 
    HW125_CT_SDC2_BYTE   = 0x05    // SDC version 2 - byte address 
} hw125_card_type_t; 


/**
 * @brief HW125 arguments
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_ARG_NONE  = 0x00000000,  // Zero argument 
    HW125_ARG_SUPV  = 0x000001AA,  // Check supply voltage range 
    HW125_ARG_BL512 = 0x00000200,  // Block length 512 bytes 
    HW125_ARG_HCS   = 0x40000000   // HCS bit set 
} hw125_args_t;


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
 * @param hw125_slave_pin 
 * @return uint8_t 
 */
uint8_t hw125_init(uint16_t hw125_slave_pin);


/**
 * @brief HW125 power on sequence
 * 
 * @details 
 * 
 * @param hw125_slave_pin 
 */
void hw125_power_on(uint16_t hw125_slave_pin);


/**
 * @brief HW125 initiate initialization sequence
 * 
 * @details 
 * 
 * @param cmd 
 * @param arg 
 * @param resp 
 * @return uint8_t 
 */
uint8_t hw125_initiate_init(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t *resp);

//=======================================================================================


//=======================================================================================
// Read and write

/**
 * @brief HW125 send command messages and return response values 
 * 
 * @details 
 * 
 * @param cmd 
 * @param arg 
 * @param crc 
 */
void hw125_send_cmd(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t  crc,
    uint8_t *resp);


/**
 * @brief HW125 ready to receive commands 
 * 
 * @details 
 * 
 */
void hw125_ready_rec(void);

//=======================================================================================


#endif  // _HW_125_MICROSD_DRIVER_H_
