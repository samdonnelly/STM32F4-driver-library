/**
 * @file sd_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SD card driver 
 * 
 * @version 0.1
 * @date 2022-05-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "sd_driver.h"
#include "diskio.h"
#include "spi_comm.h"
#include "timers_driver.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Command values 
#define SD_INDEX_OFFSET       0x40    // First two bits of command index 

// Timers/counters 
#define SD_INIT_TIMER         1000    // Initiate initialization counter 
#define SD_INIT_DELAY         1       // time delay in ms for initiate initialization sequence
#define SD_PWR_ON_COUNTER     10      // General counter for the sd_power_on function 
#define SD_PWR_ON_RES_CNT     0x1FFF  // R1 response counter during power on sequence 
#define SD_R1_RESP_COUNT      10      // Max num of times to read R1 until appropriate response
#define SD_DT_RESP_COUNT      1000    // Max number of times to check the data token 

// Data information 
#define SD_DATA_HIGH          0xFF    // DI/MOSI setpoint and DO/MISO response value 
#define SD_TRAILING_BYTES     4       // Number of bytes in an R3/R7 response after R1 
#define SD_SINGLE_BYTE        1       // For single byte operations 
#define SD_SEC_SIZE           512     // Sector size of the card 
#define SD_CSD_REG_LEN        16      // CSD register length 
#define SD_CID_REG_LEN        16      // CID register length 

// Responses and filters values
#define SD_READY_STATE        0x00    // Drive is ready to send and receive information 
#define SD_IDLE_STATE         0x01    // Drive is in the idle state - after software reset 
#define SD_SDCV2_CHECK        0x1AA   // SDCV2 return value from CMD8 
#define SD_R1_FILTER          0x80    // Filter used to determine a valid R1 response 
#define SD_CCS_FILTER         0x40    // Isolate the CCS bit location in OCR 
#define SD_CSD_FILTER         0x03    // Isolate the CSD register version number 
#define SD_INIT_SUCCESS       0xFE    // Filter to clear the SD_STATUS_NOINIT flag 
#define SD_DR_FILTER          0x1F    // Data response filter for write operations 

// IO Control 
#define SD_LBA_OFFSET         1       // Used in sector size calculation for all cards 
#define SD_MULT_OFFSET        2       // Used in sector size calculation for SDC V1 
#define SD_MAGIC_SHIFT_V1     9       // Magic sector count format shift for CSD V1 
#define SD_MAGIC_SHIFT_V2     10      // Magic sector count format shift for CSD V2 cards 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief SD card command index 
 * 
 * @details Index that determines what command is being requested. Each index is offset 
 *          by SD_INDEX_OFFSET because every command frame sends this value every time 
 *          a command is sent. 
 */
typedef enum {
    SD_CMD0  = SD_INDEX_OFFSET + 0x00,    // GO_IDLE_STATE
    SD_CMD1  = SD_INDEX_OFFSET + 0x01,    // SEND_OP_COND
    SD_CMD8  = SD_INDEX_OFFSET + 0x08,    // SEND_IF_COND
    SD_CMD9  = SD_INDEX_OFFSET + 0x09,    // SEND_CSD
    SD_CMD10 = SD_INDEX_OFFSET + 0x0A,    // SEND_CID
    SD_CMD12 = SD_INDEX_OFFSET + 0x0C,    // STOP_TRANSMISSION
    SD_CMD16 = SD_INDEX_OFFSET + 0x10,    // SET_BLOCKLEN
    SD_CMD17 = SD_INDEX_OFFSET + 0x11,    // READ_SINGLE_BLOCK
    SD_CMD18 = SD_INDEX_OFFSET + 0x12,    // READ_MULTIPLE_BLOCK
    SD_CMD23 = SD_INDEX_OFFSET + 0x17,    // SET_BLOCK_COUNT
    SD_CMD24 = SD_INDEX_OFFSET + 0x18,    // WRITE_BLOCK
    SD_CMD25 = SD_INDEX_OFFSET + 0x19,    // WRITE_MULTIPLE_BLOCK
    SD_CMD41 = SD_INDEX_OFFSET + 0x29,    // APP_SEND_OP_COND
    SD_CMD55 = SD_INDEX_OFFSET + 0x37,    // APP_CMD
    SD_CMD58 = SD_INDEX_OFFSET + 0x3A     // READ_OCR
} sd_command_index_t;


/**
 * @brief SD card arguments
 * 
 * @details Each command needs a certain argument to be sent with it in the command frame. 
 *          For the commands used in reading and writing to a drive, these are all the 
 *          needed arguments. 
 */
typedef enum {
    SD_ARG_NONE  = 0x00000000,  // Zero argument 
    SD_ARG_SUPV  = 0x000001AA,  // Check supply voltage range 
    SD_ARG_BL512 = 0x00000200,  // Block length 512 bytes 
    SD_ARG_HCS   = 0x40000000   // HCS bit set 
} sd_args_t;


/**
 * @brief SD card CRC commands 
 * 
 * @details Each command needs a certain CRC to be sent with it at the end of the command 
 *          frame. For the commands used in reading and writing to a drive, these are all the 
 *          needed CRC values. 
 */
typedef enum {
    SD_CRC_CMDX = 0x01,  // For all other commands 
    SD_CRC_CMD8 = 0x87,  // For command 8 exclusively 
    SD_CRC_CMD0 = 0x95   // For command 0 exclusively 
} sd_crc_cmd_t;


/**
 * @brief SD card disk status 
 * 
 * @details Status of the card being used. The status is used as a check before read and 
 *          write operations to determine whether to proceed or not. If the status is any of 
 *          the options below then the card will not perform any operations. The status gets 
 *          set in the sd_init function and if initialization is successful then the 
 *          status is cleared from being SD_STATUS_NOINIT and will work as normal. 
 * 
 * @see sd_init 
 */
