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

// CONFIG register 
typedef struct nrf24l01_config_reg_s 
{
    uint8_t unused_1    : 1;     // Reserved - must be low/0 
    uint8_t mask_rx_dr  : 1;     // Mask interrupt caused by RX_DR 
    uint8_t mask_tx_ds  : 1;     // Mask interrupt caused by TX_DS 
    uint8_t mask_max_rt : 1;     // Mask interrupt caused by MAX_RT 
    uint8_t en_crc      : 1;     // Enable CRC 
    uint8_t crco        : 1;     // CRC encoding scheme 
    uint8_t pwr_up      : 1;     // Power up / power down 
    uint8_t prim_rx     : 1;     // RX/TX control 
}
nrf24l01_config_reg_t; 


// RF_CH register 
typedef struct nrf24l01_rf_ch_reg_s 
{
    uint8_t unused_1 : 1;        // Reserved - must be low/0 
    uint8_t rf_ch    : 7;        // Sets the frequency channel the device operates on 
}
nrf24l01_rf_ch_reg_t; 


// RF_SET register 
typedef struct nrf24l01_rf_set_reg_s 
{
    uint8_t cont_wave  : 1;     // Enables continuous carrier transmit when high 
    uint8_t unused_1   : 1;     // Reserved - must be low/0 
    uint8_t rf_dr_low  : 1;     // Set RF Data Rate to 250kbps. See RF_DR_HIGH for encoding.
    uint8_t pll_lock   : 1;     // Force PLL lock signal. Only used in test. 
    uint8_t rf_dr_high : 1;     // Select high speed data rate. Irrelevant if RF_DR_LOW is set.
    uint8_t rf_pwr     : 2;     // Set RF output power in TX mode 
    uint8_t unused_2   : 1;     // Obsolete - value doesn't matter 
}
nrf24l01_rf_set_reg_t; 


// STATUS register 
typedef struct nrf24l01_status_reg_s 
{
    uint8_t unused_1 : 1;       // Reserved - must be low/0 
    uint8_t rx_dr    : 1;       // Data Ready RX FIFO interrupt 
    uint8_t tx_ds    : 1;       // Data Sent TX FIFO interrupt 
    uint8_t max_rt   : 1;       // Maximum number of TX retransmits interrupt 
    uint8_t rx_p_no  : 3;       // Data pipe number for the payload available 
    uint8_t tx_full  : 1;       // TX FIFO full flag 
}
nrf24l01_status_reg_t; 


// FIFO_STATUS register 
typedef struct nrf24l01_fifo_status_reg_s 
{
    uint8_t unused_1 : 1;       // Reserved - must be low/0 
    uint8_t tx_reuse : 1;       // For reusing last transmitted payload 
    uint8_t tx_full  : 1;       // TX FIFO full flag 
    uint8_t tx_empty : 1;       // TX FIFO empty flag 
    uint8_t unused_2 : 2;       // Reserved - must be low/0 
    uint8_t rx_full  : 1;       // RX FIFO full flag 
    uint8_t rx_empty : 1;       // RX FIFO empty flag 
}
nrf24l01_fifo_status_reg_t; 

//=======================================================================================


//=======================================================================================
// Variables 

// Driver data record 
typedef struct nrf24l01_driver_data_s 
{
    // Peripherals 
    SPI_TypeDef *spi; 
    GPIO_TypeDef *gpio; 
    gpio_pin_num_t ss_pin; 

    // Status info 
    // 'status' --> bit 0: spi status 
    //          --> bit 1: init status 
    //          --> bits 2-7: not used 
    uint8_t status; 

    // Register data 
    nrf24l01_config_reg_t config; 
    nrf24l01_rf_ch_reg_t rf_ch; 
    nrf24l01_rf_set_reg_t rf_setup; 
    nrf24l01_status_reg_t status; 
    nrf24l01_fifo_status_reg_t fifo_status; 
}
nrf24l01_driver_data_t; 


// Driver data record instance 
static nrf24l01_driver_data_t nrf24l01_driver_data; 

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
    gpio_pin_num_t ss_pin, 
    nrf24l01_data_rate_t rate, 
    uint8_t rf_ch_freq)
{
    //===================================================
    // Initialize data record 

    // Peripherals 
    nrf24l01_driver_data.spi = spi; 
    nrf24l01_driver_data.gpio = gpio; 
    nrf24l01_driver_data.ss_pin = ss_pin; 

    // CONFIG register 
    nrf24l01_driver_data.config.unused_1 = CLEAR; 
    nrf24l01_driver_data.config.mask_rx_dr = CLEAR; 
    nrf24l01_driver_data.config.mask_tx_ds = CLEAR; 
    nrf24l01_driver_data.config.mask_max_rt = CLEAR; 
    nrf24l01_driver_data.config.en_crc = CLEAR; 
    nrf24l01_driver_data.config.crco = CLEAR; 
    nrf24l01_driver_data.config.pwr_up = SET_BIT; 
    nrf24l01_driver_data.config.prim_rx = SET_BIT; 

    // RF_CH register 
    nrf24l01_driver_data.rf_ch.unused_1 = CLEAR; 
    nrf24l01_driver_data.rf_ch.rf_ch = rf_ch_freq & NRF24L01_RF_CH_MASK; 

    // RF_SETUP register 
    nrf24l01_driver_data.rf_setup.cont_wave = CLEAR; 
    nrf24l01_driver_data.rf_setup.unused_1 = CLEAR; 
    nrf24l01_driver_data.rf_setup.rf_dr_low = (rate >> SHIFT_1) & NRF24L01_RF_DR_MASK; 
    nrf24l01_driver_data.rf_setup.pll_lock = CLEAR; 
    nrf24l01_driver_data.rf_setup.rf_dr_high = rate & NRF24L01_RF_DR_MASK; 
    nrf24l01_driver_data.rf_setup.rf_pwr = CLEAR; 
    nrf24l01_driver_data.rf_setup.unused_2 = CLEAR; 

    // STATUS register 
    nrf24l01_driver_data.status.unused_1 = CLEAR; 
    nrf24l01_driver_data.status.rx_dr = CLEAR; 
    nrf24l01_driver_data.status.tx_ds = CLEAR; 
    nrf24l01_driver_data.status.max_rt = CLEAR; 
    nrf24l01_driver_data.status.rx_p_no = SET_7; 
    nrf24l01_driver_data.status.tx_full = CLEAR; 

    // FIFO_STATUS register 
    nrf24l01_driver_data.fifo_status.unused_1 = CLEAR; 
    nrf24l01_driver_data.fifo_status.tx_reuse = CLEAR; 
    nrf24l01_driver_data.fifo_status.tx_full = CLEAR; 
    nrf24l01_driver_data.fifo_status.tx_empty = SET_BIT; 
    nrf24l01_driver_data.fifo_status.unused_2 = CLEAR; 
    nrf24l01_driver_data.fifo_status.rx_full = CLEAR; 
    nrf24l01_driver_data.fifo_status.rx_empty = SET_BIT; 
    
    //===================================================
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
