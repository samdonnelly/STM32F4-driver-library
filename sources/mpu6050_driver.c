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

// 
void mpu6050_init(void)
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

    //==============================================================
    // MPU-6050 Init
    //  1. Read the WHO_AM_I register to establish that there is communication 
    //  2. Wake the sensor up through the power management register 
    //  3. Set the data rate 
    //  4. Configure the accelerometer register 
    //  5. Configure the gyroscope register 
    //==============================================================
}

//=======================================================================================
