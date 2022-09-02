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
// Read 

// Read an NMEA message from the M8Q 
void m8q_read_nmea(
    I2C_TypeDef *i2c, 
    uint8_t *data)
{
    // Local variables
    uint16_t data_size = 0; 
    uint8_t address = M8Q_READ_ADDR_DATA_SIZE;  

    // Generate a start condition 
    i2c_start(i2c); 

    // Write the slave address with write access 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_W_OFFSET); 
    i2c_clear_addr(i2c); 

    // Send the register address to start reading at 
    i2c_write_master_mode(i2c, &address, I2C_1_BYTE); 

    // Generate another start condition 
    i2c_start(i2c); 

    // Send the device address again with a read offset 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET);  
    i2c_clear_addr(i2c); 

    // Read the first two bytes to get the data size 
    i2c_rxne_wait(i2c);                              // Wait for data to be ready 
    data_size |= (uint16_t)((i2c->DR) << SHIFT_8);   // Read data
    i2c_set_ack(i2c);                                // Indicated that data has been read 
    i2c_rxne_wait(i2c);  
    data_size |= (uint16_t)(i2c->DR); 
    i2c_set_ack(i2c); 

    // Read the device data using the data length 
    i2c_read_master_mode(i2c, data, data_size); 
}

//=======================================================================================
