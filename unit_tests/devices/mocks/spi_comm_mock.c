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

#include "spi_comm_mock.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define MAX_DATA_OPS 12 
#define MAX_DATA_SIZE 100 

//=======================================================================================


//=======================================================================================
// Global variables 

// Mock driver data record 
typedef struct spi_mock_driver_data_s 
{
    uint8_t spi_timeout; 
    uint8_t increment_mode_write; 
    uint8_t increment_mode_read; 

    uint8_t write_data[MAX_DATA_OPS][MAX_DATA_SIZE]; 
    uint8_t write_data_size[MAX_DATA_OPS]; 
    uint8_t write_index; 

    uint8_t read_data[MAX_DATA_OPS][MAX_DATA_SIZE]; 
    uint8_t read_index; 
}
spi_mock_driver_data_t; 

static spi_mock_driver_data_t mock_driver_data; 

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
// Driver functions 

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
// Mock functions 

// // Mock initialization 
// void spi_mock_init(
//     i2c_mock_timeout_t timeout_status, 
//     i2c_mock_increment_mode_t increment_mode_write, 
//     i2c_mock_increment_mode_t increment_mode_read)
// {
//     mock_driver_data.i2c_timeout = timeout_status; 
//     mock_driver_data.increment_mode_write = increment_mode_write; 
//     mock_driver_data.increment_mode_read = increment_mode_read; 

//     memset((void *)mock_driver_data.write_data, CLEAR, sizeof(mock_driver_data.write_data)); 
//     memset((void *)mock_driver_data.write_data_size, CLEAR, 
//             sizeof(mock_driver_data.write_data_size)); 
//     mock_driver_data.write_index = CLEAR; 

//     memset((void *)mock_driver_data.read_data, CLEAR, sizeof(mock_driver_data.read_data)); 
//     mock_driver_data.read_index = CLEAR; 
// }


// Get write data 
void spi_mock_get_write_data(
    void *write_buff, 
    uint8_t *write_data_size, 
    uint8_t write_index)
{
    if ((write_buff == NULL) || (write_data_size == NULL) || (write_index >= MAX_DATA_OPS))
    {
        return; 
    }

    memcpy(write_buff, (void *)(&mock_driver_data.write_data[write_index][0]), 
           mock_driver_data.write_data_size[write_index]); 
    *write_data_size = mock_driver_data.write_data_size[write_index]; 
}


// Set read data 
void spi_mock_set_read_data(
    const void *read_data, 
    uint16_t read_data_size, 
    uint8_t read_index)
{
    if ((read_data == NULL) || (read_index >= MAX_DATA_OPS))
    {
        return; 
    }

    memcpy((void *)(&mock_driver_data.read_data[read_index][0]), read_data, read_data_size); 
}

//=======================================================================================
