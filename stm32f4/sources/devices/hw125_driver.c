/**
 * @file hw125_driver.c
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

//=======================================================================================
// Includes 

#include "hw125_driver.h"

// Drivers 
#include "timers_driver.h"
#include "spi_comm.h"
#include "uart_comm.h"

//=======================================================================================


//=======================================================================================
// Macros 

//==================================================
// User defined 

// Command values 
#define HW125_INDEX_OFFSET       0x40    // First two bits of command index 

// Timers/counters 
#define HW125_INIT_TIMER         1000    // Initiate initialization counter 
#define HW125_INIT_DELAY         1       // time delay in ms for initiate initialization sequence
#define HW125_PWR_ON_COUNTER     10      // General counter for the hw125_power_on function 
#define HW125_PWR_ON_RES_CNT     0x1FFF  // R1 response counter during power on sequence 
#define HW125_R1_RESP_COUNT      10      // Max num of times to read R1 until appropriate response
#define HW125_DT_RESP_COUNT      1000    // Max number of times to check the data token 

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

//==================================================

//==================================================
// Command codes for IO control --> copied from diskio.h 

// Generic command (Used by FatFs) 
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

//==================================================

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief HW125 command index 
 * 
 * @details Index that determines what command is being requested. Each index is offset 
 *          by HW125_INDEX_OFFSET because every command frame sends this value every time 
 *          a command is sent. 
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
 */
typedef enum {
    HW125_CRC_CMDX = 0x01,  // For all other commands 
    HW125_CRC_CMD8 = 0x87,  // For command 8 exclusively 
    HW125_CRC_CMD0 = 0x95   // For command 0 exclusively 
} hw125_crc_cmd_t;


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
 */
typedef enum {
    HW125_PWR_OFF,
    HW125_PWR_ON,
    HW125_PWR_CHECK
} hw125_pwr_status_t;


/**
 * @brief HW125 data token 
 * 
 * @details Data tokens associated with various commands. When reading or writing data, 
 *          information is sent in the form of data packets which consist of a data token 
 *          followed by a data block and a CRC. If reading data from a drive then the code 
 *          looks for the appropriate data token before reading data. If writing data to a 
 *          drive the appropriate data token is sent before the data. When writing multiple 
 *          data packets then a stop token is needed to terminate the transaction.  
 *          
 *          CMD17: Read a single data packet 
 *          CMD18: Read multiple data packets 
 *          CMD24: Write a single data packet 
 *          CMD25: Write multiple data packets 
 */
typedef enum {
    HW125_DT_ZERO = 0xFC,    // Data token for CMD25 
    HW125_DT_ONE  = 0xFD,    // Stop token for CMD25 
    HW125_DT_TWO  = 0xFE     // Data token for CMD17/18/24
} hw125_data_token_t;


/**
 * @brief HW125 data response filters 
 * 
 * @details When writing to a drive a data response byte will be received immediately after 
 *          the data packet has finished sending. This data response will indicate if the 
 *          transaction was successful or if there were errors. 
 */
typedef enum {
    HW125_DR_ZERO = 0x05,   // Data accepted 
    HW125_DR_ONE  = 0x0B,   // Data rejected due to a CRC error 
    HW125_DR_TWO  = 0x0D    // Data rejected due to a write error 
} hw125_data_response_filter_t;


/**
 * @brief CSD register version 
 * 
 * @details Different SD card drives contain different versions of the CSD register. Each 
 *          version stored slightly different information and at different locations in the 
 *          register. The values in this enum are used to verify the CSD register version in  
 *          the hw125_ioctl function so the drives sector count can be calculated correctly. 
 * 
 * @see hw125_ioctl
 */
typedef enum {
    HW125_CSD_V1,   // Version 1.0 
    HW125_CSD_V2,   // Version 2.0 
    HW125_CSD_V3    // Version 3.0 
} hw125_csd_version_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief HW125 power on sequence
 * 
 * @details This functions sets the SD card into it's native operating mode where it is 
 *          ready to accept native commands. It is called at the beginning of the 
 *          hw125_init function to prepare the card. It can also be called in the ioctl 
 *          function by the FATFS module layer if needed. <br><br> 
 *          
 *          To prepare the card this function deselects the slave, sets the DI/MOSI 
 *          line to high (0xFF) and sends a minumum of 74 SCLK pulses. <br><br> 
 *          
 *          After the card enters it's native operating mode a software reset seqence is 
 *          performed. In this sequence the SD card is selected and CMD0 is sent to the 
 *          card. If the command is successfully received then the card will enter SPI 
 *          mode and respond with an IDLE state (0x01). <br><br> 
 *          
 *          At the end of the sequence the pwr_flag for the sd_card is set the 
 *          HW125_PWR_ON. 
 * 
 * @see hw125_init
 * @see hw125_power_status
 * 
 * @param hw125_slave_pin : SS pin that the SD card is connected to 
 */
DISK_RESULT hw125_power_on(uint16_t hw125_slave_pin); 


