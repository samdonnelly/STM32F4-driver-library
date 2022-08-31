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

#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "i2c_comm.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Controller info 
#define MPU6050_7_BIT_ADDRESS 0x68  // MPU6050 address before read/write bit shift

// Temperature sensor 
#define MPU6050_TEMP_SCALAR 100   // User defined temp scalar to eliminate decimals 
#define MPU6050_TEMP_SENSIT 340   // Sensitivity (LSB/degC) - MPU6050 defined scalar
#define MPU6050_TEMP_OFFSET 3653  // MPU6050 defined temp offset scaled by MPU6050_TEMP_SCALAR

// Accelerometer info 
#define MPU6050_NUM_ACCEL_AXIS 3       // Number of acclerometer axes 
#define MPU6050_AFS_SEL_MASK 24        // 0x18 - Mask for reading accel full scale range
#define MPU6050_ACCEL_FT_MAX_ERROR 14  // Max % change from factory trim acceptable

// Gyro info 
#define MPU6050_NUM_GYRO_AXIS 3        // Number of gyroscope axes 
#define MPU6050_GYRO_SCALAR_SCALAR 10  // Unscales scaled mpu6050_gyro_scalars_t values 
#define MPU6050_FS_SEL_MASK 24         // 0x18 - Mask for reading gyro full scale range 
#define MPU6050_GYRO_FT_MAX_ERROR 14   // Max % change from factory trim acceptable

//=======================================================================================



//=======================================================================================
// Enums 

/**
 * @brief MPU6050 Addresses 
 * 
 * @details The AD0 pin on the accelerometer can be set externally as either 0 or 1 to 
 *          allow for two different MPU's to be on the same I2C bus. The default address
 *          of the MPU6050 is 0x68 (pin AD0 = 0). <br><br>
 *          
 *          The 7-bit address of a slave I2C device is held in the upper 7-bits of the 
 *          byte and the LSB is left to indicate read/receive (1) or write/transmit (0). 
 *          This makes the addresses of the MPU6050 (0x68) shifted to the left by 1. The 
 *          resulting addresses become:                    <br>
 *             - 0xD0 (write) and 0xD1 (read) if AD0 is 0  <br>
 *             - 0xD2 (write) and 0xD3 (read) if AD0 is 1  <br><br>
 *          
 *          Note that only the write addresses are shown below. mpu6050_rw_offset_t is 
 *          used to provide the needed offset when calling the address. 
 * 
 * @see mpu6050_rw_offset_t
 * 
 */
typedef enum {
    MPU6050_1_ADDRESS = 0xD0,
    MPU6050_2_ADDRESS = 0xD2
} mpu6050_i2c_addresses_t;

/**
 * @brief MPU6050 read and write offset 
 * 
 * @details These provide the necessary offset to the devices I2C address to either read 
 *          or write to from it. Simply add this value to the end of the address when 
 *          reading or writing.  
 * 
 * @see mpu6050_i2c_addresses_t
 * 
 */
typedef enum {
    MPU6050_W_OFFSET,
    MPU6050_R_OFFSET
} mpu6050_rw_offset_t;


/**
 * @brief MPU6050 register indexes 
 * 
 * @details The MPU6050 registers are often grouped into more than a single byte. For 
 *          example all of the self-test register data is held across 4 registers or bytes. 
 *          These provide an easy way to index the register information once it's 
 *          already been read so that you can work with it.
 * 
 */
typedef enum {
    MPU6050_REG_IDX_0,
    MPU6050_REG_IDX_1,
    MPU6050_REG_IDX_2,
    MPU6050_REG_IDX_3
} mpu6050_register_index_t;


/**
 * @brief MPU6050 register byte size
 * 
 * @details This is used to specify the number of registers or bytes to read from the 
 *          MPU6050. Not all register information in contained within a single byte 
 *          so this enum provides notation to specify how many bytes to read or write. 
 * 
 */
typedef enum {
    MPU6050_REG_1_BYTE = 1,
    MPU6050_REG_2_BYTE = 2,
    MPU6050_REG_3_BYTE = 3,
    MPU6050_REG_4_BYTE = 4,
    MPU6050_REG_5_BYTE = 5,
    MPU6050_REG_6_BYTE = 6
} mpu6050_reg_byte_size_t;

/**
 * @brief MPU6050 register map
 * 
 * @details The following are registers in the MPU6050. These provide the address of 
 *          where you should start reading or writing data. When reading or writing data,
 *          if you read or write more than a single byte, the address will be incremented 
 *          automatically which is why for example only MPU6050_ACCEL_XOUT_H is specified 
 *          and not Y and Z axes registers. 
 * 
 */
