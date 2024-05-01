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

#define RF_CH_GET_DATA_0 0x55 
#define RF_CH_GET_DATA_1 0x2A 

#define RF_CH_SET_DATA_0 0x47 
#define RF_CH_SET_DATA_1 0x38 

#define RF_SETUP_GET_DATA_0 0x26   // 250kbps and 0dbm 
#define RF_SETUP_GET_DATA_1 0x08   // 2Mbps and 18dbm 

#define RF_SETUP_SET_DATA_0 0x24   // 250kbps and 6dbm 
#define RF_SETUP_SET_DATA_1 0x02   // 1Mbps and 12dbm 

#define CONFIG_GET_DATA_0 0x03 
#define CONFIG_GET_DATA_1 0xFC 
#define CONFIG_GET_DATA_2 0x03 
#define CONFIG_GET_DATA_3 0x01 

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
        // Initialize mock data 
        spi_mock_init(
            SPI_MOCK_TIMEOUT_DISABLE, 
            SPI_MOCK_INC_MODE_ENABLE, 
            SPI_MOCK_INC_MODE_ENABLE); 
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
// Initialization 
//=======================================================================================


//=======================================================================================
// Send and Receive payload 
//=======================================================================================


//=======================================================================================
// RF_CH register test 

// RF_CH register bits 
// Bits 0-6: rf_ch --> Frequency channel operated on 
// Bit 7: unused_1 --> Reserved - must be low/0 

// RF_CH - read and get data 
TEST(nrf24l01_driver_test, rf_ch_get_data)
{
    // See the bit legend above and the getter checks below to see where these numbers 
    // come from. 
    uint8_t rf_ch_mock_0 = RF_CH_GET_DATA_0; 
    uint8_t rf_ch_mock_1 = RF_CH_GET_DATA_1; 

    uint8_t rf_ch; 

    // Set up mock data. Reading from the device involves two operations: writing the 
    // command / reading the status back, then reading the data requested. We only care 
    // about the data so we set indexes 1 and 3 to skip the command part. 
    spi_mock_set_read_data((void *)&rf_ch_mock_0, BYTE_1, SPI_MOCK_INDEX_1); 
    spi_mock_set_read_data((void *)&rf_ch_mock_1, BYTE_1, SPI_MOCK_INDEX_3); 

    // Read the mock data and check the register contents. 
    nrf24l01_rf_ch_read(); 
    rf_ch = nrf24l01_get_rf_ch(); 

    LONGS_EQUAL(RF_CH_GET_DATA_0, rf_ch); 

    nrf24l01_rf_ch_read(); 
    rf_ch = nrf24l01_get_rf_ch(); 

    LONGS_EQUAL(RF_CH_GET_DATA_1, rf_ch); 
}


// RF_CH - set and write data 
TEST(nrf24l01_driver_test, rf_ch_set_data)
{
    uint8_t rf_ch_mock = CLEAR; 
    uint8_t rf_ch_mock_size = CLEAR; 

    nrf24l01_set_rf_ch(RF_CH_SET_DATA_0); 
    nrf24l01_rf_ch_write(); 

    // See the bit legend and setters above to see where the data check value come from 
    spi_mock_get_write_data((void *)&rf_ch_mock, &rf_ch_mock_size, SPI_MOCK_INDEX_0); 
    LONGS_EQUAL(RF_CH_SET_DATA_0, rf_ch_mock); 
    LONGS_EQUAL(BYTE_1, rf_ch_mock_size); 

    nrf24l01_set_rf_ch(RF_CH_SET_DATA_1); 
    nrf24l01_rf_ch_write(); 

    // See the bit legend and setters above to see where the data check value come from 
    spi_mock_get_write_data((void *)&rf_ch_mock, &rf_ch_mock_size, SPI_MOCK_INDEX_1); 
    LONGS_EQUAL(RF_CH_SET_DATA_1, rf_ch_mock); 
    LONGS_EQUAL(BYTE_1, rf_ch_mock_size); 
}

//=======================================================================================


//=======================================================================================
// RF_SETUP register test 

// RF_SETUP register bits 
// Bit 0:    unused_2 ----> Obsolete - value doesn't matter 
// Bits 1-2: rf_pwr ------> Set RF output power in TX mode 
// Bit 3:    rf_dr_high --> Sets 2Mbps if RF_DR_LOW not set 
// Bit 4:    pll_lock ----> Force PLL lock signal. Only used in test. 
// Bit 5:    rf_dr_low ---> Set RF Data Rate to 250kbps 
// Bit 6:    unused_1 ----> Reserved - must be low/0 
// Bit 7:    cont_wave ---> Enables continuous carrier transmit 

// RF_SETUP - read and get data 
TEST(nrf24l01_driver_test, rf_setup_get_data)
{
    // See the bit legend above and the getter checks below to see where these numbers 
    // come from. 
    uint8_t rf_setup_mock_0 = RF_SETUP_GET_DATA_0; 
    uint8_t rf_setup_mock_1 = RF_SETUP_GET_DATA_1; 

    nrf24l01_data_rate_t data_rate; 
    nrf24l01_rf_pwr_t power_output; 

    // Set up mock data. Reading from the device involves two operations: writing the 
    // command / reading the status back, then reading the data requested. We only care 
    // about the data so we set indexes 1 and 3 to skip the command part. 
    spi_mock_set_read_data((void *)&rf_setup_mock_0, BYTE_1, SPI_MOCK_INDEX_1); 
    spi_mock_set_read_data((void *)&rf_setup_mock_1, BYTE_1, SPI_MOCK_INDEX_3); 

    // Read the mock data and check the register contents. 
    nrf24l01_rf_setup_read(); 
    data_rate = nrf24l01_get_rf_setup_dr(); 
    power_output = nrf24l01_get_rf_setup_pwr(); 

    LONGS_EQUAL(NRF24L01_DR_250KBPS, data_rate); 
    LONGS_EQUAL(NRF24L01_RF_PWR_0DBM, power_output); 

    nrf24l01_rf_setup_read(); 
    data_rate = nrf24l01_get_rf_setup_dr(); 
    power_output = nrf24l01_get_rf_setup_pwr(); 

    LONGS_EQUAL(NRF24L01_DR_2MBPS, data_rate); 
    LONGS_EQUAL(NRF24L01_RF_PWR_18DBM, power_output); 
}


