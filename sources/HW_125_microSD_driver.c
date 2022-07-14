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
#include "uart_comm.h"

//=======================================================================================


//=======================================================================================
// Global variables 

// Debugging 
extern volatile uint8_t fail_state; 
extern volatile uint8_t func_num[30]; 
extern volatile uint8_t mount_seq[100]; 
extern volatile uint8_t mount_it; 

//=======================================================================================


//=======================================================================================
// Function Prototypes 

//==============================================
// Initialization functions 

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
 * @brief HW125 power on sequence
 * 
 * @details 
 * 
 * @param hw125_slave_pin 
 */
void hw125_power_on(uint16_t hw125_slave_pin);


/**
 * @brief HW125 Power Flag set to off 
 * 
 * @details 
 * 
 */
void hw125_power_off(void);

//==============================================


//==============================================
// Command functions 

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

//==============================================


//==============================================
// Status functions 

/**
 * @brief HW125 Power Flag status 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t hw125_power_status(void);


/**
 * @brief HW125 ready to receive commands 
 * 
 * @details 
 * 
 */
void hw125_ready_rec(void);

//==============================================


//==============================================
// Data functions 

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


/**
 * @brief HW125 write data packet 
 * 
 * @details 
 * 
 * @param buff 
 * @param sector_size 
 * @return DISK_RESULT 
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
 * @details 
 * 
 * @param buff 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_ioctl_get_sector_count(void *buff);


/**
 * @brief HW125 IO Control - Get Sector Size 
 * 
 * @details 
 * 
 * @param buff 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_ioctl_get_sector_size(void *buff);


/**
 * @brief HW125 IO Control - Control Power 
 * 
 * @details 
 * 
 * @param buff 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_ioctl_ctrl_pwr(void *buff);


/**
 * @brief HW125 IO Control - Get CSD Register 
 * 
 * @details 
 * 
 * @param buff 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_ioctl_get_csd(void *buff);


/**
 * @brief HW125 IO Control - Get CID Register 
 * 
 * @details 
 * 
 * @param buff 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_ioctl_get_cid(void *buff);


/**
 * @brief HW125 IO Control - Get OCR Register 
 * 
 * @details 
 * 
 * @param buff 
 * @return DISK_RESULT 
 */
DISK_RESULT hw125_ioctl_get_ocr(void *buff);

//==============================================

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

//=======================================================================================


//=======================================================================================
// Variables 

// SD card information 
static hw125_disk_info_t sd_card;

//=======================================================================================


//=======================================================================================
// Initialization functions 

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

    func_num[0] = func_num[0] + 1; 
    mount_seq[mount_it] = 0; 
    mount_it++; 

    // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
    if (pdrv) 
    {
        fail_state = 0; 
        return HW125_STATUS_NOINIT; 
    }

    // Power on 
    hw125_power_on(sd_card.ss_pin);

    // Select the sd card slave 
    spi2_slave_select(sd_card.ss_pin);

    // Send CMD0 with no arg and a valid CRC value 
    hw125_send_cmd(HW125_CMD0, HW125_ARG_NONE, HW125_CRC_CMD0, &do_resp);

    // Check the R1 response from CMD0 send in hw125_power_on
    if (do_resp == HW125_IDLE_STATE)
    {
        // In idle state - Send CMD8 with arg = 0x000001AA and a valid CRC
        hw125_send_cmd(HW125_CMD8, HW125_ARG_SUPV, HW125_CRC_CMD8, &do_resp);

        // Check the R1 response from CMD8 
        if (do_resp == HW125_IDLE_STATE)
        {
            // No command wrror - Read the trailing 32-bit R7 response 
            spi2_write_read(HW125_DATA_HIGH, v_range, HW125_TRAIL_RESP_BYTES);

            // Check lower 12-bits of R7 response (big endian format) 
            if ((uint16_t)((v_range[BYTE_2] << SHIFT_8) | (v_range[BYTE_3])) 
                == HW125_CMD8_R7_RESP)
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
                        // Unsuccessful CMD58 
                        fail_state = 1; 
                        sd_card.card_type = HW125_CT_UNKNOWN;
                    }
                }
                else
                {
                    // Initialization timer timeout 
                    fail_state = 2; 
                    sd_card.card_type = HW125_CT_UNKNOWN;
                }
            }
            else 
            {
                // 0x1AA mismatched 
                fail_state = 3; 
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
                    fail_state = 4; 
                    sd_card.card_type = HW125_CT_UNKNOWN;
                }
            }
        }
    }
    else
    {
        // Not in idle state
        fail_state = 5; 
        sd_card.card_type = HW125_CT_UNKNOWN;

        // TODO add timer in spi read function that will return an error if it times out
    }

    // Deselect slave 
    spi2_slave_deselect(sd_card.ss_pin);
    
    // TODO Perform a write_read after deselecting the slave --> Why? 
    spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 

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

    // Return the card type for fault handling 
    return sd_card.disk_status;
}


