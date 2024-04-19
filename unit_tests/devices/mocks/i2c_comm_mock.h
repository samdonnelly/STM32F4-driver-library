/**
 * @file i2c_comm_mock.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Mock I2C communication driver interface - for unit testing 
 * 
 * @version 0.1
 * @date 2023-12-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _I2C_COMM_MOCK_H_
#define _I2C_COMM_MOCK_H_

//=======================================================================================
// Includes 

#include "i2c_comm.h" 

//=======================================================================================


//=======================================================================================
// Enums 

// Index for I2C mock driver data buffers 
typedef enum {
    I2C_MOCK_INDEX_0, 
    I2C_MOCK_INDEX_1, 
    I2C_MOCK_INDEX_2, 
    I2C_MOCK_INDEX_3, 
    I2C_MOCK_INDEX_4, 
    I2C_MOCK_INDEX_5, 
    I2C_MOCK_INDEX_6, 
    I2C_MOCK_INDEX_7, 
    I2C_MOCK_INDEX_8, 
    I2C_MOCK_INDEX_9, 
    I2C_MOCK_INDEX_10, 
    I2C_MOCK_INDEX_11 
} i2c_mock_buff_index_t; 


// I2C mock driver timeout behavior selection 
typedef enum {
    I2C_MOCK_TIMEOUT_DISABLE, 
    I2C_MOCK_TIMEOUT_ENABLE 
} i2c_mock_timeout_t; 


// I2C mock driver increment mode selection 
typedef enum {
    I2C_MOCK_INC_MODE_DISABLE, 
    I2C_MOCK_INC_MODE_ENABLE 
} i2c_mock_increment_mode_t; 

//=======================================================================================


//=======================================================================================
// Mock functions 

// Mock initialization 
void i2c_mock_init(
    i2c_mock_timeout_t timeout_status, 
    i2c_mock_increment_mode_t increment_mode_write, 
    i2c_mock_increment_mode_t increment_mode_read); 


// Get write data 
void i2c_mock_get_write_data(
    void *data_buff, 
    uint8_t *data_size, 
    uint8_t write_index); 


// Set read data 
void i2c_mock_set_read_data(
    const void *read_data, 
    uint16_t data_size, 
    uint8_t read_index); 

//=======================================================================================

#endif  // _I2C_COMM_MOCK_H_ 
