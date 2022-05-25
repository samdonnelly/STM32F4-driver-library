/**
 * @file spi_comm.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SPI communication driver
 * 
 * @version 0.1
 * @date 2022-05-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//=======================================================================================
// Includes 

#include "spi_comm.h"

//=======================================================================================


//=======================================================================================
// Inititalization 

//==============================================================
// I2C Setup Steps 
//  1. Enable the I2C clock and the GPIO clock
//      a) Enable I2CX clock
//      b) Enable GPIOX clock
//  2. Configure the I2C pins for alternative functions
//      a) Select alternate function in MODER register
//      b) Select Open Drain Output
//      c) Select High SPEED for the PINs 
//      d) Select Pill-up for both the Pins 
//      e) Configure the Alternate Function in AFR Register
//  3. Reset the I2C
//  4. Ensure PE is disabled before setting up the I2C
//  5. Program the peripheral input clock in I2C_CR2 register
//  6. Configure the clock control register based on the run mode
//      a) Choose Sm or Fm mode 
//      b) Calculated clock control register based on PCLK1 & SCL frquency 
//  7. Configure the rise time register 
//  8. Program the I2C_CR1 register to enable the peripheral 
//  9. Set the START but in the I2C_CR1 register to generate a 
//     Start condition 
//==============================================================

//==============================================================
// SPI Master Mode Setup Steps 
//  X. Enable the SPI clock and the GPIO clock 
//  X. Configure the SPI pins for alternative functions
//  X. Reset and disable the SPI before manking any changes
//  X. Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
//  X. Select CPOL and CPHA bits to define data transfer and serial clock relationship.
//  X. Set the DFF bit to define 8-bit or 16-bit data frame format. 
//  X. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
//  X. Configure the NSS pin. 
//     a) 
//     b) 
//  X. Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
//  X. Set the MSTR and SPE bits. 
//==============================================================


// SPI2 initialization
void spi2_init(
    uint8_t baud_rate_ctrl,
    uint8_t clock_mode,
    uint8_t data_frame_format)
{
    //==============================================================
    // Pin information for SPI2
    //  PB10: SCK
    //  PB12: NSS
    //  PB14: MISO
    //  PB15: MOSI
    //==============================================================

    // X. Enable the SPI clock and the GPIO clock

    // X. Configure the SPI pins for alternative functions

    // X. Reset and disable the SPI before manking any changes
    SPI2->CR1 = CLEAR;

    // X. Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
    SPI2->CR1 |= (baud_rate_ctrl << SHIFT_3);

    // X. Select CPOL and CPHA bits to define data transfer and serial clock relationship.
    SPI2->CR1 |= (clock_mode << SHIFT_0);

    // X. Set the DFF bit to define 8-bit or 16-bit data frame format. 
    SPI2->CR1 |= (data_frame_format << SHIFT_11);

    // X. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
    SPI2->CR1 &= ~(SET_BIT << SHIFT_7);

    // X. Configure the NSS pin. 

    // X. Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
    SPI2->CR2 &= ~(SET_BIT << SHIFT_4);

    // X. Set the MSTR and SPE bits. 
    SPI2->CR1 |= (SET_BIT << SHIFT_2);  // Set to master mode 
    SPI2->CR1 |= (SET_BIT << SHIFT_6);  // Enable SPI 
}

//=======================================================================================


//=======================================================================================
// Read and write 

//==============================================================
// Notes 
//  - The CubeMX FATFS file system code will be used instead of creating the code from 
//    scratch like the other drivers. This is because the code/process seems complex 
//    with little online supporting material and therefore is not a good use of time 
//    given that the filesystem is not within your defined scope of learning. It's also 
//    been recommended online to not reinvent this process. With that being said, it 
//    looks like SPI and SD card drivers can still be made that are self-contained 
//    (don't use STM code). The functions made in these drivers can be referenced by the
//    STM code. 
//  - For the SD card driver you will need to reverse engineer the controllerstech code 
//    and/or find where he got the informartion to create that driver. The information 
//    may lie with the SD card manual itself. 
//==============================================================

//==============================================================
// Procedure to transmit and receive data 
//  1. Enable SPI by setting the SPE bit to 1 
//  2. Write the first data item to be transmitted into the SPI_DR register (clears TXE
//     flag)
//  3. Wait until TXE=1  and write the second data item to be transmitted
//  4. Wait until RXNE=1 and read the SPI_DR regsiter to get the first received data 
//     item (cleas RXNE bit) 
//  5. Repeat operations 3 and 4 for each data item 
//  6. Wait for RXNE=1 and read the last received data 
//  7. Wait until TXE=1 and then wait until BSY=0 before disabling SPI 
//==============================================================

//==============================================================
// Full duplex 
//  - The sequence begins when data is written into the SPI_DR register (TX buffer). 
//  - Data is parallel loaded into a shift register then shifted out serially over 
//    the MOSI pin. 
//  - At the same time the received data on the MISO pin is shifted in serially to 
//    the shift register then parallel loaded into the RX buffer. 
// 
// Bidirectional mode 
//  - Transmitting
//    - Sequence begins when data is written into the TX buffer. 
//    - Data is shifted out serially to the MOSI pin. 
//    - No data is received. 
//  - Receiving 
//    - The sequence begins as soon as soon as SPE=1 and BIDIOE=0 
//    - The received data on the MOSI pin eventaully gets loaded into the RX buffer. 
//    - No data is shifted out. 
//==============================================================

//==============================================================
// Receive sequence 
//  - When data transfer is complete: 
//    - The data in the shift register is transferred to the RX buffer and the RXNE 
//      flag is set 
//    - An interrupt is generated if the RXNEIE bit is set in the SPI_CR2 register. 
//  - Data is received and stored into an internal RX buffer to be read. Read access to 
//    the SPI_DR register returns the RX buffer value. 
//==============================================================

// SPI2 Read
void spi2_read(void)
{
    // 
}


//==============================================================
// Transmit sequence
//  - Transmit sequence begins when a byte is written to the TX buffer 
//  - TXE flag must be set to 1 to indicate that the data has been moved to the 
//    shift register and you can load more data to the TX register. 
//  - Data is first stored into an internal TX buffer before being transmitted. Write 
//    access to the SPI_DR register stores the written data into the TX buffer. 
//  
//  TODO : figure out which transmission type you need (full-duplex, half-duplex, etc.)
//==============================================================

// SPI2 write 
void spi2_write(void)
{
    // 
}

//=======================================================================================