typedef enum {
    SD_STATUS_NOINIT  = 0x01,  // Device has not been initialized and not ready to work 
    SD_STATUS_NODISK  = 0x02,  // No medium in the drive 
    SD_STATUS_PROTECT = 0x04   // Medium is write protected 
} sd_disk_status_t;


/**
 * @brief SD card power status 
 * 
 * @details Status and commands for the power flag (pwr_flag). The power flag is used as a 
 *          reference for the SD card module layer. SD_PWR_OFF and SD_PWR_ON are used to 
 *          set the power flag and determine if the SD card layer wants to set or clear the flag. 
 *          SD_PWR_CHECK is used to identify that the SD card layer wants to know the state 
 *          of the power flag. If the power on sequence during initialization is successful 
 *          then the pwoer flag gets set to on.
 */
typedef enum {
    SD_PWR_OFF,
    SD_PWR_ON,
    SD_PWR_CHECK
} sd_pwr_status_t;


/**
 * @brief SD card data token 
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
    SD_DT_ZERO = 0xFC,    // Data token for CMD25 
    SD_DT_ONE  = 0xFD,    // Stop token for CMD25 
    SD_DT_TWO  = 0xFE     // Data token for CMD17/18/24
} sd_data_token_t;


/**
 * @brief SD card data response filters 
 * 
 * @details When writing to a drive a data response byte will be received immediately after 
 *          the data packet has finished sending. This data response will indicate if the 
 *          transaction was successful or if there were errors. 
 */
typedef enum {
    SD_DR_ZERO = 0x05,   // Data accepted 
    SD_DR_ONE  = 0x0B,   // Data rejected due to a CRC error 
    SD_DR_TWO  = 0x0D    // Data rejected due to a write error 
} sd_data_response_filter_t;


/**
 * @brief CSD register version 
 * 
 * @details Different SD card drives contain different versions of the CSD register. Each 
 *          version stored slightly different information and at different locations in the 
 *          register. The values in this enum are used to verify the CSD register version in  
 *          the sd_ioctl function so the drives sector count can be calculated correctly. 
 * 
 * @see sd_ioctl
 */
typedef enum {
    SD_CSD_V1,   // Version 1.0 
    SD_CSD_V2,   // Version 2.0 
    SD_CSD_V3    // Version 3.0 
} sd_csd_version_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief SD card initialization 
 * 
 * @details Puts the SD card into the ready state so it can start to accept generic read and 
 *          write commands. The type of card is also determined which is used throughout the 
 *          driver to know how to handle data. If all initialization operations are 
 *          successful then the function will clear the SD_STATUS_NOINIT flag and 
 *          return that as the status. If unsuccessful then SD_STATUS_NOINIT will be 
 *          returned and the no further calls can be made to the card. 
 *          
 *          This function is called by the SD card module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @see sd_disk_status_t
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @return DSTATUS : status of the disk drive 
 */
DSTATUS sd_init(BYTE pdrv);


/**
 * @brief SD card disk status 
 * 
 * @details Returns the current status of the card. 
 *          
 *          This function is called by the SD card module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @see sd_disk_status_t
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0)
 * @return DSTATUS : status of the disk drive 
 */
DSTATUS sd_status(BYTE pdrv);


/**
 * @brief SD card read 
 * 
 * @details Reads single or multiple data packets from the SD card. The address to start 
 *          reading from is specified as an argument and the data read gets stored into 
 *          a pointer to a buffer. The function returns the result of the operation. 
 *          
 *          This function is called by the SD card module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @param buff : pointer to the read data buffer that stores the information read 
 * @param sector : start sector number - address to begin reading from 
 * @param count : number of sectors to read 
 * @return DRESULT : result of the read operation 
 */
DRESULT sd_read(
    BYTE pdrv, 
    BYTE *buff,
    DWORD sector,
    UINT count);


/**
 * @brief SD card write 
 * 
 * @details Writes single or multiple data packets to the SD card. The address to start 
 *          writing to and a pointer to a buffer that stores the data to be written are passed 
 *          as arguments. The function returns the result of the operation. 
 *          
 *          This function is called by the SD card module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @param buff : pointer to the data to be written 
 * @param sector : sector number (address) that specifies where to begin writing data 
 * @param count : number of sectors to write (determines single or multiple data packet write) 
 * @return DRESULT : result of the write operation 
 */
DRESULT sd_write(
    BYTE pdrv, 
    const BYTE *buff,
    DWORD sector,
    UINT count);


/**
 * @brief SD card IO control 
 * 
 * @details This function is called to control device specific features and misc functions 
 *          other than generic read and write. Which function to call is specified by the 
 *          cmd argument. The buff argument is a generic void pointer that can be used for 
 *          any of the functions specified by cmd. Each function can cast the pointer to the 
 *          needed data type. buff can also serves as further specification of the operation to 
 *          perform within each sub function. 
 *          
 *          This function is called by the SD card module layer and should not be called 
 *          manually in the application layer. 
 * 
 * @param pdrv : physical drive number to distinguish between target devices (starts at 0) 
 * @param cmd : control command code - specifies sub operation to execute 
 * @param buff : parameter and data buffer - supports the sub operation specified by cmd 
 * @return DRESULT : result of the IO control operation 
 */
DRESULT sd_ioctl(
    BYTE pdrv, 
    BYTE cmd, 
    void *buff);


/**
 * @brief SD card power on sequence
 * 
 * @details This functions sets the SD card into it's native operating mode where it is 
 *          ready to accept native commands. It is called at the beginning of the 
 *          sd_init function to prepare the card. It can also be called in the ioctl 
 *          function by the SD card module layer if needed. <br><br> 
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
 *          SD_PWR_ON. 
 * 
 * @see sd_init
 * @see sd_power_status
 * 
 * @param sd_slave_pin : SS pin that the SD card is connected to 
 * @return DRESULT : result of the power on operation 
 */
