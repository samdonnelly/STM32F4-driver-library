/**
 * @file i2c_comm.h
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

#ifndef _I2C_COMM_H_
#define _I2C_COMM_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Enums 
//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief Initiate I2C 1 in Mater Mode
 * 
 */
void i2c1_init_master_mode(void);


/**
 * @brief Initiate I2C 1 in Slave Mode 
 * 
 */
void i2c1_init_slave_mode(void);


/**
 * @brief Write data to a device with I2C 1
 * 
 */
void i2c1_write(void);


/**
 * @brief Read data from a device with I2C 1 
 * 
 */
void i2c1_write(void);

//=======================================================================================


#endif  // _I2C_COMM_H_ 
