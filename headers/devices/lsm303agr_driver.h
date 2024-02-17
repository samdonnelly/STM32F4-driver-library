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

#include "stm32f411xe.h"
#include "i2c_comm.h"

//=======================================================================================


//=======================================================================================
// Macros 

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


// Generic device setting disable/enable 
typedef enum {
    LSM303AGR_CFG_DISABLE, 
    LSM303AGR_CFG_ENABLE 
} lsm303agr_cfg_t; 


// Magnetometer output data rate 
typedef enum {
    LSM303AGR_M_ODR_10,   // 10 Hz 
    LSM303AGR_M_ODR_20,   // 20 Hz 
    LSM303AGR_M_ODR_50,   // 50 Hz 
    LSM303AGR_M_ODR_100   // 100 Hz 
} lsm303agr_m_odr_cfg_t; 


// Magnetometer system mode 
typedef enum {
    LSM303AGR_M_MODE_CONT,     // Continuous - continuous measurements 
    LSM303AGR_M_MODE_SINGLE,   // Single - single measurement then returns to idle mode 
    LSM303AGR_M_MODE_IDLE      // Idle mode 
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
 * @details Configures the magnetometer portion of the driver and device. This function 
 *          must be called before interfacing with the magnetometer. See the datasheet 
 *          for further details on device settings. 
 *          
 *          Errors between the actual and calculated headings can be specified using the 
 *          "offsets" argument. This function calls lsm303agr_m_heading_calibration using 
 *          these offsets. These offsets are used to generate equations that adjust the 
 *          heading to be more accurate. See the calibration function for further details 
 *          on how this works. If no correction is desired then set all offsets to zero. 
 *          
 *          The gain for the low pass filter (LPF) used to calculate the updated heading 
 *          can be set. The range of the gain is: 0 < gain <= 1.0. A gain of 1 will 
 *          negate any LPF affect. Smaller gains will filter more noise but will 
 *          increasingly lag behind the true signal. If the LPF gain is set to 0 then 
 *          the heading will not update when lsm303agr_m_get_heading is called. Gains 
 *          greater than 1 will cause the calculated heading value to oscillate. 
 * 
 * @see lsm303agr_m_heading_calibration 
 * 
 * @param i2c : I2C port to use for communicating with the device 
 * @param offsets : errors between actual and calculated headings 
 * @param heading_lpf_gain : low pass filter gain for calculating the heading 
 * @param m_odr : output data rate 
 * @param m_mode : system mode 
 * @param m_off_canc : offset cancellation enable/disable 
 * @param m_lpf : built in digital low-pass filter enable/disable 
 * @param m_int_mag_pin : INT_MAG_PIN interrupt signal enable/disable 
 * @param m_int_mag : DRDY pin enable/disable 
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
 * @details Generate equations that help correct errors in the calculated heading. The 
 *          magnetometer is prone to errors and interference which can inaccurately 
 *          represent the direction the device is pointing. To correct this, the 
 *          difference between the actual and calculated heading is used to adjust the 
 *          calculated heading to a value that's more accurate. The errors seen by the 
 *          device is application and device dependent which is why calibration is 
 *          needed. 
 *          
 *          These generated equations get using in lsm303agr_m_get_heading where the 
 *          heading is updated. This function can be called independently to update the 
 *          offsets but it also gets called during lsm303agr_m_init. 
 *          
 *          The "offsets" argument stores the errors needed to generate the equations. 
 *          "offsets" is a buffer with LSM303AGR_M_NUM_DIR elements that represent the 
 *          following directions: 0 (360), 45, 90, 135, 180, 225, 270, and 315 degrees. 
 *          These directions/headings are relative to magnetic north (0/360 degrees) 
 *          and increase in the clockwise direction. The X-axis of the device is 
 *          considered the forward direction or the direction the magnetometer is 
 *          pointing. To find the values to use in "offsets", the following procedure 
 *          can be used: 
 *          
 *          1. Generate a sample program that reads magnetometer data, calculates the 
 *             heading and outputs the value of the heading somewhere to be seen in real 
 *             time. Initialize the magnetometer with 0's for all the offsets. 
 *          
 *          2. Position the magnetometer so its forward direction is aligned with the 
 *             forward direction of your system. Note that device errors are system 
 *             dependent so make sure this calibration is being done using your system 
 *             setup. Place an external compass on/in your system, align the compass 
 *             needle with the forward direction of your system (parallel to the 
 *             magnetometer), and make sure the compass reading is visible. Note that 
 *             depending on the compass used, the compass can cause interference with 
 *             the magnetometer (ex. if you're using your phone as a compass) so it's 
 *             best to make sure the compass isn't placed too close. 
 *          
 *          3. Run the program and use the compass to align the forward direction of the 
 *             system with each of the following directions one at a time: 0, 45, 90, 
 *             135, 180, 225, 270, 315 degrees. Note that the calculated heading and 
 *             the errors stored in "offsets" are of units degrees*10. At each direction 
 *             make sure the magnetometer reading is stable, then calculate the offset 
 *             for a given direction by subtrating the magnetometer heading from the 
 *             compass heading (ex. 45 degrees --> offsets[1] = 450 - heading degrees*10). 
 *             The index of "offsets" corresponds to the direction index*45 (ex. 
 *             offsets[0] --> 0 degrees, offsets[7] --> 315 degrees, etc.). 
 *           
 *          4. Once all the offsets have been recored, pass "offsets" with the updated 
 *             values either to this function or lsm303agr_m_init to and start using 
 *             the magnetometer as a compass by calling lsm303agr_m_get_heading. 
 * 
 * @see lsm303agr_m_init 
 * @see lsm303agr_m_get_heading 
 * 
 * @param offsets : error between magnetometer heading and true heading in each direction 
 */
void lsm303agr_m_heading_calibration(const int16_t *offsets); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Read the most recent magnetometer data 
 * 
 * @details Reads the magnetometer axis data from the device and saves it in the driver 
 *          data record. This function must be called to get updated data. 
 * 
 * @return LSM303AGR_STATUS : status of the read operation 
 */
LSM303AGR_STATUS lsm303agr_m_update(void); 


/**
 * @brief Get magnetometer axis data 
 * 
 * @details Saves the last read magnetometer axis data to the buffer. lsm303agr_m_update 
 *          must be called before this function to read the most recent magnetometer 
 *          data or else the data retrieved by this function will not change. Note that 
 *          the buffer must be large enough to store data for each axis. 
 * 
 * @see lsm303agr_m_update 
 * 
 * @param m_axis_data : buffer to store the magnetometer axis data 
 */
void lsm303agr_m_get_axis_data(int16_t *m_axis_data); 


/**
 * @brief Get magnetometer applied magnetic field reading for each axis 
 * 
 * @details Calculates the applied magnetic field (mgauss) for each axis using the 
 *          last read magnetometer data and saves the result in the buffer. This is 
 *          calculated by scaling the device digital output by the magnetometer 
 *          sensitivity (see the datasheet for further details). lsm303agr_m_update 
 *          must be called before this function to read the most recent magnetometer 
 *          data or else the data retrieved by this function will not change. Note that 
 *          the buffer must be large enough to store data for each axis. 
 * 
 * @see lsm303agr_m_update 
 * 
 * @param m_field_data : buffer to store the applied magnetic field data 
 */
void lsm303agr_m_get_field(int32_t *m_field_data); 


/**
 * @brief Get magnetometer (compass) heading 
 * 
 * @details Uses the last read magnetometer data to calculate the heading of the device 
 *          (i.e. the direction the device is pointing). lsm303agr_m_update must be 
 *          called before this function to read the most recent magnetometer data or else 
 *          the heading will not change. 
 *          
 *          The heading calculation works under the assumption that the device has its XY 
 *          plane parallel to the ground (X and Y markings are on the device). The 
 *          forward direction of the device is considered to be the positive X-axis (the 
 *          way the X-axis arrow is pointing on the device). 
 *          
 *          The returned heading is in the range 0-359.9 degrees but the returned value 
 *          has units of degrees*10 (i.e. 0-3599 range). Heading is relative to magnetic 
 *          north (0/360 degrees) and heading value increases in the clockwise direction. 
 *          Part of the calculation involves correcting for errors in device data which 
 *          comes from the equations generated in lsm303agr_m_heading_calibration. 
 * 
 * @see lsm303agr_m_update 
 * @see lsm303agr_m_heading_calibration 
 * 
 * @return int16_t : magnetometer heading 
 */
int16_t lsm303agr_m_get_heading(void); 

//=======================================================================================

#endif   // _LSM303AGR_DRIVER_H_ 
