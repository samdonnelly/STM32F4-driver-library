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
// Macros 

// Device info 
#define MPU6050_7BIT_ADDR 0x68           // 7-bit default address (excluding r/w bit) 
#define MPU6050_FT_MAX_ERROR 14          // Max % change from factory trim acceptable 

// Register control 
#define MPU6050_STBY_STATUS_MASK 0x3F    // Pwr mgmt 2 standby status mask 
#define MPU6050_FSR_MASK 0x18            // Mask for reading gyro and accel full scale range 
#define MPU6050_EXT_SYNC_DISABLE 0       // Disables the FSYNC feature 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief MPU6050 Addresses 
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
 * @brief MPU6050 read and write offset 
 * 
 * @details These provide the necessary offset to the devices I2C address to either read 
 *          or write to from it. Simply add this value to the end of the address when 
 *          reading or writing.  
 * 
 * @see mpu6050_i2c_addr_t
 */
typedef enum {
    MPU6050_W_OFFSET,
    MPU6050_R_OFFSET
} mpu6050_rw_offset_t;


/**
 * @brief MPU6050 DLPF_CFG setpoint
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
 * @brief MPU6050 - GYRO_CONFIG : FS_SEL setpoint 
 * 
 * @details Selects the full scale range used by the gyroscope. This is passed as an 
 *          argument to mpu6050_init to configure the device. Higher ranges cover 
 *          a wider range of angular velocity but have less precision. 
 *          
 *          Each of the gyroscope ranges has a scalar used to convert it's raw 
 *          value into a human readable form. These scalars are listed below and are used 
 *          to divide the raw 16-bit value read from the gyroscope measurement register 
 *          to get the angular acceleration in deg/s. These values are scaled by a factor 
 *          of 10 to eliminate the decimal place. When calculating the actual value the 
 *          values are unscaled. 
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
 * @brief MPU6050 - ACCEL_CONFIG : AFS_SEL setpoint
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
 * @brief MPU6050 - PWR_MGMT_1 : CLKSEL
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for the
 *          selection of the devices clock source. The clock source is selected as 
 *          follows:  
 *              - CLKSEL = 0 : Internal 8MHz oscillator                                
 *              - CLKSEL = 1 : PPL with X-axis gyro reference                          
 *              - CLKSEL = 2 : PPL with Y-axis gyro reference                          
 *              - CLKSEL = 3 : PPL with Z-axis gyro reference                          
 *              - CLKSEL = 4 : PPL with external 32.768kHz reference                   
 *              - CLKSEL = 5 : PPL with external 19.2MHz reference                     
 *              - CLKSEL = 6 : Reserved                                                
 *              - CLKSEL = 7 : Stops the clock and keeps the timing generator on reset 
 *            
 *           It is recommended the internal oscillator is not used as the other options 
 *           are much more stable.  
 */
typedef enum {
    MPU6050_CLKSEL_0,  // Internal 8MHz oscillator
    MPU6050_CLKSEL_1,  // PPL with X-axis gyro reference 
    MPU6050_CLKSEL_2,  // PPL with Y-axis gyro reference 
    MPU6050_CLKSEL_3,  // PPL with Z-axis gyro reference 
    MPU6050_CLKSEL_4,  // PPL with external 32.768kHz reference
    MPU6050_CLKSEL_5,  // PPL with external 19.2MHz reference
    MPU6050_CLKSEL_6,  // Reserved 
    MPU6050_CLKSEL_7   // Stops the clock and keeps the timing generator on reset
} mpu6050_clksel_t;


/**
 * @brief MPU6050 - PWR_MGMT_2 : LP_WAKE_CTRL
 * 
 * @details Used to configure power management register 2 in mpu6050_init. This allows 
 *          for configuring of the frequency of wake-ups in low power mode. In this mode 
 *          the device will power off all functions except for the primary i2c interface
 *          waking up only the accelerometer at fixed intervals to take a single 
 *          measurements. Values of LP_WAKE_CTRL correspond to the following wake-up
 *          frequencies: 
 *              - LP_WAKE_CTRL = 0 : 1.25 Hz  
 *              - LP_WAKE_CTRL = 1 : 5  Hz    
 *              - LP_WAKE_CTRL = 2 : 20 Hz    
 *              - LP_WAKE_CTRL = 3 : 40 Hz    
 *          
 *          Low power mode can be configured using the following steps carried out in 
 *          power management register 1:             
 *              - Set CYCLE to 1                     
 *              - Set SLEEP to 0                     
 *              - Set TEMP_DIS to 1                  
 *              - Set STBY_XG, STBY_YG, STBY_ZG to 1 
 */
typedef enum {
    MPU6050_LP_WAKE_CTRL_0,    // 1.25 Hz wakeup frequency
    MPU6050_LP_WAKE_CTRL_1,    // 5 Hz wakeup frequency
    MPU6050_LP_WAKE_CTRL_2,    // 20 Hz wakeup frequency
    MPU6050_LP_WAKE_CTRL_3     // 40 Hz wakeup frequency
} mpu6050_lp_wake_ctrl_t;


