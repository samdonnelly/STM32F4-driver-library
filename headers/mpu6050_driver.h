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

#define MPU6050_7_BIT_ADDRESS 0x68  // MPU-6050 address before read/write bit shift

//=======================================================================================



//=======================================================================================
// Enums 

/**
 * @brief MPU-6050 Addresses 
 * 
 * @details The AD0 pin on the accelerometer can be set as either 0 or 1 to allow for 
 *          two different MPU's to be on the same I2C bus. The default address of the 
 *          MPU-6050 is 0x68 (pin AD0 = 0). 
 *          
 *          The 7-bit address of a slave I2C device is held in the upper 7-bits of the 
 *          byte and LSB is left to indicate read/receive (1) or write/transmit (0). 
 *          This makes the addresses if the MPU-6050 bit shifted to the left by 1. The 
 *          resulting addresses become: 
 *             - 0xD0 and 0xD1 if AD0 is 0 
 *             - 0xD2 and 0xD3 if AD0 is 1 
 * 
 */
typedef enum {
    MPU6050_1_ADDRESS = 0xD0,
    MPU6050_2_ADDRESS = 0xD2
} mpu6050_i2c_addresses_t;

/**
 * @brief MPU-6050 read and write offset 
 * 
 */
typedef enum {
    MPU6050_W_OFFSET,
    MPU6050_R_OFFSET
} mpu6050_rw_offset_t;

/**
 * @brief MPU-6050 register byte size
 * 
 */
typedef enum {
    MPU6050_REG_1_BYTE = 1,
    MPU6050_REG_2_BYTE = 2
} mpu6050_reg_byte_size_t;

/**
 * @brief Register map for the MPU-6050
 * 
 */
typedef enum {
    MPU6050_SMPRT_DIV    = 0x19,   // Register 25
    MPU6050_CONFIG       = 0x1A,   // Register 26
    MPU6050_GYRO_CONFIG  = 0x1B,   // Register 27
    MPU6050_ACCEL_CONFIG = 0x1C,   // Register 28 
    MPU6050_PWR_MGMT_1   = 0x6B,   // Register 107 
    MPU6050_PWR_MGMT_2   = 0x6C,   // Register 108 
    MPU6050_WHO_AM_I     = 0x75    // Register 117
} mpu6050_register_addresses_t;

/**
 * @brief SMPLRT_DIV setpoint 
 * 
 * @details Unit of kHz
 * 
 */
typedef enum {
    SMPLRT_DIV_0 = 0,
    SMPLRT_DIV_7 = 7
} mpu6050_smplrt_div_t;

/**
 * @brief EXT_SYNC_SET setpoint
 * 
 * @details 
 * 
 */
typedef enum {
    EXT_SYNC_SET_0,  // Input disabled 
    EXT_SYNC_SET_1,
    EXT_SYNC_SET_2,
    EXT_SYNC_SET_3,
    EXT_SYNC_SET_4,
    EXT_SYNC_SET_5,
    EXT_SYNC_SET_6,
    EXT_SYNC_SET_7
} mpu6050_ext_sync_set_t;

/**
 * @brief DLPF_CFG setpoint
 * 
 * @details 
 * 
 */
typedef enum {
    DLPF_CFG_0,
    DLPF_CFG_1,
    DLPF_CFG_2,
    DLPF_CFG_3,
    DLPF_CFG_4,
    DLPF_CFG_5,
    DLPF_CFG_6,
    DLPF_CFG_7
} mpu6050_dlpf_cfg_t;

/**
 * @brief XG_ST, YG_ST and ZG_ST setpoint 
 * 
 */
typedef enum {
    GYRO_SELF_TEST_DISABLE,
    GYRO_SELF_TEST_ENABLE
} mpu6050_gyro_self_test_set_t;

/**
 * @brief FS_SEL setpoint 
 * 
 * @details Selects the full scale range of the gyroscope 
 * 
 */
typedef enum {
    FS_SEL_250,
    FS_SEL_500,
    FS_SEL_1000,
    FS_SEL_2000
} mpu6050_fs_sel_set_t;

/**
 * @brief XA_ST, YA_SET and ZA_ST setpoint 
 * 
 */
typedef enum {
    ACCEL_SELF_TEST_DISABLE,
    ACCEL_SELF_TEST_ENABLE
} mpu6050_accel_self_test_set_t;

