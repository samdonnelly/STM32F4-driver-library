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
    GPIO_TypeDef *gpio_ss; 
    GPIO_TypeDef *gpio_en; 
    gpio_pin_num_t ss_pin; 
    gpio_pin_num_t en_pin; 

    // Status info 
    // 'status' --> bit 0: spi status 
    //          --> bit 1: init status 
    //          --> bits 2-7: not used 
    uint8_t driver_status; 

    // Register data 
    nrf24l01_config_reg_t config; 
    nrf24l01_rf_ch_reg_t rf_ch; 
    nrf24l01_rf_set_reg_t rf_setup; 
    nrf24l01_status_reg_t status_reg; 
    nrf24l01_fifo_status_reg_t fifo_status; 
}
nrf24l01_driver_data_t; 


// Driver data record instance 
static nrf24l01_driver_data_t nrf24l01_driver_data; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Receive data from another transceiver 
 * 
 * @details Takes a command and a data length and reads data from the device into the 
 *          read buffer over SPI. The driver status is updated with the result of the 
 *          SPI transaction and status register is updated in the data record since 
 *          the status is sent by the device when the master sends a command. 
 * 
 * @param cmd : device command 
 * @param rec_buff : buffer to store received data 
 * @param data_len : length of data to read (excluding status) 
 */
void nrf24l01_receive(
    uint8_t cmd, 
    uint8_t *rec_buff, 
    uint8_t data_len); 


/**
 * @brief Send data to another transceiver 
 * 
 * @details Takes a command and a data length and writes data from the send buffer to 
 *          the device over SPI. The driver status is updated with the result of the 
 *          SPI transaction and status register is updated in the data record since 
 *          the status is sent by the device when the master sends a command. 
 * 
 * @param cmd : device command 
 * @param send_buff : buffer that stores data to be sent to the device 
 * @param data_len : length of data to send (excluding command) 
 */
