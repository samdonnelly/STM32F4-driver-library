/**
 * @file hw125_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 SD card reader driver interface 
 * 
 * @version 0.1
 * @date 2022-05-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HW125_DRIVER_H_
#define _HW125_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

// f_mount opt argument 
#define HW125_MOUNT_LATER 0         // To be mounted on the first access to the volume 
#define HW125_MOUNT_NOW 1           // Force mount to check if it's ready to work 

// f_open mode flags (some are FatFs mode flags) 
#define HW125_MODE_R        (FA_READ)                                  // 0x01 - "r" 
#define HW125_MODE_RR       (FA_READ | FA_WRITE)                       // 0x03 - "r+" 
#define HW125_MODE_W        (FA_CREATE_ALWAYS | FA_WRITE)              // 0x0A - "w" 
#define HW125_MODE_WW       (FA_CREATE_ALWAYS | FA_WRITE | FA_READ)    // 0x0B - "w+" 
#define HW125_MODE_A        (FA_OPEN_APPEND | FA_WRITE)                // 0x32 - "a" 
#define HW125_MODE_AA       (FA_OPEN_APPEND | FA_WRITE | FA_READ)      // 0x33 - "a+" 
#define HW125_MODE_WX       (FA_CREATE_NEW | FA_WRITE)                 // 0x06 - "wx" 
#define HW125_MODE_WWX      (FA_CREATE_NEW | FA_WRITE | FA_READ)       // 0x07 - "w+x" 
#define HW125_MODE_OAW      (FA_OPEN_ALWAYS | FA_WRITE)                // 0x12 
#define HW125_MODE_OAWR     (FA_OPEN_ALWAYS | FA_WRITE | FA_READ)      // 0x13 
#define HW125_MODE_OEW      (FA_OPEN_EXISTING | FA_WRITE)              // 0x02 
#define HW125_MODE_OEWR     (FA_OPEN_EXISTING | FA_WRITE | FA_READ)    // 0x03 

// Functions 
#define f_unmount(path) f_mount(NULL, path, HW125_MOUNT_NOW)     // Unmount card 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HW125 card types 
 * 
 * @details Identifiers for the card type. The card type is used internally for determining 
 *          how to handle a particular drive when read and write operations are called 
 *          by the FATFS module layer. The card type is determined during the drive 
 *          initialization process. 
 * 
 * @see hw125_init 
 */
typedef enum {
    HW125_CT_UNKNOWN     = 0x00,   // Unknown card type - failed to read 
    HW125_CT_MMC         = 0x01,   // MMC version 3
    HW125_CT_SDC1        = 0x02,   // SDC version 1
    HW125_CT_SDC2_BYTE   = 0x04,   // SDC version 2 - byte address 
    HW125_CT_SDC2_BLOCK  = 0x0C    // SDC version 2 - block address 
} hw125_card_type_t;


/**
 * @brief HW125 disk function results 
 * 
 * @details The result of the read and write operations. The return type of numerous functions 
 *          in the driver is DISK_RESULT which is a typedef of this enum. If the operation is 
 *          successful then HW125_RES_OK will be returned. If not then the problem will be 
 *          reflected in the result. 
 */
typedef enum {
    HW125_RES_OK,       // the function succeeded
    HW125_RES_ERROR,    // an error occured 
    HW125_RES_WRPRT,    // the medium is write protected 
    HW125_RES_NOTRDY,   // device has not been initialized 
    HW125_RES_PARERR    // command code or parameter is invalid 
} hw125_disk_results_t;

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t DISK_STATUS;
typedef hw125_disk_results_t DISK_RESULT; 
typedef hw125_card_type_t CARD_TYPE; 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief HW125 user init 
 * 
 * @details This functions is called directly by the user and used to set parameters for 
 *          the hw125 driver that define the characteristics of the drive. Characteristics 
 *          such as the disk status, card type, power flag and slave select pin are 
 *          initialized here but currently only the slave select pin is configurable 
 *          through a call to this function. 
 *          
 *          This function should be called during initialization in the application code. 
 * 
 * @param spi : SPI port used by the SD card 
 * @param gpio : GPIO port used for the SD card slave select pin 
 * @param hw125_slave_pin : slave pin (GPIO pin) used to select the slave device 
 */
void hw125_user_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio,  
    uint16_t hw125_slave_pin);


/**
 * @brief HW125 get card type 
 * 
 * @details Getter for use by application code. 
 * 
 * @return CARD_TYPE : card type of volume 
 */
