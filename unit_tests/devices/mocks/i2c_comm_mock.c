/**
 * @file i2c_comm_mock.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C communication driver implementation - mock 
 * 
 * @version 0.1
 * @date 2023-13-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "i2c_comm.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define MAX_DATA_OPS 2 
#define MAX_DATA_SIZE 300 

//=======================================================================================


//=======================================================================================
// Global variables 

// Mock driver data record 
typedef struct i2c_mock_driver_data_s 
{
    uint8_t i2c_timeout; 
    uint8_t increment_mode; 

    uint8_t write_data[MAX_DATA_OPS][MAX_DATA_SIZE]; 
    uint8_t write_data_size[MAX_DATA_OPS]; 
    uint8_t write_index; 

    uint8_t read_data[MAX_DATA_OPS][MAX_DATA_SIZE]; 
    uint8_t read_index; 
}
i2c_mock_driver_data_t; 

static i2c_mock_driver_data_t mock_driver_data; 

//=======================================================================================


//=======================================================================================
// User functions 

// I2C generate start condition 
I2C_STATUS i2c_start(
    I2C_TypeDef *i2c)
{
    if (mock_driver_data.i2c_timeout)
    {
        return I2C_TIMEOUT; 
    }
    
    return I2C_OK; 
}


// I2C generate a stop condition by setting the stop generation bit 
void i2c_stop(
    I2C_TypeDef *i2c)
{
    // 
}


// Read the SR1 and SR2 registers to clear ADDR
void i2c_clear_addr(
    I2C_TypeDef *i2c)
{
    // 
}


// I2C send address 
I2C_STATUS i2c_write_addr(
    I2C_TypeDef *i2c, 
    uint8_t i2c_address)
{
    return I2C_OK; 
}


// I2C send data to a device 
I2C_STATUS i2c_write(
    I2C_TypeDef *i2c, 
    const uint8_t *data, 
    uint8_t data_size)
{
    if ((data == NULL) || (mock_driver_data.write_index >= MAX_DATA_OPS))
    {
        return I2C_NULL_PTR; 
    }

    memcpy((void *)(&mock_driver_data.write_data[mock_driver_data.write_index][0]), 
           (void *)data, data_size); 
    mock_driver_data.write_data_size[mock_driver_data.write_index] = data_size; 

    if (mock_driver_data.increment_mode)
    {
        mock_driver_data.write_index++; 
    }

    return I2C_OK; 
}


// I2C read data from a device
I2C_STATUS i2c_read(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size)
{
    if ((data == NULL) || (mock_driver_data.read_index >= MAX_DATA_OPS))
    {
        return I2C_NULL_PTR; 
    }

    memcpy((void *)data, 
           (void *)(&mock_driver_data.read_data[mock_driver_data.read_index][0]), 
           data_size); 

    if (mock_driver_data.increment_mode)
    {
        mock_driver_data.read_index++; 
    }

    return I2C_OK; 
}


// I2C read data until a termination character is seen 
I2C_STATUS i2c_read_to_term(
    I2C_TypeDef *i2c, 
    uint8_t *data,  
    uint8_t term_char, 
    uint16_t bytes_remain)
{
    return I2C_OK; 
}


// I2C read data of a certain length that is defined within the message 
I2C_STATUS i2c_read_to_len(
    I2C_TypeDef *i2c, 
    uint8_t address, 
    uint8_t *data, 
    uint8_t len_location, 
    uint8_t len_bytes, 
    uint8_t add_bytes)
{
    return I2C_OK; 
}

//=======================================================================================


//=======================================================================================
// Mocks 

// Mock initialization 
void i2c_mock_init(
    i2c_mock_timeout_t timeout_status, 
    i2c_mock_increment_mode_t increment_mode)
{
    mock_driver_data.i2c_timeout = timeout_status; 
    mock_driver_data.increment_mode = increment_mode; 

    memset((void *)mock_driver_data.write_data, CLEAR, sizeof(mock_driver_data.write_data)); 
    memset((void *)mock_driver_data.write_data_size, CLEAR, 
            sizeof(mock_driver_data.write_data_size)); 
    mock_driver_data.write_index = CLEAR; 

    memset((void *)mock_driver_data.read_data, CLEAR, sizeof(mock_driver_data.read_data)); 
    mock_driver_data.read_index = CLEAR; 
}


// Get write data 
void i2c_mock_get_write_data(
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
void i2c_mock_set_read_data(
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
