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

// Device axis index 
typedef enum {
    X_AXIS, 
    Y_AXIS, 
    Z_AXIS, 
    NUM_AXES 
} lsm303agr_axis_t; 


// LSM303AGR driver status 
typedef enum {
    LSM303AGR_OK,            // No problem with the LSM303AGR device 
    LSM303AGR_INVALID_PTR,   // Invalid pointer provided to function 
    LSM303AGR_WHOAMI,        // WHO AM I register doesn't match 
    LSM303AGR_WRITE_FAULT,   // A problem occurred while writing via I2C 
    LSM303AGR_READ_FAULT     // A problem occurred while reading via I2C 
} lsm303agr_status_t; 


// Device setting disable/enable 
typedef enum {
    LSM303AGR_CFG_DISABLE, 
    LSM303AGR_CFG_ENABLE 
} lsm303agr_cfg_t; 


// Magnetometer output data rate 
typedef enum {
    LSM303AGR_M_ODR_10, 
    LSM303AGR_M_ODR_20, 
    LSM303AGR_M_ODR_50, 
    LSM303AGR_M_ODR_100 
} lsm303agr_m_odr_cfg_t; 


// Magnetometer system mode 
typedef enum {
    LSM303AGR_M_MODE_CONT, 
    LSM303AGR_M_MODE_SINGLE, 
    LSM303AGR_M_MODE_IDLE 
} lsm303agr_m_sys_mode_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef uint8_t LSM303AGR_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief Magnetometer initialization 
 * 
 * @details 
 *          
 *          LPF gain must not be zero or else data will not be updated 
 * 
 * @param i2c : 
 * @param offsets : 
 * @param heading_lpf_gain : 
 * @param m_odr : 
 * @param m_mode : 
 * @param m_off_canc : 
 * @param m_lpf : 
 * @param m_int_mag_pin : 
 * @param m_int_mag : 
 * @return LSM303AGR_STATUS : 
 */
LSM303AGR_STATUS lsm303agr_m_init(
    I2C_TypeDef *i2c, 
    const int16_t *offsets, 
    double heading_lpf_gain, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag); 


/**
 * @brief Calibrate the magnetometer heading 
 * 
 * @details Generate heading offset equations. 
 *          
 *          // TODO procedure to get the offsets 
 *          
 *          Set offsets. These are used to correct for errors in the magnetometer readings. This 
 *          is application/device dependent so it is part of the device init and not integrated 
 *          into the driver/library. For a given application/device, these values should not change 
 *          so it is ok to have them hard coded into the application code. 
 *          
 *          Calibration steps: 
 *          1. Set the below values to zero. 
 *          2. Make sure LSM303AGR_TEST_HEADING is enabled and build the project. 
 *          3. Connect the LSM303AGR to the STM32, connect the STM32F4 to your machine, open PuTTy 
 *             so you can see the output and flash the code. 
 *          4. Using a trusted compasss (such as the one on your phone), align it in the North 
 *             direction, then align the long edge of the LSM303AGR (X-axis) with the compass so 
 *             they're both pointing North. 
 *          5. Observe the output of the magnetometer via Putty (Note that depending on the compass 
 *             you use you may have to move it away from the magnetometer once it's aligned or else 
 *             else you could get magnetometer interference - this happens with phone compasses). 
 *             Note the difference between the magnetometer output and the compass heading and 
 *             record it in offsets[0] below. If the magnetometer reading is clockwise of the compass 
 *             heading then the value recorded will be negative. Recorded offsets are scaled by 10. 
 *             Compass and magnetometer heading are from 0-359 degrees. For example, if the compass 
 *             reads 0 degrees (Magnetic North) and the magnetometer output reads +105 (10.5 degrees) 
 *             then the offset recorded is -105. 
 *          6. Repeat steps 4 and 5 for all directions in 45 degree increments (NE, E, SE, etc.) and 
 *             record each subsequent direction in the next 'offsets' element. 
 * 
 * @see lsm303agr_m_get_heading 
 * 
 * @param offsets : error between magnetometer heading and true heading in each direction 
 */
void lsm303agr_m_heading_calibration(const int16_t *offsets); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Get the most recent magnetometer data 
 * 
 * @details 
 * 
 * @return LSM303AGR_STATUS : 
 */
LSM303AGR_STATUS lsm303agr_m_update(void); 


/**
 * @brief Get magnetometer axis data 
 * 
 * @details 
 * 
 * @param m_axis_data 
 */
void lsm303agr_m_get_axis_data(int16_t *m_axis_data); 


/**
 * @brief Get magnetometer applied magnetic field reading for each axis 
 * 
 * @details 
 * 
 * @param m_field_data 
 */
void lsm303agr_m_get_field(int32_t *m_field_data); 


/**
 * @brief Get magnetometer (compass) heading 
 * 
 * @details 
 * 
 * @return int16_t 
 */
int16_t lsm303agr_m_get_heading(void); 

//=======================================================================================

#endif   // _LSM303AGR_DRIVER_H_ 
