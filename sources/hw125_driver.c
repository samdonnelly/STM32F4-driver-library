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
#include "timers.h"
#include "spi_comm.h"
#include "uart_comm.h"

//=======================================================================================


//=======================================================================================
// Function Prototypes 

//==============================================
// Initialization functions 

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
// void hw125_power_on(uint16_t hw125_slave_pin); 
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
 * 
 */
void hw125_power_off(void);

//==============================================


//==============================================
// Command functions 

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

//==============================================


//==============================================
// Status functions 

/**
 * @brief HW125 Power Flag status 
 * 
 * @details Returns the current status of the pwr_flag for sd_card. This function is called
 *          by the FATFS module layer in the ioctl function. 
 * 
 * @return uint8_t : pwr_flag 
 */
uint8_t hw125_power_status(void);

//==============================================


//==============================================
// Data functions 

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

//==============================================


//==============================================
// IO Control functions 

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

//==============================================

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
// Initialization functions 

// HW125 user initialization 
void hw125_user_init(
    GPIO_TypeDef *gpio, 
    SPI_TypeDef *spi, 
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


// HW125 initialization 
DISK_STATUS hw125_init(uint8_t pdrv)
{
    // Local variables 
    uint8_t do_resp; 
    uint8_t init_timer_status;
    uint8_t ocr[HW125_TRAILING_BYTES];
    uint8_t v_range[HW125_TRAILING_BYTES];

    // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
    if (pdrv) return HW125_STATUS_NOINIT; 

    //===================================================
    // Power ON or card insertion and software reset 

    // hw125_power_on(sd_card.ss_pin); 
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

    // TODO integrate this with the power_on function 
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
            if ((uint16_t)((v_range[BYTE_2] << SHIFT_8) | (v_range[BYTE_3])) 
                == HW125_SDCV2_CHECK)
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
    
    // TODO Perform a write_read after deselecting the slave --> Why? 
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


// HW125 power on sequence and software reset 
// void hw125_power_on(uint16_t hw125_slave_pin)
DISK_RESULT hw125_power_on(
    uint16_t hw125_slave_pin)
{
    // Local variables 
    uint8_t di_cmd; 
    uint8_t do_resp; 
    uint8_t cmd_frame[SPI_6_BYTES];

    // TODO test the size of this counter 
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
        spi_write(sd_card.spi, &di_cmd, SPI_1_BYTE);
    }

    //===================================================
    
    //===================================================
    // Software reset 

    // Slave select 
    spi_slave_select(sd_card.gpio, hw125_slave_pin); 

    // Generate a command frame 
    for (uint8_t i = 0; i < SPI_6_BYTES; i++)
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
    spi_write(sd_card.spi, cmd_frame, SPI_6_BYTES); 

    // Read R1 response until it is valid or until it times out 
    do 
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 
    }
    while ((do_resp != HW125_IDLE_STATE) && --num_read); 

    // Slave deselect 
    spi_slave_deselect(sd_card.gpio, hw125_slave_pin); 
    
    // Response timeout 
    if (!num_read) return HW125_RES_ERROR; 

    //===================================================

    // TODO send a data high byte? 
    spi_write(sd_card.spi, &di_cmd, SPI_1_BYTE);

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
    uint8_t  cmd,
    uint32_t arg,
    uint8_t *resp)
 {
    // Local variables 
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
    if (init_timer) return TRUE; 
    
    // Timeout 
    return FALSE; 
}

//=======================================================================================


//=======================================================================================
// Status functions 

// HW125 disk status 
DISK_STATUS hw125_status(uint8_t pdrv)
{
   // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
   if (pdrv) return HW125_STATUS_NOINIT; 
   
   // Return the existing disk status 
   return sd_card.disk_status;
}


// HW125 ready to receive commands 
// TODO Add a status return to account for timeouts 
DISK_RESULT hw125_ready_rec(void)
{
    // Local variables 
    uint8_t resp; 
    uint16_t timer = HW125_PWR_ON_RES_CNT;   // TODO verify/test timer/counter size 

    // Read DO/MISO continuously until it is ready to receive commands 
    do 
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, &resp, SPI_1_BYTE); 
    }
    while (resp != HW125_DATA_HIGH && --timer); 

    if (timer) return HW125_RES_OK; 

    return HW125_RES_ERROR; 
}


// Return the Power Flag status 
uint8_t hw125_power_status(void)
{
    return sd_card.pwr_flag; 
}


// Get the card type 
CARD_TYPE hw125_get_card_type(void) 
{
    return sd_card.card_type; 
}


// Get card presence status 
DISK_RESULT hw125_get_existance(void)
{
    // Call the power on function with the right slave pin 
    return hw125_power_on(sd_card.ss_pin); 
}

//=======================================================================================


//=======================================================================================
// Command functions 

// TODO disable spi in an error state if spi comms go wrong. 
// Otherwise SPI will be enabled at all times 

