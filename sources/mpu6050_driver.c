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

//==============================================================
// MPU-6050 Init
//  1. Read the WHO_AM_I register to establish that there is communication 
//  2. Choose which sensors to use and frquency of CYCLE mode (see PWR_MGMT_1)
//  3. Wake the sensor up through the power management 1 register 
//  4. Set the output rate of the gyro and accelerometer 
//  5. Set the Sample Rate (data rate) 
//  6. Configure the accelerometer register 
//  7. Configure the gyroscope register
//==============================================================

// MPU-6050 Initialization 
uint8_t mpu6050_init(
    uint8_t mpu6050_address,
    uint8_t dlpf_cfg,
    uint8_t smplrt_div,
    uint8_t afs_sel,
    uint8_t fs_sel)
{
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
        dlpf_cfg);
    
    // 5. Set the Sample Rate (data rate)
    mpu6050_smprt_div_write(
        mpu6050_address,
        smplrt_div);
    
    // 6. Configure the accelerometer register 
    mpu6050_accel_config_write(
        mpu6050_address,
        ACCEL_SELF_TEST_DISABLE,
        afs_sel);
    
    // 7. Configure the gyroscope register
    mpu6050_gyro_config_write(
        mpu6050_address,
        GYRO_SELF_TEST_DISABLE,
        fs_sel);

    // Initialization completed successfully 
    return TRUE;
}