/**
 * @brief MPU6050 - GYRO_CONFIG : XG_ST, YG_ST and ZG_ST setpoint 
 * 
 * @details This is used to enable and disable self-test on the gyroscope. During 
 *          initialization self-test is disabled, but when mpu6050_self_test is called 
 *          self-test is temporarily enabled. 
 * 
 * @see mpu6050_self_test
 */
typedef enum {
    MPU6050_GYRO_ST_DISABLE,
    MPU6050_GYRO_ST_ENABLE
} mpu6050_gyro_self_test_set_t;


/**
 * @brief MPU6050 - ACCEL_CONFIG : XA_ST, YA_SET and ZA_ST setpoint
 * 
 * @details This is used to enable and disable self-test on the accelerometer. During 
 *          initialization self-test is disabled, but when mpu6050_self_test is called 
 *          self-test is temporarily enabled. 
 * 
 * @see mpu6050_self_test 
 */
typedef enum {
    MPU6050_ACCEL_ST_DISABLE,
    MPU6050_ACCEL_ST_ENABLE
} mpu6050_accel_self_test_set_t;


/**
 * @brief MPU6050 - PWR_MGMT_1 : DEVICE_RESET
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for 
 *          a reset to the devices default settings. 
 */
typedef enum {
    MPU6050_RESET_DISABLE,
    MPU6050_RESET_ENABLE
} mpu6050_device_reset_t; 


/**
 * @brief MPU6050 - PWR_MGMT_1 : SLEEP
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


/**
 * @brief MPU6050 - PWR_MGMT_1 : CYCLE 
 * 
 * @details Used to configure power management register 1 in mpu6050_init. If cycle is 
 *          enabled and sleep mode is disabled, the device will wake up from sleep mode 
 *          periodically to take a single sample of data at a frquency dictated by 
 *          LP_WAKE_CTRL. 
 * 
 * @see mpu6050_sleep_mode_t
 * @see mpu6050_lp_wake_ctrl_t
 */
typedef enum {
    MPU6050_CYCLE_SLEEP_DISABLED,
    MPU6050_CYCLE_SLEEP_ENABLED
} mpu6050_cycle_t;


/**
 * @brief MPU6050 - PWR_MGMT_1 : TEMP_DIS
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for 
 *          enabling or disabling of the temperature sensor. 
 */
typedef enum {
    MPU6050_TEMP_SENSOR_ENABLE,
    MPU6050_TEMP_SENSOR_DISABLE
} mpu6050_temp_sensor_t;


/**
 * @brief MPU6050 - INT_PIN_CFG (register 55): LATCH_INT_EN 
 */
typedef enum {
    MPU6050_INT_LATCH_PULSE,   // INT pin emits 50us long pulses 
    MPU6050_INT_LATCH_HIGH     // INT pin held high until interrupt is cleared 
} mpu6050_int_latch_t; 


/**
 * @brief MPU6050 - INT_PIN_CFG (register 55): INT_RD_CLEAR 
 */
typedef enum {
    MPU6050_INT_CLEAR_RD_STAT,   // INT status is only cleared by reading INT_STATUS 
    MPU6050_INT_CLEAR_RD_ANY     // INT status is cleared by any read 
} mpu6050_int_clear_t; 


/**
 * @brief MPU6050 - INT_ENABLE (register 56): DATA_RDY_EN  
 */
typedef enum {
    MPU6050_INT_DATA_RDY_DISABLE,   // Disable the data ready interrupt 
    MPU6050_INT_DATA_RDY_ENABLE     // Enable the data ready interrupt 
} mpu6050_int_data_rdy_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t MPU6050_REG_ADDR;        // Register address 
typedef uint8_t MPU6050_INT_STATUS;      // INT pin status 
typedef uint8_t MPU6050_INIT_STATUS;     // Initialization status 
typedef uint8_t MPU6050_SMPLRT_DIV;      // Sample Rate Divider 
typedef uint8_t MPU6050_ST_RESULT;       // Self-Test Result 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief MPU5060 initialization 
 * 
 * @details Configures device registers and creates a new data record for the device. The 
 *          initialization status is returned. If the init status is not zero then a fault 
 *          occured. The init status follows the first 8-bits of the driver fault code. 
 * 
 * @param device_num : data record address of device 
 * @param i2c : I2C port used by device 
 * @param mpu6050_address : I2C address of device 
 * @param standby_status : 1 byte bitmap specifying the data to put into standby mode 
 * @param dlpf_cfg : digital low pass filter selection 
 * @param smplrt_div : sample rate divider 
 * @param afs_sel : full scale range of accelerometer 
 * @param fs_sel : full scale range of gyroscope 
 */
