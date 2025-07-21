/**
 * @file lsm303agr_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR IMU driver interface 
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

#include "stm32f411xe.h" 
#include "tools.h" 
#include "i2c_comm.h" 

//=======================================================================================


//=======================================================================================
// Enums 

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
 *          for further details on device settings. The return status of the function 
 *          can be used to see if reading and writing to the device was successful. 
 * 
 * @param i2c : I2C port to use for communicating with the device 
 * @param m_odr : output data rate 
 * @param m_mode : system mode 
 * @param m_off_canc : offset cancellation enable/disable 
 * @param m_lpf : built in digital low-pass filter enable/disable 
 * @param m_int_mag_pin : INT_MAG_PIN interrupt signal enable/disable 
 * @param m_int_mag : DRDY pin enable/disable 
 * @return LSM303AGR_STATUS : device read and write status 
 */
LSM303AGR_STATUS lsm303agr_m_init(
    I2C_TypeDef *i2c, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag); 


/**
 * @brief Set the hard-iron offset registers 
 * 
 * @details Writes the provided hard-iron offsers to the device. If successfully written, 
 *          these offsets will automatically be applied to the device axis readings and 
 *          will be reflected when getting the axis readings from the driver. If using 
 *          this method for correcting the axis data, then it's recommended to not use 
 *          the calibration value setter below as these two functions do not account for 
 *          what the other does. If this correction method is not desired, then this 
 *          function can simply not be called when setting up the device. 
 *          
 *          One method to obtain these hard-iron offsets is through the MotionCal 
 *          software. The process is described in further detail in the calibration 
 *          setter function description but the offsets obtained can simply be rounded 
 *          then cast from floats to integers. These offsets are integers because the 
 *          registers that will hold them are in int16_t format. 
 *          
 *          Note that it's the users responsibility to provide a valid buffer of offsets 
 *          to be written. A buffer that is NULL or too small will not update all the 
 *          axis offsets. 
 * 
 * @see lsm303agr_m_get_axis 
 * @see lsm303agr_m_calibration_set 
 * 
 * @param offset_reg : hard-iron offsets to update the registers with 
 * @return LSM303AGR_STATUS : status of the register write 
 */
LSM303AGR_STATUS lsm303agr_m_offset_reg_set(const int16_t *offset_reg); 


/**
 * @brief Set the hard and soft-iron calibration values 
 * 
 * @details Set the provided hard-iron offsets and soft-iron correction values within the 
 *          driver. These values will be used to correct the axis data when getting the 
 *          calibrated axis values. If using this method for correcting the axis data, 
 *          it's recommended to not use the hard-iron offset register setter above as 
 *          these two functions do not account for what the other does. If this 
 *          correction method is not desired, then this function can simply not be called 
 *          when setting up the device. It is possible to set only the hard-iron offsets 
 *          or the soft-iron correction values and not the other. 
 *          
 *          The provided correction values are made up of hard-iron (hi) offsets and 
 *          soft-iron values that make up a soft-iron (si) correction matrix. The soft-
 *          iron matrix is made up of diagonal (sid) values and off-diagonal (sio) 
 *          values. The calibration math is as follows: 
 *          
 *          [ sid.x  sio.x  sio.y ]   | (x-axis_data) - hi.x | 
 *          [ sio.x  sid.y  sio.z ] * | (y-axis_data) - hi.y | 
 *          [ sio.y  sio.z  sid.z ]   | (z-axis_data) - hi.z | 
 *          
 *          A common way to obtain these calibration values is through the MotionCal 
 *          software. There are videos online showing how to use it but some key details 
 *          not mentioned are: 
 *          - MotionCal uses a 115200 baudrate to read serial data. 
 *          - MotionCal looks for magnetometer axis data as an integer with units of 
 *            microteslas*10 (uT*10) which is the same as milligauss (mG) which happens 
 *            to be the units this device outputs axis data in. This means no output 
 *            data modification is needed. 
 *          - Hard-iron offsets and soft-iron scalars are determined and displayed as 
 *            floats in MotionCal despite the provided data being integers. The hard-
 *            iron float values have the units of microteslas (uT) so to make them 
 *            compatible with this devices data output, they must be multiplied by 10 
 *            once the process is done which gives them units of milligauss (mG). 
 *          
 *          Note that soft-iron diagonal correction values cannot be zero or else axis 
 *          data will always come out as zero. Also note that it's the users 
 *          responsibility to provide valid buffers for these values. Buffers that are 
 *          NULL or too small will not update all the values. 
 * 
 * @see lsm303agr_m_get_calibrated_axis 
 * 
 * @param hi_offsets : hard-iron offsets to set (milligauss) 
 * @param sid_values : soft-iron diagonal values to set (milligauss) 
 * @param sio_values : soft-iron off-diagonal values to set (milligauss) 
 */
