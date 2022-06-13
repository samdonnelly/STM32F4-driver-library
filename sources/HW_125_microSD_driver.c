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
// Initialization 

// HW125 initialization 
uint8_t hw125_init(uint16_t hw125_slave_pin)
{
    // Local variables 
    uint8_t do_resp; 
    uint8_t init_timer_status;
    uint8_t card_type; 
    uint8_t ocr[HW125_TRAIL_RESP_BYTES];
    uint8_t v_range[HW125_TRAIL_RESP_BYTES];

    // Power on 
    hw125_power_on(hw125_slave_pin);

    // Select the sd card slave 
    spi2_slave_select(hw125_slave_pin);

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
                        card_type = HW125_CT_SDC2_BLOCK;
                    }
                    else
                    {
                        // SDC V2 (byte address)
                        card_type = HW125_CT_SDC2_BYTE;
                    }
                }
                else
                {
                    // Initiate initialization timer timeout 
                    card_type = HW125_CT_UNKNOWN;
                }
            }
            else 
            {
                // 0x1AA mismatched 
                card_type = HW125_CT_UNKNOWN;
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
                card_type = HW125_CT_SDC1;
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
                    card_type = HW125_CT_MMC;
                }
                else
                {
                    // Initiate initialization timer timeout 
                    card_type = HW125_CT_UNKNOWN;
                }
            }
        }
    }
    else
    {
        // Not in idle state
        card_type = HW125_CT_UNKNOWN;

        // TODO add timer in spi read function that will return an error if it times out
    }

    // Change the block size if needed 
    if ((card_type != HW125_CT_UNKNOWN) && (card_type != HW125_CT_SDC2_BLOCK))
    {
        // Send CMD16 to change the block size to 512 bytes (for FAT)
        hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
    }

    // Power off sequence 
    if (card_type == HW125_CT_UNKNOWN)
    {
        // Power off 
    }

    // Deselect slave 
    spi2_slave_deselect(hw125_slave_pin);

    // Return the card type for fault handling 
    return card_type;
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

//=======================================================================================


//=======================================================================================
// Read and write

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
    uint8_t counter = 10;

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

    // Skip a stop byte when stop_transmission? 

    // Read R1 response 
    do 
    {
        spi2_write_read(HW125_DATA_HIGH, resp, SPI_1_BYTE);
    }
    while((*resp & 0x80) && --counter);
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
