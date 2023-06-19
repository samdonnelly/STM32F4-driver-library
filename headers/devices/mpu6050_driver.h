/**
 * @file mpu6050_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver for the MPU6050 accelerometer
 * 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MPU6050_DRIVER_H_
#define _MPU6050_DRIVER_H_

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
#define MPU6050_NUM_AXIS 3               // Number of acclerometer axes 

// Register control 
#define MPU6050_STBY_STATUS_MASK 0x3F    // Pwr mgmt 2 standby status mask 
#define MPU6050_FSR_MASK 0x18            // Mask for reading gyro and accel full scale range 
#define MPU6050_EXT_SYNC_DISABLE 0       // Disables the FSYNC feature 

// Registers 
#define MPU6050_SELF_TEST    0x0D        // Register 13  - Self-test 
#define MPU6050_SMPRT_DIV    0x19        // Register 25  - Sample Rate Divider 
#define MPU6050_CONFIG       0x1A        // Register 26  - Configuration 
#define MPU6050_GYRO_CONFIG  0x1B        // Register 27  - Gyroscope configuration 
#define MPU6050_ACCEL_CONFIG 0x1C        // Register 28  - Accelerometer configuration 
#define MPU6050_INT_CONFIG   0x37        // Register 55  - Interrupt configuration 
#define MPU6050_INT_ENABLE   0x38        // Register 56  - Interrupt enable 
#define MPU6050_ACCEL_XOUT_H 0x3B        // Register 59  - Accelerometer x-axis high byte 
#define MPU6050_TEMP_OUT_H   0x41        // Register 65  - Temperature high byte 
#define MPU6050_GYRO_XOUT_H  0x43        // Register 67  - Gyroscope x-axis high byte 
#define MPU6050_PWR_MGMT_1   0x6B        // Register 107 - Power management 1 
#define MPU6050_PWR_MGMT_2   0x6C        // Register 108 - Power management 2 
#define MPU6050_WHO_AM_I     0x75        // Register 117 - Who Am I 

// Temperature sensor 
#define MPU6050_TEMP_SCALAR 100          // User defined temp scalar to eliminate decimals 
#define MPU6050_TEMP_SENSIT 340          // Sensitivity (LSB/degC) - MPU6050 defined scalar
#define MPU6050_TEMP_OFFSET 3653         // Temperature offset scaled by MPU6050_TEMP_SCALAR

// Accelerometer 
#define MPU6050_AFS_SEL_MAX 16384        // Max accelerometer calculation scalar 
#define MPU6050_ACCEL_ST_FT_C1 142       // Accelerometer factory trim calc constant 1 
#define MPU6050_ACCEL_ST_FT_C2 6056      // Accelerometer factory trim calc constant 2 
#define MPU6050_ACCEL_ST_FT_C4 13452     // Accelerometer factory trim calc constant 3 
#define MPU6050_ACCEL_ST_FT_C3 45752     // Accelerometer factory trim calc constant 4 

// Gyroscope 
#define MPU6050_FS_SEL_MAX 1310          // Max gyroscopic calculation scalar 
#define MPU6050_FS_CORRECTION 0x02       // Gyroscope calculation correction mask 
#define MPU6050_GYRO_SCALAR 10           // Unscales scaled mpu6050_gyro_scalars_t values 
#define MPU6050_GYRO_ST_FT_C1 1001       // Gyroscope factory trim calc constant 1 
#define MPU6050_GYRO_ST_FT_C3 15056      // Gyroscope factory trim calc constant 2 
#define MPU6050_GYRO_ST_FT_C2 19244      // Gyroscope factory trim calc constant 3 
#define MPU6050_GYRO_ST_FT_C4 31125      // Gyroscope factory trim calc constant 4 

// Self-Test 
#define MPU6050_ST_MASK_ZA_TEST_LO 0x03  // Mask to parse self-test y-axis accelerometer data 
#define MPU6050_ST_MASK_YA_TEST_LO 0x0C  // Mask to parse self-test z-axis accelerometer data 
#define MPU6050_ST_MASK_X_TEST     0x1F  // Mask to parse self-test gyroscope data 
#define MPU6050_ST_MASK_XA_TEST_LO 0x30  // Mask to parse self-test x-axis accelerometer data 
#define MPU6050_ST_MASK_A_TEST_HI  0xE0  // Mask to parse self-test x, y and z axis accel data 
#define MPU6050_STR_SHIFT_ACCEL    0x01  // Bit shift for accel self-test results 
#define MPU6050_STR_SHIFT_GYRO     0x08  // Bit shift for gyro self-test results 

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

//=======================================================================================


//=======================================================================================
// Configuration functions 

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
void mpu6050_calibrate(
    device_number_t device_num); 


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
// Register Function

/**
 * @brief MPU6050 Accelerometer Measurements (ACCEL_OUT) registers read
 * 
 * @details Register number: 59-64 
 *          Register size: 6 bytes 
 *          
 *          Register data: 
 *          - ACCEL_XOUT: x-axis acceleration - 16-bit signed value (reg 59-60) 
 *          - ACCEL_YOUT: y-axis acceleration - 16-bit signed value (reg 61-62) 
 *          - ACCEL_ZOUT: z-axis acceleration - 16-bit signed value (reg 63-64) 
 *          
 *          These registers store the most recent (unformatted) accelerometer measurements. 
 *          These values are written to the registers at the Sample Rate but are updated 
 *          within the device at a frequency of 1 kHz. This function must be called to 
 *          so the new data gets updated to the device data record. After updating the 
 *          data, the raw or formatted values can be read from the getters. 
 *          
 *          When the serial interface (i2c interface) is active, the values in the 
 *          registers are held constant so that you can read all values (burst read)
 *          at one instance in time. When the serial interface is idle then these 
 *          registers will go back to being written to at the Sample Rate. 
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_accel_read(
    device_number_t device_num);


/**
 * @brief MPU6050 Gyroscope Measurements (GYRO_OUT) registers read
 * 
 * @details Register number: 67-72 
 *          Register size: 6 bytes 
 *          
 *          Register data: 
 *          - GYRO_XOUT: x-axis angular velocity - 16-bit signed value (reg 67-68) 
 *          - GYRO_YOUT: y-axis angular velocity - 16-bit signed value (reg 69-70) 
 *          - GYRO_ZOUT: z-axis angular velocity - 16-bit signed value (reg 71-72) 
 *          
 *          These registers store the most recent (unformatted) gyroscope measurements. 
 *          These values are written to the registers at the Sample Rate. This function 
 *          must be called to so the new data gets updated to the device data record. 
 *          After updating the data, the raw or formatted values can be read from the 
 *          getters. 
 *          
 *          When the serial interface (i2c interface) is active, the values in the 
 *          registers are held constant so that you can read all values (burst read)
 *          at one instance in time. When the serial interface is idle then these 
 *          registers will go back to being written to at the Sample Rate. 
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_gyro_read(
    device_number_t device_num);


/**
 * @brief MPU6050 Temperature Measurements (TEMP_OUT) registers read
 * 
 * @details Register number: 65-66 
 *          Register size: 2 bytes 
 *          
 *          Register data: 
 *          - TEMP_OUT: device temperature - 16-bit signed value 
 *          
 *          These registers store the most recent (unformatted) temperature sensor 
 *          measurements. This value is written to the registers at the Sample Rate. 
 *          This function must be called to so the new data gets updated to the device 
 *          data record. After updating the data, the raw or formatted values can be read 
 *          from the getters. 
 *          
 *          When the serial interface (i2c interface) is active, the values in the 
 *          registers are held constant so that you can read all values (burst read)
 *          at one instance in time. When the serial interface is idle then these 
 *          registers will go back to being written to at the Sample Rate. 
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_temp_read(
    device_number_t device_num); 


/**
 * @brief MPU6050 read all 
 * 
 * @details Performs the same data record update as the accelerometer, gyroscope and temp 
 *          sensor read functions combined in a burst read. This allows for keeping 
 *          hold of the I2C bus and ensures all the data read is from the same instance in time. 
 *          This is useful for when all the data needs to be read. 
 * 
 * @see mpu6050_accel_read
 * @see mpu6050_gyro_read
 * @see mpu6050_temp_read
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_read_all(
    device_number_t device_num); 

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
MPU6050_ST_RESULT mpu6050_self_test(
    device_number_t device_num);

//=======================================================================================


//=======================================================================================
// Setters and getters 

/**
 * @brief MPU6050 clear device driver fault flag 
 * 
 * @param device_num : data record address of device 
 */