DRESULT sd_power_on(uint16_t sd_slave_pin);


/**
 * @brief SD card power off 
 * 
 * @details Sets the pwr_flag of the sd_card to SD_PWR_OFF. The power flag only serves to 
 *          update the status check read from sd_power_status. This function is called 
 *          during initialization if the card type is unknown (initialization failed) and in 
 *          the ioctl function if the SD card module layer requets it. 
 * 
 * @see sd_power_status 
 */
void sd_power_off(void);


/**
 * @brief SD card initiate initialization sequence
 * 
 * @details A sequence that occurs during the initialization process used to initiate 
 *          initialization in the SD card. This sequence is common among all card type 
 *          initialization. Depending on the card, CMD41 or CMD1 is sent repeatedly until 
 *          the card enters its ready state (0x00) or an error has occured. The sd_init 
 *          function calls this function. 
 * 
 * @see sd_init
 * 
 * @param cmd : command to send (either CMD1 or CMD41) 
 * @param arg : argument in the command message that corresponds to the cmd specified 
 * @param resp : pointer to where the command response is stored 
 * @return uint8_t : status of the timer - TRUE or FALSE (for timout) 
 */
uint8_t sd_initiate_init(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t *resp);


/**
 * @brief SD card send command messages to the SD card 
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
void sd_send_cmd(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t  crc,
    uint8_t *resp);


/**
 * @brief SD card read data packet 
 * 
 * @details Verifies the data tocken sent from the card and if correct then reads the 
 *          info from the data packet. The function only reads a single data packet so if 
 *          multiple are required then the function is repeatedly called. The function is 
 *          used by sd_read and sd_ioctl. 
 * 
 * @see sd_read 
 * @see sd_ioctl 
 * 
 * @param buff : buffer to store info from the data packet 
 * @param sector_size : number of bytes read in the packet - sector size of the card 
 * @return DRESULT : result of the read operation 
 */
DRESULT sd_read_data_packet(
    uint8_t *buff,
    uint32_t sector_size);


/**
 * @brief SD card write data packet 
 * 
 * @details Sends a data token to the card to indicate the write operation then proceeds 
 *          to write the data packet to the card. The functions writes a single data packet 
 *          so if multiple packets are needed then the functionis called repeatedly. The 
 *          function is used by sd_write. 
 * 
 * @see sd_write 
 * 
 * @param buff : buffer that stores the info in the packet to be written 
 * @param sector_size : number of bytes written in the packet - sector size of the card 
 * @return DRESULT : result of the write operation 
 */
DRESULT sd_write_data_packet(
    const uint8_t *buff,
    uint32_t sector_size,
    uint8_t data_token);


/**
 * @brief SD card IO Control - Get Sector Count 
 * 
 * @details Reads the sector count from the card based on the card type determined during 
 *          initialization and stores it in a buffer for use in the SD card module layer. 
 *          <br><br>
 *          
 *          The SD card layer calls the sd_ioctl function and requests the sector count. The 
 *          sd_ioctl function then in turn calls this function. 
 * 
 * @param buff : pointer to store sector count - gets typecast to the correct data type 
 * @return DRESULT : result of the read operation 
 */
DRESULT sd_ioctl_get_sector_count(void *buff);


/**
 * @brief SD card IO Control - Get Sector Size 
 * 
 * @details Reads the sector size from the code and stores it in a buffer for use in the 
 *          SD card module layer. <br><br>
 *          
 *          The SD card layer calls the sd_ioctl function and requests the sector size. The 
 *          sd_ioctl function then in turn calls this function. 
 * 
 * @param buff : pointer to store the sector size - gets typecast to the correct data type 
 * @return DRESULT : result of the read operation 
 */
DRESULT sd_ioctl_get_sector_size(void *buff);


/**
 * @brief SD card IO Control - Control Power 
 * 
 * @details Sets the pwr_flag status or reads the pwr_flag status from the code. Called by 
 *          the SD card module layer. <br><br>
 *          
 *          The SD card layer calls the sd_ioctl function and requests a pwr_flag operation. 
 *          The sd_ioctl function then in turn calls this function. 
 * 
 * @param buff : pointer to store the pwr_flag status - gets typecast to pwr_flag status type
 * @return DRESULT : result of the status check 
 */
DRESULT sd_ioctl_ctrl_pwr(void *buff);


/**
 * @brief SD card IO Control - Get CSD Register 
 * 
 * @details Reads the CSD register from the card and stores it in a buffer for use in the 
 *          SD card module layer. <br><br>
 *          
 *          The SD card layer calls the sd_ioctl function and requests the contents of the 
 *          CSD register. The sd_ioctl function then in turn calls this function.
 * 
 * @param buff : pointer to store the CSD register - gets typecast to CSD register type 
 * @return DRESULT : result of the read operation 
 */
DRESULT sd_ioctl_get_csd(void *buff);


/**
 * @brief SD card IO Control - Get CID Register 
 * 
 * @details Reads the CID register from the card and stores it in a buffer for use in the 
 *          SD card module layer. <br><br>
 *          
 *          The SD card layer calls the sd_ioctl function and requests the contents of the 
 *          CID register. The sd_ioctl function then in turn calls this function.
 * 
 * @param buff : pointer to store the CID register - gets typecast to CID register type 
 * @return DRESULT : result of the read operation 
 */
DRESULT sd_ioctl_get_cid(void *buff);


/**
 * @brief SD card IO Control - Get OCR Register 
 * 
 * @details Reads the OCR register from the card and stores it in a buffer for use in the 
 *          SD card module layer. <br><br>
 *          
 *          The SD card layer calls the sd_ioctl function and requests the contents of the 
 *          OCR register. The sd_ioctl function then in turn calls this function.
 * 
 * @param buff : pointer to store the OCR register - gets typecast to the OCR register type 
 * @return DRESULT : result of the read operation 
 */