/**
 * @brief HW125 power off 
 * 
 * @details Sets the pwr_flag of the sd_card to HW125_PWR_OFF. The power flag only serves to 
 *          update the status check read from hw125_power_status. This function is called 
 *          during initialization if the card type is unknown (initialization failed) and in 
 *          the ioctl function if the FATFS module layer requets it. 
 * 
 * @see hw125_power_status 
 */
void hw125_power_off(void);


/**
 * @brief HW125 initiate initialization sequence
 * 
 * @details A sequence that occurs during the initialization process used to initiate 
 *          initialization in the SD card. This sequence is common among all card type 
 *          initialization. Depending on the card, CMD41 or CMD1 is sent repeatedly until 
 *          the card enters its ready state (0x00) or an error has occured. The hw125_init 
 *          function calls this function. 
 * 
 * @see hw125_init
 * 
 * @param cmd : command to send (either CMD1 or CMD41) 
 * @param arg : argument in the command message that corresponds to the cmd specified 
 * @param resp : pointer to where the command response is stored 
 * @return uint8_t : status of the timer - TRUE or FALSE (for timout) 
 */
uint8_t hw125_initiate_init(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t *resp);


/**
 * @brief HW125 send command messages to the SD card 
 * 
 * @details Generates a command frame based on the arguments, transmits the command to 
 *          the SD card and waits for an appropriate R1 response. This function can be 
 *          called for any command. 
 * 
 * @param cmd : command to send 
 * @param arg : argument for the corresponding command 
 * @param crc : crc value for the corresponding command 
 * @param resp : pointer to where the R1 response is stored 
 */
void hw125_send_cmd(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t  crc,
    uint8_t *resp);


/**
 * @brief HW125 read data packet 
 * 
 * @details Verifies the data tocken sent from the card and if correct then reads the 
 *          info from the data packet. The function only reads a single data packet so if 
 *          multiple are required then the function is repeatedly called. The function is 
 *          used by hw125_read and hw125_ioctl. 
 * 
 * @see hw125_read 
 * @see hw125_ioctl 
 * 
 * @param buff : buffer to store info from the data packet 
 * @param sector_size : number of bytes read in the packet - sector size of the card 
 * @return DISK_RESULT : result of the read operation 
 */
DISK_RESULT hw125_read_data_packet(
    uint8_t *buff,
    uint32_t sector_size);


/**
 * @brief HW125 write data packet 
 * 
 * @details Sends a data token to the card to indicate the write operation then proceeds 
 *          to write the data packet to the card. The functions writes a single data packet 
 *          so if multiple packets are needed then the functionis called repeatedly. The 
 *          function is used by hw125_write. 
 * 
 * @see hw125_write 
 * 
 * @param buff : buffer that stores the info in the packet to be written 
 * @param sector_size : number of bytes written in the packet - sector size of the card 
 * @return DISK_RESULT : result of the write operation 
 */
DISK_RESULT hw125_write_data_packet(
    const uint8_t *buff,
    uint32_t sector_size,
    uint8_t data_token);


/**
 * @brief HW125 IO Control - Get Sector Count 
 * 
 * @details Reads the sector count from the card based on the card type determined during 
 *          initialization and stores it in a buffer for use in the FATFS module layer. 
 *          <br><br>
 *          
 *          The FATFS layer calls the hw125_ioctl function and requests the sector count. The 
 *          hw125_ioctl function then in turn calls this function. 
 * 
 * @param buff : pointer to store sector count - gets typecast to the correct data type 
 * @return DISK_RESULT : result of the read operation 
 */
DISK_RESULT hw125_ioctl_get_sector_count(void *buff);


/**
 * @brief HW125 IO Control - Get Sector Size 
 * 
 * @details Reads the sector size from the code and stores it in a buffer for use in the 
 *          FATFS module layer. <br><br>
 *          
 *          The FATFS layer calls the hw125_ioctl function and requests the sector size. The 
 *          hw125_ioctl function then in turn calls this function. 
 * 
 * @param buff : pointer to store the sector size - gets typecast to the correct data type 
 * @return DISK_RESULT : result of the read operation 
 */
DISK_RESULT hw125_ioctl_get_sector_size(void *buff);


/**
 * @brief HW125 IO Control - Control Power 
 * 
 * @details Sets the pwr_flag status or reads the pwr_flag status from the code. Called by 
 *          the FATFS module layer. <br><br>
 *          
 *          The FATFS layer calls the hw125_ioctl function and requests a pwr_flag operation. 
 *          The hw125_ioctl function then in turn calls this function. 
 * 
 * @param buff : pointer to store the pwr_flag status - gets typecast to pwr_flag status type
 * @return DISK_RESULT : result of the status check 
 */
DISK_RESULT hw125_ioctl_ctrl_pwr(void *buff);


/**
 * @brief HW125 IO Control - Get CSD Register 
 * 
 * @details Reads the CSD register from the card and stores it in a buffer for use in the 
 *          FATFS module layer. <br><br>
 *          
 *          The FATFS layer calls the hw125_ioctl function and requests the contents of the 
 *          CSD register. The hw125_ioctl function then in turn calls this function.
 * 
 * @param buff : pointer to store the CSD register - gets typecast to CSD register type 
 * @return DISK_RESULT : result of the read operation 
 */
