/**
 * @file lsm303agr_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR 6-axis magnetometer and accelerometer driver interface 
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

//=======================================================================================


//=======================================================================================
// Macros 

// Calculation 
#define LSM303AGR_M_NUM_DIR 8   // Number of directions used for error correction 

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
    const int16_t *offsets, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag); 


//==================================================
// Dev 

/**
 * @brief Magnetometer initialization 
 * 
 * @details 
 * 
 * @param i2c : 
 * @param offsets : 
 * @param heading_lpf_gain : 
 */
void lsm303agr_m_init(
    I2C_TypeDef *i2c, 
    const int16_t *offsets, 
    double heading_lpf_gain); 


/**
 * @brief Calibrate the magnetometer heading 
 * 
 * @details Generate heading offset equations. 
 *          
 *          // TODO procedure to get the offsets 
 * 
 * @see lsm303agr_m_get_heading 
 * 
 * @param offsets : error between magnetometer heading and true heading in each direction 
 */
void lsm303agr_m_heading_calibration(const int16_t *offsets); 

//==================================================

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


//==================================================
// Dev 

/**
 * @brief Get the most recent magnetometer data 
 * 
 * @details 
 */
void lsm303agr_m_update(void); 


/**
 * @brief Get magnetometer axis data 
 * 
 * @details 
 * 
 * @param m_axis_data 
 */
void lsm303agr_m_get_axis_data(int16_t *m_axis_data); 


/**
 * @brief Get magnetometer (compass) heading 
 * 
 * @details 
 * 
 * @return int16_t 
 */
int16_t lsm303agr_m_get_heading_dev(void); 

//==================================================

//=======================================================================================


//=======================================================================================
// Status 

/**
 * @brief LSM303AGR clear device driver fault flag 
 * 
 * @details 
 */
void lsm303agr_clear_status(void); 


/**
 * @brief LSM303AGR get device driver fault code 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t lsm303agr_get_status(void); 

//=======================================================================================

#endif   // _LSM303AGR_DRIVER_H_ 
