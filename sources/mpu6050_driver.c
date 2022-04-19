/**
 * @file mpu6050_driver.c
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

//=======================================================================================
// Includes 

#include "mpu6050_driver.h"

//=======================================================================================


//=======================================================================================
// Initialization 

// MPU-6050 Initialization 
uint8_t mpu6050_init(uint8_t mpu6050_address)
{
    //==============================================================
    // Notes: 
    //  - The accelerometer must be woken up from sleep mode and have default 
    //    settings changed on startup. 
    //  - It looks like to init the accelerometer you have to choose how you want it 
    //    configured based on the registers meaning there is no exact way to init 
    //    the device with the exception of the point above. 
    //==============================================================

    //==============================================================
    // MPU-6050 Init
    //  1. Read the WHO_AM_I register to establish that there is communication 
    //  2. Wake the sensor up through the power management register 
    //  3. Set the data rate 
    //  4. Configure the accelerometer register 
    //  5. Configure the gyroscope register 
    //==============================================================

    // TODO create configuration packages for the accelerometer to make this modular 
    // TODO add a section for self test and trigger an error if it fails 

    // 1. Read the WHO_AM_I register to establish that there is communication 
    if (mpu6050_who_am_i_read(mpu6050_address) != MPU6050_7_BIT_ADDRESS)
    {
        return FALSE;
    }

    // 2. Choose which sensors to use and frquency of CYCLE mode (see PWR_MGMT_1)
    mpu6050_pwr_mgmt_2_write(
        mpu6050_address,
        LP_WAKE_CTRL_0,
        STBY_XA_DISABLE,
        STBY_YA_DISABLE,
        STBY_ZA_DISABLE,
        STBY_XG_DISABLE,
        STBY_YG_DISABLE,
        STBY_ZG_DISABLE);

    // 3. Wake the sensor up through the power management 1 register 
    mpu6050_pwr_mgmt_1_write(
        mpu6050_address,
        DEVICE_RESET_DISABLE,
        SLEEP_MODE_DISABLE,
        CYCLE_SLEEP_DISABLED,
        TEMP_SENSOR_ENABLE,
        CLKSEL_5);

    // 4. Set the output rate of the gyro and accelerometer 
    mpu6050_config_write(
        mpu6050_address,
        EXT_SYNC_SET_0,
        DLPF_CFG_1);
    
    // 5. Set the Sample Rate (data rate)
    mpu6050_smprt_div_write(
        mpu6050_address,
        SMPLRT_DIV_0);
    
    // 6. Configure the accelerometer register 
    mpu6050_accel_config_write(
        mpu6050_address,
        ACCEL_SELF_TEST_DISABLE,
        AFS_SEL_4);
    
    // 7. Configure the gyroscope register
    mpu6050_gyro_config_write(
        mpu6050_address,
        GYRO_SELF_TEST_DISABLE,
        FS_SEL_500);

    // Initialization completed successfully 
    return TRUE;
}

//=======================================================================================


//=======================================================================================
// Read and Write Functions 

// Write to the MPU-6050
void mpu6050_write(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register,
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c1_start(); 

    // Send the MPU6050 address with a write offset
    i2c1_write_address(mpu6050_address + MPU6050_W_OFFSET);
    i2c1_clear_addr();

    // Send the register address that is going to be written to 
    i2c1_write_master_mode(&mpu6050_register, MPU6050_REG_1_BYTE);

    // Write the data to the MPU6050 
    i2c1_write_master_mode(mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c1_stop(); 
}

// Read from the MPU-6050
void mpu6050_read(
    uint8_t mpu6050_address, 
    uint8_t mpu6050_register, 
    uint8_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c1_start(); 

    // Send the MPU6050 address with a write offset 
    i2c1_write_address(mpu6050_address + MPU6050_W_OFFSET);
    i2c1_clear_addr();

    // Send the register address that is going to be read 
    i2c1_write_master_mode(&mpu6050_register, MPU6050_REG_1_BYTE);

    // Create another start signal 
    i2c1_start(); 

    // Send the MPU6050 address with a read offset 
    i2c1_write_address(mpu6050_address + MPU6050_R_OFFSET);

    // Read the data sent by the MPU6050 
    i2c1_read_master_mode(mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c1_stop(); 
}

//=======================================================================================


//=======================================================================================
// Register Functions

// SMPRT_DIV write - Register 25
void mpu6050_smprt_div_write(
    uint8_t mpu6050_address,
    uint8_t smprt_div)
{
    // Write to the Sample Rate Divider register
    mpu6050_write(
        mpu6050_address,
        MPU6050_SMPRT_DIV,
        MPU6050_REG_1_BYTE,
        &smprt_div);
}

// CONFIG write - Register 26 
void mpu6050_config_write(
    uint8_t mpu6050_address, 
    uint8_t ext_sync_set,
    uint8_t dlpf_cfg)
{
    // Configure the data
    uint8_t mpu6050_config = (ext_sync_set << SHIFT_3) | (dlpf_cfg << SHIFT_0);

    // Write to the Configuration register 
    mpu6050_write(
        mpu6050_address,
        MPU6050_CONFIG,
        MPU6050_REG_1_BYTE,
        &mpu6050_config);
}

// GYRO_CONFIG write - Register 27 
void mpu6050_gyro_config_write(
    uint8_t mpu6050_address,
    uint8_t gyro_self_test,
    uint8_t fs_sel)
{
    // Configure the data 
    uint8_t mpu6050_gyro_config = (gyro_self_test << SHIFT_5) | (fs_sel << SHIFT_3);

    // Write to the Gyroscope Configuration register 
    mpu6050_write(
        mpu6050_address,
        MPU6050_GYRO_CONFIG,
        MPU6050_REG_1_BYTE,
        &mpu6050_gyro_config);
}

// GYRO_CONFIG read - Register 27
uint8_t mpu6050_gyro_config_read(uint8_t mpu6050_address)
{
    // Place to store the value of GYRO_CONFIG
    uint8_t mpu6050_gyro_config;

    // Read the value ofGYRO_CONFIG register 
    mpu6050_read(
        mpu6050_address, 
        MPU6050_GYRO_CONFIG, 
        MPU6050_REG_1_BYTE,
        &mpu6050_gyro_config);

    // Return the value of GYRO_CONFIG 
    return mpu6050_gyro_config;
}

// ACCEL_CONFIG write - Register 28 
void mpu6050_accel_config_write(
    uint8_t mpu6050_address,
    uint8_t accel_self_test,
    uint8_t afs_sel)
{
    // Configure the data 
    uint8_t mpu6050_accel_config = (accel_self_test << SHIFT_5) | (afs_sel << SHIFT_3);

    // Write to the Gyroscope Configuration register 
    mpu6050_write(
        mpu6050_address,
        MPU6050_ACCEL_CONFIG,
        MPU6050_REG_1_BYTE,
        &mpu6050_accel_config);
}

// ACCEL_CONFIG read - Register 28 
uint8_t mpu6050_accel_config_read(uint8_t mpu6050_address)
{
    // Place to store the value of ACCEL_CONFIG
    uint8_t mpu6050_accel_config;

    // Read the value of ACCEL_CONFIG register 
    mpu6050_read(
        mpu6050_address, 
        MPU6050_ACCEL_CONFIG, 
        MPU6050_REG_1_BYTE,
        &mpu6050_accel_config);

    // Return the value of ACCEL_CONFIG 
    return mpu6050_accel_config;
}

// ACCEL_OUT - Registers 59-64
void mpu6050_accel_read(
    uint8_t  mpu6050_address,
    uint16_t *accel_data)
{
    // Temporary data storage 
    uint8_t accel_data_reg_val[MPU6050_REG_6_BYTE];

    // Read the accelerometer data 
    mpu6050_read(
        mpu6050_address,
        MPU6050_ACCEL_XOUT_H,
        MPU6050_REG_6_BYTE,
        accel_data_reg_val);
    
    // Combine the return values into signed integers - values are unformatted
    for (uint8_t i = 0; i < MPU6050_NUM_ACCEL_AXIS; i++)
    {
        // Read consecutive bytes 3 times to form 16-bit values for each axis
        *accel_data = (uint16_t)((accel_data_reg_val[2*i]   << SHIFT_8) |
                                 (accel_data_reg_val[2*i+1] << SHIFT_0));
        accel_data++;
    } 
}

// TEMP_OUT - Registers 65-66
// TODO create a lookup table to check for temperature ranges and trigger errors
uint16_t mpu6050_temp_read(uint8_t mpu6050_address)
{
    // Store the temperature data 
    uint16_t mpu6050_temp_sensor_val;
    uint8_t  mpu6050_temp_sensor_reg_val[MPU6050_REG_2_BYTE];

    // Read the temperature data 
    mpu6050_read(
        mpu6050_address,
        MPU6050_TEMP_OUT_H,
        MPU6050_REG_2_BYTE,
        mpu6050_temp_sensor_reg_val);
    
    // Combine the return values into a signed integer - value is unformatted 
    mpu6050_temp_sensor_val = (uint16_t)((mpu6050_temp_sensor_reg_val[0] << SHIFT_8)  |
                                         (mpu6050_temp_sensor_reg_val[1] << SHIFT_0));

    // Return unformatted temperature 
    return mpu6050_temp_sensor_val;
}

// GYRO_OUT - Registers 67-72
void mpu6050_gyro_read(
    uint8_t  mpu6050_address,
    uint16_t *gyro_data)
{
    // Temporary data storage 
    uint8_t gyro_data_reg_val[MPU6050_REG_6_BYTE];

    // Read the accelerometer data 
    mpu6050_read(
        mpu6050_address,
        MPU6050_ACCEL_XOUT_H,
        MPU6050_REG_6_BYTE,
        gyro_data_reg_val);
    
    // Combine the return values into signed integers - values are unformatted
    for (uint8_t i = 0; i < MPU6050_NUM_GYRO_AXIS; i++)
    {
        // Read consecutive bytes 3 times to form 16-bit values for each axis
        *gyro_data = (uint16_t)((gyro_data_reg_val[2*i]   << SHIFT_8) |
                                (gyro_data_reg_val[2*i+1] << SHIFT_0));
        gyro_data++;
    } 
}

// PWR_MGMT_1 write - Register 107 
void mpu6050_pwr_mgmt_1_write(
    uint8_t mpu6050_address,
    uint8_t device_reset,
    uint8_t sleep,
    uint8_t cycle,
    uint8_t temp_dis,
    uint8_t clksel)
{
    // Configure the data
    uint8_t mpu6050_pwr_mgmt_1 = (device_reset << SHIFT_7) |
                                 (sleep        << SHIFT_6) |
                                 (cycle        << SHIFT_5) |
                                 (temp_dis     << SHIFT_3) |
                                 (clksel       << SHIFT_0);
    
    // Write to the Power Management 1 register 
    mpu6050_write(
        mpu6050_address,
        MPU6050_PWR_MGMT_1,
        MPU6050_REG_1_BYTE,
        &mpu6050_pwr_mgmt_1);
}

// PWR_MGMT_2 write - Register 108 
void mpu6050_pwr_mgmt_2_write(
    uint8_t mpu6050_address,
    uint8_t lp_wake_ctrl,
    uint8_t stby_xa,
    uint8_t stby_ya,
    uint8_t stby_za,
    uint8_t stby_xg,
    uint8_t stby_yg,
    uint8_t stby_zg)
{
    // Configure the data 
    uint8_t mpu6050_pwr_mgmt_2 = (lp_wake_ctrl << SHIFT_6) |
                                 (stby_xa      << SHIFT_5) |
                                 (stby_ya      << SHIFT_4) |
                                 (stby_za      << SHIFT_3) |
                                 (stby_xg      << SHIFT_2) |
                                 (stby_yg      << SHIFT_1) |
                                 (stby_zg      << SHIFT_0);
    
    // Write to the Power Management 2 register 
    mpu6050_write(
        mpu6050_address,
        MPU6050_PWR_MGMT_2,
        MPU6050_REG_1_BYTE,
        &mpu6050_pwr_mgmt_2);
}

// WHO_AM_I read - Register 117 
uint8_t mpu6050_who_am_i_read(uint8_t mpu6050_address)
{
    // Place to store the value WHO_AM_I
    uint8_t mpu6050_who_am_i;

    // Read the value of WHO_AM_I register 
    mpu6050_read(
        mpu6050_address, 
        MPU6050_WHO_AM_I, 
        MPU6050_REG_1_BYTE,
        &mpu6050_who_am_i);

    // Return the value of who_am_i 
    return mpu6050_who_am_i;
}

//=======================================================================================


//=======================================================================================
// Calculation Functions 

// 
float mpu6050_accel_x_calc(uint8_t mpu6050_address, uint16_t accel_x_axis_raw)
{
    // Get the raw value scalar and calculate the true z-axis acceleration
    float accel_scalar = mpu6050_accel_scalar(mpu6050_address);
    float accel_x_axis = ((int16_t)(accel_x_axis_raw)) / accel_scalar;
    
    // Return the true acceleration 
    return accel_x_axis;
}

// 
float mpu6050_accel_y_calc(uint8_t mpu6050_address, uint16_t accel_y_axis_raw)
{
    // Get the raw value scalar and calculate the true z-axis acceleration
    float accel_scalar = mpu6050_accel_scalar(mpu6050_address);
    float accel_y_axis = ((int16_t)(accel_y_axis_raw)) / accel_scalar;
    
    // Return the true acceleration 
    return accel_y_axis;
}

// 
float mpu6050_accel_z_calc(uint8_t mpu6050_address, uint16_t accel_z_axis_raw)
{
    // Get the raw value scalar and calculate the true z-axis acceleration
    float accel_scalar = mpu6050_accel_scalar(mpu6050_address);
    float accel_z_axis = ((int16_t)(accel_z_axis_raw)) / accel_scalar;
    
    // Return the true acceleration 
    return accel_z_axis;
}

// 
float mpu6050_accel_scalar(uint8_t mpu6050_address)
{
    float accel_scalar;

    // Get AFS_SEL
    uint8_t afs_sel = ((mpu6050_accel_config_read(mpu6050_address) & 
                        MPU6050_AFS_SEL_MASK) >> SHIFT_3);
    
    // Return the scalar 
    switch (afs_sel)
    {
        case AFS_SEL_2:
            accel_scalar = (float)(ACCEL_SCALE_FS_SEL_2);
            break;
        
        case AFS_SEL_4:
            accel_scalar = (float)(ACCEL_SCALE_FS_SEL_4);
            break;
        
        case AFS_SEL_8:
            accel_scalar = (float)(ACCEL_SCALE_FS_SEL_8);
            break;
        
        case AFS_SEL_16:
            accel_scalar = (float)(ACCEL_SCALE_FS_SEL_16);
            break;
        
        default:
            accel_scalar = 0;
            break;
    }

    return accel_scalar;
}

// 
float mpu6050_temp_calc(uint16_t temp_raw)
{
    // Get the true temperature in degC
    float temperature = ((int16_t)(temp_raw)) / ((float)(MPU6050_TEMP_SENSIT)) + 
                        MPU6050_TEMP_OFFSET / ((float)(MPU6050_TEMP_SCALAR)); 

    // Return the true temperature 
    return temperature;
}

// 
float mpu6050_gyro_x_calc(uint8_t mpu6050_address, uint16_t gyro_x_axis_raw)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration
    float gyro_scalar = mpu6050_gyro_scalar(mpu6050_address);
    float gyro_x_axis = ((int16_t)(gyro_x_axis_raw)) / gyro_scalar;
    
    // Return the true angular acceleration
    return gyro_x_axis;
}

// 
float mpu6050_gyro_y_calc(uint8_t mpu6050_address, uint16_t gyro_y_axis_raw)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration
    float gyro_scalar = mpu6050_gyro_scalar(mpu6050_address);
    float gyro_y_axis = ((int16_t)(gyro_y_axis_raw)) / gyro_scalar;
    
    // Return the true angular acceleration
    return gyro_y_axis;
}

// 
float mpu6050_gyro_z_calc(uint8_t mpu6050_address, uint16_t gyro_z_axis_raw)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration
    float gyro_scalar = mpu6050_gyro_scalar(mpu6050_address);
    float gyro_z_axis = ((int16_t)(gyro_z_axis_raw)) / gyro_scalar;
    
    // Return the true angular acceleration
    return gyro_z_axis;
}

// 
float mpu6050_gyro_scalar(uint8_t mpu6050_address)
{
    float gyro_scalar;

    // Get AFS_SEL
    uint8_t afs_sel = ((mpu6050_gyro_config_read(mpu6050_address) & 
                        MPU6050_FS_SEL_MASK) >> SHIFT_3);
    
    // Return the scalar 
    switch (afs_sel)
    {
        case FS_SEL_250:
            gyro_scalar = (float)(GYRO_SCALE_FS_SEL_250);
            break;
        
        case FS_SEL_500:
            gyro_scalar = (float)(GYRO_SCALE_FS_SEL_500);
            break;
        
        case FS_SEL_1000:
            gyro_scalar = (float)(GYRO_SCALE_FS_SEL_1000);
            break;
        
        case FS_SEL_2000:
            gyro_scalar = (float)(GYRO_SCALE_FS_SEL_2000);
            break;
        
        default:
            gyro_scalar = 0;
            break;
    }

    // Divide the scalar to restore decimal places
    return (gyro_scalar / (float)(MPU6050_GYRO_SCALAR_SCALAR));
}

//=======================================================================================
