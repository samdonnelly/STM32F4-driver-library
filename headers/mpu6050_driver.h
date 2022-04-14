/**
 * @file mpu6050_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver for the MPU6050 accelerometer
 * 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MPU6050_DRIVER_H_
#define _MPU6050_DRIVER_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "i2c_comm.h"

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief MPU-6050 Addresses 
 * 
 * @details The AD0 pin on the accelerometer can be set as either 0 or 1 to allow for 
 *          two different MPU's to be on the same I2C bus. The default address of the 
 *          MPU-6050 is 0x68 (pin AD0 = 0). 
 * 
 */
typedef enum {
    MPU6050_1_ADDRESS = 0x68,
    MPU6050_2_ADDRESS = 0x69
} mpu6050_addresses_t;

/**
 * @brief MPU-6050 register byte size
 * 
 */
typedef enum {
    MPU6050_REG_1_BYTE = 1,
    MPU6050_REG_2_BYTE = 2
} mpu6050_reg_byte_size_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief MPU5060 initialization 
 * 
 */
uint8_t mpu6050_init(uint8_t mpu6050_address);

//=======================================================================================


#endif  // _MPU6050_DRIVER_H_
