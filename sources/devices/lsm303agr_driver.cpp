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
// Notes 

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

// 

// Write (after completing the general start procedure) 
// - Transmit data to the slave - slave acknowledge between each byte 
// - Generate a stop condition 

// Low power mode can achieved by setting the low power bit but also putting the device into 
// idle mode as well 

//=======================================================================================


//=======================================================================================
// Variables 

// 
typedef struct lsm303agr_driver_data_s 
{
    // Peripherals 
    I2C_TypeDef *i2c; 

    // Device info 
    uint8_t addr; 

    // Status info 
    // 'status' --> bit 0: i2c status (see i2c_status_t) 
    //          --> bit 1: init status (WHO_AM_I) 
    //          --> bits 2-7: self test results 
    uint8_t status; 
}
lsm303agr_driver_data_t; 


// 
static lsm303agr_driver_data_t lsm303agr_driver_data; 

//=======================================================================================


//=======================================================================================
// Initialization 

// Config magnetometer 
// Config accelerometer 
// Run self test 
void lsm303agr_init(
    I2C_TypeDef *i2c, 
    uint8_t i2c_addr)
{
    // Initialize data record 
    lsm303agr_driver_data.i2c = i2c; 
    lsm303agr_driver_data.addr = i2c_addr; 
    lsm303agr_driver_data.status = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// Read and write 

// Read (after completing the general start procedure) 
// - 
// - 
// - 
// - Read data (byte) from slave 
// - If reading one byte 
//   - Go to second last step 
// - If read multiple bytes: 
//   - Master acknowledge 
//   - Read data (byte) from slave 
//   - Repeat the above two steps for all the needed data then proceed to the next step 
// - Master non-acknowledge 
// - 

// Read 
void lsm303agr_read(void)
{
    // Local variables 
    uint8_t i2c_status = I2C_OK; 

    // Generate a start condition 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the slave address with a write bit 
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          lsm303agr_driver_data.addr + LSM303AGR_W_OFFSET); 
    i2c_clear_addr(lsm303agr_driver_data.i2c); 

    // Write register address 

    // Start condition 

    //Slave address with read bit 

    // Read data 

    // Generate a stop condition 
    i2c_stop(lsm303agr_driver_data.i2c); 

    // i2c_status |= i2c_write(lsm303agr_driver_data.i2c, uint8_t, uint8_t); 
    // i2c_status |= i2c_read(lsm303agr_driver_data.i2c, uint8_t, uint16_t); 

    // Update the driver status 
    lsm303agr_driver_data.status |= i2c_status; 
}


// Write 
void lsm303agr_write(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Self-test 

// Self-test procedure outlined on page 26 of the user manual 

//=======================================================================================


//=======================================================================================
// Register functions 

// Magnetometer data read 
void lsm303agr_mag_read(void)
{
    // 
}


// Magnetometer configuration register A write/read 
void lsm303agr_mag_cfga_write(void)
{
    // 
}


// Magnetometer configuration register B write/read 
void lsm303agr_mag_cfga_write(void)
{
    // 
}


// Magnetometer configuration register C write/read 
void lsm303agr_mag_cfga_write(void)
{
    // 
}


// Magnetometer status register read 
void lsm303agr_mag_status_read(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// User functions 
//=======================================================================================
