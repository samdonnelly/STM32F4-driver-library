/**
 * @file i2c_comm.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2022-03-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "i2c_comm.h"

//=======================================================================================


//=======================================================================================
// Initiate I2C 

// Initialize I2C 1 in master mode 
void i2c1_init_master_mode(void)
{
    //==============================================================
    // I2C Mater Mode 
    //  1. Program the peripheral input clock in I2C_CR2 register
    //  2. Configure the clock control register 
    //  3. Program the I2C_CR1 register to enable the peripheral 
    //  4. Set the START but in the I2C_CR1 register to generate a 
    //     Start condition 
    //==============================================================
}


// Initialize I2C 1 in slave mode 
void i2c1_init_slave_mode(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Write I2C 

// Send data to a device using I2C 1 
void i2c1_write(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Read I2C 

// Read data from a device using I2C 1 
void i2c1_read(void)
{
    // 
}

//=======================================================================================