DISK_RESULT hw125_ioctl_get_csd(void *buff);


/**
 * @brief HW125 IO Control - Get CID Register 
 * 
 * @details Reads the CID register from the card and stores it in a buffer for use in the 
 *          FATFS module layer. <br><br>
 *          
 *          The FATFS layer calls the hw125_ioctl function and requests the contents of the 
 *          CID register. The hw125_ioctl function then in turn calls this function.
 * 
 * @param buff : pointer to store the CID register - gets typecast to CID register type 
 * @return DISK_RESULT : result of the read operation 
 */
DISK_RESULT hw125_ioctl_get_cid(void *buff);


/**
 * @brief HW125 IO Control - Get OCR Register 
 * 
 * @details Reads the OCR register from the card and stores it in a buffer for use in the 
 *          FATFS module layer. <br><br>
 *          
 *          The FATFS layer calls the hw125_ioctl function and requests the contents of the 
 *          OCR register. The hw125_ioctl function then in turn calls this function.
 * 
 * @param buff : pointer to store the OCR register - gets typecast to the OCR register type 
 * @return DISK_RESULT : result of the read operation 
 */
DISK_RESULT hw125_ioctl_get_ocr(void *buff);

//=======================================================================================


//=======================================================================================
// Variables 

// HW125 disk record 
typedef struct 
{
    // Peripherals 
    GPIO_TypeDef *gpio;                 // GPIO port used for slave selection 
    SPI_TypeDef *spi;                   // SPI port used for SD card communication 

    // Tracking information 
    hw125_disk_status_t disk_status;    // Disk status - used as a check before read/write 
    hw125_card_type_t card_type;        // Type of storage device 
    hw125_pwr_status_t pwr_flag;        // Status flag for the FatFs layer 

    // Pins 
    uint16_t ss_pin;                    // Slave select pin for the card (GPIO pin for SPI) 
} 
hw125_disk_info_t;


// SD card (drive 0) information 
static hw125_disk_info_t sd_card;

//=======================================================================================


//=======================================================================================
// User functions 

// HW125 user initialization 
void hw125_user_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio, 
    uint16_t hw125_slave_pin)
{
    // Peripherals 
    sd_card.gpio = gpio; 
    sd_card.spi = spi; 

    // Tracking information 
    sd_card.disk_status = HW125_STATUS_NOINIT;
    sd_card.card_type = HW125_CT_UNKNOWN;
    sd_card.pwr_flag = HW125_PWR_OFF; 

    // Pins 
    sd_card.ss_pin = hw125_slave_pin;
}


// Get the card type 
CARD_TYPE hw125_get_card_type(void)
{
    return sd_card.card_type; 
}


// HW125 ready to receive commands 
DISK_RESULT hw125_ready_rec(void)
{
    uint8_t resp; 
    uint16_t timer = HW125_PWR_ON_RES_CNT; 

    // Read DO/MISO continuously until it is ready to receive commands 
    do 
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, &resp, BYTE_1); 
    }
    while (resp != HW125_DATA_HIGH && --timer); 

    if (timer)
    {
        return HW125_RES_OK; 
    }

    return HW125_RES_ERROR; 
}


// Check if the card is present 
DISK_RESULT hw125_get_existance(void)
{
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);
    DISK_RESULT exist = hw125_ready_rec();
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);
    return exist;
}

//=======================================================================================


//=======================================================================================
// diskio functions 