DRESULT sd_ioctl_get_ocr(void *buff);

//=======================================================================================


//=======================================================================================
// Variables 

// SD card disk record 
typedef struct 
{
    // Peripherals 
    SPI_TypeDef *spi;                   // SPI port used for SD card communication 
    GPIO_TypeDef *gpio;                 // GPIO port used for slave selection 
    TIM_TypeDef *timer;                 // Generic timer used for delays 

    // Tracking information 
    sd_disk_status_t disk_status;       // Disk status - used as a check before read/write 
    sd_card_type_t card_type;           // Type of storage device 
    sd_pwr_status_t pwr_flag;           // Status flag for the FatFs layer 

    // Pins 
    uint16_t ss_pin;                    // Slave select pin for the card (GPIO pin for SPI) 
} 
sd_disk_info_t;


// SD card (drive 0) information 
static sd_disk_info_t sd_card;

//=======================================================================================


//=======================================================================================
// User functions 

// SD card user initialization 
void sd_user_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio, 
    TIM_TypeDef *timer,
    uint16_t sd_slave_pin)
{
    // Peripherals 
    sd_card.spi = spi;
    sd_card.gpio = gpio;
    sd_card.timer = timer;

    // Tracking information 
    sd_card.disk_status = SD_STATUS_NOINIT;
    sd_card.card_type = SD_CT_UNKNOWN;
    sd_card.pwr_flag = SD_PWR_OFF;

    // Pins 
    sd_card.ss_pin = sd_slave_pin;

    // Link the hardware functions to the diskio layer 
    const diskio_dispatch_t dispatch_functions = 
    {
        .disk_initialize = &sd_init,
        .disk_status = &sd_status,
        .disk_read = &sd_read,
        .disk_write = &sd_write,
        .disk_ioctl = &sd_ioctl
    };
    disk_link(&dispatch_functions);
}


// Get the card type 
CARD_TYPE sd_get_card_type(void)
{
    return sd_card.card_type;
}


// SD card ready to receive commands 
DISK_STATUS sd_ready_rec(void)
{
    uint8_t resp;
    uint16_t timer = SD_PWR_ON_RES_CNT;

    // Read DO/MISO continuously until it is ready to receive commands 
    do 
    {
        spi_write_read(sd_card.spi, SD_DATA_HIGH, &resp, BYTE_1);
    }
    while ((resp != SD_DATA_HIGH) && --timer);

    return (DISK_STATUS)(timer != 0);
}


// Check if the card is present 
DISK_STATUS sd_get_existance(void)
{
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);
    DISK_STATUS exist = sd_ready_rec();
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);
    return exist;
}

//=======================================================================================


//=======================================================================================
// diskio functions 

// SD card initialization 
DSTATUS sd_init(BYTE pdrv)
{
    uint8_t do_resp;
    uint8_t init_timer_status;
    uint8_t ocr[SD_TRAILING_BYTES];
    uint8_t v_range[SD_TRAILING_BYTES];

    if (pdrv >= FF_VOLUMES)
    {
        return SD_STATUS_NOINIT;
    }

    //===================================================
    // Power ON or card insertion and software reset 

    if (sd_power_on(sd_card.ss_pin)) 
    {
        sd_card.card_type = SD_CT_UNKNOWN;
        sd_card.disk_status = SD_STATUS_NOINIT;
        return sd_card.disk_status;
    }

    //===================================================

    //===================================================
    // Initialization 
    
    // Select the sd card 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);

    // Send CMD0 with no arg and a valid CRC value 
    sd_send_cmd(SD_CMD0, SD_ARG_NONE, SD_CRC_CMD0, &do_resp);

    // Check the R1 response from CMD0 sent in sd_power_on - do_resp is local so not true 
    if (do_resp == SD_IDLE_STATE)
    {
        // In idle state - Send CMD8 with arg = 0x000001AA and a valid CRC
        sd_send_cmd(SD_CMD8, SD_ARG_SUPV, SD_CRC_CMD8, &do_resp);

        // Check the R1 response from CMD8 
        if (do_resp == SD_IDLE_STATE)
        {
            // No command wrror - Read the trailing 32-bit R7 response 
            spi_write_read(sd_card.spi, SD_DATA_HIGH, v_range, SD_TRAILING_BYTES);

            // Check lower 12-bits of R7 response (big endian format) 
            if ((uint16_t)((v_range[BYTE_2] << SHIFT_8) | (v_range[BYTE_3])) == SD_SDCV2_CHECK)
            {
                // 0x1AA matched (SDCV2+) - Send ACMD41 with the HCS bit set in the arg 
                init_timer_status = sd_initiate_init(SD_CMD41, SD_ARG_HCS, &do_resp);

                // Check timeout
                if (init_timer_status && (do_resp == SD_READY_STATE))
                {
                    // Itialization begun - Send CMD58 to check the OCR (trailing 32-bits)
                    sd_send_cmd(SD_CMD58, SD_ARG_NONE, SD_CRC_CMDX, &do_resp);

                    if (do_resp == SD_READY_STATE)
                    {
                        // Successful CMD58 - proceed to read the OCR register 
                        spi_write_read(sd_card.spi, SD_DATA_HIGH, ocr, SD_TRAILING_BYTES);

                        // Check CCS bit (bit 30) in OCR response (big endian format) 
                        if (ocr[BYTE_0] & SD_CCS_FILTER)
                        {
                            // SDC V2 (block address)
                            sd_card.card_type = SD_CT_SDC2_BLOCK;
                        }
                        else
                        {
                            // SDC V2 (byte address)
                            sd_card.card_type = SD_CT_SDC2_BYTE;

                            // Send CMD16 to change the block size to 512 bytes (for FAT)
                            sd_send_cmd(SD_CMD16, SD_ARG_BL512, SD_CRC_CMDX, &do_resp);
                        }
                    }
                    else
                    {
                        // Unsuccessful CMD58 
                        sd_card.card_type = SD_CT_UNKNOWN;
                    }
                }
                else
                {
                    // Initialization timer timeout 
                    sd_card.card_type = SD_CT_UNKNOWN;
                }
            }
            else 
            {
                // 0x1AA mismatched 
                sd_card.card_type = SD_CT_UNKNOWN;
            }
        }
        else 
        {
            // CMD8 rejected with illegal command error (0x05) - Send ACMD41 
            init_timer_status = sd_initiate_init(SD_CMD41, SD_ARG_NONE, &do_resp);

            // Check timeout and R1 response 
            if (init_timer_status && (do_resp == SD_READY_STATE))
            {
                // Initialization begun - Card is SDC V1 
                sd_card.card_type = SD_CT_SDC1;

                // Send CMD16 to change the block size to 512 bytes (for FAT)
                sd_send_cmd(SD_CMD16, SD_ARG_BL512, SD_CRC_CMDX, &do_resp);
            }

            else
            {
                // Error or timeout - Send CMD1 
                init_timer_status = sd_initiate_init(SD_CMD1, SD_ARG_NONE, &do_resp);

                // Check timeout and R1 response 
                if (init_timer_status && (do_resp == SD_READY_STATE))
                {
                    // Initialization begun - Card is MMC V3
                    sd_card.card_type = SD_CT_MMC;

                    // Send CMD16 to change the block size to 512 bytes (for FAT)
                    sd_send_cmd(SD_CMD16, SD_ARG_BL512, SD_CRC_CMDX, &do_resp);
                }
                else
                {
                    // Initialization timer timeout 
                    sd_card.card_type = SD_CT_UNKNOWN;
                }
            }
        }
    }
    else
    {
        // Not in idle state
        sd_card.card_type = SD_CT_UNKNOWN;
    }

    //===================================================

    // Deselect slave 
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);
    
    // Perform a write_read after deselecting the slave --> Why? 
    spi_write_read(sd_card.spi, SD_DATA_HIGH, &do_resp, SD_SINGLE_BYTE);

    // Status check 
    if (sd_card.card_type == SD_CT_UNKNOWN)
    {
        // Power off the card 
        sd_power_off();

        // Set no init flag 
        sd_card.disk_status = SD_STATUS_NOINIT;
    }
    else
    {
        // Clear no init flag 
        sd_card.disk_status = (SD_STATUS_NOINIT & SD_INIT_SUCCESS);
    }

    return sd_card.disk_status;
}


