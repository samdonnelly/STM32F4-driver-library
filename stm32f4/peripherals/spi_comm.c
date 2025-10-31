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
// Macros 

#define SPI_TIMEOUT_COUNT 10000

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief SPI enable 
 * 
 * @details Sets the SPE bit to enable the specified SPI 
 * 
 * @param spi : pointer to spi port 
 */
void spi_enable(SPI_TypeDef *spi);


/**
 * @brief SPI disable 
 * 
 * @details Clears the SPE bit to disable the specified SPI 
 * 
 * @param spi : pointer to spi port 
 */
void spi_disable(SPI_TypeDef *spi);


/**
 * @brief SPI TXE wait 
 * 
 * @details Wait for the TXE bit to set before proceeding. The TXE bit is the transmit 
 *          buffer empty status which indicates when more data can be loaded into the 
 *          transmit buffer for sending. If data is written to the transmit buffer 
 *          before it is empty then data will be overwritten. 
 * 
 * @param spi : pointer to spi port 
 */
void spi_txe_wait(SPI_TypeDef *spi);


/**
 * @brief SPI RXNE wait 
 * 
 * @details Wait for the RXNE bit to set before proceeding. The RXNE bit is the receive 
 *          buffer not empty status which indicates when new data can be read from the data 
 *          register during a read operation. If data is read without this bit being set then 
 *          old data will be read from the receieve buffer. 
 * 
 * @param spi : pointer to spi port 
 */
void spi_rxne_wait(SPI_TypeDef *spi);


/**
 * @brief SPI BSY wait 
 * 
 * @details Waits for the busy flag to clear. The busy flag indicates when the SPI is busy 
 *          or when the TX buffer is not empty. This is typically used at the end of read 
 *          and write sequences to make sure the operation is done before ending. 
 * 
 * @param spi : pointer to spi port 
 */
void spi_bsy_wait(SPI_TypeDef *spi);

//=======================================================================================


//=======================================================================================
// Inititalization 

// SPI initialization 
void spi_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_sck, 
    pin_selector_t sck_pin, 
    GPIO_TypeDef *gpio_data, 
    pin_selector_t miso_pin, 
    pin_selector_t mosi_pin, 
    spi_baud_rate_ctrl_t baud_rate_ctrl,
    spi_clock_mode_t clock_mode)
{
    //==================================================
    // Enable the SPI clock 

    if (spi == SPI1)
    {
        // SPI1 
        RCC->APB2ENR |= (SET_BIT << SHIFT_12); 
    }
    else if ((spi == SPI2) || (spi == SPI3))
    {
        // SPI2 and SPI3 
        RCC->APB1ENR |= (SET_BIT << (SHIFT_14 + (uint8_t)((uint32_t)(spi - SPI2) >> SHIFT_10))); 
    }
    else
    {
        return;
    }
    
    //==================================================

    //==================================================
    // Configure the pins for alternative functions 
    
    // SCK pin 
    gpio_pin_init(gpio_sck, sck_pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio_sck, sck_pin, SET_5); 

    // MISO pin 
    gpio_pin_init(gpio_data, miso_pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio_data, miso_pin, SET_5); 

    // MOSI pin 
    gpio_pin_init(gpio_data, mosi_pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio_data, mosi_pin, SET_5); 
    
    //==================================================

    //==================================================
    // Configure the SPI 

    // Reset and disable the SPI before making any changes 
    spi->CR1 = CLEAR;

    // Set the BR bits in the SPI_CR1 register to define the serial clock baud rate. 
    spi->CR1 |= (baud_rate_ctrl << SHIFT_3);

    // Select CPOL and CPHA bits to define data transfer and serial clock relationship.
    spi->CR1 |= (clock_mode << SHIFT_0);

    // Set the DFF bit to define an 8-bit data frame format. 
    spi->CR1 &= ~(SET_BIT << SHIFT_11);

    // Enable software slave management
    spi->CR1 |= (SET_BIT << SHIFT_9);
    spi->CR1 |= (SET_BIT << SHIFT_8);

    // Set full-duplex mode 
    spi->CR1 &= ~(SET_BIT << SHIFT_10);

    // Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. 
    spi->CR1 &= ~(SET_BIT << SHIFT_7);  // MSB first 

    // Set the FRF bit in SPI_CR2 to select the TI protocol for serial comm. 
    spi->CR2 &= ~(SET_BIT << SHIFT_4);  // No TI protocol 

    // Set the MSTR bit to enable master mode 
    spi->CR1 |= (SET_BIT << SHIFT_2);

    // Set the SPE bit to enable SPI 
    spi_enable(spi);
    
    //==================================================
}