// HW125 initialization 
DISK_STATUS hw125_init(uint8_t pdrv)
{
    uint8_t do_resp; 
    uint8_t init_timer_status;
    uint8_t ocr[HW125_TRAILING_BYTES];
    uint8_t v_range[HW125_TRAILING_BYTES];

    // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
    if (pdrv)
    {
        return HW125_STATUS_NOINIT; 
    }

    //===================================================
    // Power ON or card insertion and software reset 

    if (hw125_power_on(sd_card.ss_pin)) 
    {
        sd_card.card_type = HW125_CT_UNKNOWN; 
        sd_card.disk_status = HW125_STATUS_NOINIT; 
        return sd_card.disk_status; 
    }

    //===================================================

    //===================================================
    // Initialization 
    
    // Select the sd card 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);

    // Send CMD0 with no arg and a valid CRC value 
    hw125_send_cmd(HW125_CMD0, HW125_ARG_NONE, HW125_CRC_CMD0, &do_resp);

    // Check the R1 response from CMD0 sent in hw125_power_on - do_resp is local so not true 
    if (do_resp == HW125_IDLE_STATE)
    {
        // In idle state - Send CMD8 with arg = 0x000001AA and a valid CRC
        hw125_send_cmd(HW125_CMD8, HW125_ARG_SUPV, HW125_CRC_CMD8, &do_resp);

        // Check the R1 response from CMD8 
        if (do_resp == HW125_IDLE_STATE)
        {
            // No command wrror - Read the trailing 32-bit R7 response 
            spi_write_read(sd_card.spi, HW125_DATA_HIGH, v_range, HW125_TRAILING_BYTES);

            // Check lower 12-bits of R7 response (big endian format) 
            if ((uint16_t)((v_range[BYTE_2] << SHIFT_8) | (v_range[BYTE_3])) == HW125_SDCV2_CHECK)
            {
                // 0x1AA matched (SDCV2+) - Send ACMD41 with the HCS bit set in the arg 
                init_timer_status = hw125_initiate_init(HW125_CMD41, HW125_ARG_HCS, &do_resp);

                // Check timeout
                if (init_timer_status && (do_resp == HW125_READY_STATE))
                {
                    // Itialization begun - Send CMD58 to check the OCR (trailing 32-bits)
                    hw125_send_cmd(HW125_CMD58, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

                    if (do_resp == HW125_READY_STATE)
                    {
                        // Successful CMD58 - proceed to read the OCR register 
                        spi_write_read(sd_card.spi, HW125_DATA_HIGH, ocr, HW125_TRAILING_BYTES);

                        // Check CCS bit (bit 30) in OCR response (big endian format) 
                        if (ocr[BYTE_0] & HW125_CCS_FILTER)
                        {
                            // SDC V2 (block address)
                            sd_card.card_type = HW125_CT_SDC2_BLOCK;
                        }
                        else
                        {
                            // SDC V2 (byte address)
                            sd_card.card_type = HW125_CT_SDC2_BYTE;

                            // Send CMD16 to change the block size to 512 bytes (for FAT)
                            hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
                        }
                    }
                    else
                    {
                        // Unsuccessful CMD58 
                        sd_card.card_type = HW125_CT_UNKNOWN;
                    }
                }
                else
                {
                    // Initialization timer timeout 
                    sd_card.card_type = HW125_CT_UNKNOWN;
                }
            }
            else 
            {
                // 0x1AA mismatched 
                sd_card.card_type = HW125_CT_UNKNOWN;
            }
        }
        else 
        {
            // CMD8 rejected with illegal command error (0x05) - Send ACMD41 
            init_timer_status = hw125_initiate_init(HW125_CMD41, HW125_ARG_NONE, &do_resp);

            // Check timeout and R1 response 
            if (init_timer_status && (do_resp == HW125_READY_STATE))
            {
                // Initialization begun - Card is SDC V1 
                sd_card.card_type = HW125_CT_SDC1;

                // Send CMD16 to change the block size to 512 bytes (for FAT)
                hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
            }

            else
            {
                // Error or timeout - Send CMD1 
                init_timer_status = hw125_initiate_init(HW125_CMD1, HW125_ARG_NONE, &do_resp);

                // Check timeout and R1 response 
                if (init_timer_status && (do_resp == HW125_READY_STATE))
                {
                    // Initialization begun - Card is MMC V3
                    sd_card.card_type = HW125_CT_MMC;

                    // Send CMD16 to change the block size to 512 bytes (for FAT)
                    hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
                }
                else
                {
                    // Initialization timer timeout 
                    sd_card.card_type = HW125_CT_UNKNOWN;
                }
            }
        }
    }
    else
    {
        // Not in idle state
        sd_card.card_type = HW125_CT_UNKNOWN;
    }

    //===================================================

    // Deselect slave 
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);
    
    // Perform a write_read after deselecting the slave --> Why? 
    spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 

    // Status check 
    if (sd_card.card_type == HW125_CT_UNKNOWN)
    {
        // Power off the card 
        hw125_power_off(); 

        // Set no init flag 
        sd_card.disk_status = HW125_STATUS_NOINIT;
    }
    else
    {
        // Clear no init flag 
        sd_card.disk_status = (HW125_STATUS_NOINIT & HW125_INIT_SUCCESS); 
    }

    return sd_card.disk_status;
}


// HW125 disk status 
DISK_STATUS hw125_status(uint8_t pdrv)
{
    // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
    if (pdrv)
    {
        return HW125_STATUS_NOINIT; 
    }

    // Return the existing disk status 
    return sd_card.disk_status;
}