void mpu6050_clear_status(
    device_number_t device_num); 


/**
 * @brief MPU6050 get device driver fault flag 
 * 
 * @param device_num : data record address of device 
 * @return uint8_t : driver status code for a given device number 
 */
uint8_t mpu6050_get_status(
    device_number_t device_num); 


/**
 * @brief MPU6050 INT pin status 
 * 
 * @details Returns the INT pin status of the device. This is done by reading the GPIO input 
 *          pin configured during INT pin initialization. If the pin is high then the INT pin 
 *          is set indicating there is data available for reading. 
 *          
 *          NOTE: this function is only useful if the interrupt pin has been configured. 
 * 
 * @see mpu6050_int_pin_init
 * 
 * @param device_num : data record address of device 
 * @return MPU6050_INT_STATUS : INT pin status 
 */
MPU6050_INT_STATUS mpu6050_int_status(
    device_number_t device_num); 


// /**
//  * @brief MPU6050 raw x-axis acceleration value 
//  * 
//  * @details Returns the unformatted x-axis acceleration data. 
//  * 
//  * @param device_num : data record address of device 
//  * @return int16_t : unformatted x-axis acceleration 
//  */
// int16_t mpu6050_get_accel_x_raw(
//     device_number_t device_num); 


// /**
//  * @brief MPU6050 raw y-axis acceleration value 
//  * 
//  * @details Returns the unformatted y-axis acceleration data. 
//  * 
//  * @param device_num : data record address of device 
//  * @return int16_t : unformatted y-axis acceleration 
//  */
// int16_t mpu6050_get_accel_y_raw(
//     device_number_t device_num); 


