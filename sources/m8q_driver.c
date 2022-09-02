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

    // Generate a start condition 
    i2c_start(I2C1); 

    // Write the slave address with write access 
    i2c_write_address(I2C1, M8Q_I2C_8_BIT_ADDR + M8Q_W_OFFSET); 
    i2c_clear_addr(I2C1); 

    // Send the register address to start reading at 
    i2c_write_master_mode(I2C1, (uint8_t *)M8Q_READ_ADDR_0XFD, I2C_1_BYTE); 

    // Generate another start condition 
    i2c_start(I2C1); 

    // Send the device address again with a read offset 
    i2c_write_address(I2C1, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET); 
    i2c_clear_addr(I2C1); 

    // Read the first two bytes to get the data length 

    // Read the device data using the data length 

    // Generate a stop condition 
}

//=======================================================================================
