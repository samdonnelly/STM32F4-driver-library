/**
 * @file HW_125_microSD_driver.c
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

#include "HW_125_microSD_driver.h"
#include "timers.h"
#include "spi_comm.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

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


/**
 * @brief HW125 read data packet 
 * 
 * @details Gets called by hw125_read for both single and multiple data packet reads
 *          to get all the requesed information from the storage device. 
 * 
 * @param buff 
 * @param sector_size 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_read_data_packet(
    uint8_t *buff,
    uint32_t sector_size);

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief HW125 disk information 
 * 
 */
typedef struct {
    uint8_t  disk_status;
    uint8_t  card_type;
    uint8_t  pwr_flag;
    uint16_t ss_pin;
} hw125_disk_info_t;

// TODO figure out if the pwr_flag is needed 

//=======================================================================================


//=======================================================================================
// Variables 

// SD card information 
static hw125_disk_info_t sd_card;

//=======================================================================================


// TODO add all functions to user_diskio.c functions for FATFS reference 

//=======================================================================================
// Initialization and status functions 

// HW125 user initialization 
void hw125_user_init(uint16_t hw125_slave_pin)
{
    // Define SD card information 
    sd_card.disk_status = HW125_STATUS_NOINIT;
    sd_card.card_type = HW125_CT_UNKNOWN;
    sd_card.pwr_flag = HW125_PWR_OFF;
    sd_card.ss_pin = hw125_slave_pin;
}


