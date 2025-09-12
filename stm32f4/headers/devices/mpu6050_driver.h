/**
 * @file mpu6050_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 IMU driver interface 
 * 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MPU6050_DRIVER_H_
#define _MPU6050_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "i2c_comm.h"
#include "gpio_driver.h"
#include "linked_list_driver.h"

//=======================================================================================


//=======================================================================================
// Enums 

// MPU-6050 driver status 
typedef enum {
    MPU6050_OK          = 0x00000000,   // No problem with the MPU-6050 driver 
    MPU6050_INVALID_PTR = 0x00000001,   // Invalid pointer provided to the driver 
    MPU6050_WHOAMI      = 0x00000002,   // WHO AM I register doesn't match 
    MPU6050_WRITE_FAULT = 0x00000004,   // A problem occurred while writing via I2C 
    MPU6050_READ_FAULT  = 0x00000008,   // A problem occurred while reading via I2C 
    MPU6050_ST_FAULT    = 0x00000010    // A problem occured during self test 
} mpu6050_status_t; 


/**
 * @brief MPU-6050 addresses 
 * 
 * @details The AD0 pin on the accelerometer can be set externally as either 0 or 1 to 
 *          allow for two different MPU's to be on the same I2C bus. The default address
 *          of the MPU6050 is 0x68 (pin AD0 = 0). 
 *          
 *          The 7-bit address of a slave I2C device is held in the upper 7-bits of the 
 *          byte and the LSB is left to indicate read/receive (1) or write/transmit (0). 
 *          This makes the addresses of the MPU6050 (0x68) shifted to the left by 1. The 
 *          resulting addresses become:                    
 *             - 0xD0 (write) and 0xD1 (read) if AD0 is 0  
 *             - 0xD2 (write) and 0xD3 (read) if AD0 is 1  
 *          
 *          Note that only the write addresses are shown below. mpu6050_rw_offset_t is 
 *          used to provide the needed offset when calling the address. 
 * 
 * @see mpu6050_rw_offset_t
 */
typedef enum {
    MPU6050_ADDR_1 = 0xD0,
    MPU6050_ADDR_2 = 0xD2
} mpu6050_i2c_addr_t;


/**
 * @brief DLPF_CFG setpoint
 * 
 * @details A digital low pass filter (DLPF) can be specified in the CONFIG register 
 *          which is used to filter accelerometer and gyroscope data. The value of the 
 *          DLPF dictates the gyroscope output frquency as descibed below: 
 *             - DLPF = 0 or 7 -> gyroscope output rate = 8 kHz 
 *             - DLPF = 1 - 6  -> gyroscope output rate = 1 kHz  
 *          
 *          A summary of cutoff frquencies are outlined below. Note that lower cutoff
 *          frequencies come with longer delay times.     
 *             - DLPF = 0 : accel = 260 Hz, gyro = 256 Hz  
 *             - DLPF = 1 : accel = 184 Hz, gyro = 188 Hz  
 *             - DLPF = 2 : accel = 94  Hz, gyro = 98  Hz  
 *             - DLPF = 3 : accel = 44  Hz, gyro = 42  Hz  
 *             - DLPF = 4 : accel = 21  Hz, gyro = 20  Hz  
 *             - DLPF = 5 : accel = 10  Hz, gyro = 10  Hz  
 *             - DLPF = 6 : accel = 5   Hz, gyro = 5   Hz  
 *             - DLPF = 7 : RESERVED   
 *          
 *          Refer to the Register Map datasheet for the MPU6050 for more information on 
 *          DLPF settings.
 */
typedef enum {
    MPU6050_DLPF_CFG_0,  // accel = 260 Hz, gyro = 256 Hz
    MPU6050_DLPF_CFG_1,  // accel = 184 Hz, gyro = 188 Hz
    MPU6050_DLPF_CFG_2,  // accel = 94  Hz, gyro = 98  Hz
    MPU6050_DLPF_CFG_3,  // accel = 44  Hz, gyro = 42  Hz
    MPU6050_DLPF_CFG_4,  // accel = 21  Hz, gyro = 20  Hz
    MPU6050_DLPF_CFG_5,  // accel = 10  Hz, gyro = 10  Hz
    MPU6050_DLPF_CFG_6,  // accel = 5   Hz, gyro = 5   Hz
    MPU6050_DLPF_CFG_7   // RESERVED
} mpu6050_dlpf_cfg_t;