void nrf24l01_send(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len); 


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
    GPIO_TypeDef *gpio_ss, 
    gpio_pin_num_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    gpio_pin_num_t en_pin, 
    nrf24l01_data_rate_t rate, 
    uint8_t rf_ch_freq)
{
    // Configure GPIO for enable and slave select pins 

    //===================================================
    // Initialize data record 

    // Peripherals 
    nrf24l01_driver_data.spi = spi; 
    nrf24l01_driver_data.gpio_ss = gpio_ss; 
    nrf24l01_driver_data.gpio_en = gpio_en; 
    nrf24l01_driver_data.ss_pin = ss_pin; 
    nrf24l01_driver_data.en_pin = en_pin; 

    // Driver status 
    nrf24l01_driver_data.driver_status = CLEAR; 

    // CONFIG register 
    nrf24l01_driver_data.config.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.config.mask_rx_dr = CLEAR_BIT; 
    nrf24l01_driver_data.config.mask_tx_ds = CLEAR_BIT; 
    nrf24l01_driver_data.config.mask_max_rt = CLEAR_BIT; 
    nrf24l01_driver_data.config.en_crc = CLEAR_BIT; 
    nrf24l01_driver_data.config.crco = CLEAR_BIT; 
    nrf24l01_driver_data.config.pwr_up = SET_BIT; 
    nrf24l01_driver_data.config.prim_rx = SET_BIT; 

    // RF_CH register 
    nrf24l01_driver_data.rf_ch.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.rf_ch.rf_ch = rf_ch_freq & NRF24L01_RF_CH_MASK; 

    // RF_SETUP register 
    nrf24l01_driver_data.rf_setup.cont_wave = CLEAR_BIT; 
    nrf24l01_driver_data.rf_setup.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.rf_setup.rf_dr_low = (rate >> SHIFT_1) & NRF24L01_RF_DR_MASK; 
    nrf24l01_driver_data.rf_setup.pll_lock = CLEAR_BIT; 
    nrf24l01_driver_data.rf_setup.rf_dr_high = rate & NRF24L01_RF_DR_MASK; 
    nrf24l01_driver_data.rf_setup.rf_pwr = CLEAR; 
    nrf24l01_driver_data.rf_setup.unused_2 = CLEAR_BIT; 

    // STATUS register 
    nrf24l01_driver_data.status_reg.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.status_reg.rx_dr = CLEAR_BIT; 
    nrf24l01_driver_data.status_reg.tx_ds = CLEAR_BIT; 
    nrf24l01_driver_data.status_reg.max_rt = CLEAR_BIT; 
    nrf24l01_driver_data.status_reg.rx_p_no = SET_7; 
    nrf24l01_driver_data.status_reg.tx_full = CLEAR_BIT; 

    // FIFO_STATUS register 
    nrf24l01_driver_data.fifo_status.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.fifo_status.tx_reuse = CLEAR_BIT; 
    nrf24l01_driver_data.fifo_status.tx_full = CLEAR_BIT; 
    nrf24l01_driver_data.fifo_status.tx_empty = SET_BIT; 
    nrf24l01_driver_data.fifo_status.unused_2 = CLEAR; 
    nrf24l01_driver_data.fifo_status.rx_full = CLEAR_BIT; 
    nrf24l01_driver_data.fifo_status.rx_empty = SET_BIT; 
    
    //===================================================

    //===================================================
    // Configure the device 

    // Set PWR_UP=1 to start up the device --> ~1.5ms to enter standby-1 mode 

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Read and write 

// Read 
// - Write-read the first byte (command and status return) then dummy write and read the 
//   remaining data 
// RX mode 
// - You can't go directly from RX to TX mode or vice versa, you have to go through the 
//   standby-1 state. 
// - From standby-1 state: 
//   - Set PRIM_RX=1 
//   - Set CE=1 
//   - There will be a 130us delay (RX settling state) before reading data 
// - Data can be read when available --> have to check the FIFO status to see if there 
//   is data first. 
// - When configured in the RX state then the device will automatically read incoming 
//   data and store it in the RX FIFO. If the RX FIFO is full then new incoming data 
//   will be discarded. 
// RX mode 
//   - Configure CE pin (GPIO output) 
//   - To enter this mode, the PWR_UP bit, PRIM_RX bit and CE pin must be set high 
//   - Valid received data is stored in the RX FIFO. If the RX FIFO is full then new data 
//     coming in is discarded. 

// Receive data from another transceiver 
void nrf24l01_receive(
    uint8_t cmd, 
    uint8_t *rec_buff, 
    uint8_t data_len)
{
    // Local variables 
    uint8_t spi_status = SPI_OK; 
    uint8_t status_reg = CLEAR;   // TODO update driver data record 

    // Select slave 
    spi_slave_select(nrf24l01_driver_data.gpio_ss, nrf24l01_driver_data.ss_pin); 

    // Write the command and read the status back from the slave 
    spi_status = (uint8_t)spi_write_read(nrf24l01_driver_data.spi, 
                                         cmd, 
                                         &status_reg, 
                                         BYTE_1); 

    // Dummy write while reading the returned info 
    spi_status = (uint8_t)spi_write_read(nrf24l01_driver_data.spi, 
                                         NRF24L01_DUMMY_WRITE, 
                                         rec_buff, 
                                         data_len); 

    // Deselect slave 
    spi_slave_deselect(nrf24l01_driver_data.gpio_ss, nrf24l01_driver_data.ss_pin); 

    // Update the status register data record 

    // Update the driver status 
    nrf24l01_driver_data.driver_status |= spi_status; 
}


// Write 
// - CSN left high normally, set low to start transaction and keep there until done. 
// - If just writing and not reading the status that gets sent back by the device when 
//   writing a command then maybe clear the controllers read buffer when done --> this 
//   may already be handled by the SPI write function. 
// TX mode 
// - You can't go directly from RX to TX mode or vice versa, you have to go through the 
//   standby-1 state. 
// - From the standby-1 state: 
//   - Load the TX FIFO with the data 
//   - Set PRIM_RX=0 
//   - Set CE=1 
//   - There will be a 130us delay (TX settling state) before sending data 
// - When in the TX state: 
//   - If there is more data to be sent then reload the TX FIFO as data is sent out. 
//   - Hold CE=1 until data is done being sent at which point set CE=0. 
// - When wanting to exit the TX mode/state then set CE=0. 
// TX mode 
//   - Configure CE pin (GPIO output) 
//   - To enter this mode, the PWR_UP bit must be set high, PRIM_RX set low, there must be 
//     a payload in the TX FIFO and a high pulse on the CE for more than 10us. 
//   - The device stays in this mode until it finished transmitting a packet. 
//   - If CE is low then the device returns to standby-1 mode. 
//   - If CE is high and the TX FIFO is not empty then it stays in TX mode. 
//   - If CE is high and the TX FIFO is empty then the device enters standby-2 mode. 
//   - TX mode should not be longer than 4ms 
//     - This means data sent should be short or in short bursts. 
//   - Make sure to pulse the CE pin instead of holding it high so 1 packet is sent at a 
//     time and 4ms is not exceeded. 

// Send data to another transceiver 
void nrf24l01_send(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len)
{
    // Local variables 
    uint8_t spi_status = SPI_OK; 
    uint8_t status_reg = CLEAR;   // TODO update driver data record 

    // Select slave 
    spi_slave_select(nrf24l01_driver_data.gpio_ss, nrf24l01_driver_data.ss_pin); 

    // Write the command and read the status back from the slave 
    spi_status = (uint8_t)spi_write_read(nrf24l01_driver_data.spi, 
                                         cmd, 
                                         &status_reg, 
                                         BYTE_1); 

    // Write the data to the device 
    spi_status = (uint8_t)spi_write(nrf24l01_driver_data.spi, 
                                    send_buff, 
                                    data_len); 

    // Deselect slave 
    spi_slave_deselect(nrf24l01_driver_data.gpio_ss, nrf24l01_driver_data.ss_pin); 

    // Update the status register data record 

    // Update the driver status 
    nrf24l01_driver_data.driver_status |= spi_status; 
}

//=======================================================================================


//=======================================================================================
// User functions 

// Receive payload 
void nrf24l01_receive_payload(void)
{
    // 
}


// Send payload 
void nrf24l01_send_payload(void)
{
    // 
}


// Set frequency channel 
void nrf24l01_set_channel(void)
{
    // 
}


// RF data rate set 
void nrf24l01_set_rate(void)
{
    // 
}


// Status read --> non-operation write 
void nrf24l01_get_status(void)
{
    // 
}


// Set RX mode 
// - Set PRIM_RX=1 
// - Set CE=1 
void nrf24l01_set_rx_mode(void)
{
    // 
}


// Set TX mode 
// - Set PRIM_RX=0 
// - Set CE=1 
void nrf24l01_set_tx_mode(void)
{
    // 
}


// Low power mode 
// - Make sure current data transfers are wrapped up. 
// - Set CE=0 to enter standby-1 state. 
// - Set PWR_UP=0 to enter power down state 
void nrf24l01_set_low_pwr(void)
{
    // 
}


// Standby mode 
void nrf24l01_set_standby(void)
{
    // 
}

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