// HW125 send command messages and return response values 
// TODO Add a status return to account for timeouts 
void hw125_send_cmd(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t  crc,
    uint8_t *resp)
{
    // Local variables 
    uint8_t cmd_frame[SPI_6_BYTES];
    uint8_t num_read = HW125_R1_RESP_COUNT;

    // Wait until the device is ready to accept commands 
    hw125_ready_rec();

    // Generate a command frame 
    for (uint8_t i = 0; i < SPI_6_BYTES; i++)
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
    spi_write(sd_card.spi, cmd_frame, SPI_6_BYTES);

    // Skip the stuff byte sent following CMD12 (stop transmission) 
    if (cmd == HW125_CMD12) spi_write_read(sd_card.spi, HW125_DATA_HIGH, resp, HW125_SINGLE_BYTE);

    // Read R1 response until it is valid or until it times out 
    do 
    {
        spi_write_read(sd_card.spi, HW125_DATA_HIGH, resp, HW125_SINGLE_BYTE);
    }
    while ((*resp & HW125_R1_FILTER) && --num_read);
}

//=======================================================================================


//=======================================================================================
// Read functions 

// HW125 read 
DISK_RESULT hw125_read(
    uint8_t  pdrv, 
    uint8_t  *buff,
    uint32_t sector,
    uint16_t count)
{
    // Local variables 
    DISK_RESULT read_resp;
    uint8_t do_resp = 255;

    // Check that the drive number is zero 
    if (pdrv) return HW125_RES_PARERR;
    
    // Check that the count is valid 
    if (count == NONE) return HW125_RES_PARERR;

    // Check the init status 
    // TODO sometimes this fails 
    if (sd_card.disk_status == HW125_STATUS_NOINIT) return HW125_RES_NOTRDY;

    // Convert the sector number to byte address if it's not SDC V2 (byte address)
    // TODO is this needed for all SDC V2 cards or just block address ones? 
    // if (sd_card.card_type != HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;
    if (sd_card.card_type & HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;

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

    // TODO dummy read? 
    spi_write_read(sd_card.spi, HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 

    // Return the result 
    return read_resp;
}


// HW125 read data packet 
DISK_RESULT hw125_read_data_packet(
    uint8_t *buff,
    uint32_t sector_size)
{
    // Local variables 
    DISK_RESULT read_resp;
    uint8_t do_resp = 200; 
    uint16_t num_read = HW125_DT_RESP_COUNT;   // TODO verify timer/counter size 

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

//=======================================================================================


//=======================================================================================
// Write functions 

// HW125 write 
DISK_RESULT hw125_write(
    uint8_t       pdrv, 
    const uint8_t *buff,
    uint32_t      sector,
    uint16_t      count)
{
    // Local variables 
    DISK_RESULT write_resp; 
    uint8_t do_resp; 
    uint8_t stop_trans = HW125_DT_ONE; 

    // Check that the drive number is zero 
    if (pdrv) return HW125_RES_PARERR;

    // Check that the count is valid 
    if (count == NONE) return HW125_RES_PARERR;

    // Check the init status 
    if (sd_card.disk_status == HW125_STATUS_NOINIT) return HW125_RES_NOTRDY;

    // Check write protection 
    if (sd_card.disk_status == HW125_STATUS_PROTECT) return HW125_RES_WRPRT; 

    // TODO see this line of code in the hw125_read function 
    // Convert the sector number to byte address if it's not SDC V2 (byte address) 
    if (sd_card.card_type != HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;

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
        // Specify the number of sectors to pre-erase to optimize write performance
        // TODO No error condition in place. Also unclear if it's only SDCV1. 
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


// HW125 write data packet 
DISK_RESULT hw125_write_data_packet(
    const uint8_t *buff,
    uint32_t sector_size,
    uint8_t data_token)
{
    // Local variables 
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

    // TODO clear the receive buffer? 

    // Return the response 
    return write_resp; 
}

//=======================================================================================


//=======================================================================================
// IO Control functions 

// HW125 IO control 
DISK_RESULT hw125_ioctl(
    uint8_t pdrv, 
    uint8_t cmd, 
    void    *buff)
{
    // Local variables 
    DISK_RESULT result; 

    // Check that the drive number is zero 
    if (pdrv) return HW125_RES_PARERR;

    // Check the init status 
    if ((sd_card.disk_status == HW125_STATUS_NOINIT) && (cmd != HW125_CTRL_POWER)) 
        return HW125_RES_NOTRDY;

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

    // TODO read a byte? 

    return result; 
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
                    n =   ((uint32_t)csd[BYTE_5]  & FILTER_4_LSB) + 
                        ((((uint32_t)csd[BYTE_10] & FILTER_1_MSB) >> SHIFT_7) + 
                         (((uint32_t)csd[BYTE_9]  & FILTER_2_LSB) << SHIFT_1)) + 
                        HW125_MULT_OFFSET;

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
                    c_size =  (uint32_t) csd[BYTE_9] + 
                            ((uint32_t) csd[BYTE_8] << SHIFT_8) + 
                            ((uint32_t)(csd[BYTE_7] & FILTER_6_LSB) << SHIFT_16) + 
                            HW125_LBA_OFFSET; 

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
    // Local variables 
    uint8_t result; 

    // Assign pre-defined sector size 
    *(uint16_t *)buff = (uint16_t)HW125_SEC_SIZE; 
    result = HW125_RES_OK; 

    return result; 
}


// HW125 IO Control - Control Power 
DISK_RESULT hw125_ioctl_ctrl_pwr(void *buff)
{
    // Local variables 
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
            *(param++) = hw125_power_status(); 
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
    // Local variables 
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
    // Local variables 
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
    // Local variables 
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