/**
 * @brief GYRO_CONFIG : FS_SEL setpoint 
 * 
 * @details Selects the full scale range used by the gyroscope. This is passed as an 
 *          argument to mpu6050_init to configure the device. Higher ranges cover 
 *          a wider range of angular velocity but have less precision. 
 *          
 *          Each of the gyroscope ranges has a scalar used to convert it's raw value into 
 *          a human readable form. These scalars are listed below and are used to divide 
 *          the raw 16-bit value read from the gyroscope measurement register to get the 
 *          angular rate in deg/s. These values are scaled by a factor of 10 to eliminate 
 *          the decimal place. When calculating the actual value the values are unscaled. 
 *          - GYRO_SCALE_FS_SEL_2000 = 164, ---> 16.4 * 10 
 *          - GYRO_SCALE_FS_SEL_1000 = 328, ---> 32.8 * 10 
 *          - GYRO_SCALE_FS_SEL_500  = 655, ---> 65.5 * 10 
 *          - GYRO_SCALE_FS_SEL_250  = 1310 ---> 131.0 * 10 
 */
typedef enum {
    MPU6050_FS_SEL_250,   // +/- 250  deg/s ---> Scalar = 1310 
    MPU6050_FS_SEL_500,   // +/- 500  deg/s ---> Scalar = 655 
    MPU6050_FS_SEL_1000,  // +/- 1000 deg/s ---> Scalar = 328 
    MPU6050_FS_SEL_2000   // +/- 2000 deg/s ---> Scalar = 164 
} mpu6050_fs_sel_set_t;


/**
 * @brief ACCEL_CONFIG : AFS_SEL setpoint
 * 
 * @details Selects the full scale range used by the acclerometer. This is passed as an 
 *          argument to mpu6050_init to configure the device. Higher ranges cover 
 *          a wider range of accelerations but have less precision. 
 *          
 *          Each of the accelerometer ranges has a scalar used to convert it's raw 
 *          value into a human readable form. These scalars are listed below and are used 
 *          to divide the raw 16-bit value read from the accelerometer measurement register 
 *          to get the linear acceleration in g's: 
 *          - ACCEL_SCALE_AFS_SEL_16 = 2048,
 *          - ACCEL_SCALE_AFS_SEL_8  = 4096,
 *          - ACCEL_SCALE_AFS_SEL_4  = 8192,
 *          - ACCEL_SCALE_AFS_SEL_2  = 16384
 */
typedef enum {
    MPU6050_AFS_SEL_2,   // +/- 2g ---> Scalar = 16384 
    MPU6050_AFS_SEL_4,   // +/- 4g ---> Scalar = 8192 
    MPU6050_AFS_SEL_8,   // +/- 8g ---> Scalar = 4096 
    MPU6050_AFS_SEL_16   // +/- 16g --> Scalar = 2048 
} mpu6050_afs_sel_set_t;


/**
 * @brief PWR_MGMT_1 : SLEEP
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for 
 *          sleep mode to be enabled or disabled. Sleep mode is a low power mode where 
 *          data is not recorded. When mpu6050_cycle_t is set to 1 and sleep mode is 
 *          disabled the device will cycle between sleep mode and waking up to take a 
 *          single sample of data at a frquency dictated by LP_WAKE_CTRL. 
 * 
 * @see mpu6050_cycle_t
 * @see mpu6050_lp_wake_ctrl_t
 */