// SPI slave select pin init 
void spi_ss_init(
    GPIO_TypeDef *gpio, 
    pin_selector_t ss_pin)
{
    gpio_pin_num_t pin_num = (SET_BIT << ss_pin); 
    
    // Configure the slave select pin and GPIO then deselect the slave device 
    gpio_pin_init(gpio, ss_pin, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
    spi_slave_deselect(gpio, pin_num); 
}

//=======================================================================================


//=======================================================================================
// SPI register functions 

// Set the SPE bit to enable SPI 
void spi_enable(SPI_TypeDef *spi)
{
    spi->CR1 |= (SET_BIT << SHIFT_6);
}


// Clear the SPE bit to disable SPI 
void spi_disable(SPI_TypeDef *spi)
{
    spi->CR1 &= ~(SET_BIT << SHIFT_6);
}


// Wait for TXE bit to set 
void spi_txe_wait(SPI_TypeDef *spi)
{
    uint16_t timeout = SPI_TIMEOUT_COUNT;
    while(!(spi->SR & (SET_BIT << SHIFT_1)) && --timeout); 
}

// Wait for RXNE bit to set 
void spi_rxne_wait(SPI_TypeDef *spi)
{
    uint16_t timeout = SPI_TIMEOUT_COUNT;
    while(!(spi->SR & (SET_BIT << SHIFT_0)) && --timeout);
}


// Wait for BSY bit to clear
void spi_bsy_wait(SPI_TypeDef *spi)
{
    while(spi->SR & (SET_BIT << SHIFT_7));
}


// Select an SPI slave 
void spi_slave_select(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num)
{
    gpio_write(gpio, slave_num, GPIO_LOW);
}


// Deselect an SPI slave 
void spi_slave_deselect(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num)
{
    gpio_write(gpio, slave_num, GPIO_HIGH);
}

//=======================================================================================


//=======================================================================================
// Read and write 

// SPI write 
SPI_STATUS spi_write(
    SPI_TypeDef *spi, 
    const uint8_t *write_data, 
    uint32_t data_len)
{
    SPI_STATUS spi_status = SPI_OK; 

    // Argument check - NULL pointers and zero length 
    if (spi == NULL || write_data == NULL || !data_len) 
    {
        return spi_status; 
    }

    // Iterate through all data to be sent 
    for (uint32_t i = 0; i < data_len; i++)
    {
        spi_txe_wait(spi); 
        spi->DR = *write_data++; 
    }

    // Wait for TXE bit to set 
    spi_txe_wait(spi);

    // Wait for BSY to clear 
    spi_bsy_wait(spi);

    // Read the data and status registers to clear the RX buffer and overrun error bit
    dummy_read(spi->DR); 
    dummy_read(spi->SR); 

    return spi_status; 
}


// SPI write then read 
SPI_STATUS spi_write_read(
    SPI_TypeDef *spi, 
    uint8_t write_data, 
    uint8_t *read_data, 
    uint32_t data_len)
{
    SPI_STATUS spi_status = SPI_OK; 

    // Argument check - NULL pointers and zero length 
    if (spi == NULL || read_data == NULL || !data_len) 
    {
        return spi_status; 
    }

    // Write the first piece of data 
    spi_txe_wait(spi); 
    spi->DR = write_data; 

    // Iterate through all data to be sent and received 
    for (uint16_t i = 0; i < data_len-1; i++)
    {
        spi_txe_wait(spi);          // Wait for TXE bit to set 
        spi->DR = write_data;       // Write data to the data register 

        spi_rxne_wait(spi);         // Wait for the RXNE bit to set 
        *read_data++ = spi->DR;     // Read data from the data register 
    }

    // Read the last piece of data
    spi_rxne_wait(spi);
    *read_data = spi->DR;

    // Wait for TXE bit to set 
    spi_txe_wait(spi);

    // Wait for BSY to clear 
    spi_bsy_wait(spi);

    return spi_status; 
}

//=======================================================================================