// 
void mpu6050_calibrate(
    uint8_t mpu6050_address, 
    int16_t *mpu6050_accel_offset,
    int16_t *mpu6050_gyro_offset)
{
    // 
    mpu6050_accel_read(MPU6050_1_ADDRESS, mpu6050_accel_offset);

    // 
    mpu6050_gyro_read(MPU6050_1_ADDRESS, mpu6050_gyro_offset);
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

// SELF_TEST read - Registers 13-16 
void mpu6050_self_test_read(
    uint8_t mpu6050_address,
    uint8_t *accel_st_data,
    uint8_t *gyro_st_data)
{
    // Store unformatted datat 
    uint8_t st_data[MPU6050_REG_4_BYTE];

    // Read the value of SELF_TEST registers 
    mpu6050_read(
        mpu6050_address, 
        MPU6050_SELF_TEST, 
        MPU6050_REG_4_BYTE,
        st_data);
    
    // Parse the data 

    // Acceleration in X
    *accel_st_data = ((st_data[MPU6050_REG_IDX_0] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                     ((st_data[MPU6050_REG_IDX_3] & SELF_TEST_MASK_XA_TEST_LO) >> SHIFT_4);
    accel_st_data++;

    // Acceleration in Y
    *accel_st_data = ((st_data[MPU6050_REG_IDX_1] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                     ((st_data[MPU6050_REG_IDX_3] & SELF_TEST_MASK_YA_TEST_LO) >> SHIFT_2);
    accel_st_data++;

    // Acceleration in Z
    *accel_st_data = ((st_data[MPU6050_REG_IDX_2] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                     ((st_data[MPU6050_REG_IDX_3] & SELF_TEST_MASK_ZA_TEST_LO) >> SHIFT_0);

    // Gyro in X
    *gyro_st_data = st_data[MPU6050_REG_IDX_0] & SELF_TEST_MASK_X_TEST;
    gyro_st_data++;

    // Gyro in Y
    *gyro_st_data = st_data[MPU6050_REG_IDX_1] & SELF_TEST_MASK_X_TEST;
    gyro_st_data++;

    // Gyro in Z
    *gyro_st_data = st_data[MPU6050_REG_IDX_2] & SELF_TEST_MASK_X_TEST;
}

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

    // Read the value of GYRO_CONFIG register 
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
    int16_t *accel_data)
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
        *accel_data = (int16_t)((accel_data_reg_val[2*i]   << SHIFT_8) |
                                (accel_data_reg_val[2*i+1] << SHIFT_0));
        accel_data++;
    } 
}

// TEMP_OUT - Registers 65-66
int16_t mpu6050_temp_read(uint8_t mpu6050_address)
{
    // Store the temperature data 
    int16_t mpu6050_temp_sensor_val;
    uint8_t mpu6050_temp_sensor_reg_val[MPU6050_REG_2_BYTE];

    // Read the temperature data 
    mpu6050_read(
        mpu6050_address,
        MPU6050_TEMP_OUT_H,
        MPU6050_REG_2_BYTE,
        mpu6050_temp_sensor_reg_val);
    
    // Combine the return values into a signed integer - value is unformatted 
    mpu6050_temp_sensor_val = (int16_t)((mpu6050_temp_sensor_reg_val[0] << SHIFT_8)  |
                                        (mpu6050_temp_sensor_reg_val[1] << SHIFT_0));

    // Return unformatted temperature 
    return mpu6050_temp_sensor_val;
}

// GYRO_OUT - Registers 67-72
void mpu6050_gyro_read(
    uint8_t  mpu6050_address,
    int16_t *gyro_data)
{
    // Temporary data storage 
    uint8_t gyro_data_reg_val[MPU6050_REG_6_BYTE];

    // Read the gyroscope data 
    mpu6050_read(
        mpu6050_address,
        MPU6050_GYRO_XOUT_H,
        MPU6050_REG_6_BYTE,
        gyro_data_reg_val);
    
    // Combine the return values into signed integers - values are unformatted
    for (uint8_t i = 0; i < MPU6050_NUM_GYRO_AXIS; i++)
    {
        // Read consecutive bytes 3 times to form 16-bit values for each axis
        *gyro_data = (int16_t)((gyro_data_reg_val[2*i]   << SHIFT_8) |
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
// Self test functions 

//==============================================================
// Steps for self test 
//  1. Record the full scale range set in the init function 
//  2. Set the full scale range of the accel to +/- 8g and gyro to +/- 250 deg/s
//  3. Read and store the sensor values during non-self-test
//  4. Enable self test 
//  5. Read and store the sensor values during self-test
//  6. Read the self-test registers
//  7. Calculate the factory trim
//  8. Calculate self-test response
//  9. Calculate the change from factory trim and check against the acceptable range
//  10. Disable self test and set the full scale ranges back to their original values
//============================================================== 

// Self Test 
uint8_t mpu6050_self_test(uint8_t mpu6050_address)
{
    // Full scale range 
    uint8_t accel_fsr;
    uint8_t gyro_fsr;

    // Sensor readings with no self-test 
    int16_t accel_no_st[MPU6050_NUM_ACCEL_AXIS];
    int16_t gyro_no_st[MPU6050_NUM_GYRO_AXIS];

    // Sensor readings with self-test 
    int16_t accel_st[MPU6050_NUM_ACCEL_AXIS];
    int16_t gyro_st[MPU6050_NUM_GYRO_AXIS];

    // Self-test result 
    int16_t accel_str[MPU6050_NUM_ACCEL_AXIS];
    int16_t gyro_str[MPU6050_NUM_GYRO_AXIS];

    // Self-test register readings 
    uint8_t accel_test[MPU6050_NUM_ACCEL_AXIS];
    uint8_t gyro_test[MPU6050_NUM_GYRO_AXIS];

    // Factory trim calculation 
    float accel_ft[MPU6050_NUM_ACCEL_AXIS];
    float gyro_ft[MPU6050_NUM_GYRO_AXIS];

    // Status of the self-test 
    uint8_t self_test_result = 0;


    // 1. Record the full scale range set in the init function 
    accel_fsr = ((mpu6050_accel_config_read(mpu6050_address) & 
                  MPU6050_AFS_SEL_MASK) >> SHIFT_3);
    gyro_fsr  = ((mpu6050_gyro_config_read(mpu6050_address) & 
                  MPU6050_FS_SEL_MASK) >> SHIFT_3);

    // 2. Set the full scale range of the accel to +/- 8g and gyro to +/- 250 deg/s
    mpu6050_accel_config_write(
        mpu6050_address,
        ACCEL_SELF_TEST_DISABLE,
        AFS_SEL_8);
    
    mpu6050_gyro_config_write(
        mpu6050_address,
        GYRO_SELF_TEST_DISABLE,
        FS_SEL_250);

    // 3. Read and store the sensor values during non-self-test
    mpu6050_accel_read(mpu6050_address, accel_no_st);
    mpu6050_gyro_read(mpu6050_address, gyro_no_st);

    // 4. Enable self test 
    mpu6050_accel_config_write(
        mpu6050_address,
        ACCEL_SELF_TEST_ENABLE,
        AFS_SEL_8);
    
    mpu6050_gyro_config_write(
        mpu6050_address,
        GYRO_SELF_TEST_ENABLE,
        FS_SEL_250);
    
    // 5. Read and store the sensor values during self-test
    mpu6050_accel_read(mpu6050_address, accel_st);
    mpu6050_gyro_read(mpu6050_address, gyro_st);
    
    // 6. Read the self-test registers
    mpu6050_self_test_read(
        mpu6050_address,
        accel_test,
        gyro_test);
    
    // 7. Calculate the factory trim
    mpu6050_accel_ft(accel_test, accel_ft);
    mpu6050_gyro_ft(gyro_test, gyro_ft);
    
    // 8. Calculate self-test response
    mpu6050_str_calc(
        accel_str,
        accel_no_st,
        accel_st,
        MPU6050_NUM_ACCEL_AXIS);
    
    mpu6050_str_calc(
        gyro_str,
        gyro_no_st,
        gyro_st,
        MPU6050_NUM_GYRO_AXIS);
    
    // 9. Calculate the change from factory trim and check against the acceptable range
    self_test_result = mpu6050_self_test_accel_result(
                                            accel_str,
                                            accel_ft,
                                            self_test_result);
    
    self_test_result = mpu6050_self_test_gyro_result(
                                            gyro_str,
                                            gyro_ft,
                                            self_test_result);
    
    // 10. Disable self test and set the full scale ranges back to their original values
    mpu6050_accel_config_write(
        mpu6050_address,
        ACCEL_SELF_TEST_DISABLE,
        accel_fsr);

    mpu6050_gyro_config_write(
        mpu6050_address,
        GYRO_SELF_TEST_DISABLE,
        gyro_fsr);
    
    // Return the result 
    return self_test_result;
}


// Accelerometer factory trim calculation 
void mpu6050_accel_ft(uint8_t *a_test, float *accel_ft)
{
    // Constants 
    float c1 = SELF_TEST_ACCEL_FT_C1 / ((float)(DIVIDE_10000));
    float c2 = SELF_TEST_ACCEL_FT_C2 / ((float)(DIVIDE_10000));
    float c3 = SELF_TEST_ACCEL_FT_C3 / ((float)(DIVIDE_1000));
    float c4 = SELF_TEST_ACCEL_FT_C4 / ((float)(DIVIDE_10));

    // Determine the factory trim 
    for (uint8_t i = 0; i < MPU6050_NUM_ACCEL_AXIS; i++)
    {
        if (*a_test == 0)
        {
            *accel_ft = 0;
        }
        else
        {
            *accel_ft = (*a_test)*((*a_test)*(c1*(*a_test) + c2) + c3) + c4;
        }

        accel_ft++;
        a_test++;
    }
}


// Gyroscope factory trim calculation 
void mpu6050_gyro_ft(uint8_t *g_test, float *gyro_ft)
{
    // Constants 
    float c1 = SELF_TEST_GYRO_FT_C1 / ((float)(DIVIDE_10000));
    float c2 = SELF_TEST_GYRO_FT_C2 / ((float)(DIVIDE_10000));
    float c3 = SELF_TEST_GYRO_FT_C3 / ((float)(DIVIDE_100));
    float c4 = SELF_TEST_GYRO_FT_C4 / ((float)(DIVIDE_10));

    // Determine the factory trim 
    for (uint8_t i = 0; i < MPU6050_NUM_GYRO_AXIS; i++)
    {
        if (*g_test == 0)
        {
            *gyro_ft = 0;
        }
        else 
        {
            *gyro_ft = (*g_test)*((*g_test)*(c1*(*g_test) + c2) + c3) + c4;
        }

        if (i == 1)
        {
            *gyro_ft = -(*gyro_ft);
        }

        gyro_ft++;
        g_test++;
    }
}


// Calculate the self test response
void mpu6050_str_calc(
    int16_t *self_test_response,
    int16_t *no_self_test,
    int16_t *self_test,
    uint8_t num_axes)
{
    for (uint8_t i = 0; i < num_axes; i++)
    {
        *self_test_response = *self_test - *no_self_test;
        self_test_response++;
        self_test++;
        no_self_test++;
    }
}


// 
uint8_t mpu6050_self_test_accel_result(
    int16_t *accel_self_test_results,
    float  *accel_factory_trim,
    uint8_t self_test_results)
{
    // Place to store the % change 
    float ft_change;

    // Determine the result of the self test
    for (uint8_t i = 0; i < MPU6050_NUM_ACCEL_AXIS; i++)
    {
        // Check % change from factory trim 
        ft_change = (*accel_self_test_results - *accel_factory_trim) / *accel_factory_trim;
        
        // Check change against maximum allowed value
        if ((ft_change > MPU6050_ACCEL_FT_MAX_ERROR) ||
            (ft_change < -(MPU6050_ACCEL_FT_MAX_ERROR)))
        {
            self_test_results |= (SELF_TEST_RESULT_SHIFT_ACCEL << SHIFT_1*i);
        }

        accel_self_test_results++;
        accel_factory_trim++;
    }

    return self_test_results;
}


// 
uint8_t mpu6050_self_test_gyro_result(
    int16_t *gyro_self_test_results,
    float *gyro_factory_trim,
    uint8_t self_test_results)
{
    // Place to store the % change 
    float ft_change;

    // Determine the result of the self test
    for (uint8_t i = 0; i < MPU6050_NUM_GYRO_AXIS; i++)
    {
        // Check % change from factory trim 
        ft_change = (*gyro_self_test_results - *gyro_factory_trim) / *gyro_factory_trim;
        
        // Check change against maximum allowed value
        if ((ft_change > MPU6050_GYRO_FT_MAX_ERROR) ||
            (ft_change < -(MPU6050_GYRO_FT_MAX_ERROR)))
        {
            self_test_results |= (SELF_TEST_RESULT_SHIFT_GYRO << SHIFT_1*i);
        }

        gyro_self_test_results++;
        gyro_factory_trim++;
    }

    return self_test_results;
}

//=======================================================================================


//=======================================================================================
// Calculation Functions 

// 
float mpu6050_accel_x_calc(uint8_t mpu6050_address, int16_t accel_x_axis_raw)
{
    // Get the raw value scalar and calculate the true x-axis acceleration
    float accel_scalar = mpu6050_accel_scalar(mpu6050_address);
    float accel_x_axis = accel_x_axis_raw / accel_scalar;
    
    // Return the true acceleration 
    return accel_x_axis;
}

// 
float mpu6050_accel_y_calc(uint8_t mpu6050_address, int16_t accel_y_axis_raw)
{
    // Get the raw value scalar and calculate the true y-axis acceleration
    float accel_scalar = mpu6050_accel_scalar(mpu6050_address);
    float accel_y_axis = accel_y_axis_raw / accel_scalar;
    
    // Return the true acceleration 
    return accel_y_axis;
}

// 
float mpu6050_accel_z_calc(uint8_t mpu6050_address, int16_t accel_z_axis_raw)
{
    // Get the raw value scalar and calculate the true z-axis acceleration
    float accel_scalar = mpu6050_accel_scalar(mpu6050_address);
    float accel_z_axis = accel_z_axis_raw / accel_scalar;
    
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
            accel_scalar = (float)(ACCEL_SCALE_AFS_SEL_2);
            break;
        
        case AFS_SEL_4:
            accel_scalar = (float)(ACCEL_SCALE_AFS_SEL_4);
            break;
        
        case AFS_SEL_8:
            accel_scalar = (float)(ACCEL_SCALE_AFS_SEL_8);
            break;
        
        case AFS_SEL_16:
            accel_scalar = (float)(ACCEL_SCALE_AFS_SEL_16);
            break;
        
        default:
            accel_scalar = 0;
            break;
    }

    return accel_scalar;
}

// 
float mpu6050_temp_calc(int16_t temp_raw)
{
    // Get the true temperature in degC
    float temperature = temp_raw / ((float)(MPU6050_TEMP_SENSIT)) + 
                        MPU6050_TEMP_OFFSET / ((float)(MPU6050_TEMP_SCALAR)); 

    // Return the true temperature 
    return temperature;
}

// 
float mpu6050_gyro_x_calc(
    uint8_t mpu6050_address, 
    int16_t gyro_x_axis_raw,
    int16_t gyro_x_axis_offset)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration
    float gyro_scalar = mpu6050_gyro_scalar(mpu6050_address);
    float gyro_x_axis = (gyro_x_axis_raw - gyro_x_axis_offset) / gyro_scalar;
    
    // Return the true angular acceleration
    return gyro_x_axis;
}

// 
float mpu6050_gyro_y_calc(
    uint8_t mpu6050_address, 
    int16_t gyro_y_axis_raw,
    int16_t gyro_y_axis_offset)
{
    // Get the raw value scalar and calculate the true y-axis angular acceleration
    float gyro_scalar = mpu6050_gyro_scalar(mpu6050_address);
    float gyro_y_axis = (gyro_y_axis_raw - gyro_y_axis_offset) / gyro_scalar;
    
    // Return the true angular acceleration
    return gyro_y_axis;
}

// 
float mpu6050_gyro_z_calc(
    uint8_t mpu6050_address, 
    int16_t gyro_z_axis_raw,
    int16_t gyro_z_axis_offset)
{
    // Get the raw value scalar and calculate the true z-axis angular acceleration
    float gyro_scalar = mpu6050_gyro_scalar(mpu6050_address);
    float gyro_z_axis = (gyro_z_axis_raw - gyro_z_axis_offset) / gyro_scalar;
    
    // Return the true angular acceleration
    return gyro_z_axis;
}

// 
float mpu6050_gyro_scalar(uint8_t mpu6050_address)
{
    float gyro_scalar;

    // Get FS_SEL
    uint8_t fs_sel = ((mpu6050_gyro_config_read(mpu6050_address) & 
                       MPU6050_FS_SEL_MASK) >> SHIFT_3);
    
    // Return the scalar 
    switch (fs_sel)
    {
        case FS_SEL_250:
            gyro_scalar = GYRO_SCALE_FS_SEL_250 / ((float)(MPU6050_GYRO_SCALAR_SCALAR));
            break;
        
        case FS_SEL_500:
            gyro_scalar = GYRO_SCALE_FS_SEL_500 / ((float)(MPU6050_GYRO_SCALAR_SCALAR));
            break;
        
        case FS_SEL_1000:
            gyro_scalar = GYRO_SCALE_FS_SEL_1000 / ((float)(MPU6050_GYRO_SCALAR_SCALAR));
            break;
        
        case FS_SEL_2000:
            gyro_scalar = GYRO_SCALE_FS_SEL_2000 / ((float)(MPU6050_GYRO_SCALAR_SCALAR));
            break;
        
        default:
            gyro_scalar = 0;
            break;
    }

    // Divide the scalar to restore decimal places
    return gyro_scalar;
}

//=======================================================================================