// HW125 read 
DISK_RESULT hw125_read(
    uint8_t pdrv, 
    uint8_t *buff,
    uint32_t sector,
    uint16_t count)
{
    DISK_RESULT read_resp;
    uint8_t do_resp = 255;

    if (buff == NULL)
    {
        return HW125_RES_ERROR;
    }

    // Check that the drive number is zero and that the count is valid 
    if (pdrv || (count == NONE))
    {
        return HW125_RES_PARERR;
    }

    // Check the init status 
    if (sd_card.disk_status == HW125_STATUS_NOINIT)
    {
        return HW125_RES_NOTRDY;
    }

    // Convert the sector number to byte address if it's not SDC V2 (byte address)
    // Is this needed for all SDC V2 cards or just block address ones? 
    if (sd_card.card_type & HW125_CT_SDC2_BYTE)
    {
        sector *= HW125_SEC_SIZE;
    }

    // Select the slave device 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);

    // Determine the read operation 
    if (count == HW125_SINGLE_BYTE)   // Read one data packet if count == 1
    {
        // Send CMD17 with an arg that specifies the address to start to read 
        hw125_send_cmd(HW125_CMD17, sector, HW125_CRC_CMDX, &do_resp);

        // Read the R1 response 
        if (do_resp == HW125_READY_STATE)
        {
            // CMD17 successful - Read initiated 
            read_resp = hw125_read_data_packet(buff, HW125_SEC_SIZE);
        } 
        else
        {
            // Unsuccessful CMD17 
            read_resp = HW125_RES_ERROR;
        }
    }
    else   // Read multiple data packets if count > 1
    {
        // Send CMD18 with an arg that specifies the address to start a sequential read 
        hw125_send_cmd(HW125_CMD18, sector, HW125_CRC_CMDX, &do_resp);

        // Read the R1 response 
        if (do_resp == HW125_READY_STATE)
        {
            // CMD18 successfull - read initiated 
            do 
            {
                read_resp = hw125_read_data_packet(buff, HW125_SEC_SIZE);
                buff += HW125_SEC_SIZE; 
            }
            while (--count && (read_resp != HW125_RES_ERROR));

            // Send CMD12 to terminate the read transaction 
            hw125_send_cmd(HW125_CMD12, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

            if (do_resp != HW125_READY_STATE)
            {
                // CMD12 unsuccessfull 
                read_resp = HW125_RES_ERROR;
            }
        }
        else
        {
            // Unsuccessful CMD18
            read_resp = HW125_RES_ERROR;
        }
    }

    // Deselect the slave device 
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);

    // Dummy read 
    spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 

    // Return the result 
    return read_resp;
}


// HW125 write 
DISK_RESULT hw125_write(
    uint8_t pdrv, 
    const uint8_t *buff,
    uint32_t sector,
    uint16_t count)
{
    DISK_RESULT write_resp; 
    uint8_t do_resp; 
    uint8_t stop_trans = HW125_DT_ONE;

    if (buff == NULL)
    {
        return HW125_RES_ERROR;
    }

    // Check that the drive number is zero and that the count is valid 
    if (pdrv || (count == NONE))
    {
        return HW125_RES_PARERR;
    }

    // Check the init status 
    if (sd_card.disk_status == HW125_STATUS_NOINIT)
    {
        return HW125_RES_NOTRDY;
    }

    // Check write protection 
    if (sd_card.disk_status == HW125_STATUS_PROTECT)
    {
        return HW125_RES_WRPRT;
    }

    // Convert the sector number to byte address if it's not SDC V2 (byte address) 
    if (sd_card.card_type != HW125_CT_SDC2_BYTE)
    {
        sector *= HW125_SEC_SIZE;
    }

    // Select the slave device 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);

    // Wait until the card is no longer busy before sending a CMD 
    hw125_ready_rec();

    // Determine the write operation 
    if (count ==  HW125_SINGLE_BYTE)  // Send one data packet if count == 1
    {
        // Send CMD24 with an arg that specifies the address to start to write 
        hw125_send_cmd(HW125_CMD24, sector, HW125_CRC_CMDX, &do_resp);

        // Check the R1 response 
        if (do_resp == HW125_READY_STATE)
        {
            // Successfull CMD24 - Write data packet to card 
            write_resp = hw125_write_data_packet(buff, HW125_SEC_SIZE, HW125_DT_TWO);
        }
        else
        {
            // Unsuccessfull CMD24 
            write_resp = HW125_RES_ERROR;
        }
    }
    else  // Send multiple data packets if count > 1
    {
        // Specify the number of sectors to pre-erase to optimize write performance - no 
        // error condition in place. Also unclear if it's only SDCV1. 
        if (sd_card.card_type == HW125_CT_SDC1)
        {
            hw125_send_cmd(HW125_CMD55, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);
            hw125_send_cmd(HW125_CMD23, count, HW125_CRC_CMDX, &do_resp);
        }

        // Send CMD25 that specifies the address to start to write 
        hw125_send_cmd(HW125_CMD25, sector, HW125_CRC_CMDX, &do_resp);

        // Check the R1 response 
        if (do_resp == HW125_READY_STATE)
        {
            // CMD25 successful - Write all the sectors or until there is an error 
            do 
            {
                write_resp = hw125_write_data_packet(buff, HW125_SEC_SIZE, HW125_DT_ZERO);
                buff += HW125_SEC_SIZE; 
            }
            while (--count && (write_resp != HW125_RES_ERROR)); 

            // Wait on busy flag to clear 
            hw125_ready_rec();

            // Send stop token 
            spi_write(sd_card.spi, &stop_trans, HW125_SINGLE_BYTE);
        }
        else
        {
            // Unsuccessfull CMD25 
            write_resp = HW125_RES_ERROR;
        }
    }

    // Wait on busy flag to clear 
    hw125_ready_rec();

    // Deselect the slave device
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin); 

    // Return the write opration status 
    return write_resp; 
}