typedef enum {
    MPU6050_SELF_TEST    = 0x0D,   // Register 13 
    MPU6050_SMPRT_DIV    = 0x19,   // Register 25
    MPU6050_CONFIG       = 0x1A,   // Register 26
    MPU6050_GYRO_CONFIG  = 0x1B,   // Register 27
    MPU6050_ACCEL_CONFIG = 0x1C,   // Register 28 
    MPU6050_ACCEL_XOUT_H = 0x3B,   // Register 59
    MPU6050_TEMP_OUT_H   = 0x41,   // Register 65
    MPU6050_GYRO_XOUT_H  = 0x43,   // Register 67
    MPU6050_PWR_MGMT_1   = 0x6B,   // Register 107 
    MPU6050_PWR_MGMT_2   = 0x6C,   // Register 108 
    MPU6050_WHO_AM_I     = 0x75    // Register 117
} mpu6050_register_addresses_t;


/**
 * @brief MPU6050 self-test masks 
 * 
 * @details During self-test, after reading the self-test registers, the following masks 
 *          are used to parse the X, Y and Z accleroemeter and gyroscope data apart from 
 *          one another so that the data is readable/usable. 
 * 
 */
typedef enum {
    SELF_TEST_MASK_ZA_TEST_LO = 0x03,
    SELF_TEST_MASK_YA_TEST_LO = 0x0C,
    SELF_TEST_MASK_X_TEST     = 0x1F,
    SELF_TEST_MASK_XA_TEST_LO = 0x30,
    SELF_TEST_MASK_A_TEST_HI  = 0xE0
} mpu6050_self_test_masks_t;


/**
 * @brief MPU6050 self-test results shifter 
 * 
 * @details When self-test is executed the final result is a single byte of data that 
 *          indicated a pass or fail for all 6 (3 accel + 3 gyro) axes. These shifters 
 *          are used to format that final result so that the result of each axes doesn't
 *          conflict with the result of another. 
 * 
 * @see mpu6050_self_test
 * 
 */
typedef enum {
    SELF_TEST_RESULT_SHIFT_ACCEL = 0x01,
    SELF_TEST_RESULT_SHIFT_GYRO  = 0x08
} self_test_result_shift_t;


/**
 * @brief MPU6050 accelerometer factory trim equation constants 
 * 
 * @details The Register Map document for the MPU6050 outlines how to calculate the 
 *          factory trim for each axes in order to perform a self-test. The factory trim 
 *          equation however is complex, so to avoid performing this complex calculation
 *          in the code, the equation was plotted in Excel using all possible outcomes 
 *          of the 5-bit result from the self-test register and then approximated using 
 *          a third order polynominal equation. The following values are the constants 
 *          from the approximated accelerometer factory trim equation. 
 * 
 */
typedef enum {
    SELF_TEST_ACCEL_FT_C1 = 142,
    SELF_TEST_ACCEL_FT_C2 = 6056,
    SELF_TEST_ACCEL_FT_C4 = 13452,
    SELF_TEST_ACCEL_FT_C3 = 45752
} self_test_accel_constants_t;


/**
 * @brief MPU6050 gyroscope factory trim equation constants 
 * 
 * @details The Register Map document for the MPU6050 outlines how to calculate the 
 *          factory trim for each axes in order to perform a self-test. The factory trim 
 *          equation however is complex, so to avoid performing this complex calculation
 *          in the code, the equation was plotted in Excel using all possible outcomes 
 *          of the 5-bit result from the self-test register and then approximated using 
 *          a third order polynominal equation. The following values are the constants 
 *          from the approximated gyroscope factory trim equation. 
 * 
 */
typedef enum {
    SELF_TEST_GYRO_FT_C1 = 1001,
    SELF_TEST_GYRO_FT_C3 = 15056,
    SELF_TEST_GYRO_FT_C2 = 19244,
    SELF_TEST_GYRO_FT_C4 = 31125
} self_test_gyro_constants_t;


/**
 * @brief MPU6050 sample rate divider setpoint 
 * 
 * @details These values are dividers used to divide the gyroscope output rate to get the 
 *          system sample rate. The gyroscope output rate is dependent on the digital 
 *          low pass filter setpoint so this value changes to reflect that value along 
 *          with the desired sample rate you want. The values used as an argument to 
 *          mpu6050_init for setting up the device. The units of this value is kHz. 
 * 
 * @see mpu6050_init
 * @see mpu6050_dlpf_cfg_t
 * 
 */
