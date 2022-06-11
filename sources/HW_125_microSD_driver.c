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
void hw125_init(uint16_t hw125_slave_pin)
{
    // Local variables 
    uint8_t di_cmd;
    uint8_t do_resp; 
    uint8_t ocr[HW125_TRAIL_RESP_BYTES];
    uint8_t volt_range[HW125_TRAIL_RESP_BYTES];
    uint16_t init_timer = HW125_INIT_TIMER;
    uint8_t card_type; 

    // Wait 1ms to allow for voltage to reach above 2.2V
    tim9_delay_ms(HW125_INIT_DELAY);

    // Deselect the sd card slave
    spi2_slave_deselect(hw125_slave_pin);

    // Set the DI/MOSI command high (0xFF) 
    di_cmd = HW125_DI_HIGH;

    // Send DI/MOSI (0xFF) 10x to wait for more than 74 clock pulses 
    for (uint8_t i = 0; i < HW125_DUMMY_CLOCK; i++)
    {
        spi2_write(&di_cmd, SPI_1_BYTE);
    }

    // Select the sd card slave 
    spi2_slave_select(hw125_slave_pin);

    // Send CMD0 with arg = 0 and a valid CRC value (0x95)
    hw125_send_cmd(HW125_CMD0, HW125_ARG_NONE, HW125_CRC_CMD0, &do_resp);

    // Check the R1 response from CMD0 
    if (do_resp == HW125_IDLE_STATE)
    {
        // Idle state 

        // Send CMD8 with arg = 0x000001AA and a valid CRC (0x87)
        hw125_send_cmd(HW125_CMD8, HW125_ARG_SUPV, HW125_CRC_CMD8, &do_resp);

        // Check the R1 response from CMD8 
        if (do_resp == HW125_IDLE_STATE)
        {
            // Idle state - 32 trailing bits incoming 

            // Read trailing 32-bits 
            spi2_write_read(HW125_DI_HIGH, volt_range, HW125_TRAIL_RESP_BYTES);

            // Check lower 12-bits of R7 response (big endian format) 
            if ((uint16_t)((volt_range[BYTE_2] << SHIFT_8) | (volt_range[BYTE_3])) 
                == HW125_CMD8_R7_RESP)
            {
                // 0x1AA matched - voltage range 2.7-3.6V
                
                // Send ACMD41 until appropriate response - allow up to 1 second
                do
                {
                    // Send ACMD41 (CMD55+CMD41) with HCS bit set in the arg 
                    hw125_send_cmd(HW125_CMD55, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);
                    hw125_send_cmd(HW125_CMD41, HW125_ARG_HCS, HW125_CRC_CMDX, &do_resp);

                    // Delay 1ms 
                    tim9_delay_ms(HW125_INIT_DELAY);

                    // Decrement timer 
                    init_timer--;
                }
                while(init_timer && (do_resp == HW125_IDLE_STATE));

                // Check timeout
                if (init_timer)
                {
                    // No init timer timeout 

                    // Check R1 response 
                    if (do_resp == HW125_INIT_STATE)
                    {
                        // Initiate initialization begun 

                        // Send CMD58 with no arg to check the OCR (trailing 32-bits)
                        hw125_send_cmd(HW125_CMD58, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);
                        spi2_write_read(HW125_DI_HIGH, ocr, HW125_TRAIL_RESP_BYTES);

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

                            // Send CMD16 to change the block size to 512 bytes (for FAT)
                            hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
                        }
                    }
                    else
                    {
                        // TODO this state may be able to be wrapped up in the next error 

                        // Error: Unknwon card 
                        card_type = HW125_CT_UNKNOWN;
                    }
                    
                }
                else
                {
                    // Init timer timeout 

                    // Error: Unknown card 
                    card_type = HW125_CT_UNKNOWN;
                }
            }
            else 
            {
                // 0x1AA mismatched 

                // Error: Unknown card 
                card_type = HW125_CT_UNKNOWN;
            }
        }
        else 
        {
            // CMD8 rejected with illegal command error (0x05)

            // Send ACMD41 until appropriate response - allow up to 1 second
            do 
            {
                // Send ACMD41 (CMD55+CMD41) with no arg 
                hw125_send_cmd(HW125_CMD55, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);
                hw125_send_cmd(HW125_CMD41, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

                // Delay 1ms 
                tim9_delay_ms(HW125_INIT_DELAY);

                // Decrement timer 
                init_timer--;
            }
            while (init_timer && (do_resp == HW125_IDLE_STATE));

            // Check timeout and R1 response 
            if (init_timer && (do_resp == HW125_INIT_STATE))
            {
                // Initiate initialization begun - No init timer timeout 

                // SDC V1 
                card_type = HW125_CT_SDC1;

                // Send CMD16 to change the block size to 512 bytes (for FAT)
                hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
            }

            else
            {
                // Error or timeout 
                // TODO find a new timer - this one has already been used! 

                // Send ACMD41 until appropriate response - allow up to 1 second
                do 
                {
                    // Send CMD1 with no arg 
                    hw125_send_cmd(HW125_CMD1, HW125_ARG_NONE, HW125_CRC_CMDX, &do_resp);

                    // Delay 1ms 
                    tim9_delay_ms(HW125_INIT_DELAY);

                    // Decrement timer 
                    init_timer--;
                }
                while (init_timer && (do_resp == HW125_IDLE_STATE));

                // Check timeout and R1 response 
                if (init_timer && (do_resp == HW125_INIT_STATE))
                {
                    // Initiate initialization begun - No init timer timeout 

                    // MMC V3
                    card_type = HW125_CT_MMC;

                    // Send CMD16 to change the block size to 512 bytes (for FAT)
                    hw125_send_cmd(HW125_CMD16, HW125_ARG_BL512, HW125_CRC_CMDX, &do_resp);
                }
            }
        }
    }
    else
    {
        // Error: Unknown card --> power off --> init failed 
        // TODO add timer in spi read function that will return an error if it times out
    }
}

//=======================================================================================


//=======================================================================================
// SPI read and write

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
    spi2_write_read(HW125_DI_HIGH, resp, SPI_1_BYTE);
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
        spi2_write_read(HW125_DI_HIGH, &resp, SPI_1_BYTE);
    }
    while(resp != HW125_DO_HIGH);
}

//=======================================================================================