// HW125 IO control 
DISK_RESULT hw125_ioctl(
    uint8_t pdrv, 
    uint8_t cmd, 
    void    *buff)
{
    DISK_RESULT result; 

    // Check that the drive number is zero 
    if (pdrv)
    {
        return HW125_RES_PARERR;
    }

    // Check the init status 
    if ((sd_card.disk_status == HW125_STATUS_NOINIT) && (cmd != HW125_CTRL_POWER))
    {
        return HW125_RES_NOTRDY;
    }

    // Select the slave card 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin); 

    // Choose the misc function 
    switch(cmd)
    {
        case HW125_CTRL_SYNC:  // Ensure write operation in disk_write is complete. 
            hw125_ready_rec(); 
            result = HW125_RES_OK; 
            break; 
        
        case HW125_GET_SECTOR_COUNT:  // Get the size of the disk 
            result = hw125_ioctl_get_sector_count(buff); 
            break; 
        
        case HW125_GET_SECTOR_SIZE:  // Get the sector size 
            result = hw125_ioctl_get_sector_size(buff); 
            break; 
        
        case HW125_GET_BLOCK_SIZE:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_TRIM:  // Not needed 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_POWER:  // Get/set the power status 
            result = hw125_ioctl_ctrl_pwr(buff); 
            break; 
        
        case HW125_CTRL_LOCK:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_EJECT:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_FORMAT:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_MMC_GET_TYPE:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_MMC_GET_CSD:  // Read and the CSD register 
            result = hw125_ioctl_get_csd(buff); 
            break; 
        
        case HW125_MMC_GET_CID:  // Read the CID register 
            result = hw125_ioctl_get_cid(buff); 
            break; 
        
        case HW125_MMC_GET_OCR:  // Read tje OCR register 
            result = hw125_ioctl_get_ocr(buff); 
            break; 
        
        case HW125_MMC_GET_SDSTAT:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_ATA_GET_REV:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_ATA_GET_MODEL:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_ATA_GET_SN:  // Currently unsupported 
            result = HW125_RES_PARERR; 
            break; 
        
        default:  // Unknown 
            result = HW125_RES_PARERR; 
            break;
    }

    // Deselect the slave card 
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin); 

    return result; 
}


//=======================================================================================


//=======================================================================================
// Helper functions 

// HW125 power on sequence and software reset 
DISK_RESULT hw125_power_on(uint16_t hw125_slave_pin)
{
    uint8_t di_cmd; 
    uint8_t do_resp; 
    uint8_t cmd_frame[BYTE_6];
    uint16_t num_read = HW125_PWR_ON_RES_CNT; 

    //===================================================
    // Power ON or card insertion 

    // Wait for >1ms - delay for after the supply voltage reaches above 2.2V
    tim_delay_ms(TIM9, HW125_PWR_ON_COUNTER);

    // Deselect the sd card slave
    spi_slave_deselect(sd_card.gpio, hw125_slave_pin);

    // Set the DI/MOSI command high (0xFF) 
    di_cmd = HW125_DATA_HIGH; 

    // Send DI/MOSI high 10x to wait for more than 74 clock pulses 
    for (uint8_t i = 0; i < HW125_PWR_ON_COUNTER; i++) 
    {
        spi_write(sd_card.spi, &di_cmd, BYTE_1);
    }

    //===================================================
    
    //===================================================
    // Software reset 

    // Slave select 
    spi_slave_select(sd_card.gpio, hw125_slave_pin); 

    // Generate a command frame 
    for (uint8_t i = 0; i < BYTE_6; i++)
    {
        switch (i)
        {
            case BYTE_0:
                cmd_frame[i] = HW125_CMD0;
                break;
            case BYTE_5:
                 cmd_frame[i] = HW125_CRC_CMD0;
                break;
            default:
                cmd_frame[i] = (uint8_t)(HW125_ARG_NONE >> SHIFT_8*(BYTE_4 - i));
                break;
        }
    }

    // Transmit command 
    spi_write(sd_card.spi, cmd_frame, BYTE_6); 

    // Read R1 response until it is valid or until it times out 
    do 
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 
    }
    while ((do_resp != HW125_IDLE_STATE) && --num_read); 

    // Slave deselect 
    spi_slave_deselect(sd_card.gpio, hw125_slave_pin); 
    
    // Response timeout 
    if (!num_read)
    {
        return HW125_RES_ERROR; 
    }

    //===================================================

    // Send a data high byte --> Why? 
    spi_write(sd_card.spi, &di_cmd, BYTE_1);

    // Set the Power Flag status to on 
    sd_card.pwr_flag = HW125_PWR_ON; 

    return HW125_RES_OK; 
}


// Set the Power Flag status to off 
void hw125_power_off(void)
{
    sd_card.pwr_flag = HW125_PWR_OFF; 
}


// HW125 initiate initialization sequence
uint8_t hw125_initiate_init(
    uint8_t cmd,
    uint32_t arg,
    uint8_t *resp)
 {
    uint16_t init_timer = HW125_INIT_TIMER;

    // Send CMD1 or ACMD41 (depending on the card type) to initiate initialization 
    do
    {
        if (cmd == HW125_CMD1)
        {
            hw125_send_cmd(HW125_CMD1, HW125_ARG_NONE, HW125_CRC_CMDX, resp);
        }
        else
        {
            hw125_send_cmd(HW125_CMD55, HW125_ARG_NONE, HW125_CRC_CMDX, resp);
            hw125_send_cmd(HW125_CMD41, arg, HW125_CRC_CMDX, resp);
        }

        // Delay 1ms --> HW125_INIT_DELAY * HW125_INIT_TIMER = 1000ms (recommended delay) 
        tim_delay_ms(TIM9, HW125_INIT_DELAY);
    }
    while ((*resp == HW125_IDLE_STATE) && --init_timer); 

    // Initialization can begin 
    if (init_timer)
    {
        return TRUE; 
    }
    
    // Timeout 
    return FALSE; 
}