typedef enum {
    SMPLRT_DIV_0 = 0,  // 1 kHz when DLPF = 0 or 7 
    SMPLRT_DIV_7 = 7   // 1 kHz when DLPF = 1 - 6
} mpu6050_smplrt_div_t;

/**
 * @brief MPU6050 EXT_SYNC_SET setpoint
 * 
 * @details These are setpointf for the EXT_SYNC_SET value in the CONFIG register. This 
 *          value can be used to configure sampling of an external signal via the FSYNC
 *          pin. The possible values for EXT_SYNC_SET are detailed as follows: <br><br>
 *          
 *          0 : Input disabled   <br> 
 *          1 : TEMP_OUT_L[0]    <br> 
 *          2 : GYRO_XOUT_L[0]   <br> 
 *          3 : GYRO_YOUT_L[0]   <br> 
 *          5 : ACCEL_XOUT_L[0]  <br> 
 *          6 : ACCEL_YOUT_L[0]  <br> 
 *          7 : ACCEL_ZOUT_L[0]  <br><br>
 *          
 *          Currently this input is hardcoded to 0 and isn't an argument to mpu6050_init. 
 * 
 */
typedef enum {
    EXT_SYNC_SET_0,  // Input disabled 
    EXT_SYNC_SET_1,  // TEMP_OUT_L[0]
    EXT_SYNC_SET_2,  // GYRO_XOUT_L[0]
    EXT_SYNC_SET_3,  // GYRO_YOUT_L[0]
    EXT_SYNC_SET_4,  // GYRO_ZOUT_L[0]
    EXT_SYNC_SET_5,  // ACCEL_XOUT_L[0]
    EXT_SYNC_SET_6,  // ACCEL_YOUT_L[0]
    EXT_SYNC_SET_7   // ACCEL_ZOUT_L[0]
} mpu6050_ext_sync_set_t;

/**
 * @brief MPU6050 DLPF_CFG setpoint
 * 
 * @details A digital low pass filter (DLPF) can be specified in the CONFIG register 
 *          which is used to filter accelerometer and gyroscope data. The value of the 
 *          DLPF dictates the gyroscope output frquency as descibed below: <br>
 *             - DLPF = 0 or 7 -> gyroscope output rate = 8 kHz <br>
 *             - DLPF = 1 - 6  -> gyroscope output rate = 1 kHz <br><br> 
 *          
 *          A summary of cutoff frquencies are outlined below. Note that lower cutoff
 *          frequencies come with longer delay times.     <br>
 *             - DLPF = 0 : accel = 260 Hz, gyro = 256 Hz <br> 
 *             - DLPF = 1 : accel = 184 Hz, gyro = 188 Hz <br> 
 *             - DLPF = 2 : accel = 94  Hz, gyro = 98  Hz <br> 
 *             - DLPF = 3 : accel = 44  Hz, gyro = 42  Hz <br> 
 *             - DLPF = 4 : accel = 21  Hz, gyro = 20  Hz <br> 
 *             - DLPF = 5 : accel = 10  Hz, gyro = 10  Hz <br> 
 *             - DLPF = 6 : accel = 5   Hz, gyro = 5   Hz <br> 
 *             - DLPF = 7 : RESERVED <br> <br> 
 *          
 *          Refer to the Register Map datasheet for the MPU6050 for more information on 
 *          DLPF settings.
 * 
 */
typedef enum {
    DLPF_CFG_0,  // accel = 260 Hz, gyro = 256 Hz
    DLPF_CFG_1,  // accel = 184 Hz, gyro = 188 Hz
    DLPF_CFG_2,  // accel = 94  Hz, gyro = 98  Hz
    DLPF_CFG_3,  // accel = 44  Hz, gyro = 42  Hz
    DLPF_CFG_4,  // accel = 21  Hz, gyro = 20  Hz
    DLPF_CFG_5,  // accel = 10  Hz, gyro = 10  Hz
    DLPF_CFG_6,  // accel = 5   Hz, gyro = 5   Hz
    DLPF_CFG_7   // RESERVED
} mpu6050_dlpf_cfg_t;

/**
 * @brief MPU6050 - GYRO_CONFIG : XG_ST, YG_ST and ZG_ST setpoint 
 * 
 * @details This is used to enable and disable self-test on the gyroscope. During 
 *          initialization self-test is disabled, but when mpu6050_self_test is called 
 *          self-test is temporarily enabled. 
 * 
 * @see mpu6050_self_test
 * 
 */
typedef enum {
    GYRO_SELF_TEST_DISABLE,
    GYRO_SELF_TEST_ENABLE
} mpu6050_gyro_self_test_set_t;

