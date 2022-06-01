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
// Write sequence
//  1. Enable the SPI. 
//  2. Loop through the data to be written. 
//      a) Wait for the TXE bit to set. 
//      b) Write data to the data register. 
//      c) Increment to the next piece of data. 
//  3. Wait for the TXE bit to set. 
//  4. Wait for the BSY flag to clear. 
//  5. Read the data and status registers to clear the RX buffer and overrun error bit. 
//  6. Disable the SPI. 
//==============================================================

// SPI2 write 
void spi2_write(
    uint16_t *write_data, 
    uint16_t data_len)
{
    // Local variables 
    static uint16_t dummy_read; 

    // Enable the SPI 
    spi2_enable();

    // Iterate through all data to be sent 
    for (uint8_t i = 0; i < data_len; i++)
    {
        spi2_txe_wait();          // Wait for TXE bit to set 
        SPI2->DR = *write_data;   // Write data to the data register 
        write_data++;             // Increment to next piece of data 
    }

    // Wait for TXE bit to set 
    spi2_txe_wait();

    // Wait for BSY to clear 
    spi2_bsy_wait();

    // Read the data and status registers to clear the RX buffer and overrun error bit
    dummy_read = SPI2->DR;
    dummy_read = SPI2->SR;

    // Disable the SPI 
    spi2_disable();
}


//==============================================================
// Write-Read sequence
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

// SPI2 write then read 
void spi2_write_read(
    uint16_t *write_data, 
    uint16_t *read_data, 
    uint16_t data_len)
{
    // Enable the SPI 
    spi2_enable();

    // Write the first piece of data 
    spi2_txe_wait();
    SPI2->DR = *write_data;
    write_data++; 

    // Iterate through all data to be sent and recieved
    for (uint8_t i = 0; i < data_len-1; i++)
    {
        spi2_txe_wait();          // Wait for TXE bit to set 
        SPI2->DR = *write_data;   // Write data to the data register 
        write_data++;             // Increment to next piece of data 

        spi2_rxne_wait();         // Wait for the RXNE bit to set
        *read_data = SPI2->DR;    // Read data from the data register 
        read_data++;              // Increment to next space in memeory to store data
    }

    // Read the last piece of data 
    spi2_rxne_wait();
    *read_data = SPI2->DR;

    // Wait for TXE bit to set 
    spi2_txe_wait();

    // Wait for BSY to clear 
    spi2_bsy_wait();

    // Disable the SPI 
    spi2_disable();
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

//=======================================================================================