// Disable spi in an error state if spi comms go wrong. 
// Otherwise SPI will be enabled at all times 

// HW125 send command messages and return response values - add a status return for timeouts 
void hw125_send_cmd(
    uint8_t cmd,
    uint32_t arg,
    uint8_t crc,
    uint8_t *resp)
{
    uint8_t cmd_frame[BYTE_6];
    uint8_t num_read = HW125_R1_RESP_COUNT;

    // Wait until the device is ready to accept commands 
    hw125_ready_rec();

    // Generate a command frame 
    for (uint8_t i = CLEAR; i < BYTE_6; i++)
    {
        switch (i)
        {
            case BYTE_0:
                cmd_frame[i] = cmd;
                break;
            case BYTE_5:
                 cmd_frame[i] = crc;
                break;
            default:
                cmd_frame[i] = (uint8_t)(arg >> SHIFT_8*(BYTE_4 - i));
                break;
        }
    }

    // Transmit command 
    spi_write(sd_card.spi, cmd_frame, BYTE_6);

    // Skip the stuff byte sent following CMD12 (stop transmission) 
    if (cmd == HW125_CMD12)
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, resp, HW125_SINGLE_BYTE);
    }

    // Read R1 response until it is valid or until it times out 
    do 
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, resp, HW125_SINGLE_BYTE);
    }
    while ((*resp & HW125_R1_FILTER) && --num_read);
}


// HW125 read data packet 
DISK_RESULT hw125_read_data_packet(
    uint8_t *buff,
    uint32_t sector_size)
{
    DISK_RESULT read_resp;
    uint8_t do_resp = 200; 
    uint16_t num_read = HW125_DT_RESP_COUNT; 

    // Read the data token 
    do 
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 
    }
    while ((do_resp != HW125_DT_TWO) && --num_read); 

    // Check the R1 response 
    if (do_resp == HW125_DT_TWO)
    {
        // Valid data token is detected - read the data packet 
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, buff, sector_size);

        // Discard the two CRC bytes 
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);

        // Operation success 
        read_resp = HW125_RES_OK;
    }
    else
    {
        // Incorrect or error token received 
        read_resp = HW125_RES_ERROR;
    }

    // Return the status of the data packet response 
    return read_resp;
}


// HW125 write data packet 
DISK_RESULT hw125_write_data_packet(
    const uint8_t *buff,
    uint32_t sector_size,
    uint8_t data_token)
{
    DISK_RESULT write_resp;
    uint8_t do_resp; 
    uint8_t crc = HW125_CRC_CMDX; 

    // Wait until the card is no longer busy before sending a CMD 
    hw125_ready_rec();

    // Send data token 
    spi_write(sd_card.spi, &data_token, HW125_SINGLE_BYTE);

    // Send data block 
    spi_write(sd_card.spi, buff, sector_size); 

    // Send CRC 
    spi_write(sd_card.spi, &crc, HW125_SINGLE_BYTE);
    spi_write(sd_card.spi, &crc, HW125_SINGLE_BYTE);

    // Read data response 
    spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);

    // Check the data response 
    if ((do_resp & HW125_DR_FILTER) == HW125_DR_ZERO)
    {
        // Data accepted 
        write_resp = HW125_RES_OK;
    }
    else
    {
        // Data rejected duw to write error or CRC error 
        write_resp = HW125_RES_ERROR; 
    }

    // Return the response 
    return write_resp; 
}


