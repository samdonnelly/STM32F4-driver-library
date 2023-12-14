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
    char write_data[150]; 
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
    uint8_t *data, 
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
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 

    // // Read the data until the termination character is seen 
    // do
    // {
    //     i2c_status |= i2c_rxne_wait(i2c); 
    //     *data = i2c->DR; 
    //     i2c_set_ack(i2c); 
    // } 
    // while (*data++ != term_char); 

    // // Read the remaining bytes and terminate the data 
    // i2c_status |= i2c_read(i2c, data, bytes_remain); 
    // data += bytes_remain; 
    // *data = 0; 

    return i2c_status; 
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
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 
    // uint16_t msg_length = 0; 

    // // Read up to and including the part of the message that specifies the length 
    // i2c_status |= i2c_read(i2c, data, len_location + len_bytes); 

    // // Extract the length and correct it using the 'add_bytes' argument 
    // data += len_location; 
    
    // if (len_bytes == BYTE_1)
    // {
    //     msg_length = (uint16_t)(*data++) + add_bytes; 
    // }
    // else if (len_bytes == BYTE_2)
    // {
    //     msg_length  = (uint16_t)(*data++); 
    //     msg_length |= (uint16_t)(*data++ << SHIFT_8); 
    //     msg_length += add_bytes; 
    // }
    
    // // Read the rest of the message 
    // i2c_start(i2c); 
    // i2c_status |= i2c_write_addr(i2c, address); 
    // i2c_clear_addr(i2c); 
    // i2c_status |= i2c_read(i2c, data, msg_length); 

    return i2c_status; 
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
    char *data_buff, 
    uint8_t *data_size)
{
    memcpy((void *)data_buff, (void *)mock_driver_data.write_data, mock_driver_data.write_data_size); 
    *data_size = mock_driver_data.write_data_size; 
}

//=======================================================================================
