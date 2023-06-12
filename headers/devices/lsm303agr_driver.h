/**
 * @file lsm303agr.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR accelerometer and magnetometer driver header 
 * 
 * @version 0.1
 * @date 2023-06-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _LSM303AGR_DRIVER_H_ 
#define _LSM303AGR_DRIVER_H_ 

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 
#include "i2c_comm.h"
#include "gpio_driver.h"
#include "linked_list_driver.h"

//=======================================================================================


//=======================================================================================
// Macros 

// I2C addresses (datasheet page 39) 
#define LSM303AGR_ACCEL_7BIT_ADDR 0x19        // Accelerometer 7-bit I2C address - no R/W bit 
#define LSM303AGR_MAG_7BIT_ADDR 0x1E          // Magnetometer 7-bit I2C address - no R/W bit 
#define LSM303AGR_ACCEL_ADDR 0x32             // Accelerometer I2C address - with default W bit 
#define LSM303AGR_MAG_ADDR 0x3C               // Magnetometer I2C address - with default W bit 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief 
 */
typedef enum {
    LSM303AGR_W_OFFSET, 
    LSM303AGR_R_OFFSET 
} lsm303agr_rw_offset_t; 

//=======================================================================================

#endif   // _LSM303AGR_DRIVER_H_ 