typedef enum {
    MPU6050_SLEEP_MODE_DISABLE,
    MPU6050_SLEEP_MODE_ENABLE
} mpu6050_sleep_mode_t;

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint32_t MPU6050_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief MPU-5060 initialization 
 * 
 * @details Configures device registers and creates a new data record for the device. The 
 *          initialization status is returned. If the init status is not zero then a fault 
 *          occured. The init status follows the first 8-bits of the driver fault code. 
 * 
 * @param device_num : ID used to get device info 
 * @param i2c : I2C port used by device 
 * @param mpu6050_address : I2C address of device 
 * @param standby_status : 1 byte bitmap specifying the data to put into standby mode 
 * @param dlpf_cfg : digital low pass filter selection 
 * @param smplrt_div : sample rate divider 
 * @param afs_sel : full scale range of accelerometer 
 * @param fs_sel : full scale range of gyroscope 
 * @return MPU6050_STATUS : initialization status 
 */
MPU6050_STATUS mpu6050_init(
    device_number_t device_num, 
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t mpu6050_address,
    uint8_t standby_status, 
    mpu6050_dlpf_cfg_t dlpf_cfg,
    uint8_t smplrt_div,
    mpu6050_afs_sel_set_t afs_sel,
    mpu6050_fs_sel_set_t fs_sel);


/**
 * @brief INT pin initialization 
 * 
 * @details Configures an input pin to read the status of the INT pin on the device and 
 *          configures the device to set the INT pin high when data is ready to be read. 
 *          This is separate from the init function because this feature is not always 
 *          needed. The main init function should be called before this function. 
 * 
 * @param device_num : ID used to get device info 
 * @param gpio : GPIO port to use for INT pin 
 * @param pin : pin number to use for INT pin 
 * @return MPU6050_STATUS : status of the initialization 
 */
MPU6050_STATUS mpu6050_int_pin_init(
    device_number_t device_num, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin); 


/**
 * @brief Low power mode config 
 * 
 * @details Write the value of "sleep" to the power management 1 register to either set 
 *          or disable sleep (low power) mode. When set to low power mode the device will 
 *          not update sensor data and therefore consume less power. 
 * 
 * @see mpu6050_sleep_mode_t
 * 
 * @param device_num : ID used to get device info 
 * @param sleep : low power mode configuration 
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_low_pwr_config(
    device_number_t device_num, 
    mpu6050_sleep_mode_t sleep);


/**
 * @brief Correct offsets in the accelerometer and gyroscope readings 
 * 
 * @param device_num : ID used to get device info 
 * @param accel_offset : buffer of offsets used to zero each accelerometer axis 
 * @param gyro_offset : buffer of offsets used to zero each gyroscope axis 
 * @return MPU6050_STATUS : status of the setting operation 
 */
MPU6050_STATUS mpu6050_set_offsets(
    device_number_t device_num, 
    const int16_t *accel_offset, 
    const int16_t *gyro_offset); 

//=======================================================================================


//=======================================================================================
// Update and get data 

/**
 * @brief Read the most recent IMU data 
 * 
 * @details Performs the same data record update as the accelerometer, gyroscope and temp 
 *          sensor read functions combined in a burst read. This allows for keeping 
 *          hold of the I2C bus and ensures all the data read is from the same instance in time. 
 *          This is useful for when all the data needs to be read. 
 * 
 * @param device_num : ID used to get device info 
 * @return MPU6050_STATUS : status of the update 
 */
MPU6050_STATUS mpu6050_update(device_number_t device_num); 


/**
 * @brief Get accelerometer axis data 
 * 
 * @details Stores the most recently read unformatted acceleration values in the buffers 
 *          passed as arguments. Note that the data is updated using one of the read 
 *          functions, this function only returns the read value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : ID used to get device info 
 * @param accel_axis : buffer to store the accelerometer axis data (raw) 
 */
void mpu6050_get_accel_axis(
    device_number_t device_num, 
    int16_t *accel_axis); 


/**
 * @brief Get accelerometer axis g's 
 * 
 * @details Formats and stores the most recently read acceleration values in the buffers 
 *          passed as arguments. Note that the data is updated using one of the read 
 *          functions, this function only returns the read value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : ID used to get device info 
 * @param accel_axis_gs : buffer to store the accelerometer axis data (g's) 
 */
