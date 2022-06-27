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
 * @brief HW125 Power Flag set to off 
 * 
 * @details 
 * 
 */
void hw125_power_off(void);


/**
 * @brief HW125 Power Flag status 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t hw125_power_status(void);


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
                if (init_timer_status && (do_resp == HW125_READY_STATE))
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
            if (init_timer_status && (do_resp == HW125_READY_STATE))
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
                if (init_timer_status && (do_resp == HW125_READY_STATE))
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
    return (uint8_t)(sd_card.pwr_flag); 
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

    // Check write protection 
    if (sd_card.disk_status == HW125_STATUS_PROTECT) return HW125_RES_WRPRT; 

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
            // Successfull CMD24 - proceed to send a data packet to the card 

            // Write initiated 
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
        hw125_send_cmd(HW125_CMD55, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);
        hw125_send_cmd(HW125_CMD23, count, HW125_CRC_CMDX, &do_resp);

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

                // Wait on busy flag to clear 
                hw125_ready_rec();

                // Send stop token 
                spi2_write(&stop_trans, HW125_SINGLE_BYTE);
            }
            else
            {
                // Unsuccessfull CMD25 
                write_resp = HW125_RES_ERROR;
            }
        }
        else 
        {
            // Unsuccessfull ACMD23 
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

    // Wait until the card is no longer busy before sending a CMD? 
    hw125_ready_rec();

    // Send data token 
    spi2_write(&data_token, HW125_SINGLE_BYTE);

    // Send data block 
    // TODO make sure this case away from const doesn't cause issues 
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
    uint8_t *param = buff;  // Cast the parameter and data buffer to a uint8_t where needed 
    uint8_t do_resp; 
    uint8_t csd[HW125_CSD_REG_LEN];  

    // Check that the drive number is zero 
    if (pdrv) return HW125_RES_PARERR;

    // Check the init status 
    if ((sd_card.disk_status == HW125_STATUS_NOINIT) && (cmd != HW125_CTRL_POWER)) 
    {
        return HW125_RES_NOTRDY;
    }

    // 
    switch(cmd)
    {
        case HW125_CTRL_SYNC:
            // This is not not needed if the write operation is completed within the 
            // disk_write function which it is. 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_GET_SECTOR_COUNT:
            // Send CMD9 
            hw125_send_cmd(HW125_CMD9, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

            // Check the response 
            if (do_resp == HW125_CSD_V2)
            {
                // Read the CSD register data 
                spi2_write_read(HW125_DATA_HIGH, csd, HW125_CSD_REG_LEN);
            }
            else 
            {
                // 
            }

            // Read the bytes that follow 

            // Check the CSD version number 
                // CSD version 2.0 --> SDC V2 
                    // Read the "device size" --> bits 48-69 
                // CSD Version X --> MMC and SDC V1 
                    // Read the "device size" --> bits 62-73 

            result = HW125_RES_OK; 
            break; 
        
        case HW125_GET_SECTOR_SIZE:  // Get the sector size 
            *(uint16_t *)buff = (uint16_t)HW125_SEC_SIZE; 
            result = HW125_RES_OK; 
            break; 
        
        case HW125_GET_BLOCK_SIZE:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_TRIM:
            // Not needed 
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_POWER:  // Get/set the power status 
            result = HW125_RES_OK; 
            
            switch (*param)  // TODO why do we need a separate variable? 
            {
                case HW125_PWR_OFF:  // Turn the Power Flag off 
                    hw125_power_off(); 
                    break;
                
                case HW125_PWR_ON:  // Turn the Power Flag on 
                    hw125_power_on(sd_card.ss_pin);
                    break;
                
                case HW125_PWR_CHECK:  // Check the status of the Power Flag 
                    *(param++) = hw125_power_status(); 
                    break;
                
                default:  // Invalid request 
                    result = HW125_RES_PARERR; 
                    break; 
            }
            break; 
        
        case HW125_CTRL_LOCK:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_EJECT:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_CTRL_FORMAT:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_MMC_GET_TYPE:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_MMC_GET_CSD:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_MMC_GET_CID:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_MMC_GET_OCR:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_MMC_GET_SDSTAT:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_ATA_GET_REV:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_ATA_GET_MODEL:
            result = HW125_RES_PARERR; 
            break; 
        
        case HW125_ATA_GET_SN:
            result = HW125_RES_PARERR; 
            break; 
        
        default:
            result = HW125_RES_PARERR; 
            break;
    }

    // Return the result
    return result; 
}

//=======================================================================================
