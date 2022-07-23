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

//======================================================
// User defined 

// Command values 
#define HW125_INDEX_OFFSET       0x40    // First two bits of command index 

// Timers/counters 
#define HW125_INIT_TIMER         1000    // Initiate initialization counter 
#define HW125_INIT_DELAY         1       // time delay in ms for initiate initialization sequence
#define HW125_PWR_ON_COUNTER     10      // General counter for the hw125_power_on function 
#define HW125_PWR_ON_RES_CNT     0x1FFF  // R1 response counter during power on sequence 
#define HW125_R1_RESP_COUNT      10      // Max num of times to read R1 until appropriate response
#define HW125_DT_RESP_COUNT      10      // Max number of times to check the data token 

// Data information 
#define HW125_DATA_HIGH          0xFF    // DI/MOSI setpoint and DO/MISO response value 
#define HW125_TRAILING_BYTES     4       // Number of bytes in an R3/R7 response after R1 
#define HW125_SINGLE_BYTE        1       // For single byte operations 
#define HW125_SEC_SIZE           512     // Sector size of the card 
#define HW125_CSD_REG_LEN        16      // CSD register length 
#define HW125_CID_REG_LEN        16      // CID register length 

// Responses and filters values
#define HW125_READY_STATE        0x00    // Drive is ready to send and receive information 
#define HW125_IDLE_STATE         0x01    // Drive is in the idle state - after software reset 
#define HW125_SDCV2_CHECK        0x1AA   // SDCV2 return value from CMD8 
#define HW125_R1_FILTER          0x80    // Filter used to determine a valid R1 response 
#define HW125_CCS_FILTER         0x40    // Isolate the CCS bit location in OCR 
#define HW125_CSD_FILTER         0x03    // Isolate the CSD register version number 
#define HW125_INIT_SUCCESS       0xFE    // Filter to clear the HW125_STATUS_NOINIT flag 
#define HW125_DR_FILTER          0x1F    // Data response filter for write operations 

// IO Control 
#define HW125_LBA_OFFSET         1       // Used in sector size calculation for all cards 
#define HW125_MULT_OFFSET        2       // Used in sector size calculation for SDC V1 
#define HW125_MAGIC_SHIFT_V1     9       // Magic sector count format shift for CSD V1 
#define HW125_MAGIC_SHIFT_V2     10      // Magic sector count format shift for CSD V2 cards 

//======================================================


//======================================================
// Command codes for IO control --> copied from diskio.h 

/* Generic command (Used by FatFs) */
#define HW125_CTRL_SYNC           0      // Complete pending write process (needed at _FS_READONLY == 0) 
#define HW125_GET_SECTOR_COUNT    1      // Get media size (needed at _USE_MKFS == 1) 
#define HW125_GET_SECTOR_SIZE     2      // Get sector size (needed at _MAX_SS != _MIN_SS) 
#define HW125_GET_BLOCK_SIZE      3      // Get erase block size (needed at _USE_MKFS == 1) 
#define HW125_CTRL_TRIM           4      // Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) 

// Generic command (Not used by FatFs) 
#define HW125_CTRL_POWER          5      // Get/Set power status 
#define HW125_CTRL_LOCK           6      // Lock/Unlock media removal 
#define HW125_CTRL_EJECT          7      // Eject media 
#define HW125_CTRL_FORMAT         8      // Create physical format on the media 

// MMC/SDC specific ioctl command 
#define HW125_MMC_GET_TYPE        10     // Get card type 
#define HW125_MMC_GET_CSD         11     // Get CSD 
#define HW125_MMC_GET_CID         12     // Get CID 
#define HW125_MMC_GET_OCR         13     // Get OCR 
#define HW125_MMC_GET_SDSTAT      14     // Get SD status 

// ATA/CF specific ioctl command 
#define HW125_ATA_GET_REV         20     // Get F/W revision 
#define HW125_ATA_GET_MODEL       21     // Get model name 
#define HW125_ATA_GET_SN          22     // Get serial number 

//======================================================

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HW125 command index 
 * 
 * @details Index that determines what command is being requested. Each index is offset 
 *          by HW125_INDEX_OFFSET because every command frame sends this value every time 
 *          a command is sent. 
 * 
 */
