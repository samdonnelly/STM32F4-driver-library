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

// Data tools 
#define LSM303AGR_BIT_MASK 0x01               // Mask to filter out status bits 
#define LSM303AGR_ADDR_INC 0x80               // Register address increment bit 

// Magnetometer configuration 
#define LSM303AGR_ID_M 0x40                   // Value returned from the WHO AM I register 

// Magnetometer register addresses 
#define LSM303AGR_WHO_AM_I_M 0x4F             // WHO AM I 
#define LSM303AGR_CFG_A_M 0x60                // Configuration register A 
#define LSM303AGR_CFG_B_M 0x61                // Configuration register B 
#define LSM303AGR_CFG_C_M 0x62                // Configuration register C 
#define LSM303AGR_STATUS_M 0x67               // Status register 
#define LSM303AGR_X_L_M 0x68                  // X component of magnetic field (first data reg) 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief I2C write and read bits 
 */
typedef enum {
    LSM303AGR_W_OFFSET, 
    LSM303AGR_R_OFFSET 
} lsm303agr_rw_offset_t; 


/**
 * @brief Device setting disable/enable 
 * 
 * @details 
 */
typedef enum {
    LSM303AGR_CFG_DISABLE, 
    LSM303AGR_CFG_ENABLE 
} lsm303agr_cfg_t; 


/**
 * @brief Magnetometer output data rate 
 * 
 * @details 
 */
typedef enum {
    LSM303AGR_M_ODR_10, 
    LSM303AGR_M_ODR_20, 
    LSM303AGR_M_ODR_50, 
    LSM303AGR_M_ODR_100 
} lsm303agr_m_odr_cfg_t; 


/**
 * @brief Magnetometer system mode 
 * 
 * @details 
 */
typedef enum {
    LSM303AGR_M_MODE_CONT, 
    LSM303AGR_M_MODE_SINGLE, 
    LSM303AGR_M_MODE_IDLE 
} lsm303agr_m_sys_mode_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef uint8_t LSM303AGR_REG_ADDR; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief Initialization 
 * 
 * @details 
 * 
 * @param i2c 
 * @param m_odr 
 * @param m_mode 
 * @param m_off_canc 
 * @param m_lpf 
 * @param m_int_mag_pin 
 * @param m_int_mag 
 */
void lsm303agr_init(
    I2C_TypeDef *i2c, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag); 

//=======================================================================================


//=======================================================================================
// Read and write 

// Read from register 
void lsm303agr_read(
    LSM303AGR_REG_ADDR addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_reg_size); 


// Write to register 
void lsm303agr_write(
    LSM303AGR_REG_ADDR reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_reg_size); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Magnetometer data read 
 * 
 * @details 
 */
void lsm303agr_m_read(void); 


/**
 * @brief Get magnetometer data 
 * 
 * @details 
 * 
 * @param m_x_data 
 * @param m_y_data 
 * @param m_z_data 
 */
void lsm303agr_m_get_data(
    int16_t *m_x_data, 
    int16_t *m_y_data, 
    int16_t *m_z_data); 

//=======================================================================================

#endif   // _LSM303AGR_DRIVER_H_ 
