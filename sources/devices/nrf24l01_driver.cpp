/**
 * @file nrf24l01_driver.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module driver 
 * 
 * @version 0.1
 * @date 2023-07-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "nrf24l01_driver.h" 

//=======================================================================================


//=======================================================================================
// Notes 

// 0-10Mbps 4-wire SPI --> Max data rate of 10Mbps 

// States available for the device: 
// - Power down (low power) 
//   - PWR_UP bit in the CONFIG register gets set to 0 (low_ )
//   - Register values are maintained and SPI is kept active meaning registers can still 
//     be updated in this state. 
// 
// - Standby 
// 
// - RX mode 
//   - Configure CE pin (GPIO output) 
//   - To enter this mode, the PWR_UP bit, PRIM_RX bit and CE pin must be set high 
//   - Valid received data is stored in the RX FIFO. If the RX FIFO is full then new data 
//     coming in is discarded. 
// 
// - TX mode 
//   - Configure CE pin (GPIO output) 
//   - To enter this mode, the PWR_UP bit must be set high, PRIM_RX set low, there must be 
//     a payload in the TX FIFO and a high pulse on the CE for more than 10us. 
//   - The device stays in this mode until it finished transmitting a packet. 
//   - If CE is low then the device returns to standby-1 mode. 
//   - If CE is high and the TX FIFO is not empty then it stays in TX mode. 
//   - If CE is high and the TX FIFO is empty then the device enters standby-2 mode. 
//   - TX mode should not be longer than 4ms --> ensure to enable Enhanced ShockBurst feature 
//     - This means data sent should be short or in short bursts. 
//   - Make sure to pulse the CE pin instead of holding it high so 1 packet is sent at a 
//     time and 4ms is not exceeded. 

//=======================================================================================


//=======================================================================================
// Register data 
//=======================================================================================


//=======================================================================================
// Variables 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief CONFIG register write 
 * 
 * @details 
 */
void nrf24l01_config_reg_write(void); 


/**
 * @brief RF_CH register write 
 * 
 * @details 
 */
void nrf24l01_rf_ch_reg_write(void); 

//=======================================================================================


//=======================================================================================
// Initialization 

// Allow for power on reset state of the device to be run (100ms) 
// Set frequency channel to operate on 
// Set RF data rate 

// nRF24L01 initialization 
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t ss_pin)
{
    // Initialize data record 
}

//=======================================================================================


//=======================================================================================
// Functions 

// Read 


// Write 
// - CSN left high normally, set low to start transaction ad kept there until done. 


// Set frequency channel 


// RF data rate set 


// Status read --> non-operation write 


// Set TX and RX modes? 

//=======================================================================================


//=======================================================================================
// Register functions 

// CONFIG register write 
void nrf24l01_config_reg_write(void)
{
    // 
}


// RF_CH register write 
void nrf24l01_rf_ch_reg_write(void)
{
    // 
}

//=======================================================================================