/**
 * @brief MPU6050 - GYRO_CONFIG : FS_SEL setpoint 
 * 
 * @details Selects the full scale range used by the gyroscope. This is passed as an 
 *          argument to mpu6050_init to configure the device. Higher ranges cover 
 *          a wider range of angular accelerations but have less precision. 
 * 
 */
typedef enum {
    FS_SEL_250,   // +/- 250  deg/s
    FS_SEL_500,   // +/- 500  deg/s
    FS_SEL_1000,  // +/- 1000 deg/s
    FS_SEL_2000   // +/- 2000 deg/s
} mpu6050_fs_sel_set_t;

/**
 * @brief MPU6050 gyroscope value scalar 
 * 
 * @details These values are used to divide the raw 16-bit value read from the gyroscope 
 *          measurement register to get the angular acceleration in deg/s. These values 
 *          are scaled by a factor of 10 to eliminate the decimal place. When calculating  
 *          the actual value the values are unscaled. 
 * 
 */
typedef enum {
    GYRO_SCALE_FS_SEL_2000 = 164,  // 16.4  * 10
    GYRO_SCALE_FS_SEL_1000 = 328,  // 32.8  * 10
    GYRO_SCALE_FS_SEL_500  = 655,  // 65.5  * 10
    GYRO_SCALE_FS_SEL_250  = 1310  // 131.0 * 10
} mpu6050_gyro_scalars_t;

/**
 * @brief MPU6050 - ACCEL_CONFIG : XA_ST, YA_SET and ZA_ST setpoint
 * 
 * @details This is used to enable and disable self-test on the accelerometer. During 
 *          initialization self-test is disabled, but when mpu6050_self_test is called 
 *          self-test is temporarily enabled. 
 * 
 * @see mpu6050_self_test 
 * 
 */
typedef enum {
    ACCEL_SELF_TEST_DISABLE,
    ACCEL_SELF_TEST_ENABLE
} mpu6050_accel_self_test_set_t;

/**
 * @brief MPU6050 - ACCEL_CONFIG : AFS_SEL setpoint
 * 
 * @details Selects the full scale range of the acclerometer 
 *          Selects the full scale range used by the gyroscope. This is passed as an 
 *          argument to mpu6050_init to configure the device. Higher ranges cover 
 *          a wider range of angular accelerations but have less precision. 
 * 
 */
typedef enum {
    AFS_SEL_2,   // +/- 2g
    AFS_SEL_4,   // +/- 4g
    AFS_SEL_8,   // +/- 8g
    AFS_SEL_16   // +/- 16g
} mpu6050_afs_sel_set_t;

/**
 * @brief Accelerometer value scalar 
 * 
 * @details These values are used to divide the raw 16-bit value read from the  
 *          accelerometer measurement register to get the linear acceleration in g's. 
 * 
 */
typedef enum {
    ACCEL_SCALE_AFS_SEL_16 = 2048,
    ACCEL_SCALE_AFS_SEL_8  = 4096,
    ACCEL_SCALE_AFS_SEL_4  = 8192,
    ACCEL_SCALE_AFS_SEL_2  = 16384
} mpu6050_accel_scalars_t;

/**
 * @brief MPU6050 - PWR_MGMT_1 : DEVICE_RESET
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for 
 *          a reset to the devices default settings. 
 * 
 */
