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
// Global variables 

// Mock driver data record 
typedef struct i2c_mock_driver_data_s 
{
    uint8_t write_data[150]; 
    uint8_t write_data_size; 

    uint8_t timeout; 
}
i2c_mock_driver_data_t; 

i2c_mock_driver_data_t mock_driver_data; 

//=======================================================================================


//=======================================================================================
// User functions 

// I2C generate start condition 
I2C_STATUS i2c_start(
    I2C_TypeDef *i2c)
{
    if (mock_driver_data.timeout)
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
    memcpy((void *)mock_driver_data.write_data, (void *)data, data_size); 
    mock_driver_data.write_data_size = data_size; 

    return I2C_OK; 
}


// I2C read data from a device
I2C_STATUS i2c_read(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint16_t data_size)
{
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 

    // // Check the amount of data to be received 
    // switch(data_size)
    // {
    //     case BYTE_0:  // No data specified - no transmission
    //         break;

    //     case BYTE_1:  // One-byte transmission 
    //         // Clear the ACK bit to send a NACK pulse to the slave
    //         i2c_clear_ack(i2c);

    //         // Read SR1 and SR2 to clear ADDR
    //         i2c_clear_addr(i2c);

    //         // Generate stop condition
    //         i2c_stop(i2c);

    //         // Wait for RxNE bit to set indicating data is ready 
    //         i2c_status |= i2c_rxne_wait(i2c);

    //         // Read the data regsiter
    //         *data = i2c->DR;

    //         break;

    //     default:  // Greater than one-byte transmission 
    //         // Read SR1 and SR2 to clear ADDR
    //         i2c_clear_addr(i2c);

    //         // Normal reading 
    //         for (uint8_t i = 0; i < (data_size - BYTE_2); i++)
    //         {
    //             i2c_status |= i2c_rxne_wait(i2c);  // Indicates when data is ready 
    //             *data++ = i2c->DR;                 // Read data
    //             i2c_set_ack(i2c);                  // Set the ACK bit 
    //         }

    //         // Read the second last data byte 
    //         i2c_status |= i2c_rxne_wait(i2c);
    //         *data = i2c->DR;

    //         // Clear the ACK bit to send a NACK pulse to the slave
    //         i2c_clear_ack(i2c);

    //         // Generate stop condition
    //         i2c_stop(i2c);

    //         // Read the last data byte
    //         data++;
    //         i2c_status |= i2c_rxne_wait(i2c);
    //         *data = i2c->DR;

    //         break;
    // }

    return i2c_status; 
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
    uint8_t timeout_status)
{
    mock_driver_data.timeout = timeout_status; 

    // Initialize mock data 
    memset((void *)mock_driver_data.write_data, CLEAR, sizeof(mock_driver_data.write_data)); 
    mock_driver_data.write_data_size = CLEAR; 
}


// Get write data 
void i2c_mock_get_write_data(
    void *data_buff, 
    uint8_t *data_size)
{
    memcpy(data_buff, (void *)mock_driver_data.write_data, mock_driver_data.write_data_size); 
    *data_size = mock_driver_data.write_data_size; 
}

//=======================================================================================