// SD card disk status 
DSTATUS sd_status(BYTE pdrv)
{
    if (pdrv >= FF_VOLUMES)
    {
        return SD_STATUS_NOINIT;
    }

    return sd_card.disk_status;
}


// SD card read 
DRESULT sd_read(
    BYTE pdrv, 
    BYTE *buff,
    DWORD sector,
    UINT count)
{
    DRESULT read_resp = RES_ERROR;
    uint8_t do_resp = 255;

    if (buff == NULL)
    {
        return RES_ERROR;
    }

    // Check that the drive number and count are both valid 
    if ((pdrv >= FF_VOLUMES) || (count == NONE))
    {
        return RES_PARERR;
    }

    // Check the init status 
    if (sd_card.disk_status == SD_STATUS_NOINIT)
    {
        return RES_NOTRDY;
    }

    // Convert the sector number to byte address if it's not SDC V2 (byte address)
    // Is this needed for all SDC V2 cards or just block address ones? 
    if (sd_card.card_type & SD_CT_SDC2_BYTE)
    {
        sector *= SD_SEC_SIZE;
    }

    // Select the slave device 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);

    // Determine the read operation 
    if (count == SD_SINGLE_BYTE)   // Read one data packet if count == 1
    {
        // Send CMD17 with an arg that specifies the address to start to read 
        sd_send_cmd(SD_CMD17, sector, SD_CRC_CMDX, &do_resp);

        // Read the R1 response 
        if (do_resp == SD_READY_STATE)
        {
            // CMD17 successful - Read initiated 
            read_resp = sd_read_data_packet(buff, SD_SEC_SIZE);
        }
    }
    else   // Read multiple data packets if count > 1
    {
        // Send CMD18 with an arg that specifies the address to start a sequential read 
        sd_send_cmd(SD_CMD18, sector, SD_CRC_CMDX, &do_resp);

        // Read the R1 response 
        if (do_resp == SD_READY_STATE)
        {
            // CMD18 successfull - read initiated 
            do 
            {
                read_resp = sd_read_data_packet(buff, SD_SEC_SIZE);
                buff += SD_SEC_SIZE;
            }
            while (--count && (read_resp != RES_ERROR));

            // Send CMD12 to terminate the read transaction 
            sd_send_cmd(SD_CMD12, SD_ARG_NONE, SD_CRC_CMDX, &do_resp);

            if (do_resp != SD_READY_STATE)
            {
                // CMD12 unsuccessfull 
                read_resp = RES_ERROR;
            }
        }
    }

    // Deselect the slave device 
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);

    // Dummy read 
    spi_write_read(sd_card.spi, SD_DATA_HIGH, &do_resp, SD_SINGLE_BYTE);

    return read_resp;
}