typedef enum {
    DEVICE_RESET_DISABLE,
    DEVICE_RESET_ENABLE
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
 * 
 */
typedef enum {
    SLEEP_MODE_DISABLE,
    SLEEP_MODE_ENABLE
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
 * 
 */
typedef enum {
    CYCLE_SLEEP_DISABLED,
    CYCLE_SLEEP_ENABLED
} mpu6050_cycle_t;

/**
 * @brief MPU6050 - PWR_MGMT_1 : TEMP_DIS
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for 
 *          enabling or disabling of the temperature sensor. 
 * 
 */
typedef enum {
    TEMP_SENSOR_ENABLE,
    TEMP_SENSOR_DISABLE
} mpu6050_temp_sensor_t;

/**
 * @brief MPU6050 - PWR_MGMT_1 : CLKSEL
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for the
 *          selection of the devices clock source. The clock source is selected as 
 *          follows: <br><br> 
 *              - CLKSEL = 0 : Internal 8MHz oscillator                                <br>
 *              - CLKSEL = 1 : PPL with X-axis gyro reference                          <br>
 *              - CLKSEL = 2 : PPL with Y-axis gyro reference                          <br>
 *              - CLKSEL = 3 : PPL with Z-axis gyro reference                          <br>
 *              - CLKSEL = 4 : PPL with external 32.768kHz reference                   <br>
 *              - CLKSEL = 5 : PPL with external 19.2MHz reference                     <br>
 *              - CLKSEL = 6 : Reserved                                                <br>
 *              - CLKSEL = 7 : Stops the clock and keeps the timing generator on reset <br>
 *           <br> 
 *           It is recommended the internal oscillator is not used as the other options 
 *           are much more stable.  
 * 
 */
typedef enum {
    CLKSEL_0,  // Internal 8MHz oscillator
    CLKSEL_1,  // PPL with X-axis gyro reference 
    CLKSEL_2,  // PPL with Y-axis gyro reference 
    CLKSEL_3,  // PPL with Z-axis gyro reference 
    CLKSEL_4,  // PPL with external 32.768kHz reference
    CLKSEL_5,  // PPL with external 19.2MHz reference
    CLKSEL_6,  // Reserved 
    CLKSEL_7   // Stops the clock and keeps the timing generator on reset
} mpu6050_clksel_t;

/**
 * @brief MPU6050 - PWR_MGMT_2 : LP_WAKE_CTRL
 * 
 * @details Used to configure power management register 2 in mpu6050_init. This allows 
 *          for configuring of the frequency of wake-ups in low power mode. In this mode 
 *          the device will power off all functions except for the primary i2c interface
 *          waking up only the accelerometer at fixed intervals to take a single 
 *          measurements. Values of LP_WAKE_CTRL correspond to the following wake-up
 *          frequencies: <br>
 *              - LP_WAKE_CTRL = 0 : 1.25 Hz  <br>
 *              - LP_WAKE_CTRL = 1 : 5  Hz    <br>
 *              - LP_WAKE_CTRL = 2 : 20 Hz    <br>
 *              - LP_WAKE_CTRL = 3 : 40 Hz    <br><br>
 *          
 *          Low power mode can be configured using the following steps carried out in 
 *          power management register 1:             <br>
 *              - Set CYCLE to 1                     <br>
 *              - Set SLEEP to 0                     <br>
 *              - Set TEMP_DIS to 1                  <br>
 *              - Set STBY_XG, STBY_YG, STBY_ZG to 1 <br>
 * 
 */
typedef enum {
    LP_WAKE_CTRL_0,  // 1.25 Hz wakeup frequency
    LP_WAKE_CTRL_1,  // 5  Hz wakeup frequency
    LP_WAKE_CTRL_2,  // 20 Hz wakeup frequency
    LP_WAKE_CTRL_3   // 40 Hz wakeup frequency
} mpu6050_lp_wake_ctrl_t;

/**
 * @brief MPU6050 - PWR_MGMT_2 : STBY_XA
 * 
 * @details Used to configure power management register 2 in mpu6050_init. Allows for 
 *          putting the x-axis accelerometer into standy mode.
 * 
 */
typedef enum {
    STBY_XA_DISABLE,
    STBY_XA_ENABLE
} mpu6050_stby_xa_t;

/**
 * @brief MPU6050 - PWR_MGMT_2 : STBY_YA
 * 
 * @details Used to configure power management register 2 in mpu6050_init. Allows for 
 *          putting the y-axis accelerometer into standy mode.
 * 
 */
typedef enum {
    STBY_YA_DISABLE,
    STBY_YA_ENABLE
} mpu6050_stby_ya_t;

/**
 * @brief MPU6050 - PWR_MGMT_2 : STBY_ZA
 * 
 * @details Used to configure power management register 2 in mpu6050_init. Allows for 
 *          putting the z-axis accelerometer into standy mode.
 * 
 */
typedef enum {
    STBY_ZA_DISABLE,
    STBY_ZA_ENABLE
} mpu6050_stby_za_t;

/**
 * @brief MPU6050 - PWR_MGMT_2 : STBY_XG
 * 
 * @details Used to configure power management register 2 in mpu6050_init. Allows for 
 *          putting the x-axis gyroscope into standy mode. Note that if this axis is 
 *          used as the clock source for the device and this axis is disabled then 
 *          the clock source will automatically be changed to the internal 8 MHz 
 *          oscillator (not recommended). 
 * 
 */
typedef enum {
    STBY_XG_DISABLE,
    STBY_XG_ENABLE
} mpu6050_stby_xg_t;

/**
 * @brief MPU6050 - PWR_MGMT_2 : STBY_YG
 * 
 * @details Used to configure power management register 2 in mpu6050_init. Allows for 
 *          putting the y-axis gyroscope into standy mode. Note that if this axis is 
 *          used as the clock source for the device and this axis is disabled then 
 *          the clock source will automatically be changed to the internal 8 MHz 
 *          oscillator (not recommended).
 * 
 */
typedef enum {
    STBY_YG_DISABLE,
    STBY_YG_ENABLE
} mpu6050_stby_yg_t;

/**
 * @brief MPU6050 - PWR_MGMT_2 : STBY_ZG
 * 
 * @details Used to configure power management register 2 in mpu6050_init. Allows for 
 *          putting the z-axis gyroscope into standy mode. Note that if this axis is 
 *          used as the clock source for the device and this axis is disabled then 
 *          the clock source will automatically be changed to the internal 8 MHz 
 *          oscillator (not recommended).
 * 
 */
typedef enum {
    STBY_ZG_DISABLE,
    STBY_ZG_ENABLE
} mpu6050_stby_zg_t;

//=======================================================================================


//=======================================================================================
// Data types 

// TODO create data types for function return values 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief MPU5060 initialization 
 * 
 * @details Used to configure the device upon startup. If device initialization is 
 *          successfull the function will return true. If there was issues establishing 
 *          communication with the device the function will return false. The return 
 *          value is used to trigger fault conditions. 
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param dlpf_cfg : digital low pass filter selection 
 * @param smplrt_div : sample rate divider selection 
 * @param afs_sel : full scale range of accelerometer 
 * @param fs_sel : full scale range of gyroscope 
 * @return uint8_t : returns true (1) if successfull, false (0) if unsuccessfull
 */
uint8_t mpu6050_init(
    mpu6050_i2c_addresses_t mpu6050_address,
    mpu6050_dlpf_cfg_t dlpf_cfg,
    mpu6050_smplrt_div_t smplrt_div,
    mpu6050_afs_sel_set_t afs_sel,
    mpu6050_fs_sel_set_t fs_sel);

/**
 * @brief MPU6050 reference point set 
 * 
 * @details The readings from the accelerometer are not exact even if a self-test is 
 *          passed. This function takes a single reading of acceleromater and 
 *          gyroscope values and stores the result in a pointer passed to the function 
 *          which can be used as an error offset when analyzing other readings. 
 *          Typically this function would be called once during initialization of the 
 *          program. 
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param mpu6050_accel_offset : pointer that stores an instance of accelerometer data
 * @param mpu6050_gyro_offset : pointer that stores an instance of gyroscope data
 */
void mpu6050_calibrate(
    mpu6050_i2c_addresses_t mpu6050_address, 
    int16_t *mpu6050_accel_offset,
    int16_t *mpu6050_gyro_offset);

//=======================================================================================


//=======================================================================================
// Register Function

/**
 * @brief MPU6050 Accelerometer Measurements registers read
 * 
 * @details Read from the ACCEL_OUT registers (Registers 59-64 - 6 bytes). These 
 *          registers store the most recent (unformatted) accelerometer measurements. 
 *          These values are written to the register at the Sample Rate but are updated 
 *          at a frequency of 1 kHz. The calculation functions mpu6050_accel_x/y/z_calc 
 *          are used to getthe actual acceleration in g's. <br><br>
 *          
 *          When the serial interface (i2c interface) is active, the values in the 
 *          registers are held constant so that you can read all values (burst read)
 *          at one instance in time. When the serial interface is idle then these 
 *          registers will go back to being written to at the Sample Rate. <br><br>
 *          
 *          Register read information: <br>
 *          - ACCEL_XOUT: 16-bit signed value. Stored in the pointers first address. <br>
 *          - ACCEL_YOUT: 16-bit signed value. Stored in the pointers second address. <br>
 *          - ACCEL_ZOUT: 16-bit signed value. Stored in the pointers third address. 
 * 
 * @see mpu6050_accel_scalars_t
 * @see mpu6050_accel_x_calc
 * @see mpu6050_accel_y_calc
 * @see mpu6050_accel_z_calc
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param accel_data : pointer to array where accelerometer data is stored 
 */
void mpu6050_accel_read(
    mpu6050_i2c_addresses_t mpu6050_address,
    int16_t *accel_data);


/**
 * @brief MPU6050 Temperature Measurements registers read
 * 
 * @details Reads from the TEMP_OUT registers (Registers 65-66 - 2 bytes). These 
 *          registers store the most recent (unformatted) temperature sensor 
 *          measurements. The measurements are updated to these registers at the 
 *          Sample Rate. <br><br>
 *          
 *          When the serial interface (i2c interface) is active, the values in the 
 *          registers are held constant so that you can read all values (burst read)
 *          at one instance in time. When the serial interface is idle then these 
 *          registers will go back to being written to at the Sample Rate. <br><br>
 *          
 *          Register read information: <br>
 *          - TEMP_OUT: 16-bit signed value. 
 * 
 * @see mpu6050_temp_calc
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @return int16_t : unformatted signed temperature value 
 */
int16_t mpu6050_temp_read(
    mpu6050_i2c_addresses_t mpu6050_address);


/**
 * @brief MPU6050 Gyroscope Measurements registers read
 * 
 * @details Read from the GYRO_OUT registers (Registers 67-72 - 6 bytes). These 
 *          registers store the most recent (unformatted) gyroscope measurements. 
 *          These values are updated at the Sample Rate. The calculation functions 
 *          mpu6050_gyro_x/y/z_calc are used to get the actual gyroscopic values in 
 *          deg/s. <br><br>
 *          
 *          When the serial interface (i2c interface) is active, the values in the 
 *          registers are held constant so that you can read all values (burst read)
 *          at one instance in time. When the serial interface is idle then these 
 *          registers will go back to being written to at the Sample Rate. <br><br>
 *          
 *          Register read information: <br>
 *          - GYRO_XOUT: 16-bit signed value. Stored in the pointers first address. <br>
 *          - GYRO_YOUT: 16-bit signed value. Stored in the pointers second address. <br>
 *          - GYRO_ZOUT: 16-bit signed value. Stored in the pointers third address. 
 * 
 * @see mpu6050_gyro_scalars_t
 * @see mpu6050_gyro_x_calc
 * @see mpu6050_gyro_y_calc
 * @see mpu6050_gyro_z_calc
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param gyro_data : pointer to array where gyroscope data is stored 
 */
void mpu6050_gyro_read(
    mpu6050_i2c_addresses_t mpu6050_address,
    int16_t *gyro_data);

//=======================================================================================


//=======================================================================================
// Self-test functions 

/**
 * @brief MPU6050 self-test
 * 
 * @details This functions runs a self-test on the device to see its drift from the 
 *          factory calibration. When self-test is activated, the on-board electronics
 *          will actuate the appropriate sensor and produce a change in the sensor
 *          output. The self-test response is defined as: <br><br> 
 *          
 *          Self-test response = (sensor output with self-test enabled) - 
 *                               (sensor output with self-test disabled) <br><br> 
 *          
 *          To pass the self-test the sensor must be within 14% of it's factory 
 *          calibration. The function will return a byte that indicates the self-test 
 *          results of each accelerometer and gyroscope axis where a 0 is a pass and 
 *          a 1 is a fail. The return value breakdown is as follows: <br><br>
 *          
 *          - Bit 5: gyroscope z-axis     <br>
 *          - Bit 4: gyroscope y-axis     <br>
 *          - Bit 3: gyroscope x-axis     <br>
 *          - Bit 2: accelerometer z-axis <br>
 *          - Bit 1: accelerometer y-axis <br>
 *          - Bit 0: accelerometer x-axis <br>
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @return uint8_t : self-test results for each sensor axis 
 */
uint8_t mpu6050_self_test(
    mpu6050_i2c_addresses_t mpu6050_address);

//=======================================================================================


//=======================================================================================
// Calculation Functions

/**
 * @brief MPU6050 accelerometer x-axis calculation 
 * 
 * @details Calculates and returns the true acceleration along the x-axis in g's using 
 *          the raw sensor output from mpu6050_accel_read. The value is calculated by 
 *          taking the raw sensor output and dividing it by the appropriate scalar based
 *          on the full scale range of the accelerometer. 
 * 
 * @see mpu6050_accel_read
 * @see mpu6050_accel_scalars_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param accel_x_axis_raw : raw x-axis acceleration output 
 * @return float : x-axis acceleration in g's
 */
float mpu6050_accel_x_calc(
    mpu6050_i2c_addresses_t mpu6050_address, 
    int16_t accel_x_axis_raw);

/**
 * @brief MPU6050 accelerometer y-axis calculation 
 * 
 * @details Calculates and returns the true acceleration along the y-axis in g's using 
 *          the raw sensor output from mpu6050_accel_read. The value is calculated by 
 *          taking the raw sensor output and dividing it by the appropriate scalar based
 *          on the full scale range of the accelerometer. 
 * 
 * @see mpu6050_accel_read
 * @see mpu6050_accel_scalars_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param accel_y_axis_raw : raw y-axis acceleration output 
 * @return float : y-axis acceleration in g's
 */
float mpu6050_accel_y_calc(
    mpu6050_i2c_addresses_t mpu6050_address, 
    int16_t accel_y_axis_raw);

/**
 * @brief MPU6050 accelerometer z-axis calculation 
 * 
 * @details Calculates and returns the true acceleration along the z-axis in g's using 
 *          the raw sensor output from mpu6050_accel_read. The value is calculated by 
 *          taking the raw sensor output and dividing it by the appropriate scalar based
 *          on the full scale range of the accelerometer. 
 * 
 * @see mpu6050_accel_read
 * @see mpu6050_accel_scalars_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param accel_z_axis_raw : raw z-axis acceleration output 
 * @return float : z-axis acceleration in g's
 */
float mpu6050_accel_z_calc(
    mpu6050_i2c_addresses_t mpu6050_address, 
    int16_t accel_z_axis_raw);


/**
 * @brief MPU6050 temperature sensor calculation
 * 
 * @details Calculates and returns the true temperature reading in degC using the raw 
 *          temperature sensor output from mpu6050_temp_read. This value is calculated
 *          using the following equation: <br><br>
 *          
 *          Temperature (degC) = (16-bit register value) / 340 + 36.53
 * 
 * @param temp_raw : raw temperature sensor output
 * @return float : true temperature value in degC
 */
float mpu6050_temp_calc(int16_t temp_raw);

/**
 * @brief MPU6050 gyroscopic value calculation around x-axis 
 * 
 * @details Calculates and returns the true gyroscopic value around the x-axis in deg/s 
 *          using the raw sensor output from mpu6050_gyro_read. The value is calculated 
 *          by taking the raw sensor output and dividing it by the appropriate scalar 
 *          based on the full scale range of the gyroscope. The gyroscope is prone to 
 *          drift/errors over time so the initial value recorded from mpu6050_calibrate
 *          is passed to offset the error from the calculation. 
 * 
 * @see mpu6050_gyro_read
 * @see mpu6050_gyro_scalars_t
 * @see mpu6050_calibrate
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param gyro_x_axis_raw : raw x-axis gyroscope output 
 * @param gyro_x_axis_offset : resting drift/error in gyroscope axis
 * @return float : x-axis gyroscopic value in deg/s
 */
float mpu6050_gyro_x_calc(
    mpu6050_i2c_addresses_t mpu6050_address, 
    int16_t gyro_x_axis_raw,
    int16_t gyro_x_axis_offset);

/**
 * @brief MPU6050 gyroscopic value calculation around y-axis 
 * 
 * @details Calculates and returns the true gyroscopic value around the y-axis in deg/s 
 *          using the raw sensor output from mpu6050_gyro_read. The value is calculated 
 *          by taking the raw sensor output and dividing it by the appropriate scalar 
 *          based on the full scale range of the gyroscope. The gyroscope is prone to 
 *          drift/errors over time so the initial value recorded from mpu6050_calibrate
 *          is passed to offset the error from the calculation.
 * 
 * @see mpu6050_gyro_read
 * @see mpu6050_gyro_scalars_t
 * @see mpu6050_calibrate
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param gyro_y_axis_raw : raw y-axis gyroscope output 
 * @param gyro_y_axis_offset : resting drift/error in gyroscope axis
 * @return float : y-axis gyroscopic value in deg/s
 */
float mpu6050_gyro_y_calc(
    mpu6050_i2c_addresses_t mpu6050_address, 
    int16_t gyro_y_axis_raw,
    int16_t gyro_y_axis_offset);

/**
 * @brief MPU6050 gyroscopic value calculation around z-axis 
 * 
 * @details Calculates and returns the true gyroscopic value around the z-axis in deg/s 
 *          using the raw sensor output from mpu6050_gyro_read. The value is calculated 
 *          by taking the raw sensor output and dividing it by the appropriate scalar 
 *          based on the full scale range of the gyroscope. The gyroscope is prone to 
 *          drift/errors over time so the initial value recorded from mpu6050_calibrate
 *          is passed to offset the error from the calculation.
 * 
 * @see mpu6050_gyro_read
 * @see mpu6050_gyro_scalars_t
 * @see mpu6050_calibrate
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param gyro_z_axis_raw : raw z-axis gyroscope output 
 * @param gyro_z_axis_offset : resting drift/error in gyroscope axis
 * @return float : z-axis gyroscopic value in deg/s
 */
float mpu6050_gyro_z_calc(
    mpu6050_i2c_addresses_t mpu6050_address, 
    int16_t gyro_z_axis_raw,
    int16_t gyro_z_axis_offset);

//=======================================================================================


#endif  // _MPU6050_DRIVER_H_