void mpu6050_init(
    device_number_t device_num, 
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t mpu6050_address,
    uint8_t standby_status, 
    mpu6050_dlpf_cfg_t dlpf_cfg,
    MPU6050_SMPLRT_DIV smplrt_div,
    mpu6050_afs_sel_set_t afs_sel,
    mpu6050_fs_sel_set_t fs_sel);


/**
 * @brief MPU6050 INT pin initialization 
 * 
 * @details Configures an input pin to read the status of the INT pin on the device and 
 *          configures the device to set the INT pin high when data is ready to be read. 
 *          This is separate from the init function because this feature is not always 
 *          needed. The main init function should be called before this function. 
 * 
 * @param device_num : data record address of device 
 * @param gpio : GPIO port to use for INT pin 
 * @param pin : pin number to use for INT pin 
 */
void mpu6050_int_pin_init(
    device_number_t device_num, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin); 


/**
 * @brief MPU6050 calibration 
 * 
 * @details The readings from the device have errors from the true values even if self-test 
 *          is passed. This function takes a single reading of gyroscope values and stores 
 *          the results in the device data record which is used to correct for the error 
 *          in the gyroscope readings. In order for this method to work correctly, the 
 *          device must be kept still, otherwise the error is not correctly measured. 
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_calibrate(device_number_t device_num); 


/**
 * @brief MPU6050 low power mode config 
 * 
 * @details Write the value of "sleep" to the power management 1 register to either set 
 *          or disable sleep (low power) mode. When set to low power mode the device will 
 *          not update sensor data and therefore consume less power. 
 * 
 * @see mpu6050_sleep_mode_t
 * 
 * @param device_num : data record address of device 
 * @param sleep : low power mode configuration 
 */
void mpu6050_low_pwr_config(
    device_number_t device_num, 
    mpu6050_sleep_mode_t sleep); 

//=======================================================================================


//=======================================================================================
// Read and get data 

/**
 * @brief Read the most recent IMU data 
 * 
 * @details Performs the same data record update as the accelerometer, gyroscope and temp 
 *          sensor read functions combined in a burst read. This allows for keeping 
 *          hold of the I2C bus and ensures all the data read is from the same instance in time. 
 *          This is useful for when all the data needs to be read. 
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_update(device_number_t device_num); 


/**
 * @brief Get accelerometer axis data 
 * 
 * @details Stores the most recently read unformatted acceleration values in the buffers 
 *          passed as arguments. Note that the data is updated using one of the read 
 *          functions, this function only returns the read value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : number of device data record to access 
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
 * @param device_num : number of device data record to access 
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
 * @param device_num : number of device data record to access 
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
 * @param device_num : number of device data record to access 
 * @param gyro_axis_rate : buffer to store the angular velocity (rad/s) 
 */
void mpu6050_get_gyro_axis_rate(
    device_number_t device_num, 
    float *gyro_axis_rate); 


/**
 * @brief MPU6050 temperature sensor raw value 
 * 
 * @details Returns the unformatted temperature sensor data. Note that the data is 
 *          updated using one of the read functions, this function only returns the read 
 *          value. 
 * 
 * @see mpu6050_update 
 * 
 * @param device_num : number of device data record to access 
 * @return int16_t : unformatted temperature reading 
 */
int16_t mpu6050_get_temp_raw(device_number_t device_num); 


/**
 * @brief MPU6050 temperature sensor calculation
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
 * @param device_num : number of device data record to access 
 * @return float : true temperature value (degC) 
 */
float mpu6050_get_temp(device_number_t device_num);

//=======================================================================================


//=======================================================================================
// Status 

/**
 * @brief MPU6050 clear device driver fault flag 
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_clear_status(device_number_t device_num); 


/**
 * @brief MPU6050 get device driver fault code 
 * 
 * @details Status info / fault code: 
 *            --> bit 0: i2c status (see i2c_status_t) 
 *            --> bit 1: init status (WHO_AM_I) 
 *            --> bits 2-7: self test results 
 * 
 * @param device_num : data record address of device 
 * @return uint8_t : driver status code for a given device number 
 */
uint8_t mpu6050_get_status(device_number_t device_num); 


/**
 * @brief MPU6050 INT pin status 
 * 
 * @details Returns the INT pin status of the device. This is done by reading the GPIO 
 *          input pin configured during INT pin initialization. If the pin is high then 
 *          the INT pin is set indicating there is data available for reading. 
 *          
 *          NOTE: this function is only useful if the interrupt pin has been configured. 
 * 
 * @see mpu6050_int_pin_init
 * 
 * @param device_num : data record address of device 
 * @return MPU6050_INT_STATUS : INT pin status 
 */
MPU6050_INT_STATUS mpu6050_int_status(device_number_t device_num); 

//=======================================================================================


//=======================================================================================
// Self-test functions 

/**
 * @brief MPU6050 self-test
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
 * @param device_num : data record address of device 
 * @return MPU6050_ST_RESULT : self-test results 
 */
MPU6050_ST_RESULT mpu6050_self_test(device_number_t device_num);

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _MPU6050_DRIVER_H_
