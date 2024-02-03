//=======================================================================================
// Notes 
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

// Magnetometer data 
typedef union lsm303agr_m_data_test_s 
{
    uint8_t m_axis_bytes[2]; 
    int16_t m_axis; 
}
lsm303agr_m_data_test_t; 

//=======================================================================================


//=======================================================================================
// Helper functions 
//=======================================================================================


//=======================================================================================
// Tests 

// M8Q read - Message data record update 
TEST(lsm303agr_driver_test, test0)
{
    lsm303agr_m_data_test_t mag_data[3]; 

    mag_data[0].m_axis_bytes[0] = 0xC9; 
    mag_data[0].m_axis_bytes[1] = 0xFF; 
    mag_data[1].m_axis_bytes[0] = 0x00; 
    mag_data[1].m_axis_bytes[1] = 0x00; 
    mag_data[2].m_axis_bytes[0] = 0x00; 
    mag_data[3].m_axis_bytes[1] = 0x00; 

    std::cout << "Size = " << sizeof(lsm303agr_m_data_test_t) << std::endl; 
    std::cout << "x = " << mag_data[0].m_axis << std::endl; 
    std::cout << "y = " << mag_data[1].m_axis << std::endl; 
    std::cout << "z = " << mag_data[2].m_axis << std::endl; 
}

//=======================================================================================