// SD card write 
DRESULT sd_write(
    BYTE pdrv, 
    const BYTE *buff,
    DWORD sector,
    UINT count)
{
    DRESULT write_resp;
    uint8_t do_resp;
    uint8_t stop_trans = SD_DT_ONE;

    if (buff == NULL)
    {
        return RES_ERROR;
    }

    // Check that the drive number and count are both valid 
    if ((pdrv >= FF_VOLUMES) || (count == NONE))
    {
        return RES_PARERR;
    }

    // Check the init status 
    if (sd_card.disk_status == SD_STATUS_NOINIT)
    {
        return RES_NOTRDY;
    }

    // Check write protection 
    if (sd_card.disk_status == SD_STATUS_PROTECT)
    {
        return RES_WRPRT;
    }

    // Convert the sector number to byte address if it's not SDC V2 (byte address) 
    if (sd_card.card_type != SD_CT_SDC2_BYTE)
    {
        sector *= SD_SEC_SIZE;
    }

    // Select the slave device 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);

    // Wait until the card is no longer busy before sending a CMD 
    sd_ready_rec();

    // Determine the write operation 
    if (count ==  SD_SINGLE_BYTE)  // Send one data packet if count == 1
    {
        // Send CMD24 with an arg that specifies the address to start to write 
        sd_send_cmd(SD_CMD24, sector, SD_CRC_CMDX, &do_resp);

        // Check the R1 response 
        if (do_resp == SD_READY_STATE)
        {
            // Successfull CMD24 - Write data packet to card 
            write_resp = sd_write_data_packet(buff, SD_SEC_SIZE, SD_DT_TWO);
        }
        else
        {
            // Unsuccessfull CMD24 
            write_resp = RES_ERROR;
        }
    }
    else  // Send multiple data packets if count > 1
    {
        // Specify the number of sectors to pre-erase to optimize write performance - no 
        // error condition in place. Also unclear if it's only SDCV1. 
        if (sd_card.card_type == SD_CT_SDC1)
        {
            sd_send_cmd(SD_CMD55, SD_ARG_NONE, SD_CRC_CMDX, &do_resp);
            sd_send_cmd(SD_CMD23, count, SD_CRC_CMDX, &do_resp);
        }

        // Send CMD25 that specifies the address to start to write 
        sd_send_cmd(SD_CMD25, sector, SD_CRC_CMDX, &do_resp);

        // Check the R1 response 
        if (do_resp == SD_READY_STATE)
        {
            // CMD25 successful - Write all the sectors or until there is an error 
            do 
            {
                write_resp = sd_write_data_packet(buff, SD_SEC_SIZE, SD_DT_ZERO);
                buff += SD_SEC_SIZE;
            }
            while (--count && (write_resp != RES_ERROR));

            // Wait on busy flag to clear 
            sd_ready_rec();

            // Send stop token 
            spi_write(sd_card.spi, &stop_trans, SD_SINGLE_BYTE);
        }
        else
        {
            // Unsuccessfull CMD25 
            write_resp = RES_ERROR;
        }
    }

    // Wait on busy flag to clear 
    sd_ready_rec();

    // Deselect the slave device
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);

    return write_resp;
}


// SD card IO control 
DRESULT sd_ioctl(
    BYTE pdrv, 
    BYTE cmd, 
    void *buff)
{
    DRESULT result;

    // Check that the drive number and buffer are valid 
    if ((pdrv >= FF_VOLUMES) || (buff == NULL))
    {
        return RES_PARERR;
    }

    // Check the init status 
    if ((sd_card.disk_status == SD_STATUS_NOINIT) && (cmd != CTRL_POWER))
    {
        return RES_NOTRDY;
    }

    // Select the slave card 
    spi_slave_select(sd_card.gpio, sd_card.ss_pin);

    // Choose the misc function 
    switch(cmd)
    {
        case CTRL_SYNC:  // Ensure write operation in disk_write is complete. 
            sd_ready_rec();
            result = RES_OK;
            break;
        
        case GET_SECTOR_COUNT:  // Get the size of the disk 
            result = sd_ioctl_get_sector_count(buff);
            break;
        
        case GET_SECTOR_SIZE:  // Get the sector size 
            result = sd_ioctl_get_sector_size(buff);
            break;
        
        case GET_BLOCK_SIZE:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case CTRL_TRIM:  // Not needed 
            result = RES_PARERR;
            break;
        
        case CTRL_POWER:  // Get/set the power status 
            result = sd_ioctl_ctrl_pwr(buff);
            break;
        
        case CTRL_LOCK:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case CTRL_EJECT:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case CTRL_FORMAT:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case MMC_GET_TYPE:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case MMC_GET_CSD:  // Read and the CSD register 
            result = sd_ioctl_get_csd(buff);
            break;
        
        case MMC_GET_CID:  // Read the CID register 
            result = sd_ioctl_get_cid(buff);
            break;
        
        case MMC_GET_OCR:  // Read tje OCR register 
            result = sd_ioctl_get_ocr(buff);
            break;
        
        case MMC_GET_SDSTAT:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case ATA_GET_REV:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case ATA_GET_MODEL:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        case ATA_GET_SN:  // Currently unsupported 
            result = RES_PARERR;
            break;
        
        default:  // Unknown 
            result = RES_PARERR;
            break;
    }

    // Deselect the slave card 
    spi_slave_deselect(sd_card.gpio, sd_card.ss_pin);

    return result;
}


//=======================================================================================


//=======================================================================================
// Helper functions 

