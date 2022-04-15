/**
 * @file mpu6050_driver.c
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

//=======================================================================================
// Includes 

#include "mpu6050_driver.h"

//=======================================================================================


//=======================================================================================
// Initialization 

// MPU-6050 Initialization 
uint8_t mpu6050_init(uint8_t mpu6050_address)
{
    //==============================================================
    // Notes:
    //  - The accelerometer may only be able to run in I2C fast mode (400kHz)
    //    which means it may not be able to be used on the same bus as the screen. 
    //  - The accelerometer must be woken up from sleep mode and have default 
    //    settings changed on startup. 
    //  - It looks like to init the accelerometer you have to choose how you want it 
    //    configured based on the registers meaning there is no exact way to init 
    //    the device with the exception of the point above. 
    //==============================================================

    // Store the value of the WHO_AM_I register 
    uint8_t mpu6050_who_am_i;

    //==============================================================
    // MPU-6050 Init
    //  1. Read the WHO_AM_I register to establish that there is communication 
    //  2. Wake the sensor up through the power management register 
    //  3. Set the data rate 
    //  4. Configure the accelerometer register 
    //  5. Configure the gyroscope register 
    //==============================================================

    // TODO configure the I2C1 master mode read function 

    // 1. Read the WHO_AM_I register to establish that there is communication 
    mpu6050_who_am_i = mpu6050_who_am_i_read(mpu6050_address);

    // Check that the correct address was returned
    if (mpu6050_who_am_i != MPU6050_7_BIT_ADDRESS)
    {
        return FALSE;
    }

    // 2. Wake the sensor up through the power management register 
    
    // 3. Set the data rate 
    
    // 4. Configure the accelerometer register 
    
    // 5. Configure the gyroscope register

    // Initialization completed successfully 
    return TRUE;
}

//=======================================================================================


//=======================================================================================
// Write and read functions 

// Write to the MPU-6050
void mpu6050_write(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register,
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c1_start(); 

    // Send the MPU6050 address with a write offset
    i2c1_write_address(mpu6050_address + MPU6050_W_OFFSET);
    i2c1_clear_addr();

    // Send the register address that is going to be written to 
    // i2c1_write_address(mpu6050_register);
    i2c1_write_master_mode(&mpu6050_register, mpu6050_reg_size);
    // i2c1_clear_addr();

    // Write the data to the MPU6050 
    i2c1_write_master_mode(mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c1_stop(); 
}

// Read from the MPU-6050
void mpu6050_read(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register, 
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c1_start(); 

    // Send the MPU6050 address with a write offset 
    i2c1_write_address(mpu6050_address + MPU6050_W_OFFSET);
    i2c1_clear_addr();

    // Send the register address that is going to be read 
    // i2c1_write_address(mpu6050_register);
    i2c1_write_master_mode(&mpu6050_register, mpu6050_reg_size);

    // Create another start signal 
    i2c1_start(); 

    // Send the MPU6050 address with a read offset 
    i2c1_write_address(mpu6050_address + MPU6050_R_OFFSET);

    // Read the data sent by the MPU6050 
    i2c1_read_master_mode(mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c1_stop(); 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// WHO_AM_I read 
uint8_t mpu6050_who_am_i_read(uint8_t mpu6050_address)
{
    // Place to store the value WHO_AM_I
    uint8_t mpu6050_who_am_i;

    // Read the value of WHO_AM_I register 
    mpu6050_read(
        mpu6050_address, 
        MPU6050_WHO_AM_I, 
        MPU6050_REG_1_BYTE,
        &mpu6050_who_am_i);

    // Return the value of who_am_i 
    return mpu6050_who_am_i;
}

//=======================================================================================
