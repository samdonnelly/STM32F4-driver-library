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
#define HW125_R1_RESP_COUNT  5     // Max num of times to read R1 until appropriate response

// Data information 
#define HW125_DATA_HIGH 0xFF       // DI/MOSI setpoint and DO/MISO response value 
#define HW125_TRAIL_RESP_BYTES 4   // Number of bytes in an R3/R7 response after receiving R1 
#define HW125_SINGLE_BYTE 1        // 
#define HW125_NO_BYTE     0        // 
#define HW125_CRC_DISCARD 2        // 

// Command response values
#define HW125_INIT_STATE     0x00   // SD card has initiated initialization 
#define HW125_BEGIN_READ     0x00   // 
#define HW125_END_READ       0x00   // 
#define HW125_IDLE_STATE     0x01   // SD card is in the idle state 
#define HW125_CCS_SET        0x40   // CCS bit location in OCR 
#define HW125_CMD8_R7_RESP   0x1AA  // SDCV2 return value from CMD8 
#define HW125_R1_RESP_FILTER 0x80   // Filter used to determine a valid R1 response 

// Status 
#define HW125_INIT_SUCCESS 0xFE  // Filter to clear the HW125_STATUS_NOINIT flag 

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


/**
 * @brief HW125 disk status 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_STATUS_NOINIT  = 0x01,  // Device has not been initialized and not ready to work 
    HW125_STATUS_NODISK  = 0x02,  // No medium in the drive 
    HW125_STATUS_PROTECT = 0x04   // Medium is write protected 
} hw125_disk_status_t; 


/**
 * @brief HW125 power status 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_PWR_OFF,
    HW125_PWR_ON
} hw125_pwr_status_t;


/**
 * @brief HW125 disk function results 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_RES_OK,       // the function succeeded
    HW125_RES_ERROR,    // an error occured 
    HW125_RES_WRPRT,    // the medium is write protected 
    HW125_RES_NOTRDY,   // device has not been initialized 
    HW125_RES_PARERR    // command code or parameter is invalid 
} hw125_disk_results_t;


/**
 * @brief HW125 data token 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_DT_ZERO = 0xFC,    // Data token for CMD25 
    HW125_DT_ONE  = 0xFD,    // Stop token for CMD25
    HW125_DT_TWO  = 0xFE     // Data token for CMD17/18/24
} hw125_data_token_t; 


/**
 * @brief HW125 data response filters 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_DR_FILTER_ZERO = 0x05,   // Data accepted 
    HW125_DR_FILTER_ONE  = 0x0B,   // Data rejected due to a CRC error 
    HW125_DR_FILTER_TEO  = 0x0D    // Data rejected due to a write error 
} hw125_data_response_filter_t; 


// TODO see if the error token enum is needed 
/**
 * @brief HW125 error token filters 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_ET_FILTER_ERROR = 0x01,    // Error 
    HW125_ET_FILTER_CC    = 0x02,    // CC error 
    HW125_ET_FILTER_ECC   = 0x04,    // Card ECC failed 
    HW125_ET_FILTER_OOR   = 0x08,    // Out of range 
    HW125_ET_FILTER_LOCK  = 0x10     // Card is locked 
} hw125_error_token_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t DISK_STATUS;

typedef hw125_disk_results_t DISK_RESULT; 

//=======================================================================================


//=======================================================================================
// Initialization and status functions 

/**
 * @brief HW125 user init 
 * 
 * @details This functions is called directly by the user and used to set parameters for 
 *          the hw125 driver. The hw125 driver functions (aside from this one) are 
 *          references by the FATFS module and are not meant to be called directly by the 
 *          user within application code. 
 * 
 * @param hw125_slave_pin 
 */
void hw125_user_init(uint16_t hw125_slave_pin);


/**
 * @brief HW125 initialization 
 * 
 * @details 
 * 
 * @param hw125_slave_pin 
 * @return uint8_t 
 */
DISK_STATUS hw125_init(uint8_t pdrv);


/**
 * @brief HW125 disk status 
 * 
 * @details 
 * 
 * @param pdrv : physical drive number to identify the target device 
 * @return uint8_t 
 */
DISK_STATUS hw125_status(uint8_t pdrv);

//=======================================================================================


//=======================================================================================
// Data functions 

/**
 * @brief HW125 read 
 * 
 * @details 
 * 
 * @param pdrv : physical drive number 
 * @param buff : pointer to the read data buffer 
 * @param sector : start sector number 
 * @param count : number of sectors to read 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_read(
    uint8_t  pdrv, 
    uint8_t  *buff,
    uint32_t sector,
    uint16_t count);


/**
 * @brief HW125 write 
 * 
 * @details 
 * 
 * @param pdrv : physical drive number 
 * @param buff : pointer to the data to be written 
 * @param sector : sector number to write from 
 * @param count : number of sectors to write 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_write(
    uint8_t       pdrv, 
    const uint8_t *buff,
    uint32_t      sector,
    uint16_t      count);


/**
 * @brief HW125 IO control 
 * 
 * @details This function is called to control device specific features and misc functions 
 *          other than generic read and write. 
 * 
 * @param pdrv : drive number 
 * @param cmd : control command code 
 * @param buff : parameter and data buffer 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_ioctl(
    uint8_t pdrv, 
    uint8_t cmd, 
    void    *buff);

//=======================================================================================


#endif  // _HW_125_MICROSD_DRIVER_H_