// HW125 initialization 
DISK_STATUS hw125_init(uint8_t pdrv)
{
    // Local variables 
    uint8_t do_resp; 
    uint8_t init_timer_status;
    uint8_t ocr[HW125_TRAIL_RESP_BYTES];
    uint8_t v_range[HW125_TRAIL_RESP_BYTES];

    // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
    if (pdrv) return HW125_STATUS_NOINIT; 

    // Power on 
    hw125_power_on(sd_card.ss_pin);

    // Select the sd card slave 
    spi2_slave_select(sd_card.ss_pin);

    // Send CMD0 with no arg and a valid CRC value 
    hw125_send_cmd(HW125_CMD0, HW125_ARG_NONE, HW125_CRC_CMD0, &do_resp);

    // Check the R1 response from CMD0 
    if (do_resp == HW125_IDLE_STATE)
    {
        // Idle state 
        
        // Send CMD8 with arg = 0x000001AA and a valid CRC
        hw125_send_cmd(HW125_CMD8, HW125_ARG_SUPV, HW125_CRC_CMD8, &do_resp);

        // Check the R1 response from CMD8 
        if (do_resp == HW125_IDLE_STATE)
        {
            // Idle state - 32 trailing bits incoming 

            // Read trailing 32-bits 
            spi2_write_read(HW125_DATA_HIGH, v_range, HW125_TRAIL_RESP_BYTES);

            // Check lower 12-bits of R7 response (big endian format) 
            if ((uint16_t)((v_range[BYTE_2] << SHIFT_8) | (v_range[BYTE_3])) 
                == HW125_CMD8_R7_RESP)
            {
                // 0x1AA matched - voltage range 2.7-3.6V
                
                // Send ACMD41 with the HCS bit set in the arg 
                init_timer_status = hw125_initiate_init(HW125_CMD41, HW125_ARG_HCS, &do_resp);

                // Check timeout
                if (init_timer_status && (do_resp == HW125_INIT_STATE))
                {
                    // Initiate initialization begun - No init timer timeout 

                    // Send CMD58 with no arg to check the OCR (trailing 32-bits)
                    hw125_send_cmd(HW125_CMD58, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);
                    spi2_write_read(HW125_DATA_HIGH, ocr, HW125_TRAIL_RESP_BYTES);

                    // Check CCS bit (bit 30) in OCR response (big endian format) 
                    if (ocr[BYTE_0] & HW125_CCS_SET)
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
                    // Initiate initialization timer timeout 
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
            // CMD8 rejected with illegal command error (0x05)

            // Send ACMD41 with no arg 
            init_timer_status = hw125_initiate_init(HW125_CMD41, HW125_ARG_NONE, &do_resp);

            // Check timeout and R1 response 
            if (init_timer_status && (do_resp == HW125_INIT_STATE))
            {
                // Initiate initialization begun - No init timer timeout 

                // SDC V1 
                sd_card.card_type = HW125_CT_SDC1;

                // Send CMD16 to change the block size to 512 bytes (for FAT)
                hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
            }

            else
            {
                // Error or timeout 

                // Send CMD1 with non arg 
                init_timer_status = hw125_initiate_init(HW125_CMD1, HW125_ARG_NONE, &do_resp);

                // Check timeout and R1 response 
                if (init_timer_status && (do_resp == HW125_INIT_STATE))
                {
                    // Initiate initialization begun - No init timer timeout 

                    // MMC V3
                    sd_card.card_type = HW125_CT_MMC;

                    // Send CMD16 to change the block size to 512 bytes (for FAT)
                    hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
                }
                else
                {
                    // Initiate initialization timer timeout 
                    sd_card.card_type = HW125_CT_UNKNOWN;
                }
            }
        }
    }
    else
    {
        // Not in idle state
        sd_card.card_type = HW125_CT_UNKNOWN;

        // TODO add timer in spi read function that will return an error if it times out
    }

    // Deselect slave 
    spi2_slave_deselect(sd_card.ss_pin);
    
    // Perform a write_read after deselecting the slave --> Why? 

    // Status check 
    if (sd_card.card_type == HW125_CT_UNKNOWN)
    {
        // TODO Power off?

        // Set no init flag 
        sd_card.disk_status = HW125_STATUS_NOINIT;
    }
    else
    {
        // Clear no init flag 
        sd_card.disk_status = (HW125_STATUS_NOINIT & HW125_INIT_SUCCESS); 
    }

    // Return the card type for fault handling 
    return sd_card.disk_status;
}


// HW125 power on sequence
void hw125_power_on(uint16_t hw125_slave_pin)
{
    // Local variables 
    uint8_t di_cmd; 

    // Wait 1ms to allow for voltage to reach above 2.2V
    tim9_delay_ms(HW125_POWER_ON_DELAY);

    // Deselect the sd card slave
    spi2_slave_deselect(hw125_slave_pin);

    // Set the DI/MOSI command high (0xFF) 
    di_cmd = HW125_DATA_HIGH;

    // Send DI/MOSI (0xFF) 10x to wait for more than 74 clock pulses 
    for (uint8_t i = 0; i < HW125_POWER_ON_TIMER; i++)
    {
        spi2_write(&di_cmd, SPI_1_BYTE);
    }
}


// HW125 initiate initialization sequence
uint8_t hw125_initiate_init(
    uint8_t  cmd,
    uint32_t arg,
    uint8_t *resp)
 {
    // Local variables 
    uint16_t init_timer = HW125_INIT_TIMER;

    // Send the initiate initialization command until an appropriate response is received
    // Allow up to 1 second for the process before timing out 
    do
    {
        if (cmd == HW125_CMD1)
        {
            // Send CMD1 
            hw125_send_cmd(HW125_CMD1, HW125_ARG_NONE, HW125_CRC_CMDX, resp);
        }
        else
        {
            // Send ACMD41 (CMD55+CMD41)
            hw125_send_cmd(HW125_CMD55, HW125_ARG_NONE, HW125_CRC_CMDX, resp);
            hw125_send_cmd(HW125_CMD41, arg, HW125_CRC_CMDX, resp);
        }

        // Delay 1ms 
        tim9_delay_ms(HW125_INIT_DELAY);

        // Decrement timer 
        init_timer--;
    }
    while(init_timer && (*resp == HW125_IDLE_STATE));

    // Return the timer status 
    if (init_timer)
    {
        // Timer is not zero - no timeout 
        return TRUE;
    }
    else
    {
        // Timer is zero - timeout 
        return FALSE;
    }
 }


 // HW125 disk status 
 DISK_STATUS hw125_status(uint8_t pdrv)
 {
    if (pdrv) 
    { 
        // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
        return HW125_STATUS_NOINIT; 
    }
    else 
    {
        // Return the existing disk status 
        return sd_card.disk_status;
    }
 }

//=======================================================================================


//=======================================================================================
// Command functions 

// TODO disable spi in an error state if spi comms go wrong. 
// Otherwise SPI will be enabled at all times 

// HW125 send command messages and return response values 
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
    spi2_write(cmd_frame, SPI_6_BYTES);

    // Skip the stuff byte sent following CMD12 (stop transmission) 
    if (cmd == HW125_CMD12) spi2_write_read(HW125_DATA_HIGH, resp, HW125_SINGLE_BYTE);

    // Read R1 response until it is valid or until it times out 
    do 
    {
        spi2_write_read(HW125_DATA_HIGH, resp, HW125_SINGLE_BYTE);
    }
    while((*resp & HW125_R1_RESP_FILTER) && --num_read);
}


// HW125 ready to receive commands 
void hw125_ready_rec(void)
{
    // TODO create a timeout - what will happen if it times out? 

    // Local variables 
    uint8_t resp;

    // Read DO continuously until it is ready to receive commands 
    do 
    {
        spi2_write_read(HW125_DATA_HIGH, &resp, SPI_1_BYTE);
    }
    while(resp != HW125_DATA_HIGH);
}

//=======================================================================================


//=======================================================================================
// Read functions 

// TODO add remaining functions for FATFS requirements 

// HW125 read 
DISK_RESULT hw125_read(
    uint8_t  pdrv, 
    uint8_t  *buff,
    uint32_t sector,
    uint16_t count)
{
    // Local variables 
    DISK_RESULT read_resp;
    uint8_t do_resp;

    // Check that the drive number is zero 
    if (pdrv) return HW125_RES_PARERR;
    
    // Check that the count is valid 
    if (count == HW125_NO_BYTE) return HW125_RES_PARERR;

    // Check the init status 
    if (sd_card.disk_status == HW125_STATUS_NOINIT) return HW125_RES_NOTRDY;

    // Convert the sector number to byte address if it's not SDC V2 (byte address) 
    if (sd_card.card_type != HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;

    // Select the slave device 
    spi2_slave_select(sd_card.ss_pin);

    // Determine the read operation 
    if (count == HW125_SINGLE_BYTE)   // Read one data packet if count == 1
    {
        // Send CMD17 with an arg that specifies the address to start to read 
        hw125_send_cmd(HW125_CMD17, sector, HW125_CRC_CMDX, &do_resp);

        // Read the R1 response 
        if (do_resp == HW125_BEGIN_READ)
        {
            // Read initiated 
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
        if (do_resp == HW125_BEGIN_READ)
        {
            // Read initiated 
            do 
            {
                read_resp = hw125_read_data_packet(buff, HW125_SEC_SIZE);
                buff += HW125_SEC_SIZE; 
            }
            while (--count && (read_resp != HW125_RES_ERROR));

            // Send CMD12 to terminate the read transaction 
            hw125_send_cmd(HW125_CMD12, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

            if (do_resp != HW125_END_READ)
            {
                read_resp = HW125_RES_ERROR;
            }
        }
        else
        {
            // Unsuccessful CMD18 
            read_resp = HW125_RES_ERROR;
        }

        // For MMC a CMD23 can be sent prior to CMD18 to specify the number of transfer blocks. 
        // If this is the case then the read transaction is initiated as a pre-defined multiple
        // block transfer (not open ended) and the read operation is terminated at the last 
        // block trasnfer. 
    }

    // Deselect the slave device 
    spi2_slave_deselect(sd_card.ss_pin);

    // TODO dummy read? 

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
    uint8_t do_resp;
    uint8_t num_read = HW125_DT_RESP_COUNT; 

    // Read the data token 
    do 
    {
        spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);
    }
    while((do_resp != HW125_DT_TWO) && --num_read);

    if (do_resp == HW125_DT_TWO)
    {
        // Valid data token is detected - read the data packet(s) 
        spi2_write_read(HW125_DATA_HIGH, buff, sector_size);

        // Discard the two CRC bytes 
        spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_CRC_DISCARD);

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

    // Check that the drive number is zero 
    if (pdrv) return HW125_RES_PARERR;

    // Check that the count is valid 
    if (count == HW125_NO_BYTE) return HW125_RES_PARERR;

    // Check the init status 
    if (sd_card.disk_status == HW125_STATUS_NOINIT) return HW125_RES_NOTRDY;

    // Convert the sector number to byte address if it's not SDC V2 (byte address) 
    if (sd_card.card_type != HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;

    // Select the slave device 
    spi2_slave_select(sd_card.ss_pin);

    // Wait until the card is no longer busy before sending a CMD 

    // Determine the write operation 
    if (count ==  HW125_SINGLE_BYTE)  // Send one data packet if count == 1
    {
        // Send CMD24 with an arg that specifies the address to start to write 
        hw125_send_cmd(HW125_CMD24, sector, HW125_CRC_CMDX, &do_resp);

        // Check the R1 response 
        if (do_resp == HW125_BEGIN_WRITE)
        {
            // TODO add the following functionality 

            // Successfull CMD24 - proceed to send a data packet to the card 

            // Must wait at least 1 byte before writing the data packet. 
            // Wait until there is no internal process taking place? 

            // Wait until the card is no longer busy before sending a CMD? 

            // Send data token 

            // Send data block 

            // Send CRC 

            // Read data response 

            // Wait on busy flag to clear 

            // Packet frame is the same as read operations. Most cards cannot change the 
            // write block size that is fixed to 512 bytes. 

            // CRC field can have any fixed value unless the CRC is enabled 

            // The card responds a data response byte immediately following the data 
            // packet from the host. 

            // A busy flag trails the data response and the host must suspend the next 
            // command or data transmission until the card goes ready. 

            // When waiting for the busy flag to clear, the SS pin can be deselcted to 
            // allow other SPI devices to proceed if needed. 

            // DO will be driven low when an internal process is still taking place. 
            // Check if the card is busy prior to each CMD and data packet. 

            // The internal write process is initiated a byte after the data response. 
            // This means 8 SCLK clocks are required to initiate an internal write. This 
            // process is irrespective of the SS pin status. 
        }
        else
        {
            // Unsuccessfull CMD24 
            write_resp = HW125_RES_ERROR;
        }
    }
    else  // Send multiple data packets if count > 1
    {
        // Can send a ACMD23 prior to CMD25 to specify the number of sectors to 
        // pre-erase 

        // Send CMD25 with an arg that specifies the address to start to write 
        hw125_send_cmd(HW125_CMD25, sector, HW125_CRC_CMDX, &do_resp);

        // Check the R1 response 
        if (do_resp == HW125_BEGIN_WRITE)
        {
            // Successful CMD25 - start sequentially writing data blocks from start address

            // The write transaction continues until it is terminated with a stop token 

            // Must wait at least 1 byte between the CMD25 response and sending the first 
            // data packet 

            // The busy flag will be output after every data packet as well as the stop 
            // token so you have to wait until the drive is no longer busy 
        }
        else
        {
            // Unsuccessfull CMD25 
        }
    }

    // Deselect the slave device
    spi2_slave_deselect(sd_card.ss_pin); 

    // Return the write opration status 
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
    // 
}

//=======================================================================================
