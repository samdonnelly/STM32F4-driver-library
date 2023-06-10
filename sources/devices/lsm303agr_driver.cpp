/**
 * @file lsm303agr.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR accelerometer and magnetometer driver 
 * 
 * @version 0.1
 * @date 2023-06-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_driver.h"

//=======================================================================================


//=======================================================================================
// C file inclusion 

extern "C" 
{
    // I2C functions 
    I2C_STATUS i2c_start(I2C_TypeDef); 
    void i2c_stop(I2C_TypeDef); 
    void i2c_clear_addr(I2C_TypeDef); 
    I2C_STATUS i2c_write_addr(I2C_TypeDef, uint8_t); 
    I2C_STATUS i2c_write(I2C_TypeDef, uint8_t, uint8_t); 
    I2C_STATUS i2c_read(I2C_TypeDef, uint8_t, uint16_t); 
}

//=======================================================================================