void lsm303agr_m_calibration_set(
    const float *hi_offsets, 
    const float *sid_values, 
    const float *sio_values); 

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
 * @details Copies the last read magnetometer axis data to the provided buffer. If the 
 *          hard-iron offset registers have been set then those will automatically be 
 *          applied to the axis data by the device. If not, then the raw axis data will 
 *          be copied. 
 *          
 *          Note that it's the users responsibility to provide a buffer large enough to 
 *          store the axis data (3 axes x 2 bytes per axis == buffer of size 3 (6 total 
 *          bytes). Also lsm303agr_m_update must be called in order to read/update 
 *          magnetometer data, this function only retrieves the already read data. 
 * 
 * @see lsm303agr_m_offset_reg_set 
 * @see lsm303agr_m_update 
 * 
 * @param m_axis_buff : buffer to store the magnetometer axis data 
 */
void lsm303agr_m_get_axis(int16_t *m_axis_buff); 


/**
 * @brief Get calibrated magnetometer axis data 
 * 
 * @details Takes the last read magnetometer axis data, applies the hard and soft-iron 
 *          calibration values set by the user and copies the result to the provided 
 *          buffer. This process does not account for the hard-iron offsets that can be 
 *          set in the device registers which, when set, get automatically applied to 
 *          the axis data read from the device. It's advised to do only one of the 
 *          following: 
 *           - Manually set these calibration values, don't set the hard-iron offset
 *             registers and use this function OR 
 *           - Set the hard-iron offset registers and avoid using this function. 
 *          
 *          Note that it's the users responsibility to provide a buffer large enough to 
 *          store the axis data (3 axes x 2 bytes per axis == buffer of size 3 (6 total 
 *          bytes). Also lsm303agr_m_update must be called in order to read/update 
 *          magnetometer data, this function only retrieves the already read data. 
 * 
 * @see lsm303agr_m_calibration_set 
 * @see lsm303agr_m_update 
 * 
 * @param m_axis_buff : buffer to store the calibrated magnetometer axis data 
 */
void lsm303agr_m_get_calibrated_axis(int16_t *m_axis_buff); 


/**
 * @brief Get magnetometer (compass) heading 
 * 
 * @details Uses the last read magnetometer data to calculate the magnetic heading of the 
 *          device (i.e. the direction the device is pointing relative to the Earth's 
 *          magnetic North pole). lsm303agr_m_update must be called before this function 
 *          to read the most recent magnetometer data or else the heading will not change. 
 *          
 *          The heading calculation works under the assumption that the device has its XY 
 *          plane parallel to the ground (X and Y markings are on the device). The 
 *          forward direction of the device is considered to be the positive X-axis (the 
 *          way the X-axis arrow is pointing on the device). 
 *          
 *          The returned heading is in the range 0-359.9 degrees but the returned value 
 *          has units of degrees*10 (i.e. 0-3599 range). Heading is relative to magnetic 
 *          north (0/360 degrees) and heading value increases in the clockwise direction. 
 * 
 * @see lsm303agr_m_update 
 * @see lsm303agr_m_heading_calibration 
 * @see lsm303agr_m_offset_reg_set 
 * 
 * @return int16_t : magnetometer heading (degrees*10) 
 */
int16_t lsm303agr_m_get_heading(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _LSM303AGR_DRIVER_H_ 
