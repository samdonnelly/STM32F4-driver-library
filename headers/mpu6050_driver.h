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
// Macros 

#define MPU6050_7_BIT_ADDRESS 0x68  // MPU-6050 address before read/write bit shift

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
 *          The 7-bit address of a slave I2C device is held in the upper 7-bits of the 
 *          byte and LSB is left to indicate read/receive (1) or write/transmit (0). 
 *          This makes the addresses if the MPU-6050 bit shifted to the left by 1. The 
 *          resulting addresses become: 
 *             - 0xD0 and 0xD1 if AD0 is 0 
 *             - 0xD2 and 0xD3 if AD0 is 1 
 * 
 */
typedef enum {
    MPU6050_1_ADDRESS = 0xD0,
    MPU6050_2_ADDRESS = 0xD2
} mpu6050_addresses_t;

/**
 * @brief MPU-6050 read and write offset 
 * 
 */
typedef enum {
    MPU6050_W_OFFSET = 0x00,
    MPU6050_R_OFFSET = 0x01
} mpu6050_rw_offset_t;

/**
 * @brief MPU-6050 register byte size
 * 
 */
typedef enum {
    MPU6050_REG_1_BYTE = 1,
    MPU6050_REG_2_BYTE = 2
} mpu6050_reg_byte_size_t;


typedef enum {
    MPU6050_WHO_AM_I = 0x75
} mpu6050_register_addresses_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief MPU5060 initialization 
 * 
 */
uint8_t mpu6050_init(uint8_t mpu6050_address);

/**
 * @brief MPU-6050 write to register 
 * 
 */
void mpu6050_write(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register,
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);

/**
 * @brief MPU-6050 read from register 
 * 
 * @details 
 *          The MPU automatically increments the register address if there is more than 
 *          one byte to be read. 
 * 
 */
void mpu6050_read(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register, 
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);

/**
 * @brief 
 * 
 * @details 
 * 
 * @param mpu6050_sddress 
 * @return uint8_t : Returns 0x68 if AD0 == 0 and 0x69 is AD0 == 1
 */
uint8_t mpu6050_who_am_i_read(uint8_t mpu6050_sddress);

//=======================================================================================


#endif  // _MPU6050_DRIVER_H_