// SD card power on sequence and software reset 
DRESULT sd_power_on(uint16_t sd_slave_pin)
{
    uint8_t di_cmd;
    uint8_t do_resp;
    uint8_t cmd_frame[BYTE_6];
    uint16_t num_read = SD_PWR_ON_RES_CNT;

    //===================================================
    // Power ON or card insertion 

    // Wait for >1ms - delay for after the supply voltage reaches above 2.2V
    tim_delay_ms(sd_card.timer, SD_PWR_ON_COUNTER);

    // Deselect the sd card slave
    spi_slave_deselect(sd_card.gpio, sd_slave_pin);

    // Set the DI/MOSI command high (0xFF) 
    di_cmd = SD_DATA_HIGH;

    // Send DI/MOSI high 10x to wait for more than 74 clock pulses 
    for (uint8_t i = CLEAR; i < SD_PWR_ON_COUNTER; i++) 
    {
        spi_write(sd_card.spi, &di_cmd, BYTE_1);
    }

    //===================================================
    
    //===================================================
    // Software reset 

    // Slave select 
    spi_slave_select(sd_card.gpio, sd_slave_pin);

    // Generate a command frame 
    for (uint8_t i = CLEAR; i < BYTE_6; i++)
    {
        switch (i)
        {
            case BYTE_0:
                cmd_frame[i] = SD_CMD0;
                break;
            case BYTE_5:
                 cmd_frame[i] = SD_CRC_CMD0;
                break;
            default:
                cmd_frame[i] = (uint8_t)(SD_ARG_NONE >> SHIFT_8*(BYTE_4 - i));
                break;
        }
    }

    // Transmit command 
    spi_write(sd_card.spi, cmd_frame, BYTE_6);

    // Read R1 response until it is valid or until it times out 
    do
    {
        spi_write_read(sd_card.spi, SD_DATA_HIGH, &do_resp, SD_SINGLE_BYTE);
    }
    while ((do_resp != SD_IDLE_STATE) && --num_read);

    // Slave deselect 
    spi_slave_deselect(sd_card.gpio, sd_slave_pin);
    
    // Response timeout 
    if (!num_read)
    {
        return RES_ERROR;
    }

    //===================================================

    // Send a data high byte --> Why? 
    spi_write(sd_card.spi, &di_cmd, BYTE_1);

    // Set the Power Flag status to on 
    sd_card.pwr_flag = SD_PWR_ON;

    return RES_OK;
}


// Set the Power Flag status to off 
void sd_power_off(void)
{
    sd_card.pwr_flag = SD_PWR_OFF;
}


// SD card initiate initialization sequence
uint8_t sd_initiate_init(
    uint8_t cmd,
    uint32_t arg,
    uint8_t *resp)
 {
    uint16_t init_timer = SD_INIT_TIMER;

    // Send CMD1 or ACMD41 (depending on the card type) to initiate initialization 
    do
    {
        if (cmd == SD_CMD1)
        {
            sd_send_cmd(SD_CMD1, SD_ARG_NONE, SD_CRC_CMDX, resp);
        }
        else
        {
            sd_send_cmd(SD_CMD55, SD_ARG_NONE, SD_CRC_CMDX, resp);
            sd_send_cmd(SD_CMD41, arg, SD_CRC_CMDX, resp);
        }

        // Delay 1ms --> SD_INIT_DELAY * SD_INIT_TIMER = 1000ms (recommended delay) 
        tim_delay_ms(sd_card.timer, SD_INIT_DELAY);
    }
    while ((*resp == SD_IDLE_STATE) && --init_timer);

    // Remaining counts on the timer means the operation did not time out so we can 
    // proceed with initialization. 
    return (uint8_t)(init_timer != 0);
}


// SD card send command messages and return response values - add a status return for timeouts 
void sd_send_cmd(
    uint8_t cmd,
    uint32_t arg,
    uint8_t crc,
    uint8_t *resp)
{
    uint8_t cmd_frame[BYTE_6];
    uint8_t num_read = SD_R1_RESP_COUNT;

    // Wait until the device is ready to accept commands 
    sd_ready_rec();

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
    if (cmd == SD_CMD12)
    {
        spi_write_read(sd_card.spi, SD_DATA_HIGH, resp, SD_SINGLE_BYTE);
    }

    // Read R1 response until it is valid or until it times out 
    do 
    {
        spi_write_read(sd_card.spi, SD_DATA_HIGH, resp, SD_SINGLE_BYTE);
    }
    while ((*resp & SD_R1_FILTER) && --num_read);
}


// SD card read data packet 
DRESULT sd_read_data_packet(
    uint8_t *buff,
    uint32_t sector_size)
{
    DRESULT read_resp = RES_ERROR;
    uint8_t do_resp = 200;
    uint16_t num_read = SD_DT_RESP_COUNT;

    // Read the data token 
    do 
    {
        spi_write_read(sd_card.spi, SD_DATA_HIGH, &do_resp, SD_SINGLE_BYTE);
    }
    while ((do_resp != SD_DT_TWO) && --num_read);

    // Check the R1 response 
    if (do_resp == SD_DT_TWO)
    {
        // Valid data token is detected - read the data packet 
        spi_write_read(sd_card.spi, SD_DATA_HIGH, buff, sector_size);

        // Discard the two CRC bytes 
        spi_write_read(sd_card.spi, SD_DATA_HIGH, &do_resp, SD_SINGLE_BYTE);
        spi_write_read(sd_card.spi, SD_DATA_HIGH, &do_resp, SD_SINGLE_BYTE);

        // Operation success 
        read_resp = RES_OK;
    }

    return read_resp;
}


// SD card write data packet 
DRESULT sd_write_data_packet(
    const uint8_t *buff,
    uint32_t sector_size,
    uint8_t data_token)
{
    DRESULT write_resp = RES_ERROR;
    uint8_t do_resp;
    uint8_t crc = SD_CRC_CMDX;

    // Wait until the card is no longer busy before sending a CMD 
    sd_ready_rec();

    // Send data token 
    spi_write(sd_card.spi, &data_token, SD_SINGLE_BYTE);

    // Send data block 
    spi_write(sd_card.spi, buff, sector_size);

    // Send CRC 
    spi_write(sd_card.spi, &crc, SD_SINGLE_BYTE);
    spi_write(sd_card.spi, &crc, SD_SINGLE_BYTE);

    // Read data response 
    spi_write_read(sd_card.spi, SD_DATA_HIGH, &do_resp, SD_SINGLE_BYTE);

    // Check the data response 
    if ((do_resp & SD_DR_FILTER) == SD_DR_ZERO)
    {
        // Data accepted. If this is not true then the data was likely rejected due to a 
        // write error or CRC error. 
        write_resp = RES_OK;
    }

    return write_resp;
}


