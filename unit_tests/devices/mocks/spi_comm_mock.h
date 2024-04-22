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
// Macros 

// 

//=======================================================================================


//=======================================================================================
// Enums 

// 

//=======================================================================================


//=======================================================================================
// Mock functions 

// // Mock initialization 
// void spi_mock_init(
//     i2c_mock_timeout_t timeout_status, 
//     i2c_mock_increment_mode_t increment_mode_write, 
//     i2c_mock_increment_mode_t increment_mode_read); 


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
