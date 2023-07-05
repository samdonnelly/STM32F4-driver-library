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


//=======================================================================================
// Read and Write 

// Procedures start on page 38 of the datasheet 

// General Starting Procedure 
// - Generate start condition 
// - Write slave address with a write bit 
// - Slave acknowledges 
// - Sub address sent to slave: 
//   - bits 0-6 --> register address of slave to read/write from/to 
//   - bit 7 -----> address increment bit 
//     - 1 == increment to next address after read/write - allows for multiple data read/writes 
//     - 0 == don't increment address afterwards 

// Read (after completing the general start procedure) 
// - Generate a start condition 
// - Send the slave address with a read bit 
// - Slave acknowledges 
// - Read data (byte) from slave 
// - If reading one byte 
//   - Go to second last step 
// - If read multiple bytes: 
//   - Master acknowledge 
//   - Read data (byte) from slave 
//   - Repeat the above two steps for all the needed data then proceed to the next step 
// - Master non-acknowledge 
// - Generate a stop condition 

// Write (after completing the general start procedure) 
// - Transmit data to the slave - slave acknowledge between each byte 
// - Generate a stop condition 

//=======================================================================================


//=======================================================================================
// Self-test 

// Self-test procedure outlined on page 26 of the user manual 

//=======================================================================================


//=======================================================================================
// Register functions 

// 

//=======================================================================================
