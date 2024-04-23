/**
 * @file nrf24l01_driver_utest.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 driver unit tests 
 * 
 * @version 0.1
 * @date 2024-03-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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
    #include "nrf24l01_driver.h" 
    #include "nrf24l01_config_test.h" 
    #include "spi_comm.h" 
    #include "spi_comm_mock.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 



//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(nrf24l01_driver_test)
{
    // Global test group variables 
    SPI_TypeDef SPI_FAKE; 

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
// Helper functions 



//=======================================================================================


//=======================================================================================
// Tests 

// RF_SETUP 
TEST(nrf24l01_driver_test, rf_setup)
{
    uint8_t mock_data = 32; 
    // uint8_t mock_data_size = CLEAR; 

    // Set up mock 
    spi_mock_init(SPI_MOCK_TIMEOUT_DISABLE, SPI_MOCK_INC_MODE_ENABLE, SPI_MOCK_INC_MODE_ENABLE); 
    spi_mock_set_read_data((void *)&mock_data, BYTE_1, SPI_MOCK_INDEX_1); 

    nrf24l01_data_rate_t data_rate = nrf24l01_get_rf_dr(); 

    // nrf24l01_set_rf_dr(NRF24L01_DR_250KBPS); 
    // spi_mock_get_write_data((void *)&mock_data, &mock_data_size, SPI_MOCK_INDEX_0); 

    std::cout << std::endl << (size_t)data_rate << std::endl; 
    // std::cout << std::endl << (size_t)mock_data << std::endl; 
}

//=======================================================================================