// HW125 power on sequence
void hw125_power_on(uint16_t hw125_slave_pin)
{
    // Local variables 
    uint8_t di_cmd; 
    uint8_t do_resp; 

    func_num[1] = func_num[1] + 1; 
    mount_seq[mount_it] = 1; 
    mount_it++; 

    //=================================
    // Power Sequence 

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

    //=================================


    //=================================
    // Software reset - set to IDLE state 

    // // Select the SD card device 
    // spi2_slave_select(hw125_slave_pin);

    // // Send CMD0 with no arg and a valid CRC value 
    // hw125_send_cmd(HW125_CMD0, HW125_ARG_NONE, HW125_CRC_CMD0, &do_resp);

    // // Deselect the card device 
    // spi2_slave_deselect(hw125_slave_pin);

    // Local variables 
    uint8_t cmd_frame[SPI_6_BYTES];
    // uint8_t num_read = HW125_R1_RESP_COUNT;
    uint16_t num_read = 0x1FFF;
    // TODO create a proper macro for this timer value 

    // Slave select 
    spi2_slave_select(hw125_slave_pin); 

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
    spi2_write(cmd_frame, SPI_6_BYTES);

    // Read R1 response until it is valid or until it times out 
    do 
    {
        spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);
    }
    while((do_resp != 0x01) && --num_read);

    if (num_read == 0) fail_state = 6; // Sometimes this fails 

    //=================================

    // Slave select 
    spi2_slave_deselect(hw125_slave_pin); 

    // TODO send a data high byte? 
    spi2_write(&di_cmd, SPI_1_BYTE);

    // Set the Power Flag status to on 
    sd_card.pwr_flag = HW125_PWR_ON; 
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

    func_num[2] = func_num[2] + 1; 
    mount_seq[mount_it] = 2; 
    mount_it++; 

    // Send CMD1 or ACMD41 to initiate initialization 
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

//=======================================================================================


//=======================================================================================
// Status functions 