// /**
//  * @brief MPU6050 raw z-axis acceleration value 
//  * 
//  * @details Returns the unformatted z-axis acceleration data. 
//  * 
//  * @param device_num : data record address of device 
//  * @return int16_t : unformatted z-axis acceleration 
//  */
// int16_t mpu6050_get_accel_z_raw(
//     device_number_t device_num); 


/**
 * @brief Accelerometer raw values 
 * 
 * @details 
 * 
 * @param device_num 
 * @param accel_x 
 * @param accel_y 
 * @param accel_z 
 */
void mpu6050_get_accel_raw(
    device_number_t device_num, 
    int16_t *accel_x, 
    int16_t *accel_y, 
    int16_t *accel_z); 


// /**
//  * @brief MPU6050 x-axis acceleration calculation 
//  * 
//  * @details Calculates and returns the true acceleration along the x-axis in g's using 
//  *          the raw sensor data. The value is calculated by taking the raw sensor output 
//  *          and dividing it by the appropriate scalar based on the full scale range of 
//  *          the accelerometer. 
//  * 
//  * @see mpu6050_afs_sel_set_t
//  * 
//  * @param device_num : data record address of device 
//  * @return float : x-axis acceleration (g's)
//  */
// float mpu6050_get_accel_x(
//     device_number_t device_num); 


// /**
//  * @brief MPU6050 y-axis acceleration calculation 
//  * 
//  * @details Calculates and returns the true acceleration along the y-axis in g's using 
//  *          the raw sensor data. The value is calculated by taking the raw sensor output 
//  *          and dividing it by the appropriate scalar based on the full scale range of 
//  *          the accelerometer. 
//  * 
//  * @see mpu6050_afs_sel_set_t
//  * 
//  * @param device_num : data record address of device 
//  * @return float : y-axis acceleration (g's)
//  */
// float mpu6050_get_accel_y(
//     device_number_t device_num); 


// /**
//  * @brief MPU6050 z-axis acceleration calculation 
//  * 
//  * @details Calculates and returns the true acceleration along the z-axis in g's using 
//  *          the raw sensor data. The value is calculated by taking the raw sensor output 
//  *          and dividing it by the appropriate scalar based on the full scale range of 
//  *          the accelerometer. 
//  * 
//  * @see mpu6050_afs_sel_set_t
//  * 
//  * @param device_num : data record address of device 
//  * @return float : z-axis acceleration (g's)
//  */
// float mpu6050_get_accel_z(
//     device_number_t device_num); 


/**
 * @brief Accelerometer calculation 
 * 
 * @details 
 * 
 * @param device_num 
 * @param accel_x 
 * @param accel_y 
 * @param accel_z 
 */
void mpu6050_get_accel(
    device_number_t device_num, 
    float *accel_x, 
    float *accel_y, 
    float *accel_z); 


// /**
//  * @brief MPU6050 raw x-axis angular velocity value 
//  * 
//  * @details Returns the unformatted x-axis angular velocity data. 
//  * 
//  * @param device_num : data record address of device 
//  * @return int16_t : unformatted x-axis angular velocity 
//  */
// int16_t mpu6050_get_gyro_x_raw(
//     device_number_t device_num); 


// /**
//  * @brief MPU6050 raw y-axis angular velocity value 
//  * 
//  * @details Returns the unformatted y-axis angular velocity data. 
//  * 
//  * @param device_num : data record address of device 
//  * @return int16_t : unformatted y-axis angular velocity 
//  */
// int16_t mpu6050_get_gyro_y_raw(
//     device_number_t device_num); 