void mpu6050_get_accel_axis_gs(
    device_number_t device_num, 
    float *accel_axis_gs); 


/**
 * @brief Get the gyroscope axis data 
 * 
 * @details Stores the most recently read unformatted angular velocity values in the 
 *          buffers passed as arguments. Note that the data is updated using one of the 
 *          read functions, this function only returns the read value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : ID used to get device info 
 * @param gyro_axis : buffer to store the gyroscope axis data (raw) 
 */
void mpu6050_get_gyro_axis(
    device_number_t device_num, 
    int16_t *gyro_axis); 


/**
 * @brief Get gyroscope axis angular velocity 
 * 
 * @details Formats and stores the most recently read angular velocity values in the 
 *          buffers passed as arguments. Note that the data is updated using one of the 
 *          read functions, this function only returns the read value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : ID used to get device info 
 * @param gyro_axis_rate : buffer to store the angular velocity (rad/s) 
 */
void mpu6050_get_gyro_axis_rate(
    device_number_t device_num, 
    float *gyro_axis_rate); 


/**
 * @brief Get temperature sensor raw value 
 * 
 * @details Returns the unformatted temperature sensor data. Note that the data is 
 *          updated using one of the read functions, this function only returns the read 
 *          value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : ID used to get device info 
 * @return int16_t : unformatted temperature reading 
 */
int16_t mpu6050_get_temp_raw(device_number_t device_num); 


/**
 * @brief Get temperature sensor calculation
 * 
 * @details Calculates and returns the true temperature reading in degC using the raw 
 *          temperature sensor data. This value is calculated using the following equation 
 *          from the register map documentation: 
 *          
 *          Temperature (degC) = (16-bit register value) / 340 + 36.53 
 *          
 *          Note that the data is updated using one of the read functions, this function 
 *          only returns the read value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : ID used to get device info 
 * @return float : true temperature value (degC) 
 */
float mpu6050_get_temp(device_number_t device_num);

//=======================================================================================


//=======================================================================================
// Status 

/**
 * @brief INT pin status 
 * 
 * @details Returns the INT pin status of the device. This is done by reading the GPIO 
 *          input pin configured during INT pin initialization. If the pin is high then 
 *          the INT pin is set indicating there is data available for reading. 
 *          
 *          NOTE: this function is only useful if the interrupt pin has been configured. 
 * 
 * @see mpu6050_int_pin_init
 * 
 * @param device_num : ID used to get device info 
 * @return uint8_t : INT pin status 
 */
uint8_t mpu6050_int_status(device_number_t device_num); 

//=======================================================================================


//=======================================================================================
// Self-test functions 

/**
 * @brief Self-test
 * 
 * @details This functions runs a self-test on the device to see it has drifted from the 
 *          factory calibration. When self-test is activated, the on-board electronics
 *          will actuate the appropriate sensor and produce a change in the sensor
 *          output. The self-test response is defined as:  
 *          
 *          Self-test response = (sensor output with self-test enabled) - 
 *                               (sensor output with self-test disabled)  
 *          
 *          To pass the self-test the sensor must be within 14% of it's factory 
 *          calibration. If a self-test is failed then the sensor readings cannot be 
 *          considered accurate. The function will return a byte that indicates the self-
 *          test results of each accelerometer and gyroscope axis where a 0 is a pass and 
 *          a 1 is a fail. The return value breakdown is as follows: 
 *          
 *          - Bit 5: gyroscope z-axis 
 *          - Bit 4: gyroscope y-axis 
 *          - Bit 3: gyroscope x-axis 
 *          - Bit 2: accelerometer z-axis 
 *          - Bit 1: accelerometer y-axis 
 *          - Bit 0: accelerometer x-axis 
 * 
 * @param device_num : ID used to get device info 
 * @param st_result : result of the self test 
 * @return MPU6050_STATUS : self-test results 
 */
MPU6050_STATUS mpu6050_self_test(
    device_number_t device_num, 
    uint8_t *st_result);

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _MPU6050_DRIVER_H_