// HW125 disk status 
DISK_STATUS hw125_status(uint8_t pdrv)
{
    func_num[10] = func_num[10] + 1; 
    mount_seq[mount_it] = 10; 
    mount_it++; 

   // pdrv is 0 for single drive systems. The code doesn't support more than one drive. 
   if (pdrv) 
   { 
       return HW125_STATUS_NOINIT; 
   }
   
   // Return the existing disk status 
   return sd_card.disk_status;
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


// Return the Power Flag status 
uint8_t hw125_power_status(void)
{
    return sd_card.pwr_flag; 
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

    func_num[3] = func_num[3] + 1; 
    mount_seq[mount_it] = 3; 
    mount_it++; 

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

    if(num_read == 0) fail_state = 7; 
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

    func_num[4] = func_num[4] + 1; 
    mount_seq[mount_it] = 4; 
    mount_it++; 

    // Check that the drive number is zero 
    if (pdrv) 
    {
        fail_state = 8; 
        return HW125_RES_PARERR;
    }
    
    // Check that the count is valid 
    if (count == HW125_NO_BYTE) 
    {
        fail_state = 9; 
        return HW125_RES_PARERR;
    }

    // Check the init status 
    if (sd_card.disk_status == HW125_STATUS_NOINIT) 
    {
        fail_state = 10; 
        return HW125_RES_NOTRDY;
    }

    // Convert the sector number to byte address if it's not SDC V2 (byte address)
    // TODO  
    // if (sd_card.card_type != HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;
    if (sd_card.card_type & HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;

    // Select the slave device 
    spi2_slave_select(sd_card.ss_pin);

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
            fail_state = 11; 
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

            if (read_resp == HW125_RES_ERROR) fail_state = 12; 

            // Send CMD12 to terminate the read transaction 
            hw125_send_cmd(HW125_CMD12, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

            if (do_resp != HW125_READY_STATE)
            {
                // CMD12 unsuccessfull 
                fail_state = 13; 
                read_resp = HW125_RES_ERROR;
            }
        }
        else
        {
            // Unsuccessful CMD18
            fail_state = 14;  
            read_resp = HW125_RES_ERROR;
        }
    }

    // Deselect the slave device 
    spi2_slave_deselect(sd_card.ss_pin);

    // TODO dummy read? 
    spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE); 

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
    // uint8_t num_read = HW125_DT_RESP_COUNT;
    volatile uint32_t num_read = 0; 
    // TODO create and use a real-time timer here 

    func_num[5] = func_num[5] + 1; 
    mount_seq[mount_it] = 5; 
    mount_it++; 

    // Read the data token 
    do 
    {
        spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);
        num_read++; 
    }
    // while((do_resp != HW125_DT_TWO) && --num_read);
    while((do_resp != HW125_DT_TWO));

    if (do_resp == HW125_DT_TWO)
    {
        // Valid data token is detected - read the data packet(s) 
        spi2_write_read(HW125_DATA_HIGH, buff, sector_size);

        // Discard the two CRC bytes 
        spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);
        spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);

        // Operation success 
        read_resp = HW125_RES_OK;
    }
    else
    {
        // Incorrect or error token received 
        fail_state = 15; 
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

    func_num[6] = func_num[6] + 1; 
    mount_seq[mount_it] = 6; 
    mount_it++; 

    // Check that the drive number is zero 
    if (pdrv) 
    {
        fail_state = 16; 
        return HW125_RES_PARERR;
    }

    // Check that the count is valid 
    if (count == HW125_NO_BYTE) 
    {
        fail_state = 17; 
        return HW125_RES_PARERR;
    }

    // Check the init status 
    if (sd_card.disk_status == HW125_STATUS_NOINIT) 
    {
        fail_state = 18; 
        return HW125_RES_NOTRDY;
    }

    // Check write protection 
    if (sd_card.disk_status == HW125_STATUS_PROTECT) 
    {
        fail_state = 19; 
        return HW125_RES_WRPRT; 
    }

    // Convert the sector number to byte address if it's not SDC V2 (byte address) 
    if (sd_card.card_type != HW125_CT_SDC2_BYTE) sector *= HW125_SEC_SIZE;

    // Select the slave device 
    spi2_slave_select(sd_card.ss_pin);

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
            fail_state = 20; 
            write_resp = HW125_RES_ERROR;
        }
    }
    else  // Send multiple data packets if count > 1
    {
        // Specify the number of sectors to pre-erase to optimize write performance
        // TODO only for SDC V1? 
        // hw125_send_cmd(HW125_CMD55, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);
        // hw125_send_cmd(HW125_CMD23, count, HW125_CRC_CMDX, &do_resp);

        // Check R1 response 
        if (do_resp == HW125_READY_STATE)
        {
            // ACDM23 successful - send CMD25 that specifies the address to start to write 
            hw125_send_cmd(HW125_CMD25, sector, HW125_CRC_CMDX, &do_resp);

            // Check the R1 response 
            if (do_resp == HW125_READY_STATE)
            {
                // CMD25 successful - write initiated

                // Define the CMD25 stop token 
                uint8_t stop_trans = HW125_DT_ONE; 

                // Write all the sectors or until there is an error 
                do 
                {
                    write_resp = hw125_write_data_packet(buff, HW125_SEC_SIZE, HW125_DT_ZERO);
                    buff += HW125_SEC_SIZE; 
                }
                while(--count && (write_resp != HW125_RES_ERROR)); 

                if (write_resp == HW125_RES_ERROR) fail_state = 21; 

                // Wait on busy flag to clear 
                hw125_ready_rec();

                // Send stop token 
                spi2_write(&stop_trans, HW125_SINGLE_BYTE);
            }
            else
            {
                // Unsuccessfull CMD25 
                fail_state = 22; 
                write_resp = HW125_RES_ERROR;
            }
        }
        else 
        {
            // Unsuccessfull ACMD23 
            fail_state = 23; 
            write_resp = HW125_RES_ERROR;
        }
    }

    // Wait on busy flag to clear 
    hw125_ready_rec();

    // Deselect the slave device
    spi2_slave_deselect(sd_card.ss_pin); 

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

    func_num[7] = func_num[7] + 1; 
    mount_seq[mount_it] = 7; 
    mount_it++; 

    // Wait until the card is no longer busy before sending a CMD 
    hw125_ready_rec();

    // Send data token 
    spi2_write(&data_token, HW125_SINGLE_BYTE);

    // Send data block 
    // TODO make sure this cast away from const doesn't cause issues 
    spi2_write((uint8_t *)buff, sector_size); 

    // Send CRC 
    spi2_write(&crc, HW125_SINGLE_BYTE);
    spi2_write(&crc, HW125_SINGLE_BYTE);

    // Read data response 
    spi2_write_read(HW125_DATA_HIGH, &do_resp, HW125_SINGLE_BYTE);

    // Check the data response 
    if ((do_resp & HW125_DR_FILTER) == HW125_DR_ZERO)
    {
        // Data accepted 
        write_resp = HW125_RES_OK;
    }
    else
    {
        // Data rejected 
        fail_state = 24; 
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
    if (pdrv) 
    {
        fail_state = 25; 
        return HW125_RES_PARERR;
    }

    // Check the init status 
    if ((sd_card.disk_status == HW125_STATUS_NOINIT) && (cmd != HW125_CTRL_POWER)) 
    {
        fail_state = 26; 
        return HW125_RES_NOTRDY;
    }

    // Select the slave card 
    spi2_slave_select(sd_card.ss_pin); 

    // Choose the misc function 
    switch(cmd)
    {
        case HW125_CTRL_SYNC:
            // This is not not needed if the write operation is completed within the 
            // disk_write function which it is. 
            hw125_ready_rec(); 
            result = HW125_RES_OK; 
            // result = HW125_RES_PARERR; 
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
    spi2_slave_deselect(sd_card.ss_pin); 

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

    func_num[8] = func_num[8] + 1; 
    mount_seq[mount_it] = 8; 
    mount_it++; 

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
                case HW125_CSD_V1:
                    // CSD Version == 1.0 --> MMC or SDC V1 
                    n =   ((uint32_t)csd[BYTE_5]  & FILTER_4_LSB) + 
                        ((((uint32_t)csd[BYTE_10] & FILTER_1_MSB) >> SHIFT_7) + 
                        (((uint32_t)csd[BYTE_9]  & FILTER_2_LSB) << SHIFT_1)) + 
                        HW125_MULT_PLUS_TWO;

                    c_size = ((uint32_t)(csd[BYTE_8] & FILTER_2_MSB) >> SHIFT_6) +
                            ((uint32_t) csd[BYTE_7] << SHIFT_2) + 
                            ((uint32_t)(csd[BYTE_6] & FILTER_2_LSB) << SHIFT_10) + 
                            HW125_LBA_PLUS_ONE;
                    
                    *(uint32_t *) buff = c_size << (n - HW125_MAGIC_SHIFT_V1); 

                    result = HW125_RES_OK; 
                    break;
                
                case HW125_CSD_V2: 
                    // CSD Version == 2.0 --> SDC V2 
                    c_size =  (uint32_t) csd[BYTE_9] + 
                            ((uint32_t) csd[BYTE_8] << SHIFT_8) + 
                            ((uint32_t)(csd[BYTE_7] & FILTER_6_LSB) << SHIFT_16) + 
                            HW125_LBA_PLUS_ONE; 

                    *(uint32_t *) buff = c_size << HW125_MAGIC_SHIFT_V2;
                    
                    result = HW125_RES_OK; 
                    break;
                
                case HW125_CSD_V3: 
                    // CSD Version == 3.0 --> Currently unsupported 
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
        fail_state = 27; 
        result = HW125_RES_ERROR; 
    }

    return result; 
}


// HW125 IO Control - Get Sector Size 
DISK_RESULT hw125_ioctl_get_sector_size(void *buff)
{
    // Local variables 
    uint8_t result; 

    func_num[9] = func_num[9] + 1; 
    mount_seq[mount_it] = 9; 
    mount_it++; 

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
        // TODO check to see if CMD9 returns a valid token 
        result = hw125_read_data_packet(csd, HW125_CSD_REG_LEN); 
    }
    else
    {
        // Unsucessful CMD9 
        fail_state = 28; 
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
        fail_state = 29; 
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
        spi2_write_read(HW125_DATA_HIGH, ocr, HW125_TRAIL_RESP_BYTES);
        result = HW125_RES_OK; 
    }
    else
    {
        // Unsuccessful CMD58 
        fail_state = 30; 
        result = HW125_RES_ERROR; 
    }

    return result; 
}

//=======================================================================================