/**
 * @brief AFS_SEL setpoint
 * 
 * @details Selects the full scale range of the acclerometer 
 * 
 */
typedef enum {
    AFS_SEL_2,
    AFS_SEL_4,
    AFS_SEL_8,
    AFS_SEL_16
} mpu6050_afs_sel_set_t;

/**
 * @brief PWR_MGMT_1 - DEVICE_RESET 
 * 
 */
typedef enum {
    DEVICE_RESET_DISABLE,
    DEVICE_RESET_ENABLE
} mpu6050_device_reset_t; 

/**
 * @brief PWR_MGMT_1 - SLEEP
 * 
 */
typedef enum {
    SLEEP_MODE_DISABLE,
    SLEEP_MODE_ENABLE
} mpu6050_sleep_mode_t;

/**
 * @brief PWR_MGMT_1 - CYCLE 
 * 
 */
typedef enum {
    CYCLE_SLEEP_DISABLED,
    CYCLE_SLEEP_ENABLED
} mpu6050_cycle_t;

/**
 * @brief PWR_MGMT_1 - TEMP_DIS
 * 
 */
typedef enum {
    TEMP_SENSOR_ENABLE,
    TEMP_SENSOR_DISABLE
} mpu6050_temp_sensor_t;


typedef enum {
    CLKSEL_0,  // 
    CLKSEL_1,  // 
    CLKSEL_2,  // 
    CLKSEL_3,  // 
    CLKSEL_4,  // 
    CLKSEL_5,  //
    CLKSEL_6,  // 
    CLKSEL_7   // 
} mpu6050_clksel_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief MPU5060 initialization 
 * 
 */
uint8_t mpu6050_init(uint8_t mpu6050_address);

/**
 * @brief MPU-6050 write to register 
 * 
 */
void mpu6050_write(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register,
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);

/**
 * @brief MPU-6050 read from register 
 * 
 * @details 
 *          The MPU automatically increments the register address if there is more than 
 *          one byte to be read. 
 * 
 */
void mpu6050_read(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register, 
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);

/**
 * @brief Sample Rate Divider register 
 * 
 * @details Register 25 
 * 
 * @param mpu6050_address 
 * @param smprt_div 
 */
void mpu6050_smprt_div_write(
    uint8_t mpu6050_address,
    uint8_t smprt_div);

/**
 * @brief Configuration register 
 * 
 * @details Register 26
 * 
 * @param mapu6050_address 
 * @param ext_sync_set : 
 * @param dlpf_cfg : 
 */
void mpu6050_config_write(
    uint8_t mpu6050_address,
    uint8_t ext_sync_set,
    uint8_t dlpf_cfg);

/**
 * @brief Gyroscope Configuration register 
 * 
 * @details Register 27 
 * 
 * @param mpu6050_address 
 * @param gyro_self_test 
 * @param fs_sel 
 */
void mpu6050_gyro_config_write(
    uint8_t mpu6050_address,
    uint8_t gyro_self_test,
    uint8_t fs_sel);

/**
 * @brief Accelerometer Configuration register 
 * 
 * @details Register 28 
 * 
 * @param mpu6050_address 
 * @param accel_self_test 
 * @param afs_sel 
 */
void mpu6050_accel_config_write(
    uint8_t mpu6050_address,
    uint8_t accel_self_test,
    uint8_t afs_sel);

/**
 * @brief 
 * 
 * @details Register 107 
 * 
 * @param mpu6050_address 
 * @param device_reset 
 * @param sleep 
 * @param cycle 
 * @param temp_dis 
 * @param clksel 
 */
void mpu6050_pwr_mgmt_1_write(
    uint8_t mpu6050_address,
    uint8_t device_reset,
    uint8_t sleep,
    uint8_t cycle,
    uint8_t temp_dis,
    uint8_t clksel);

/**
 * @brief Who Am I register
 * 
 * @details Register 117
 * 
 * @param mpu6050_sddress 
 * @return uint8_t : Returns 0x68 if AD0 == 0 and 0x69 is AD0 == 1
 */
uint8_t mpu6050_who_am_i_read(uint8_t mpu6050_sddress);

//=======================================================================================


#endif  // _MPU6050_DRIVER_H_
