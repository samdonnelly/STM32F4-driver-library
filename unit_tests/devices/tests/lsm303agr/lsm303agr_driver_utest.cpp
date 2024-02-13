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
//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(lsm303agr_driver_test)
{
    // Global test group variables 

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

// // Magnetometer data 
// typedef union lsm303agr_m_data_test_s 
// {
//     uint8_t m_axis_bytes[2]; 
//     int16_t m_axis; 
// }
// lsm303agr_m_data_test_t; 

//=======================================================================================


//=======================================================================================
// Helper functions 
//=======================================================================================


//=======================================================================================
// Tests 

// // M8Q read - Message data record update 
// TEST(lsm303agr_driver_test, test0)
// {
//     lsm303agr_m_data_test_t mag_data[3]; 

//     mag_data[0].m_axis_bytes[0] = 0xC9; 
//     mag_data[0].m_axis_bytes[1] = 0xFF; 
//     mag_data[1].m_axis_bytes[0] = 0x00; 
//     mag_data[1].m_axis_bytes[1] = 0x00; 
//     mag_data[2].m_axis_bytes[0] = 0x00; 
//     mag_data[2].m_axis_bytes[1] = 0x00; 

//     std::cout << "Size = " << sizeof(lsm303agr_m_data_test_t) << std::endl; 
//     std::cout << "x = " << mag_data[0].m_axis << std::endl; 
//     std::cout << "y = " << mag_data[1].m_axis << std::endl; 
//     std::cout << "z = " << mag_data[2].m_axis << std::endl; 
// }


//==================================================
// Data update and read 

// Data update - 
TEST(lsm303agr_driver_test, data_update)
{
    // Data 
    uint8_t axis_bytes[BYTE_6] = { 0xC1, 0x4D, 0x32, 0x9A, 0x57, 0x04 }; 
    int16_t axis_data[3]; 
    int16_t x_axis_check = ((int16_t)((axis_bytes[1] << SHIFT_8) | axis_bytes[0]) * 3) / 2; 
    int16_t y_axis_check = ((int16_t)((axis_bytes[3] << SHIFT_8) | axis_bytes[2]) * 3) / 2; 
    int16_t z_axis_check = ((int16_t)((axis_bytes[5] << SHIFT_8) | axis_bytes[4]) * 3) / 2; 

    // Initialize the mock I2C driver to time out 
    i2c_mock_init(
        I2C_MOCK_TIMEOUT_DISABLE, 
        I2C_MOCK_INC_MODE_DISABLE, 
        I2C_MOCK_INC_MODE_DISABLE); 

    // Set the data to be read by the driver 
    i2c_mock_set_read_data(
        axis_bytes, 
        BYTE_6, 
        I2C_MOCK_INDEX_0); 

    // Update and read the data 
    lsm303agr_m_update_dev(); 
    lsm303agr_m_get_axis_data_dev(axis_data); 

    LONGS_EQUAL(x_axis_check, axis_data[0]); 
    LONGS_EQUAL(y_axis_check, axis_data[1]); 
    LONGS_EQUAL(z_axis_check, axis_data[2]); 
}

//==================================================

//=======================================================================================
