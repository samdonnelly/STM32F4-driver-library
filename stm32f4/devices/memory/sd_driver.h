/**
 * @file sd_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SD card driver interface 
 * 
 * @version 0.1
 * @date 2022-05-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _SD_CARD_DRIVER_H_
#define _SD_CARD_DRIVER_H_

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
#define SD_MOUNT_LATER 0         // To be mounted on the first access to the volume 
#define SD_MOUNT_NOW 1           // Force mount to check if it's ready to work 

// f_open mode flags (some are FatFs mode flags) 
#define SD_MODE_R        (FA_READ)                                  // 0x01 - "r" 
#define SD_MODE_RR       (FA_READ | FA_WRITE)                       // 0x03 - "r+" 
#define SD_MODE_W        (FA_CREATE_ALWAYS | FA_WRITE)              // 0x0A - "w" 
#define SD_MODE_WW       (FA_CREATE_ALWAYS | FA_WRITE | FA_READ)    // 0x0B - "w+" 
#define SD_MODE_A        (FA_OPEN_APPEND | FA_WRITE)                // 0x32 - "a" 
#define SD_MODE_AA       (FA_OPEN_APPEND | FA_WRITE | FA_READ)      // 0x33 - "a+" 
#define SD_MODE_WX       (FA_CREATE_NEW | FA_WRITE)                 // 0x06 - "wx" 
#define SD_MODE_WWX      (FA_CREATE_NEW | FA_WRITE | FA_READ)       // 0x07 - "w+x" 
#define SD_MODE_OAW      (FA_OPEN_ALWAYS | FA_WRITE)                // 0x12 
#define SD_MODE_OAWR     (FA_OPEN_ALWAYS | FA_WRITE | FA_READ)      // 0x13 
#define SD_MODE_OEW      (FA_OPEN_EXISTING | FA_WRITE)              // 0x02 
#define SD_MODE_OEWR     (FA_OPEN_EXISTING | FA_WRITE | FA_READ)    // 0x03 

// Functions 
#define f_unmount(path) f_mount(NULL, path, SD_MOUNT_NOW)     // Unmount card 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief SD card card types 
 * 
 * @details Identifiers for the card type. The card type is used internally for determining 
 *          how to handle a particular drive when read and write operations are called 
 *          by the SD card module layer. The card type is determined during the drive 
 *          initialization process. 
 * 
 * @see sd_init 
 */
typedef enum {
    SD_CT_UNKNOWN    = 0x00,   // Unknown card type - failed to read 
    SD_CT_MMC        = 0x01,   // MMC version 3
    SD_CT_SDC1       = 0x02,   // SDC version 1
    SD_CT_SDC2_BYTE  = 0x04,   // SDC version 2 - byte address 
    SD_CT_SDC2_BLOCK = 0x0C    // SDC version 2 - block address 
} sd_card_type_t;

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t DISK_STATUS;
typedef sd_card_type_t CARD_TYPE;

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief SD card user init 
 * 
 * @details This functions is called directly by the user and used to set parameters for 
 *          the sd card driver that define the characteristics of the drive. Characteristics 
 *          such as the disk status, card type, power flag and slave select pin are 
 *          initialized here but currently only the slave select pin is configurable 
 *          through a call to this function. 
 *          
 *          This function should be called during initialization in the application code. 
 * 
 * @param spi : SPI port used by the SD card 
 * @param gpio : GPIO port used for the SD card slave select pin 
 * @param timer : TIM port used for generic timing delays 
 * @param sd_slave_pin : slave pin (GPIO pin) used to select the slave device 
 */
void sd_user_init(
    SPI_TypeDef *spi,
    GPIO_TypeDef *gpio,
    TIM_TypeDef *timer,
    uint16_t sd_slave_pin);


/**
 * @brief SD card get card type 
 * 
 * @details Getter for use by application code. 
 * 
 * @return CARD_TYPE : card type of volume 
 */
CARD_TYPE sd_get_card_type(void);


/**
 * @brief SD card ready to receive commands 
 * 
 * @details Waits for the SD card DO/MISO line to go high (0xFF) which indicates that the 
 *          card is ready to receive further instructions. The function is called before 
 *          sending a command and before writing new data packets to the card. 
 * 
 * @return DISK_STATUS : 0: not ready, 1: ready 
 */
DISK_STATUS sd_ready_rec(void); 


/**
 * @brief SD card get volume presence 
 * 
 * @details Checks that there is a volume present by checking if the volume is ready to 
 *          receive commands. If present the function will return SD_RES_OK and if not 
 *          then it will return SD_RES_ERROR. Returning SD_RES_ERROR means the 
 *          volume did not respond in enough time. 
 * 
 * @return DISK_STATUS : 0: volume not present, 1: volume present 
 */
DISK_STATUS sd_get_existance(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _SD_CARD_DRIVER_H_
