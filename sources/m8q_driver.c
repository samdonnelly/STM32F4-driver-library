/**
 * @file m8q_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS driver 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//=======================================================================================
// Includes 

// Device drivers 
#include "m8q_driver.h"

// Libraries 

//=======================================================================================


//=======================================================================================
// Initialization 

// 
void m8q_init(void)
{
    // TODO configure GPIO for txReady pin if desired 
}

//=======================================================================================


//=======================================================================================
// NMEA Read 

// Read an NMEA message from the M8Q 
NMEA_VALID m8q_read_nmea(
    I2C_TypeDef *i2c, 
    uint8_t *data)
{
    // Local variables 
    NMEA_VALID read_status = M8Q_NMEA_READ_INVALID; 
    uint8_t data_check = 0; 

    // Check for a valid data stream 
    m8q_check_nmea_stream(i2c, &data_check); 

    // Check the result 
    switch (data_check)
    {
        case M8Q_INVALID_NMEA:  // No data stream available 
            break;

        case M8Q_VALID_NMEA:  // Valid data stream - start of NMEA message 
            // Generate a start condition 
            i2c_start(i2c); 

            // Send the device address with a read offset 
            i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET); 
            i2c_clear_addr(i2c);  

            // Read the rest of the data stream until "\r\n" 
            i2c_read_to_term(i2c, data, M8Q_END_NMEA, I2C_4_BYTE); 

            read_status = M8Q_NMEA_READ_VALID; 
            break;

        default:  // Unknown data stream 
            break;
    }

    return read_status; 
}


// Read the NMEA data stream size 
void m8q_read_nmea_ds(
    I2C_TypeDef *i2c, 
    uint16_t *data_size)
{
    // Local variables 
    uint8_t num_bytes[BYTE_2];            // Store the high and low byte of the data size 
    uint8_t address = M8Q_READ_DS_ADDR;   // Address of high byte for the data size 

    // Generate a start condition 
    i2c_start(i2c); 

    // Write the slave address with write access 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_W_OFFSET); 
    i2c_clear_addr(i2c); 

    // Send the first data size register address to start reading from there 
    i2c_write_master_mode(i2c, &address, I2C_1_BYTE); 

    // Generate another start condition 
    i2c_start(i2c); 

    // Send the device address again with a read offset 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET);  
    i2c_clear_addr(i2c); 

    // Read the data size registers 
    i2c_read_master_mode(i2c, num_bytes, I2C_2_BYTE); 

    // Format the data into the data size 
    *data_size = (uint16_t)((num_bytes[BYTE_0] << SHIFT_8) | num_bytes[BYTE_1]); 
}


// Check for a valid NMEA data stream 
void m8q_check_nmea_stream(
    I2C_TypeDef *i2c, 
    uint8_t *data_check)
{
    // Generate a start condition 
    i2c_start(i2c); 

    // Send the device address with a read offset 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET); 
    i2c_clear_addr(i2c); 

    // Read the first byte of the data stream 
    i2c_read_master_mode(i2c, data_check, I2C_1_BYTE); 
}

//=======================================================================================


//=======================================================================================
// NMEA write 
//=======================================================================================


//=======================================================================================
// PUBX read 
//=======================================================================================


//=======================================================================================
// PUBX write 
//=======================================================================================


//=======================================================================================
// Message processing 
//=======================================================================================