CARD_TYPE hw125_get_card_type(void); 


/**
 * @brief HW125 ready to receive commands 
 * 
 * @details Waits for the SD card DO/MISO line to go high (0xFF) which indicates that the 
 *          card is ready to receive further instructions. The function is called before 
 *          sending a command and before writing new data packets to the card. 
 */
DISK_RESULT hw125_ready_rec(void); 


/**
 * @brief HW125 get volume presence 
 * 
 * @details Checks that there is a volume present by checking if the volume is ready to 
 *          receive commands. If present the function will return HW125_RES_OK and if not 
 *          then it will return HW125_RES_ERROR. Returning HW125_RES_ERROR means the 
 *          volume did not respond in enough time. This function is used by the HW125 
 *          controller in the "not ready" state where reading/writing is not being 
 *          performed which means a timeout indicates the volume is not busy but rather 
 *          non-existant. 
 * 
 * @return DISK_RESULT : result of the volume check 
 */
DISK_RESULT hw125_get_existance(void); 

//=======================================================================================


//=======================================================================================
// diskio functions 

// These functions should not be called by the application directly. They're used to 
// implement the functionality of the FATFS filesystem functions (ex. f_read). 

/**
 * @brief HW125 initialization 
 * 
 * @details Puts the SD card into the ready state so it can start to accept generic read and 
 *          write commands. The type of card is also determined which is used throughout the 
 *          driver to know how to handle data. If all initialization operations are 
 *          successful then the function will clear the HW125_STATUS_NOINIT flag and 
 *          return that as the status. If unsuccessful then HW125_STATUS_NOINIT will be 
 *          returned and the no further calls can be made to the card. 
 *          
 *          This function is called by the FATFS module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @see hw125_disk_status_t
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @return DISK_STATUS : status of the disk drive 
 */
DISK_STATUS hw125_init(uint8_t pdrv);


/**
 * @brief HW125 disk status 
 * 
 * @details Returns the current status of the card. 
 *          
 *          This function is called by the FATFS module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @see hw125_disk_status_t
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0)
 * @return DISK_STATUS : status of the disk drive 
 */
DISK_STATUS hw125_status(uint8_t pdrv);


/**
 * @brief HW125 read 
 * 
 * @details Reads single or multiple data packets from the SD card. The address to start 
 *          reading from is specified as an argument and the data read gets stored into 
 *          a pointer to a buffer. The function returns the result of the operation. 
 *          
 *          This function is called by the FATFS module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @see hw125_disk_results_t
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @param buff : pointer to the read data buffer that stores the information read 
 * @param sector : start sector number - address to begin reading from 
 * @param count : number of sectors to read 
 * @return DISK_RESULT : result of the read operation 
 */
DISK_RESULT hw125_read(
    uint8_t pdrv, 
    uint8_t *buff,
    uint32_t sector,
    uint16_t count);


/**
 * @brief HW125 write 
 * 
 * @details Writes single or multiple data packets to the SD card. The address to start 
 *          writing to and a pointer to a buffer that stores the data to be written are passed 
 *          as arguments. The function returns the result of the operation. 
 *          
 *          This function is called by the FATFS module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @see hw125_disk_results_t
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @param buff : pointer to the data to be written 
 * @param sector : sector number (address) that specifies where to begin writing data 
 * @param count : number of sectors to write (determines single or multiple data packet write) 
 * @return DISK_RESULT : result of the write operation 
 */
DISK_RESULT hw125_write(
    uint8_t pdrv, 
    const uint8_t *buff,
    uint32_t sector,
    uint16_t count);


/**
 * @brief HW125 IO control 
 * 
 * @details This function is called to control device specific features and misc functions 
 *          other than generic read and write. Which function to call is specified by the 
 *          cmd argument. The buff argument is a generic void pointer that can be used for 
 *          any of the functions specified by cmd. Each function can cast the pointer to the 
 *          needed data type. buff can also serves as further specification of the operation to 
 *          perform within each sub function. 
 *          
 *          This function is called by the FATFS module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @see hw125_disk_results_t
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @param cmd : control command code - specifies sub operation to execute 
 * @param buff : parameter and data buffer - supports the sub operation specified by cmd 
 * @return DISK_RESULT : result of the IO control operation 
 */
DISK_RESULT hw125_ioctl(
    uint8_t pdrv, 
    uint8_t cmd, 
    void *buff);

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _HW125_DRIVER_H_
