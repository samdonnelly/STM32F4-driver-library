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

    // Check the R1 response from CMD0 - check repeatedly until timeout or idle returned 

    // If: in idle state (0x01)
        // Send CMD8 with arg = 0x000001AA and a valid CRC (0x87)
        hw125_send_cmd(HW125_CMD8, HW125_ARG_SUPV, HW125_CRC_CMD8, &do_resp);

        // Read lower 12-bits in R7 response 

        // If: 0x1AA matched 
            // Send CMD55 with arg = 0 followed by CMD41 (ACMD41) with HCS bit set (bit 
            // 30) in arg (0x4000000)

            // Read R1 response 
                // If: still in idle (0x01)
                    // Send initiate initialization cmd and check R1 response again 

                // Else if: Error, no response, or timeout 
                    // Error - Unknown card - power off 

                    // Failed return value 

                // Else if: return 0x00
                    // Send CMD58 with arg = 0 to check OCR 

                    // Read CCS bit in the OCR response
                        // if: 1
                            // SD version 2+ (block address) 

                        // Else: 0
                            // SD version 2+ (byte address) 

                            // Send CMD16 with arg = 0x00000200 to force block size to
                            // 512 bytes to work with FAT file system

        // Else if: 0x1AA not matched
            // Error - unknown card - power off 

        // Else if: Error response or no response (or timeout?) 
            // Send CMd55 followed by CMD41 (ACMD41) with arg = 0 to init the init

            // Read R1 response
                // If: still in idle (0x01)
                    // Send initiate initialization cmd and check R1 response again 

                // Else if: 0x00
                    // SD version 1

                    // Send CMD16 with arg = 0x00000200 to force block size to
                    // 512 bytes to work with FAT file system

                // Else if: Error, no response, or timeout
                    // Send CMD1 with arg = 0 to init the init 

                    // if: still in idle (0x01)
                        // Send CMD1 and check R1 response again 

                    // Else if: Error response or no response (or timeout?) 
                        // Error - unknown card - power off 

                    // Else if: 0x00
                        // MMC version 3

                        // Send CMD16 with arg = 0x00000200 to force block size to
                        // 512 bytes to work with FAT file system
    
    // Else if: Timeout / no response 
        // Error - Unknown card - power off
        
        // Failed return value 
}

//=======================================================================================


//=======================================================================================
// SPI read and write

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

    // Genrate a command frame 
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

    // Read response 
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