typedef enum {
    HW125_CMD0  = HW125_INDEX_OFFSET + 0x00,    // GO_IDLE_STATE
    HW125_CMD1  = HW125_INDEX_OFFSET + 0x01,    // SEND_OP_COND
    HW125_CMD8  = HW125_INDEX_OFFSET + 0x08,    // SEND_IF_COND
    HW125_CMD9  = HW125_INDEX_OFFSET + 0x09,    // SEND_CSD
    HW125_CMD10 = HW125_INDEX_OFFSET + 0x0A,    // SEND_CID
    HW125_CMD12 = HW125_INDEX_OFFSET + 0x0C,    // STOP_TRANSMISSION
    HW125_CMD16 = HW125_INDEX_OFFSET + 0x10,    // SET_BLOCKLEN
    HW125_CMD17 = HW125_INDEX_OFFSET + 0x11,    // READ_SINGLE_BLOCK
    HW125_CMD18 = HW125_INDEX_OFFSET + 0x12,    // READ_MULTIPLE_BLOCK
    HW125_CMD23 = HW125_INDEX_OFFSET + 0x17,    // SET_BLOCK_COUNT
    HW125_CMD24 = HW125_INDEX_OFFSET + 0x18,    // WRITE_BLOCK
    HW125_CMD25 = HW125_INDEX_OFFSET + 0x19,    // WRITE_MULTIPLE_BLOCK
    HW125_CMD41 = HW125_INDEX_OFFSET + 0x29,    // APP_SEND_OP_COND
    HW125_CMD55 = HW125_INDEX_OFFSET + 0x37,    // APP_CMD
    HW125_CMD58 = HW125_INDEX_OFFSET + 0x3A     // READ_OCR
} hw125_command_index_t;


/**
 * @brief HW125 arguments
 * 
 * @details Each command needs a certain argument to be sent with it in the command frame. 
 *          For the commands used in reading and writing to a drive, these are all the 
 *          needed arguments. 
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
 * @details Each command needs a certain CRC to be sent with it at the end of the command 
 *          frame. For the commands used in reading and writing to a drive, these are all the 
 *          needed CRC values. 
 * 
 */
typedef enum {
    HW125_CRC_CMDX = 0x01,  // For all other commands 
    HW125_CRC_CMD8 = 0x87,  // For command 8 exclusively 
    HW125_CRC_CMD0 = 0x95   // For command 0 exclusively 
} hw125_crc_cmd_t;


/**
 * @brief HW125 card types 
 * 
 * @details Identifiers for the card type. The card type is used internally for determining 
 *          how to handle a particular drive when read and write operations are called 
 *          by the FATFS module layer. The card type is determined during the drive 
 *          initialization process. 
 * 
 * @see hw125_init 
 * 
 */
typedef enum {
    HW125_CT_UNKNOWN     = 0x00,   // Unknown card type - failed to read 
    HW125_CT_MMC         = 0x01,   // MMC version 3
    HW125_CT_SDC1        = 0x02,   // SDC version 1
    HW125_CT_SDC2_BYTE   = 0x04,   // SDC version 2 - byte address 
    HW125_CT_SDC2_BLOCK  = 0x0C    // SDC version 2 - block address 
} hw125_card_type_t; 


/**
 * @brief HW125 disk status 
 * 
 * @details Status of the card being used. The status is used as a check before read and 
 *          write operations to determine whether to proceed or not. If the status is any of 
 *          the options below then the card will not perform any operations. The status gets 
 *          set in the hw125_init function and if initialization is successful then the 
 *          status is cleared from being HW125_STATUS_NOINIT and will work as normal. 
 * 
 * @see hw125_init 
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
 * @details Status and commands for the power flag (pwr_flag). The power flag is used as a 
 *          reference for the FATFS module layer. HW125_PWR_OFF and HW125_PWR_ON are used to 
 *          set the power flag and determine if the FATFS layer wants to set or clear the flag. 
 *          HW125_PWR_CHECK is used to identify that the FATFS layer wants to know the state 
 *          of the power flag. If the power on sequence during initialization is successful 
 *          then the pwoer flag gets set to on.
 * 
 */
typedef enum {
    HW125_PWR_OFF,
    HW125_PWR_ON,
    HW125_PWR_CHECK
} hw125_pwr_status_t;


/**
 * @brief HW125 disk function results 
 * 
 * @details The result of the read and write operations. The return type of numerous functions 
 *          in the driver is DISK_RESULT which is a typedef of this enum. If the operation is 
 *          successful then HW125_RES_OK will be returned. If not then the problem will be 
 *          reflected in the result. 
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
 * @details Data tokens associated with various commands. When reading or writing data, 
 *          information is sent in the form of data packets which consist of a data token 
 *          followed by a data block and a CRC. 
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
    HW125_DR_ZERO = 0x05,   // Data accepted 
    HW125_DR_ONE  = 0x0B,   // Data rejected due to a CRC error 
    HW125_DR_TWO  = 0x0D    // Data rejected due to a write error 
} hw125_data_response_filter_t;  


/**
 * @brief CSD register version 
 * 
 * @details 
 * 
 */
typedef enum {
    HW125_CSD_V1, 
    HW125_CSD_V2, 
    HW125_CSD_V3
} hw125_csd_version_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t DISK_STATUS;

typedef hw125_disk_results_t DISK_RESULT; 

//=======================================================================================


//=======================================================================================
// Initialization functions 

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

//=======================================================================================


//=======================================================================================
// Status functions 

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

//=======================================================================================


//=======================================================================================
// IO Control functions 

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
