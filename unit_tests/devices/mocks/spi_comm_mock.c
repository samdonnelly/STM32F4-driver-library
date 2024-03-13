/**
 * @file spi_driver_mock.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Mock SPI communication driver implementation - for unit testing 
 * 
 * @version 0.1
 * @date 2024-03-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "spi_comm.h" 

// Unit testing 
#include "spi_comm_mock.h"

//=======================================================================================


//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Global variables 
//=======================================================================================


//=======================================================================================
// Prototypes 

// SPI enable 
void spi_enable(SPI_TypeDef *spi);

// SPI disable 
void spi_disable(SPI_TypeDef *spi);

// SPI TXE wait 
void spi_txe_wait(SPI_TypeDef *spi);

// SPI RXNE wait 
void spi_rxne_wait(SPI_TypeDef *spi);

// SPI BSY wait 
void spi_bsy_wait(SPI_TypeDef *spi);

//=======================================================================================


//=======================================================================================
// User functions 

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
    // 
}


// SPI slave select pin init 
void spi_ss_init(
    GPIO_TypeDef *gpio, 
    pin_selector_t ss_pin)
{
    // 
}


// Set the SPE bit to enable SPI 
void spi_enable(
    SPI_TypeDef *spi)
{
    // 
}


// Clear the SPE bit to disable SPI 
void spi_disable(
    SPI_TypeDef *spi)
{
    // 
}


// Wait for TXE bit to set 
void spi_txe_wait(
    SPI_TypeDef *spi)
{
    // 
}

// Wait for RXNE bit to set 
void spi_rxne_wait(
    SPI_TypeDef *spi)
{
    // 
}


// Wait for BSY bit to clear
void spi_bsy_wait(
    SPI_TypeDef *spi)
{
    // 
}


// Select an SPI slave 
void spi_slave_select(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num)
{
    // 
}


// Deselect an SPI slave 
void spi_slave_deselect(
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t slave_num)
{
    // 
}


// SPI write 
SPI_STATUS spi_write(
    SPI_TypeDef *spi, 
    const uint8_t *write_data, 
    uint32_t data_len)
{
    return SPI_OK; 
}


// SPI write then read 
SPI_STATUS spi_write_read(
    SPI_TypeDef *spi, 
    uint8_t write_data, 
    uint8_t *read_data, 
    uint32_t data_len)
{
    return SPI_OK; 
}

//=======================================================================================


//=======================================================================================
// Mocks 
//=======================================================================================
