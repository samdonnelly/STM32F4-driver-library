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
#include "gpio_driver.h"

//=======================================================================================


//=======================================================================================
// Inititalization 

//==============================================================
// SPI Master Mode Setup Steps 
//  1. Enable the SPI2 clock 
//  2. Enable the GPIOB clock
//  3. Configure the pins for alternative functions
//      a) Specify SPI2 pins as using alternative functions
//      b) Select high speed for the pins 
//      c) Configure the SPI alternate function in the AFR register 
//  4. Reset and disable the SPI before manking any changes
//  5. Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
//  6. Select CPOL and CPHA bits to define data transfer and serial clock relationship.
//  7. Set the DFF bit to define 8-bit or 16-bit data frame format. 
//  8. Enable software slave management
//  9. Set full-duplex mode 
//  10. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
//  11. Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
//  12. Set the MSTR bit to enable master mode 
//  13. Set the SPE bit to enable SPI 
//  14. Configure the slave select pins as GPIO 
//==============================================================


// SPI2 initialization
uint8_t spi2_init(
    uint8_t num_slaves,
    uint8_t baud_rate_ctrl,
    uint8_t clock_mode,
    uint8_t data_frame_format)
{
    //==============================================================
    // Pin information for SPI2
    //  PB9:  NSS --> GPIO slave select 2
    //  PB10: SCK
    //  PB12: NSS --> GPIO slave select 1
    //  PB14: MISO
    //  PB15: MOSI
    //==============================================================

    // 1. Enable the SPI2 clock 
    RCC->APB1ENR |= (SET_BIT << SHIFT_14);

    // 2. Enable the GPIOB clock
    RCC->AHB1ENR |= (SET_BIT << SHIFT_1);

    // 3. Configure the pins for alternative functions

    // a) Specify SPI2 pins as using alternative functions
    GPIOB->MODER |= (SET_2 << SHIFT_20);  // PB10 
    GPIOB->MODER |= (SET_2 << SHIFT_28);  // PB14 
    GPIOB->MODER |= (SET_2 << SHIFT_30);  // PB15 

    // b) Select high speed for the pins 
    GPIOB->OSPEEDR |= (SET_3 << SHIFT_20);  // PB10 
    GPIOB->OSPEEDR |= (SET_3 << SHIFT_28);  // PB14 
    GPIOB->OSPEEDR |= (SET_3 << SHIFT_30);  // PB15 

    // c) Configure the SPI alternate function in the AFR register 
    GPIOB->AFR[1] |= (SET_5 << SHIFT_8);   // PB10 
    GPIOB->AFR[1] |= (SET_5 << SHIFT_24);  // PB14 
    GPIOB->AFR[1] |= (SET_5 << SHIFT_28);  // PB15 

    // 4. Reset and disable the SPI before manking any changes
    SPI2->CR1 = CLEAR;

    // 5. Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
    SPI2->CR1 |= (baud_rate_ctrl << SHIFT_3);

    // 6. Select CPOL and CPHA bits to define data transfer and serial clock relationship.
    SPI2->CR1 |= (clock_mode << SHIFT_0);

    // 7. Set the DFF bit to define 8-bit or 16-bit data frame format. 
    SPI2->CR1 |= (data_frame_format << SHIFT_11);

    // 8. Enable software slave management
    SPI2->CR1 |= (SET_BIT << SHIFT_9);
    SPI2->CR1 |= (SET_BIT << SHIFT_8);

    // 9. Set full-duplex mode 
    SPI2->CR1 &= ~(SET_BIT << SHIFT_10);

    // 10. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
    SPI2->CR1 &= ~(SET_BIT << SHIFT_7);  // MSB first 

    // 11. Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
    SPI2->CR2 &= ~(SET_BIT << SHIFT_4);  // No TI protocol 

    // 12. Set the MSTR bit to enable master mode 
    SPI2->CR1 |= (SET_BIT << SHIFT_2);

    // 13. Set the SPE bit to enable SPI 
    spi2_enable();

    // 14. Configure the slave select pins as GPIO 
    switch (num_slaves)
    {
        case SPI2_2_SLAVE:  // Initialize PB12 as GPIO
            gpiob_init(PIN_12, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
            spi2_slave_deselect(SPI2_SS_2);  // Deselect slave 
            // Case has no break so PB9 will also be initialized

        case SPI2_1_SLAVE:  // Initialize PB9 as GPIO 
            gpiob_init(PIN_9, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
            spi2_slave_deselect(SPI2_SS_1);  // Deselect slave 
            break;

        default:  // Invalid number of slaves specified
            return FALSE;
    }

    // Return true for passing initialization 
    return TRUE;
}

//=======================================================================================


//=======================================================================================
// SPI2 register functions 

// Set the SPE bit to enable SPI2
void spi2_enable(void)
{
    SPI2->CR1 |= (SET_BIT << SHIFT_6);
}


// Clear the SPE bit to disable SPI2 
void spi2_disable(void)
{
    SPI2->CR1 &= ~(SET_BIT << SHIFT_6);
}


// Wait for TXE bit to set 
void spi2_txe_wait(void)
{
    while(!(SPI2->SR & (SET_BIT << SHIFT_1)));
}


// Wait for RXNE bit to set 
void spi2_rxne_wait(void)
{
    while(!(SPI2->SR & (SET_BIT << SHIFT_0)));
}


// Wait for BSY bit to clear
void spi2_bsy_wait(void)
{
    while(SPI2->SR & (SET_BIT << SHIFT_7));
}

// Select an SPI2 slave 
void spi2_slave_select(uint16_t slave_num)
{
    gpiob_write(slave_num, GPIO_LOW);
}


// Deselect an SPI2 slave 
void spi2_slave_deselect(uint16_t slave_num)
{
    gpiob_write(slave_num, GPIO_HIGH);
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
//  3. Wait until TXE=1 and write the second data item to be transmitted
//  4. Wait until RXNE=1 and read the SPI_DR regsiter to get the first received data 
//     item (clears RXNE bit) 
//  5. Repeat operations 3 and 4 for each data item 
//  6. Wait for RXNE=1 and read the last received data 
//  7. Wait until TXE=1 and then wait until BSY=0 before disabling SPI 
//==============================================================

//==============================================================
// Disabling the SPI 
//  1. Wait until RXNE=1 to receive the last data 
//  2. Wait until TXE=1
//  3. Wait until BSY=0 
//  4. Set SPE=0 
//==============================================================

//==============================================================
// Full duplex 
//  - The sequence begins when data is written into the SPI_DR register (TX buffer). 
//  - Data is parallel loaded into a shift register then shifted out serially over 
//    the MOSI pin. 
//  - At the same time the received data on the MISO pin is shifted in serially to 
//    the shift register then parallel loaded into the RX buffer. 
//==============================================================


//==============================================================
// Transmit sequence
//  - Transmit sequence begins when a byte is written to the TX buffer 
//  - TXE flag must be set to 1 to indicate that the data has been moved to the 
//    shift register and you can load more data to the TX register. 
//  - Data is first stored into an internal TX buffer before being transmitted. Write 
//    access to the SPI_DR register stores the written data into the TX buffer. 
//==============================================================

// SPI2 write 
void spi2_write(void)
{
    // Check 8-bit or 16-bit mode? 

    // Start a loop that iterates to the data size 

    /* loop */ 

    // Wait for TXE bit to set before writing to the data register 
    // Write data to the data register 
}


// SPI2 write then read 
void spi2_write_read(void)
{
    // Check 8-bit or 16-bit mode? 

    // Start a loop that iterates to the data size 

    /* loop */ 

    // Wait for TXE bit to set before writing to the data register 
    // Write data to the data register 

    // Wait for RXNE bit to set before reading the data register 
    // Read data from the data register 
}


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
    // Check 8-bit or 16-bit mode? 

    // Start a loop that iterates to the data size 

    /* loop */ 

    // Wait for RXNE bit to set before reading the data register 
    // Read data from the data register 
}

//=======================================================================================
