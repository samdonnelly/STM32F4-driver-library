/**
 * @file lsm303agr.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR magnetometer driver header 
 * 
 * @version 0.1
 * @date 2023-06-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _LSM303AGR_DRIVER_H_ 
#define _LSM303AGR_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 
#include "i2c_comm.h"
#include "gpio_driver.h"
#include "linked_list_driver.h"

// Standard libraries 
#include "math.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Data tools 
#define LSM303AGR_BIT_MASK 0x01               // Mask to filter out status bits 
#define LSM303AGR_ADDR_INC 0x80               // Register address increment bit 

// Accelerometer I2C addresses 
#define LSM303AGR_A_7BIT_ADDR 0x19            // Accelerometer 7-bit I2C address - no R/W bit 
#define LSM303AGR_A_ADDR 0x32                 // Accelerometer I2C address - with default W bit 

// Magnetometer I2C addresses (datasheet page 39) 
#define LSM303AGR_M_7BIT_ADDR 0x1E            // Magnetometer 7-bit I2C address - no R/W bit 
#define LSM303AGR_M_ADDR 0x3C                 // Magnetometer I2C address - with default W bit 

// Magnetometer configuration 
#define LSM303AGR_M_ID 0x40                   // Value returned from the WHO AM I register 

// Magnetometer register addresses 
#define LSM303AGR_M_WHO_AM_I 0x4F             // WHO AM I 
#define LSM303AGR_M_CFG_A 0x60                // Configuration register A 
#define LSM303AGR_M_CFG_B 0x61                // Configuration register B 
#define LSM303AGR_M_CFG_C 0x62                // Configuration register C 
#define LSM303AGR_M_STATUS 0x67               // Status register 
#define LSM303AGR_M_X_L 0x68                  // X component of magnetic field (first data reg) 

// Magnetometer data 
#define LSM303AGR_M_SENS 3                    // Magnetometer sensitivity numerator (3/2 == 1.5) 
#define LSM303AGR_M_HEAD_SCALE 10             // Heading scaling factor (to remove decimals) 
#define LSM303AGR_M_DIR_OFFSET 450            // 45deg (*10) - heading sections (ex. N-->NE) 
#define LSM303AGR_M_HEAD_MAX 3600             // Max heading value - scaled (360deg * 10)
#define LSM303AGR_M_HEAD_DIFF 1800            // Heading different threshold for filtering 
#define LSM303AGR_M_N 0                       // North direction heading - scaled 
#define LSM303AGR_M_NE 450                    // North-East direction heading - scaled 
#define LSM303AGR_M_E 900                     // East direction heading - scaled 
#define LSM303AGR_M_SE 1350                   // South-East direction heading - scaled 
#define LSM303AGR_M_S 1800                    // South direction heading - scaled 
#define LSM303AGR_M_SW 2250                   // South-West direction heading - scaled 
#define LSM303AGR_M_W 2700                    // West direction heading - scaled 
#define LSM303AGR_M_NW 3150                   // North-West direction heading - scaled 
#define LSM303AGR_M_GAIN 0.2                  // Magnetometer filter gain 

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
typedef uint8_t LSM303AGR_I2C_ADDR; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief Initialization 
 * 
 * @details 
 * 
 * @param i2c 
 * @param offsets : 
 * @param m_odr 
 * @param m_mode 
 * @param m_off_canc 
 * @param m_lpf 
 * @param m_int_mag_pin 
 * @param m_int_mag 
 */
void lsm303agr_init(
    I2C_TypeDef *i2c, 
    int16_t *offsets, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag); 

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


/**
 * @brief 
 * 
 * @details 
 * 
 * @return int16_t : 
 */
int16_t lsm303agr_m_get_heading(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _LSM303AGR_DRIVER_H_ 