// /**
//  * @brief MPU6050 raw z-axis angular velocity value 
//  * 
//  * @details Returns the unformatted z-axis angular velocity data. 
//  * 
//  * @param device_num : data record address of device 
//  * @return int16_t : unformatted z-axis angular velocity 
//  */
// int16_t mpu6050_get_gyro_z_raw(
//     device_number_t device_num); 


/**
 * @brief Gyroscope raw values 
 * 
 * @details 
 * 
 * @param device_num 
 * @param gyro_x_raw 
 * @param gyro_y_raw 
 * @param gyro_z_raw 
 */
void mpu6050_get_gyro_raw(
    device_number_t device_num, 
    int16_t *gyro_x_raw, 
    int16_t *gyro_y_raw, 
    int16_t *gyro_z_raw); 


// /**
//  * @brief MPU6050 x-axis angular velocity calculation 
//  * 
//  * @details Calculates and returns the true gyroscopic value around the x-axis in deg/s 
//  *          using the raw sensor data. The value is calculated by taking the raw sensor 
//  *          output and dividing it by the appropriate scalar based on the full scale range 
//  *          of the gyroscope. The gyroscope is prone to drift/errors over time so the 
//  *          offset recorded from mpu6050_calibrate is used to correct for errors during 
//  *          the calculation. 
//  * 
//  * @see mpu6050_fs_sel_set_t
//  * @see mpu6050_calibrate
//  * 
//  * @param device_num : data record address of device 
//  * @return float : x-axis angular velocity (deg/s) 
//  */
// float mpu6050_get_gyro_x(
//     device_number_t device_num);


// /**
//  * @brief MPU6050 y-axis angular velocity calculation 
//  * 
//  * @details Calculates and returns the true gyroscopic value around the y-axis in deg/s 
//  *          using the raw sensor data. The value is calculated by taking the raw sensor 
//  *          output and dividing it by the appropriate scalar based on the full scale range 
//  *          of the gyroscope. The gyroscope is prone to drift/errors over time so the 
//  *          offset recorded from mpu6050_calibrate is used to correct for errors during 
//  *          the calculation.
//  * 
//  * @see mpu6050_fs_sel_set_t
//  * @see mpu6050_calibrate
//  * 
//  * @param device_num : data record address of device 
//  * @return float : y-axis angular velocity (deg/s) 
//  */
// float mpu6050_get_gyro_y(
//     device_number_t device_num);


// /**
//  * @brief MPU6050 z-axis angular velocity calculation 
//  * 
//  * @details Calculates and returns the true gyroscopic value around the z-axis in deg/s 
//  *          using the raw sensor data. The value is calculated by taking the raw sensor 
//  *          output and dividing it by the appropriate scalar based on the full scale range 
//  *          of the gyroscope. The gyroscope is prone to drift/errors over time so the 
//  *          offset recorded from mpu6050_calibrate is used to correct for errors during 
//  *          the calculation.
//  * 
//  * @see mpu6050_fs_sel_set_t
//  * @see mpu6050_calibrate
//  * 
//  * @param device_num : data record address of device 
//  * @return float : z-axis angular velocity (deg/s) 
//  */
// float mpu6050_get_gyro_z(
//     device_number_t device_num);


/**
 * @brief Gyroscopic value calculation 
 * 
 * @details 
 * 
 * @param device_num 
 * @param gyro_x 
 * @param gyro_y 
 * @param gyro_z 
 */
void mpu6050_get_gyro(
    device_number_t device_num, 
    int16_t *gyro_x, 
    int16_t *gyro_y, 
    int16_t *gyro_z); 


/**
 * @brief MPU6050 temperature sensor raw value 
 * 
 * @details Returns the unformatted temperature sensor data. 
 * 
 * @param device_num : data record address of device 
 * @return int16_t : unformatted temperature reading 
 */
int16_t mpu6050_get_temp_raw(
    device_number_t device_num); 


/**
 * @brief MPU6050 temperature sensor calculation
 * 
 * @details Calculates and returns the true temperature reading in degC using the raw 
 *          temperature sensor data. This value is calculated using the following equation 
 *          from the register map documentation: 
 *          
 *          Temperature (degC) = (16-bit register value) / 340 + 36.53
 * 
 * @param device_num : data record address of device 
 * @return float : true temperature value (degC) 
 */
float mpu6050_get_temp(
    device_number_t device_num);

//=======================================================================================

#endif  // _MPU6050_DRIVER_H_
