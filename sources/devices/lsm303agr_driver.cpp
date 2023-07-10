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

// Data record structure 
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


// Driver data record instance 
static lsm303agr_driver_data_t lsm303agr_driver_data; 

//=======================================================================================


//=======================================================================================
// Initialization 

// Initialization 
void lsm303agr_init(
    I2C_TypeDef *i2c, 
    uint8_t i2c_addr)
{
    // Initialize data record 
    lsm303agr_driver_data.i2c = i2c; 
    lsm303agr_driver_data.addr = i2c_addr; 
    lsm303agr_driver_data.status = CLEAR; 

    // Config magnetometer 
    // Config accelerometer 
    // Run self test 
    }

//=======================================================================================


//=======================================================================================
// Read and write 

// Read from register 
void lsm303agr_read(
    LSM303AGR_REG_ADDR reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    // Local variables 
    uint8_t i2c_status = I2C_OK; 

    // Generate a start condition 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the slave address with a write bit 
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          lsm303agr_driver_data.addr + LSM303AGR_W_OFFSET); 
    i2c_clear_addr(lsm303agr_driver_data.i2c); 

    // Send the register address that is going to be read 
    i2c_status |= (uint8_t)i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1);

    // Create another start signal 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the LSM303AGR address with a read offset 
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          lsm303agr_driver_data.addr + LSM303AGR_R_OFFSET);
    
    // Read the data sent by the MPU6050 
    i2c_status |= (uint8_t)i2c_read(lsm303agr_driver_data.i2c, 
                                    lsm303agr_reg_value, 
                                    lsm303agr_data_size);

    // Generate a stop condition 
    i2c_stop(lsm303agr_driver_data.i2c); 

    // Update the driver status 
    lsm303agr_driver_data.status |= i2c_status; 
}


// Write to register 
void lsm303agr_write(
    LSM303AGR_REG_ADDR reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    // Local variables 
    uint8_t i2c_status = I2C_OK; 
    
    // Create start condition to initiate master mode 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the MPU6050 address with a write offset
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          lsm303agr_driver_data.addr + LSM303AGR_W_OFFSET);
    i2c_clear_addr(lsm303agr_driver_data.i2c);

    // Send the register address that is going to be written to 
    i2c_status |= (uint8_t)i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1);

    // Write the data to the MPU6050 
    i2c_status |= (uint8_t)i2c_write(lsm303agr_driver_data.i2c, 
                                     lsm303agr_reg_value, 
                                     lsm303agr_data_size);

    // Create a stop condition
    i2c_stop(lsm303agr_driver_data.i2c); 

    // Update the driver status 
    lsm303agr_driver_data.status |= (uint8_t)i2c_status; 
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
    // Format the data 
    uint8_t cfg_a; 

    // Write the formatted data to the device 
    lsm303agr_write(LSM303AGR_CFG_A_M, &cfg_a, BYTE_1); 
}


// Magnetometer configuration register B write/read 
void lsm303agr_mag_cfga_write(void)
{
    // Format the data 
    uint8_t cfg_b; 

    // Write the formatted data to the device 
    lsm303agr_write(LSM303AGR_CFG_B_M, &cfg_b, BYTE_1); 
}


// Magnetometer configuration register C write/read 
void lsm303agr_mag_cfga_write(void)
{
    // Format the data 
    uint8_t cfg_c; 

    // Write the formatted data to the device 
    lsm303agr_write(LSM303AGR_CFG_C_M, &cfg_c, BYTE_1); 
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
