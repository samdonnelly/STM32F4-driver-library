/**
 * @file spi_driver_mock.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Mock SPI communication driver interface - for unit testing 
 * 
 * @version 0.1
 * @date 2024-03-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _SPI_COMM_MOCK_H_ 
#define _SPI_COMM_MOCK_H_ 

//=======================================================================================
// Includes 

#include "spi_comm.h" 

//=======================================================================================


//=======================================================================================
// Enums 

// Index for SPI mock driver data buffers 
typedef enum {
    SPI_MOCK_INDEX_0, 
    SPI_MOCK_INDEX_1, 
    SPI_MOCK_INDEX_2, 
    SPI_MOCK_INDEX_3, 
    SPI_MOCK_INDEX_4, 
    SPI_MOCK_INDEX_5, 
    SPI_MOCK_INDEX_6, 
    SPI_MOCK_INDEX_7, 
    SPI_MOCK_INDEX_8, 
    SPI_MOCK_INDEX_9, 
    SPI_MOCK_INDEX_10, 
    SPI_MOCK_INDEX_11 
} spi_mock_buff_index_t; 


// SPI mock driver timeout behavior selection 
typedef enum {
    SPI_MOCK_TIMEOUT_DISABLE, 
    SPI_MOCK_TIMEOUT_ENABLE 
} spi_mock_timeout_t; 


// SPI mock driver increment mode selection 
typedef enum {
    SPI_MOCK_INC_MODE_DISABLE, 
    SPI_MOCK_INC_MODE_ENABLE 
} spi_mock_increment_mode_t; 

//=======================================================================================


//=======================================================================================
// Mock functions 

// Mock initialization 
void spi_mock_init(
    spi_mock_timeout_t timeout_status, 
    spi_mock_increment_mode_t increment_mode_write, 
    spi_mock_increment_mode_t increment_mode_read); 


// Get write data 
void spi_mock_get_write_data(
    void *data_buff, 
    uint8_t *data_size, 
    uint8_t write_index); 


// Set read data 
void spi_mock_set_read_data(
    const void *read_data, 
    uint16_t data_size, 
    uint8_t read_index); 

//=======================================================================================

#endif   // _SPI_COMM_MOCK_H_ 
