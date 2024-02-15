//=======================================================================================
// Notes 

// Tests: 
// - Calibration 
// - Init 
//   - Values in register records 
// - Update & get axis 
// - Update and get heading 

//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

#include <iostream> 

extern "C"
{
	// Add your C-only include files here 
    #include "lsm303agr_driver.h" 
    #include "lsm303agr_config_test.h" 
    #include "i2c_comm.h" 
    #include "i2c_comm_mock.h"
}

//=======================================================================================


//=======================================================================================
// Macros 

#define NO_LPF_GAIN 1 
#define LPF_GAIN 0.2 
#define WHOAMI_REG_ID 0x40 

//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(lsm303agr_driver_test)
{
    // Global test group variables 
    I2C_TypeDef I2C_FAKE; 

    // Constructor 
    void setup()
    {
        // 
    }

    // Destructor 
    void teardown()
    {
        // 
    }
}; 

//=======================================================================================


//=======================================================================================
// Data 

// Device axis index 
typedef enum {
    X_AXIS, 
    Y_AXIS, 
    Z_AXIS, 
    NUM_AXES 
} lsm303agr_test_axis_t; 

//=======================================================================================


//=======================================================================================
// Helper functions 

// Format axis check data 
void lsm303agr_driver_test_axis_check_format(
    const uint8_t *axis_bytes, 
    int16_t *axis_checks)
{
    if ((axis_bytes == NULL) || (axis_checks == NULL))
    {
        return; 
    }

    axis_checks[X_AXIS] = (int16_t)((axis_bytes[1] << SHIFT_8) | axis_bytes[0]); 
    axis_checks[Y_AXIS] = (int16_t)((axis_bytes[3] << SHIFT_8) | axis_bytes[2]); 
    axis_checks[Z_AXIS] = (int16_t)((axis_bytes[5] << SHIFT_8) | axis_bytes[4]); 
}

//=======================================================================================


//=======================================================================================
// Tests 

//==================================================
// Initialization 

// Magnetometer - Initialization - WHO_AM_I incorrect 
TEST(lsm303agr_driver_test, lsm303agr_m_init_fail)
{
    LSM303AGR_STATUS init_check; 
    uint8_t whoami_reg_value = WHOAMI_REG_ID + 1; 

    i2c_mock_init(
        I2C_MOCK_TIMEOUT_DISABLE, 
        I2C_MOCK_INC_MODE_ENABLE, 
        I2C_MOCK_INC_MODE_ENABLE); 

    // Set the WHO_AM_I register data to be read 
    i2c_mock_set_read_data(&whoami_reg_value, BYTE_1, I2C_MOCK_INDEX_0); 

    init_check = lsm303agr_m_init_dev(
        &I2C_FAKE, 
        lsm303agr_calibrate_offsets, 
        NO_LPF_GAIN, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 

    LONGS_EQUAL(LSM303AGR_WHOAMI, init_check); 
}


// Magnetometer - Initialization - WHO_AM_I correct, check data written to registers 
TEST(lsm303agr_driver_test, lsm303agr_m_init_success)
{
    LSM303AGR_STATUS init_check; 
    uint8_t whoami_reg_value = WHOAMI_REG_ID; 
    uint8_t cfga_reg = CLEAR, cfgb_reg = CLEAR, cfgc_reg = CLEAR, data_size; 

    i2c_mock_init(
        I2C_MOCK_TIMEOUT_DISABLE, 
        I2C_MOCK_INC_MODE_ENABLE, 
        I2C_MOCK_INC_MODE_ENABLE); 

    // Set the WHO_AM_I register data to be read 
    i2c_mock_set_read_data(&whoami_reg_value, BYTE_1, I2C_MOCK_INDEX_0); 

    init_check = lsm303agr_m_init_dev(
        &I2C_FAKE, 
        lsm303agr_calibrate_offsets, 
        NO_LPF_GAIN, 
        LSM303AGR_M_ODR_20, 
        LSM303AGR_M_MODE_IDLE, 
        LSM303AGR_CFG_ENABLE, 
        LSM303AGR_CFG_ENABLE, 
        LSM303AGR_CFG_ENABLE, 
        LSM303AGR_CFG_ENABLE); 

    i2c_mock_get_write_data(&cfga_reg, &data_size, I2C_MOCK_INDEX_2); 
    i2c_mock_get_write_data(&cfgb_reg, &data_size, I2C_MOCK_INDEX_4); 
    i2c_mock_get_write_data(&cfgc_reg, &data_size, I2C_MOCK_INDEX_6); 

    LONGS_EQUAL(LSM303AGR_OK, init_check); 
    // The following checks are based on the parameters passed in the init function 
    LONGS_EQUAL(0x06, cfga_reg); 
    LONGS_EQUAL(0x03, cfgb_reg); 
    LONGS_EQUAL(0x41, cfgc_reg); 
}

//==================================================

//==================================================
// Data update and read 

// Magnetometer - Data update successful and not successful 
TEST(lsm303agr_driver_test, lsm303agr_m_data_update_success)
{
    // Initialize the mock I2C driver to not time out, set the data to be read, update 
    // the driver data and check that the update went ok. 
    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_DISABLE); 
    i2c_mock_set_read_data(axis_bytes_0, BYTE_6, I2C_MOCK_INDEX_0); 
    LONGS_EQUAL(LSM303AGR_OK, lsm303agr_m_update_dev()); 

    // Do the same as before but initialize the the mock I2C driver to time out and check 
    // that the update failed. 
    i2c_mock_init(I2C_MOCK_TIMEOUT_ENABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_DISABLE); 
    i2c_mock_set_read_data(axis_bytes_0, BYTE_6, I2C_MOCK_INDEX_0); 
    LONGS_EQUAL(LSM303AGR_READ_FAULT, lsm303agr_m_update_dev()); 
}