// SD card IO Control - Get Sector Count 
DRESULT sd_ioctl_get_sector_count(void *buff)
{
    // Sector count variables 
    DRESULT result = RES_ERROR;
    uint8_t do_resp;
    uint8_t csd[SD_CSD_REG_LEN];
    uint8_t csd_struc;
    uint8_t n;
    uint32_t c_size;
    uint32_t *sector_count = (uint32_t *)buff;

    // Send CMD9 to read the CSD register 
    sd_send_cmd(SD_CMD9, SD_ARG_NONE, SD_CRC_CMDX, &do_resp);

    // Check the R1 response 
    if (do_resp == SD_READY_STATE)
    {
        // Successfull CMD9 
        // Read the CSD register data 
        result = sd_read_data_packet(csd, SD_CSD_REG_LEN);

        if (result == RES_OK)
        {
            // No issues reading the data packet - Get the version number 
            csd_struc = (csd[BYTE_0] >> SHIFT_6) & SD_CSD_FILTER;

            // Check the version number to know which bits to read 
            switch (csd_struc) 
            {
                case SD_CSD_V1:  // CSD Version == 1.0 --> MMC or SDC V1 
                    // Filter the register data 
                    n = ((uint32_t)csd[BYTE_5] & FILTER_4_LSB) + ((((uint32_t)csd[BYTE_10] & FILTER_1_MSB) >> SHIFT_7) + 
                        (((uint32_t)csd[BYTE_9] & FILTER_2_LSB) << SHIFT_1)) + SD_MULT_OFFSET;

                    c_size = ((uint32_t)(csd[BYTE_8] & FILTER_2_MSB) >> SHIFT_6) +
                             ((uint32_t) csd[BYTE_7] << SHIFT_2) + 
                             ((uint32_t)(csd[BYTE_6] & FILTER_2_LSB) << SHIFT_10) + 
                             SD_LBA_OFFSET;
                    
                    // Format the sector count 
                    *sector_count = c_size << (n - SD_MAGIC_SHIFT_V1);

                    result = RES_OK;
                    break;
                
                case SD_CSD_V2:  // CSD Version == 2.0 --> SDC V2 
                    // Filter the register data 
                    c_size = (uint32_t)csd[BYTE_9] + ((uint32_t)csd[BYTE_8] << SHIFT_8) + 
                            ((uint32_t)(csd[BYTE_7] & FILTER_6_LSB) << SHIFT_16) + SD_LBA_OFFSET;

                    // Format the sector count 
                    *sector_count = c_size << SD_MAGIC_SHIFT_V2;
                    
                    result = RES_OK;
                    break;
                
                case SD_CSD_V3:  // CSD Version == 3.0 --> Currently unsupported 
                    result = RES_PARERR;
                    break;
                
                default:  // Unknown 
                    result = RES_ERROR;
                    break;
            }
        }
    }

    return result;
}


// SD card IO Control - Get Sector Size 
DRESULT sd_ioctl_get_sector_size(void *buff)
{
    // Assign pre-defined sector size 
    *(uint16_t *)buff = (uint16_t)SD_SEC_SIZE;
    return RES_OK;
}


// SD card IO Control - Control Power 
DRESULT sd_ioctl_ctrl_pwr(void *buff)
{
    DRESULT result;
    uint8_t *param = (uint8_t *)buff;
    
    // Choose the power operation 
    switch (*param) 
    {
        case SD_PWR_OFF:  // Turn the Power Flag off 
            sd_power_off();
            result = RES_OK;
            break;
        
        case SD_PWR_ON:  // Turn the Power Flag on 
            sd_power_on(sd_card.ss_pin);
            result = RES_OK;
            break;
        
        case SD_PWR_CHECK:  // Check the status of the Power Flag 
            *(param++) = (uint8_t)sd_card.pwr_flag;
            result = RES_OK;
            break;
        
        default:  // Invalid request 
            result = RES_PARERR;
            break;
    }

    return result;
}


// SD card IO Control - Get CSD Register 
DRESULT sd_ioctl_get_csd(void *buff)
{
    DRESULT result = RES_ERROR;
    uint8_t do_resp;
    uint8_t *csd = (uint8_t *)buff;

    // Send CMD9 to read the CSD register 
    sd_send_cmd(SD_CMD9, SD_ARG_NONE, SD_CRC_CMDX, &do_resp);

    // Check the R1 response 
    if (do_resp == SD_READY_STATE)
    {
        // Successful CMD9 - proceed to read the CSD register 
        result = sd_read_data_packet(csd, SD_CSD_REG_LEN);
    }

    return result;
}


// SD card IO Control - Get CID Register 
DRESULT sd_ioctl_get_cid(void *buff)
{
    DRESULT result = RES_ERROR;
    uint8_t do_resp;
    uint8_t *cid = (uint8_t *)buff;

    // Send CMD10 to read the CID register 
    sd_send_cmd(SD_CMD10, SD_ARG_NONE, SD_CRC_CMDX, &do_resp);

    // Check the R1 response 
    if (do_resp == SD_READY_STATE)
    {
        // Successful CMD10 - proceed to read the CID register 
        result = sd_read_data_packet(cid, SD_CID_REG_LEN);
    }

    return result;
}


// SD card IO Control - Get OCR Register 
DRESULT sd_ioctl_get_ocr(void *buff)
{
    DRESULT result = RES_ERROR;
    uint8_t do_resp;
    uint8_t *ocr = (uint8_t *)buff;

    // Send CMD58 with no arg to check the OCR (trailing 32-bits)
    sd_send_cmd(SD_CMD58, SD_ARG_NONE, SD_CRC_CMDX, &do_resp);

    if (do_resp == SD_READY_STATE)
    {
        // Successful CMD58 - proceed to read the OCR register 
        spi_write_read(sd_card.spi, SD_DATA_HIGH, ocr, SD_TRAILING_BYTES);
        result = RES_OK;
    }

    return result;
}

//=======================================================================================