// RF_SETUP - set and write data 
TEST(nrf24l01_driver_test, rf_setup_set_data)
{
    uint8_t rf_setup_mock = CLEAR; 
    uint8_t rf_setup_mock_size = CLEAR; 

    nrf24l01_set_rf_setup_dr(NRF24L01_DR_250KBPS); 
    nrf24l01_set_rf_setup_pwr(NRF24L01_RF_PWR_6DBM); 
    nrf24l01_rf_setup_write(); 

    // See the bit legend and setters above to see where the data check value come from 
    spi_mock_get_write_data((void *)&rf_setup_mock, &rf_setup_mock_size, SPI_MOCK_INDEX_0); 
    LONGS_EQUAL(RF_SETUP_SET_DATA_0, rf_setup_mock); 
    LONGS_EQUAL(BYTE_1, rf_setup_mock_size); 

    nrf24l01_set_rf_setup_dr(NRF24L01_DR_1MBPS); 
    nrf24l01_set_rf_setup_pwr(NRF24L01_RF_PWR_12DBM); 
    nrf24l01_rf_setup_write(); 

    // See the bit legend and setters above to see where the data check value come from 
    spi_mock_get_write_data((void *)&rf_setup_mock, &rf_setup_mock_size, SPI_MOCK_INDEX_1); 
    LONGS_EQUAL(RF_SETUP_SET_DATA_1, rf_setup_mock); 
    LONGS_EQUAL(BYTE_1, rf_setup_mock_size); 
}

//=======================================================================================


//=======================================================================================
// CONFIG register test 

// CONFIG register bits 
// Bit 0: prim_rx ------> RX/TX control 
// Bit 1: pwr_up -------> Power up / power down 
// Bit 2: crco ---------> CRC encoding scheme 
// Bit 3: en_crc -------> Enable CRC 
// Bit 4: mask_max_rt --> Mask interrupt caused by MAX_RT 
// Bit 5: mask_tx_ds ---> Mask interrupt caused by TX_DS 
// Bit 6: mask_rx_dr ---> Mask interrupt caused by RX_DR 
// Bit 7: unused_1 -----> Reserved - must be low/0 

// CONFIG - read and get data 
TEST(nrf24l01_driver_test, config_get_data)
{
    // See the bit legend above and the getter checks below to see where these numbers 
    // come from. 
    uint8_t config_mock_0 = CONFIG_GET_DATA_0; 
    uint8_t config_mock_1 = CONFIG_GET_DATA_1; 

    nrf24l01_pwr_mode_t power_mode; 
    nrf24l01_mode_select_t mode_select; 

    // Set up mock data. Reading from the device involves two operations: writing the 
    // command / reading the status back, then reading the data requested. We only care 
    // about the data so we set indexes 1 and 3 to skip the command part. 
    spi_mock_set_read_data((void *)&config_mock_0, BYTE_1, SPI_MOCK_INDEX_1); 
    spi_mock_set_read_data((void *)&config_mock_1, BYTE_1, SPI_MOCK_INDEX_3); 

    // Read the mock data and check the register contents. 
    nrf24l01_config_read(); 
    power_mode = nrf24l01_get_config_pwr_mode(); 
    mode_select = nrf24l01_get_config_mode(); 

    LONGS_EQUAL(NRF24L01_PWR_UP, power_mode); 
    LONGS_EQUAL(NRF24L01_RX_MODE, mode_select); 

    nrf24l01_config_read(); 
    power_mode = nrf24l01_get_config_pwr_mode(); 
    mode_select = nrf24l01_get_config_mode(); 

    LONGS_EQUAL(NRF24L01_PWR_DOWN, power_mode); 
    LONGS_EQUAL(NRF24L01_TX_MODE, mode_select); 
}


// CONFIG - Power down / power up 
TEST(nrf24l01_driver_test, config_power_down_power_up)
{
    uint8_t config_mock = CLEAR; 
    uint8_t config_mock_size = CLEAR; 

    nrf24l01_pwr_up(); 

    // See the bit legend and setters above to see where the data check value come from 
    spi_mock_get_write_data((void *)&config_mock, &config_mock_size, SPI_MOCK_INDEX_0); 
    LONGS_EQUAL(CONFIG_GET_DATA_2, config_mock); 
    LONGS_EQUAL(BYTE_1, config_mock_size); 

    nrf24l01_pwr_down(); 

    // See the bit legend and setters above to see where the data check value come from 
    spi_mock_get_write_data((void *)&config_mock, &config_mock_size, SPI_MOCK_INDEX_1); 
    LONGS_EQUAL(CONFIG_GET_DATA_3, config_mock); 
    LONGS_EQUAL(BYTE_1, config_mock_size); 
}

//=======================================================================================