// Magnetometer - Data update and axis read 
TEST(lsm303agr_driver_test, lsm303agr_m_data_update_axis_read)
{
    int16_t axis_checks[NUM_AXES], axis_data[NUM_AXES]; 
    lsm303agr_driver_test_axis_check_format(axis_bytes_0, axis_checks); 

    // Initialize the mock I2C driver to not time out 
    i2c_mock_init(
        I2C_MOCK_TIMEOUT_DISABLE, 
        I2C_MOCK_INC_MODE_DISABLE, 
        I2C_MOCK_INC_MODE_DISABLE); 

    // Set the data to be read by the driver 
    i2c_mock_set_read_data(axis_bytes_0, BYTE_6, I2C_MOCK_INDEX_0); 

    // Update and read the data 
    lsm303agr_m_update_dev(); 
    lsm303agr_m_get_axis_data_dev(axis_data); 

    LONGS_EQUAL(axis_checks[X_AXIS], axis_data[X_AXIS]); 
    LONGS_EQUAL(axis_checks[Y_AXIS], axis_data[Y_AXIS]); 
    LONGS_EQUAL(axis_checks[Z_AXIS], axis_data[Z_AXIS]); 
}


// Magnetometer - Data update and applied magnetic field per axis read 
TEST(lsm303agr_driver_test, lsm303agr_m_data_update_field_read)
{
    int32_t field_checks[NUM_AXES], field_data[NUM_AXES]; 
    int16_t axis_data[NUM_AXES]; 
    lsm303agr_driver_test_axis_check_format(axis_bytes_0, axis_data); 

    // Scale the axis data by the device sensitivity to get the applied magnetic field 
    // on each axis. 
    const int16_t sens_num = 3; 
    const int16_t sens_den = 2; 
    field_checks[X_AXIS] = (axis_data[X_AXIS] * sens_num) / sens_den; 
    field_checks[Y_AXIS] = (axis_data[Y_AXIS] * sens_num) / sens_den; 
    field_checks[Z_AXIS] = (axis_data[Z_AXIS] * sens_num) / sens_den; 

    // Initialize the mock I2C driver to not time out 
    i2c_mock_init(
        I2C_MOCK_TIMEOUT_DISABLE, 
        I2C_MOCK_INC_MODE_DISABLE, 
        I2C_MOCK_INC_MODE_DISABLE); 

    // Set the data to be read by the driver 
    i2c_mock_set_read_data(axis_bytes_0, BYTE_6, I2C_MOCK_INDEX_0); 

    // Update and read the data 
    lsm303agr_m_update_dev(); 
    lsm303agr_m_get_field_dev(field_data); 

    LONGS_EQUAL(field_checks[X_AXIS], field_data[X_AXIS]); 
    LONGS_EQUAL(field_checks[Y_AXIS], field_data[Y_AXIS]); 
    LONGS_EQUAL(field_checks[Z_AXIS], field_data[Z_AXIS]); 
}

//==================================================


//==================================================
// Heading and calibration 