// HW125 IO Control - Get Sector Count 
DISK_RESULT hw125_ioctl_get_sector_count(void *buff)
{
    // Sector count variables 
    DISK_RESULT result; 
    uint8_t do_resp; 
    uint8_t csd[HW125_CSD_REG_LEN]; 
    uint8_t csd_struc; 
    uint8_t n; 
    uint32_t c_size; 

    // Send CMD9 to read the CSD register 
    hw125_send_cmd(HW125_CMD9, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

    // Check the R1 response 
    if (do_resp == HW125_READY_STATE)
    {
        // Read the CSD register data 
        result = hw125_read_data_packet(csd, HW125_CSD_REG_LEN); 

        if (result == HW125_RES_OK)
        {
            // No issues reading the data packet - Get the version number 
            csd_struc = (csd[BYTE_0] >> SHIFT_6) & HW125_CSD_FILTER; 

            // Check the version number to know which bits to read 
            switch (csd_struc) 
            {
                case HW125_CSD_V1:  // CSD Version == 1.0 --> MMC or SDC V1 
                    // Filter the register data 
                    n = ((uint32_t)csd[BYTE_5] & FILTER_4_LSB) + ((((uint32_t)csd[BYTE_10] & FILTER_1_MSB) >> SHIFT_7) + 
                        (((uint32_t)csd[BYTE_9] & FILTER_2_LSB) << SHIFT_1)) + HW125_MULT_OFFSET;

                    c_size = ((uint32_t)(csd[BYTE_8] & FILTER_2_MSB) >> SHIFT_6) +
                             ((uint32_t) csd[BYTE_7] << SHIFT_2) + 
                             ((uint32_t)(csd[BYTE_6] & FILTER_2_LSB) << SHIFT_10) + 
                             HW125_LBA_OFFSET;
                    
                    // Format the sector count 
                    *(uint32_t *) buff = c_size << (n - HW125_MAGIC_SHIFT_V1); 

                    result = HW125_RES_OK; 
                    break;
                
                case HW125_CSD_V2:  // CSD Version == 2.0 --> SDC V2 
                    // Filter the register data 
                    c_size = (uint32_t)csd[BYTE_9] + ((uint32_t)csd[BYTE_8] << SHIFT_8) + 
                            ((uint32_t)(csd[BYTE_7] & FILTER_6_LSB) << SHIFT_16) + HW125_LBA_OFFSET; 

                    // Format the sector count 
                    *(uint32_t *) buff = c_size << HW125_MAGIC_SHIFT_V2;
                    
                    result = HW125_RES_OK; 
                    break;
                
                case HW125_CSD_V3:  // CSD Version == 3.0 --> Currently unsupported 
                    result = HW125_RES_PARERR; 
                    break;
                
                default:  // Unknown 
                    result = HW125_RES_ERROR;
                    break;
            }
        }
    }
    else 
    {
        // Unsuccessfull CMD9 
        result = HW125_RES_ERROR; 
    }

    return result; 
}


// HW125 IO Control - Get Sector Size 
DISK_RESULT hw125_ioctl_get_sector_size(void *buff)
{
    // Assign pre-defined sector size 
    *(uint16_t *)buff = (uint16_t)HW125_SEC_SIZE;
    return HW125_RES_OK;
}


// HW125 IO Control - Control Power 
DISK_RESULT hw125_ioctl_ctrl_pwr(void *buff)
{
    DISK_RESULT result; 
    uint8_t *param = buff; 
    
    // Choose the power operation 
    switch (*param) 
    {
        case HW125_PWR_OFF:  // Turn the Power Flag off 
            hw125_power_off(); 
            result = HW125_RES_OK; 
            break;
        
        case HW125_PWR_ON:  // Turn the Power Flag on 
            hw125_power_on(sd_card.ss_pin);
            result = HW125_RES_OK; 
            break;
        
        case HW125_PWR_CHECK:  // Check the status of the Power Flag 
            *(param++) = (uint8_t)sd_card.pwr_flag;
            result = HW125_RES_OK; 
            break;
        
        default:  // Invalid request 
            result = HW125_RES_PARERR; 
            break; 
    }

    return result; 
}


// HW125 IO Control - Get CSD Register 
DISK_RESULT hw125_ioctl_get_csd(void *buff)
{
    DISK_RESULT result; 
    uint8_t do_resp; 
    uint8_t *csd = buff; 

    // Send CMD9 to read the CSD register 
    hw125_send_cmd(HW125_CMD9, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp); 

    // Check the R1 response 
    if (do_resp == HW125_READY_STATE)
    {
        // Successful CMD9 - proceed to read the CSD register 
        result = hw125_read_data_packet(csd, HW125_CSD_REG_LEN); 
    }
    else
    {
        // Unsucessful CMD9 
        result = HW125_RES_ERROR; 
    }

    return result; 
}


// HW125 IO Control - Get CID Register 
DISK_RESULT hw125_ioctl_get_cid(void *buff)
{
    DISK_RESULT result; 
    uint8_t do_resp; 
    uint8_t *cid = buff; 

    // Send CMD10 to read the CID register 
    hw125_send_cmd(HW125_CMD10, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp); 

    // Check the R1 response 
    if (do_resp == HW125_READY_STATE)
    {
        // Successful CMD10 - proceed to read the CID register 
        result = hw125_read_data_packet(cid, HW125_CID_REG_LEN); 
    }
    else
    {
        // Unsucessful CMD10 
        result = HW125_RES_ERROR; 
    }

    return result; 
}


// HW125 IO Control - Get OCR Register 
DISK_RESULT hw125_ioctl_get_ocr(void *buff)
{
    DISK_RESULT result; 
    uint8_t do_resp; 
    uint8_t *ocr = buff; 

    // Send CMD58 with no arg to check the OCR (trailing 32-bits)
    hw125_send_cmd(HW125_CMD58, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

    if (do_resp == HW125_READY_STATE)
    {
        // Successful CMD58 - proceed to read the OCR register 
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, ocr, HW125_TRAILING_BYTES);
        result = HW125_RES_OK; 
    }
    else
    {
        // Unsuccessful CMD58 
        result = HW125_RES_ERROR; 
    }

    return result; 
}

//=======================================================================================