// Magnetometer - No offsets, no filter, heading read 
TEST(lsm303agr_driver_test, lsm303agr_m_no_offsets_no_filter_heading)
{
    // int16_t heading; 
    uint8_t whoami_reg_value = WHOAMI_REG_ID; 

    // Initialize the mock I2C driver to not time out 
    i2c_mock_init(
        I2C_MOCK_TIMEOUT_DISABLE, 
        I2C_MOCK_INC_MODE_ENABLE, 
        I2C_MOCK_INC_MODE_ENABLE); 

    // Set the data to be read by the driver 
    i2c_mock_set_read_data(&whoami_reg_value, BYTE_1, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data(axis_bytes_1, BYTE_6, I2C_MOCK_INDEX_1); 
    i2c_mock_set_read_data(axis_bytes_2, BYTE_6, I2C_MOCK_INDEX_2); 
    i2c_mock_set_read_data(axis_bytes_5, BYTE_6, I2C_MOCK_INDEX_3); 
    i2c_mock_set_read_data(axis_bytes_6, BYTE_6, I2C_MOCK_INDEX_4); 
    i2c_mock_set_read_data(axis_bytes_9, BYTE_6, I2C_MOCK_INDEX_5); 
    i2c_mock_set_read_data(axis_bytes_10, BYTE_6, I2C_MOCK_INDEX_6); 
    i2c_mock_set_read_data(axis_bytes_13, BYTE_6, I2C_MOCK_INDEX_7); 
    i2c_mock_set_read_data(axis_bytes_14, BYTE_6, I2C_MOCK_INDEX_8); 

    // Note that for unit testing, only offsets and LPF gain matter when calling the 
    // init function. Other arguments are placeholders and can be ignored. 
    lsm303agr_m_init_dev(
        &I2C_FAKE, 
        lsm303agr_calibrate_offsets, 
        NO_LPF_GAIN, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 

    // Update the device data and calculate the heading 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[0] - lsm303agr_m_get_heading_dev()) <= 1); 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[1] - lsm303agr_m_get_heading_dev()) <= 1); 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[4] - lsm303agr_m_get_heading_dev()) <= 1); 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[5] - lsm303agr_m_get_heading_dev()) <= 1); 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[8] - lsm303agr_m_get_heading_dev()) <= 1); 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[9] - lsm303agr_m_get_heading_dev()) <= 1); 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[12] - lsm303agr_m_get_heading_dev()) <= 1); 
    lsm303agr_m_update_dev(); 
    LONGS_EQUAL(TRUE, abs(heading_checks[13] - lsm303agr_m_get_heading_dev()) <= 1); 
}


// // Magnetometer - No offsets, added filter heading read 
// TEST(lsm303agr_driver_test, lsm303agr_m_no_offsets_added_filter_heading)
// {
//     // Note that for unit testing, only offsets and LPF gain matter when calling the 
//     // init function. Other arguments are placeholders and can be ignored. 
//     lsm303agr_m_init_dev(
//         &I2C_FAKE, 
//         lsm303agr_calibrate_offsets, 
//         LPF_GAIN, 
//         LSM303AGR_M_ODR_10, 
//         LSM303AGR_M_MODE_CONT, 
//         LSM303AGR_CFG_DISABLE, 
//         LSM303AGR_CFG_DISABLE, 
//         LSM303AGR_CFG_DISABLE, 
//         LSM303AGR_CFG_DISABLE); 
// }


// // Magnetometer - Added offsets, no filter heading read 
// TEST(lsm303agr_driver_test, lsm303agr_m_added_offsets_no_filter_heading)
// {
//     // Note that for unit testing, only offsets and LPF gain matter when calling the 
//     // init function. Other arguments are placeholders and can be ignored. 
//     lsm303agr_m_init_dev(
//         &I2C_FAKE, 
//         lsm303agr_calibrate_offsets, 
//         LPF_GAIN, 
//         LSM303AGR_M_ODR_10, 
//         LSM303AGR_M_MODE_CONT, 
//         LSM303AGR_CFG_DISABLE, 
//         LSM303AGR_CFG_DISABLE, 
//         LSM303AGR_CFG_DISABLE, 
//         LSM303AGR_CFG_DISABLE); 

//     // 
//     lsm303agr_m_heading_calibration_dev(lsm303agr_config_dir_offsets); 
// }


// // Magnetometer - Calibration update, data update and heading read 
// TEST(lsm303agr_driver_test, lsm303agr_m_calibration_and_data_update_heading_read)
// {
//     // Initialize the mock I2C driver to not time out and hold multiple data sets 
//     // for reading. 
//     i2c_mock_init(
//         I2C_MOCK_TIMEOUT_DISABLE, 
//         I2C_MOCK_INC_MODE_DISABLE, 
//         I2C_MOCK_INC_MODE_ENABLE); 

//     // Set the data to be read by the driver 
//     i2c_mock_set_read_data(axis_bytes_0, BYTE_6, I2C_MOCK_INDEX_0); 
//     // i2c_mock_set_read_data(axis_bytes_1, BYTE_6, I2C_MOCK_INDEX_1); 
//     // i2c_mock_set_read_data(axis_bytes_2, BYTE_6, I2C_MOCK_INDEX_2); 
//     // i2c_mock_set_read_data(axis_bytes_3, BYTE_6, I2C_MOCK_INDEX_3); 

//     // Repeatedly update the data and check the new heading 
//     lsm303agr_m_update_dev(); 
//     LONGS_EQUAL(1, lsm303agr_m_get_heading_dev()); 

//     // lsm303agr_m_update_dev(); 
//     // LONGS_EQUAL(1, lsm303agr_m_get_heading_dev()); 

//     // lsm303agr_m_update_dev(); 
//     // LONGS_EQUAL(1, lsm303agr_m_get_heading_dev()); 

//     // lsm303agr_m_update_dev(); 
//     // LONGS_EQUAL(1, lsm303agr_m_get_heading_dev()); 
// }

//==================================================

//=======================================================================================
