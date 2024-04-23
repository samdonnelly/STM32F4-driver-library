/**
 * @file nrf24l01_driver.c
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

#include "spi_comm.h"
#include "gpio_driver.h"
#include "timers_driver.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Data handling 
#define NRF24L01_RF_CH_MASK 0x7F       // RF channel frequency mask 
#define NRF24L01_RF_DR_MASK 0x01       // RF data rate bit mask 
#define NRF24L01_DATA_SIZE_LEN 1       // Data size indicator length 
#define NRF24L01_MAX_DATA_LEN 30       // Max user data length 

// Timing 
#define NRF24L01_CE_TX_DELAY 20        // Time CE is held high in TX (us) 
#define NRF24L01_TX_TIMEOUT 0x0FFF     // Max number of times to check for successful transmission 
#define NRF24L01_PWR_ON_DELAY 100      // Device power on reset delay (ms) 
#define NRF24L01_START_DELAY 2         // Device start up delay (ms) 
#define NRF24L01_SETTLE_DELAY 130      // Device state settling time delay (us) 

// Control 
#define NRF24L01_DISABLE_REG 0x00      // Disable settings in a register 

//=======================================================================================


//=======================================================================================
// Enums 

// Commands 
typedef enum {
    NRF24L01_CMD_R_REG    = 0x00,   // Read command and status registers 
    NRF24L01_CMD_W_REG    = 0x20,   // Write command and status registers 
    NRF24L01_CMD_R_RX_PL  = 0x61,   // Read RX payload 
    NRF24L01_CMD_W_TX_PL  = 0xA0,   // Write TX payload 
    NRF24L01_CMD_FLUSH_TX = 0xE1,   // Flush TX FIFO 
    NRF24L01_CMD_FLUSH_RX = 0xE2,   // Flush RX FIFO 
    NRF24L01_CMD_REUSE_TX = 0x00,   // Reuse TX payload 
    NRF24L01_CMD_NOP      = 0xFF    // No operation 
} nrf24l01_cmds_t; 


// Register addresses 
typedef enum {
    NRF24L01_REG_CONFIG     = 0x00,   // CONFIG register address 
    NRF24L01_REG_EN_AA      = 0x01,   // EN_AA register address 
    NRF24L01_REG_EN_RXADDR  = 0x02,   // EN_RXADDR register address 
    NRF24L01_REG_SETUP_AW   = 0x03,   // SETUP_AW register address 
    NRF24L01_REG_SETUP_RETR = 0x04,   // SETUP_RETR register address 
    NRF24L01_REG_RF_CH      = 0x05,   // RF_CH register address 
    NRF24L01_REG_RF_SET     = 0x06,   // RF_SETUP register address 
    NRF24L01_REG_STATUS     = 0x07,   // STATUS register address 
    NRF24L01_REG_OBSERVE_TX = 0x08,   // OBSERVE_TX register address 
    NRF24L01_REG_RPD        = 0x09,   // RPD register address 
    NRF24L01_REG_RX_ADDR_P0 = 0x0A,   // RX_ADDR_P0 register address 
    NRF24L01_REG_RX_ADDR_P1 = 0x0B,   // RX_ADDR_P1 register address 
    NRF24L01_REG_RX_ADDR_P2 = 0x0C,   // RX_ADDR_P2 register address 
    NRF24L01_REG_RX_ADDR_P3 = 0x0D,   // RX_ADDR_P3 register address 
    NRF24L01_REG_RX_ADDR_P4 = 0x0E,   // RX_ADDR_P4 register address 
    NRF24L01_REG_RX_ADDR_P5 = 0x0F,   // RX_ADDR_P5 register address 
    NRF24L01_REG_TX_ADDR    = 0x10,   // TX_ADDR register address 
    NRF24L01_REG_RX_PW_P0   = 0x11,   // RX_PW_P0 register address 
    NRF24L01_REG_RX_PW_P1   = 0x12,   // RX_PW_P1 register address 
    NRF24L01_REG_RX_PW_P2   = 0x13,   // RX_PW_P2 register address 
    NRF24L01_REG_RX_PW_P3   = 0x14,   // RX_PW_P3 register address 
    NRF24L01_REG_RX_PW_P4   = 0x15,   // RX_PW_P4 register address 
    NRF24L01_REG_RX_PW_P5   = 0x16,   // RX_PW_P5 register address 
    NRF24L01_REG_FIFO       = 0x17,   // FIFO_STATUS register address 
    NRF24L01_REG_DYNPD      = 0x1C,   // DYNPD register address 
    NRF24L01_REG_FEATURE    = 0x1D    // FEATURE register address 
} nrf24l01_reg_addr_t; 


// Register reset values (excludes read only bits) 
typedef enum {
    NRF24L01_REG_RESET_CONFIG     = 0x08,   // CONFIG register reset value 
    NRF24L01_REG_RESET_EN_AA      = 0x3F,   // EN_AA register reset value 
    NRF24L01_REG_RESET_EN_RXADDR  = 0x03,   // EN_RXADDR register reset value 
    NRF24L01_REG_RESET_SETUP_AW   = 0x03,   // SETUP_AW register reset value 
    NRF24L01_REG_RESET_SETUP_RETR = 0x03,   // SETUP_RETR register reset value 
    NRF24L01_REG_RESET_RF_CH      = 0x02,   // RF_CH register reset value 
    NRF24L01_REG_RESET_RF_SET     = 0x0E,   // RF_SETUP register reset value 
    NRF24L01_REG_RESET_STATUS     = 0x70,   // STATUS register reset value 
    NRF24L01_REG_RESET_OBSERVE_TX = 0x00,   // OBSERVE_TX register reset value 
    NRF24L01_REG_RESET_RPD        = 0x00,   // RPD register reset value 
    NRF24L01_REG_RESET_RX_ADDR_P0 = 0xE7,   // RX_ADDR_P0 register reset value 
    NRF24L01_REG_RESET_RX_ADDR_P1 = 0xC2,   // RX_ADDR_P1 register reset value 
    NRF24L01_REG_RESET_RX_ADDR_P2 = 0xC3,   // RX_ADDR_P2 register reset value 
    NRF24L01_REG_RESET_RX_ADDR_P3 = 0xC4,   // RX_ADDR_P3 register reset value 
    NRF24L01_REG_RESET_RX_ADDR_P4 = 0xC5,   // RX_ADDR_P4 register reset value 
    NRF24L01_REG_RESET_RX_ADDR_P5 = 0xC6,   // RX_ADDR_P5 register reset value 
    NRF24L01_REG_RESET_TX_ADDR    = 0xE7,   // TX_ADDR register reset value 
    NRF24L01_REG_RESET_RX_PW_PX   = 0x00,   // RX_PW_PX (X-->0-5) register reset value 
    NRF24L01_REG_RESET_DYNPD      = 0x00,   // DYNPD register reset value 
    NRF24L01_REG_RESET_FEATURE    = 0x00    // FEATURE register reset value 
} nrf24l01_reg_reset_t; 

//=======================================================================================


//=======================================================================================
// Register data 

// The order of each register data bitfield is important for the union to work as intended. 

// CONFIG register 
typedef union nrf24l01_config_reg_s 
{
    // CONFIG register bits 
    struct 
    {
        uint8_t prim_rx     : 1;     // Bit 0: RX/TX control 
        uint8_t pwr_up      : 1;     // Bit 1: Power up / power down 
        uint8_t crco        : 1;     // Bit 2: CRC encoding scheme 
        uint8_t en_crc      : 1;     // Bit 3: Enable CRC 
        uint8_t mask_max_rt : 1;     // Bit 4: Mask interrupt caused by MAX_RT 
        uint8_t mask_tx_ds  : 1;     // Bit 5: Mask interrupt caused by TX_DS 
        uint8_t mask_rx_dr  : 1;     // Bit 6: Mask interrupt caused by RX_DR 
        uint8_t unused_1    : 1;     // Bit 7: Reserved - must be low/0 
    }; 

    // CONFIG register byte 
    uint8_t config_reg; 
}
nrf24l01_config_reg_t; 


// RF_CH register 
typedef union nrf24l01_rf_ch_reg_s 
{
    // RF_CH register bits 
    struct 
    {
        uint8_t rf_ch    : 7;        // Bits 0-6: Frequency channel operated on 
        uint8_t unused_1 : 1;        // Bit 7: Reserved - must be low/0 
    }; 

    // RF_CH register bytes 
    uint8_t rf_ch_reg; 
}
nrf24l01_rf_ch_reg_t; 


// RF_SETUP register 
typedef union nrf24l01_rf_set_reg_s 
{
    // RF_SET register bits 
    struct 
    {
        uint8_t unused_2   : 1;     // Bit 0: Obsolete - value doesn't matter 
        uint8_t rf_pwr     : 2;     // Bits 1-2: Set RF output power in TX mode 
        uint8_t rf_dr_high : 1;     // Bit 3: Sets 2Mbps if RF_DR_LOW not set 
        uint8_t pll_lock   : 1;     // Bit 4: Force PLL lock signal. Only used in test. 
        uint8_t rf_dr_low  : 1;     // Bit 5: Set RF Data Rate to 250kbps 
        uint8_t unused_1   : 1;     // Bit 6: Reserved - must be low/0 
        uint8_t cont_wave  : 1;     // Bit 7: Enables continuous carrier transmit 
    }; 

    // RF_SET register bytes 
    uint8_t rf_set_reg; 
}
nrf24l01_rf_set_reg_t; 


// STATUS register 
typedef union nrf24l01_status_reg_s 
{
    // STATUS register bits 
    struct 
    {
        uint8_t tx_full  : 1;       // Bit 0: TX FIFO full flag 
        uint8_t rx_p_no  : 3;       // Bits 1-3: Data pipe number for the payload available 
        uint8_t max_rt   : 1;       // Bit 4: Maximum number of TX retransmits interrupt 
        uint8_t tx_ds    : 1;       // Bit 5: Data Sent TX FIFO interrupt 
        uint8_t rx_dr    : 1;       // Bit 6: Data Ready RX FIFO interrupt 
        uint8_t unused_1 : 1;       // Bit 7: Reserved - must be low/0 
    }; 

    // STATUS register bytes 
    uint8_t status_reg; 
}
nrf24l01_status_reg_t; 


// FIFO_STATUS register 
typedef union nrf24l01_fifo_status_reg_s 
{
    // FIFO_STATUS register bits 
    struct 
    {
        uint8_t rx_empty : 1;       // Bit 0: RX FIFO empty flag 
        uint8_t rx_full  : 1;       // Bit 1: RX FIFO full flag 
        uint8_t unused_2 : 2;       // Bits 2-3: Reserved - must be low/0 
        uint8_t tx_empty : 1;       // Bit 4: TX FIFO empty flag 
        uint8_t tx_full  : 1;       // Bit 5: TX FIFO full flag 
        uint8_t tx_reuse : 1;       // Bit 6: For reusing last transmitted payload 
        uint8_t unused_1 : 1;       // Bit 7: Reserved - must be low/0 
    }; 

    // FIFO_STATUS register bytes 
    uint8_t fifo_status_reg; 
}
nrf24l01_fifo_status_reg_t; 

//=======================================================================================


//=======================================================================================
// Data record 

// Driver data record 
typedef struct nrf24l01_driver_s 
{
    // Peripherals 
    SPI_TypeDef *spi; 
    GPIO_TypeDef *gpio_ss; 
    GPIO_TypeDef *gpio_en; 
    gpio_pin_num_t ss_pin; 
    gpio_pin_num_t en_pin; 
    TIM_TypeDef *timer; 

    // Register data 
    nrf24l01_config_reg_t config; 
    nrf24l01_rf_ch_reg_t rf_ch; 
    nrf24l01_rf_set_reg_t rf_setup; 
    nrf24l01_status_reg_t status; 
    nrf24l01_fifo_status_reg_t fifo_status; 
}
nrf24l01_driver_t; 


// Driver data record instance 
static nrf24l01_driver_t nrf24l01_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Set CE pin 
 * 
 * @param state : pin state 
 */
void nrf24l01_set_ce(gpio_pin_state_t state); 


/**
 * @brief Set active mode (TX/RX) 
 * 
 * @details Sets the PRIM_RX parameter in the device config register to choose between 
 *          PTX or PRX mode. When the device is not sending data it stays in PRX mode. 
 *          When there is a payload to be sent, this function is called to change the 
 *          device to PTX mode and send the payload, then again to revert to PRX mode. 
 * 
 * @see nrf24l01_mode_select_t 
 * 
 * @param mode : PTX or PRX mode 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_set_data_mode(nrf24l01_mode_select_t mode); 


/**
 * @brief CONFIG register write 
 * 
 * @details Writes the config register data held in the driver data record to the device' 
 *          CONFIG register. This function is called when the config register needs to be 
 *          updated like when changing modes or powering up/down. 
 * 
 * @see nrf24l01_config_reg_t 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_config_reg_write(void); 


// /**
//  * @brief CONFIG register read 
//  * 
//  * @details Reads the CONFIG register from the device and stores the contents in the data 
//  *          record. This function is called when retrieving device info for user getters. 
//  * 
//  * @see nrf24l01_config_reg_t 
//  * 
//  * @return NRF24L01_STATUS : read operation status 
//  */
// NRF24L01_STATUS nrf24l01_config_reg_read(void); 


// /**
//  * @brief RF_CH register write 
//  * 
//  * @details Writes the RF_CH data from the data record to the RF_CH register in the 
//  *          device. This function is used when the user updates the RF channel. 
//  * 
//  * @see nrf24l01_rf_ch_reg_t 
//  * 
//  * @return NRF24L01_STATUS : write operation status 
//  */
// NRF24L01_STATUS nrf24l01_rf_ch_reg_write(void); 


// /**
//  * @brief RF_CH register read 
//  * 
//  * @details Reads the RF_CH register from the device and stores the contents in the data 
//  *          record. This function is called when retrieving the RF channel for the user. 
//  * 
//  * @see nrf24l01_rf_ch_reg_t 
//  * 
//  * @return NRF24L01_STATUS : read operation status 
//  */
// NRF24L01_STATUS nrf24l01_rf_ch_reg_read(void); 


// /**
//  * @brief RF_SETUP register write 
//  * 
//  * @details Writes the RF_SETUP data from the data record to the RF_SETUP register in the 
//  *          device. This function is used when the user updates the RF settings. 
//  * 
//  * @see nrf24l01_rf_set_reg_t 
//  * 
//  * @return NRF24L01_STATUS : write operation status 
//  */
// NRF24L01_STATUS nrf24l01_rf_setup_reg_write(void); 


// /**
//  * @brief RF_SETUP register read 
//  * 
//  * @details Reads the RF_SETUP register from the device and stores the contents in the 
//  *          data record. This function is called when retrieving RF settings for the user. 
//  * 
//  * @see nrf24l01_rf_set_reg_t 
//  * 
//  * @return NRF24L01_STATUS : read operation status 
//  */
// NRF24L01_STATUS nrf24l01_rf_setup_reg_read(void); 


/**
 * @brief STATUS register write 
 * 
 * @details Writes the STATUS data from the data record to the STATUS register in the 
 *          device. This function is used to clear the status register. 
 * 
 * @see nrf24l01_status_reg_t 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_status_reg_write(void); 


/**
 * @brief STATUS register read 
 * 
 * @details Reads the STATUS register from the device and stores the contents in the data 
 *          record. This function is called when device status is needed. 
 * 
 * @see nrf24l01_status_reg_t 
 * 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_status_reg_read(void); 


// /**
//  * @brief STATUS register state update 
//  * 
//  * @details Copies the STATUS register contents to the data record. The STATUS register 
//  *          contents are sent by the device when a command is written to it. This 
//  *          function is used by the read and write functions that record the regsiter 
//  *          contents when specifying the operation. 
//  * 
//  * @see nrf24l01_status_reg_t 
//  * 
//  * @param status : STATUS register contents read from the device 
//  */
// void nrf24l01_status_reg_update(uint8_t status); 


/**
 * @brief FIFO_STATUS register read 
 * 
 * @details Reads the FIFO_STATUS register from the device and stores the contents in the 
 *          data record. This function is called when checking transmission status. 
 * 
 * @see nrf24l01_fifo_status_reg_t 
 * 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_fifo_status_reg_read(void); 


/**
 * @brief Receive data from another transceiver 
 * 
 * @details Takes a command and a data length and reads data from the device into the 
 *          read buffer over SPI. The STATUS register contents are updated in the data 
 *          record and the status of the read operation is returned. Note that the 
 *          device sends the STATUS register contents when a command is written to it. 
 * 
 * @param cmd : device command 
 * @param rec_buff : buffer to store received data 
 * @param data_len : length of data to read (excluding status) 
 */
SPI_STATUS nrf24l01_receive(
    uint8_t cmd, 
    uint8_t *rec_buff, 
    uint8_t data_len); 


/**
 * @brief Send data to another transceiver 
 * 
 * @details Takes a command and a data length and writes data from the send buffer to 
 *          the device over SPI. The STATUS register contents are updated in the data 
 *          record and the status of the write operation is returned. Note that the 
 *          device sends the STATUS register contents when a command is written to it. 
 * 
 * @param cmd : device command 
 * @param send_buff : buffer that stores data to be sent to the device 
 * @param data_len : length of data to send (excluding command) 
 */
SPI_STATUS nrf24l01_write(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len); 


// /**
//  * @brief Register read 
//  * 
//  * @details Reads and returns the contents of the specifier register. 
//  * 
//  * @see nrf24l01_reg_addr_t 
//  * 
//  * @param reg_addr : register address 
//  * @return NRF24L01_STATUS : driver status 
//  */
// uint8_t nrf24l01_reg_read(uint8_t reg_addr); 


/**
 * @brief Register read 
 * 
 * @details 
 * 
 * @param reg_addr 
 * @param reg_data 
 * @return NRF24L01_STATUS 
 */
NRF24L01_STATUS nrf24l01_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_data); 


/**
 * @brief 
 * 
 * @param reg_addr 
 * @param reg_data 
 * @return NRF24L01_STATUS 
 */
NRF24L01_STATUS nrf24l01_reg_write_data(
    uint8_t reg_addr, 
    uint8_t reg_data); 


/**
 * @brief Register byte write 
 * 
 * @details Calls the nrf24l01_reg_write function. 
 * 
 * @see nrf24l01_reg_write 
 * 
 * @param reg_addr : register address 
 * @param reg_data : byte to write to the register address 
 * @return NRF24L01_STATUS : status of the write operation 
 */
NRF24L01_STATUS nrf24l01_reg_byte_write(
    uint8_t reg_addr, 
    uint8_t reg_data); 


/**
 * @brief Register write 
 * 
 * @details Writes the contents of the buffer to the specifier register. 
 * 
 * @see nrf24l01_reg_addr_t 
 * 
 * @param reg_addr : register address 
 * @param reg_data : buffer to write starting at the register address 
 * @param reg_size : register size (bytes) 
 * @return NRF24L01_STATUS : status of the write operation 
 */
NRF24L01_STATUS nrf24l01_reg_write(
    uint8_t reg_addr, 
    const uint8_t *reg_data, 
    uint8_t reg_size); 

//=======================================================================================


//=======================================================================================
// Initialization 

// nRF24L01 initialization 
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer, 
    uint8_t rf_ch_freq, 
    nrf24l01_data_rate_t data_rate, 
    nrf24l01_rf_pwr_t rf_pwr)
{
    // uint8_t reg_buff = CLEAR; 
    uint8_t p0_addr_buff[NRF24l01_ADDR_WIDTH]; 
    uint8_t p1_addr_buff[NRF24l01_ADDR_WIDTH]; 

    //===================================================
    // Initialize data 

    // Default addresses 
    memset((void *)p0_addr_buff, NRF24L01_REG_RESET_RX_ADDR_P0, sizeof(p0_addr_buff)); 
    memset((void *)p1_addr_buff, NRF24L01_REG_RESET_RX_ADDR_P1, sizeof(p1_addr_buff)); 

    // Reset the data record 
    memset((void *)&nrf24l01_data, CLEAR, sizeof(nrf24l01_driver_t)); 

    // Peripherals 
    nrf24l01_data.spi = spi; 
    nrf24l01_data.gpio_ss = gpio_ss; 
    nrf24l01_data.gpio_en = gpio_en; 
    nrf24l01_data.ss_pin = (gpio_pin_num_t)(SET_BIT << ss_pin); 
    nrf24l01_data.en_pin = (gpio_pin_num_t)(SET_BIT << en_pin); 
    nrf24l01_data.timer = timer; 

    // // CONFIG register 
    // nrf24l01_data.config.pwr_up = SET_BIT;   // Set to start up the device 
    // nrf24l01_data.config.prim_rx = SET_BIT;  // Default to RX mode 

    // // RF_CH register 
    // nrf24l01_data.rf_ch.rf_ch = NRF24L01_REG_RESET_RF_CH; 

    // // RF_SETUP register (set to default) 
    // nrf24l01_data.rf_setup.rf_dr_low = 
    //     ((uint8_t)NRF24L01_DR_2MBPS >> SHIFT_1) & NRF24L01_RF_DR_MASK; 
    // nrf24l01_data.rf_setup.rf_dr_high = 
    //     (uint8_t)NRF24L01_DR_2MBPS & NRF24L01_RF_DR_MASK; 
    // nrf24l01_data.rf_setup.rf_pwr = SET_3; 

    // // STATUS register (write 1 to clear) 
    // nrf24l01_data.status.rx_dr = SET_BIT; 
    // nrf24l01_data.status.tx_ds = SET_BIT; 
    // nrf24l01_data.status.max_rt = SET_BIT; 
    
    //===================================================

    //===================================================
    // Configure GPIO pins 

    // SPI slave select pin (CSN) 
    spi_ss_init(nrf24l01_data.gpio_ss, ss_pin); 

    // Device enable pin (CE) 
    gpio_pin_init(
        nrf24l01_data.gpio_en, 
        en_pin, 
        MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    
    //===================================================

    //==================================================
    // Device initialization 
    
    // Set CE low to prevent entering an active (TX/RX) state 
    nrf24l01_set_ce(GPIO_LOW); 

    // Delay to ensure power on reset state is cleared before accessing the device 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_PWR_ON_DELAY); 

    // // Flush the FIFOs to ensure no leftover data 
    // nrf24l01_write(NRF24L01_CMD_FLUSH_TX, &reg_buff, BYTE_0); 
    // nrf24l01_write(NRF24L01_CMD_FLUSH_RX, &reg_buff, BYTE_0); 

    // // Set register values to their reset/default value 
    // nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, NRF24L01_REG_RESET_CONFIG); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_EN_AA, NRF24L01_REG_RESET_EN_AA); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, NRF24L01_REG_RESET_EN_RXADDR); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_AW, NRF24L01_REG_RESET_SETUP_AW); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_RETR, NRF24L01_REG_RESET_SETUP_RETR); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, NRF24L01_REG_RESET_RF_CH); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, NRF24L01_REG_RESET_RF_SET); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_STATUS, NRF24L01_REG_RESET_STATUS); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_OBSERVE_TX, NRF24L01_REG_RESET_OBSERVE_TX); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RPD, NRF24L01_REG_RESET_RPD); 
    // nrf24l01_reg_write(NRF24L01_REG_RX_ADDR_P0, p0_addr_buff, NRF24l01_ADDR_WIDTH); 
    // nrf24l01_reg_write(NRF24L01_REG_RX_ADDR_P1, p1_addr_buff, NRF24l01_ADDR_WIDTH); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P2, NRF24L01_REG_RESET_RX_ADDR_P2); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P3, NRF24L01_REG_RESET_RX_ADDR_P3); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P4, NRF24L01_REG_RESET_RX_ADDR_P4); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P5, NRF24L01_REG_RESET_RX_ADDR_P5); 
    // nrf24l01_reg_write(NRF24L01_REG_TX_ADDR, p0_addr_buff, NRF24l01_ADDR_WIDTH); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P0, NRF24L01_REG_RESET_RX_PW_PX); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P1, NRF24L01_REG_RESET_RX_PW_PX); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P2, NRF24L01_REG_RESET_RX_PW_PX); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P3, NRF24L01_REG_RESET_RX_PW_PX); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P4, NRF24L01_REG_RESET_RX_PW_PX); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P5, NRF24L01_REG_RESET_RX_PW_PX); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_DYNPD, NRF24L01_REG_RESET_DYNPD); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_FEATURE, NRF24L01_REG_RESET_FEATURE); 

    // Configure registers to work for this driver 
    // The below settings are common between both PRX and PTX devices. Registers not 
    // changed here remain at their default value or are changed selectively by setters. 

    //==================================================
    // CONFIG - Power up the device and default to a PRX device. Delay to allow time 
    // for the start up state to pass. 
    // nrf24l01_config_reg_write(); 
    nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, NRF24L01_REG_RESET_CONFIG); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_START_DELAY); 
    //==================================================
    
    // EN_AA - disable auto acknowledgment (Enhanced Shockburst TM not used) 
    nrf24l01_reg_byte_write(NRF24L01_REG_EN_AA, NRF24L01_DISABLE_REG); 
    
    // EN_RXADDR - disable data pipes for now - configured separately 
    nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, NRF24L01_DISABLE_REG); 


    //==================================================
    // SETUP_AW 
    nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_AW, NRF24L01_REG_RESET_SETUP_AW); 
    //==================================================

    
    // SETUP_RETR - disable retransmission because auto acknowledgement not used 
    nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_RETR, NRF24L01_DISABLE_REG); 


    //==================================================
    // RF_CH 
    nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, NRF24L01_REG_RESET_RF_CH); 
    //==================================================


    //==================================================
    // RF_SETUP 
    nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, NRF24L01_REG_RESET_RF_SET); 
    //==================================================
    

    // // FIFO_STATUS - read the FIFO status register to update the driver data record 
    // nrf24l01_fifo_status_reg_read(); 

    // Set CE high to enter RX mode 
    nrf24l01_set_ce(GPIO_HIGH); 

    //==================================================

    //==================================================
    // Communication settings 

    // nrf24l01_set_rf_channel(rf_ch_freq); 
    // nrf24l01_set_rf_dr(data_rate); 
    // nrf24l01_set_rf_pwr(rf_pwr); 
    
    //==================================================
}


// Configure a devices PTX settings 
void nrf24l01_ptx_config(const uint8_t *tx_addr)
{
    // Set CE low to exit any active mode 
    nrf24l01_set_ce(GPIO_LOW); 


    //==================================================
    // RF_CH 
    // nrf24l01_set_rf_channel(rf_ch_freq); 
    nrf24l01_set_rf_ch(10); 
    //==================================================
    

    // Set up TX_ADDR - don't need to match RX_ADDR_P0 as not using auto acknowledge 
    nrf24l01_reg_write(NRF24L01_REG_TX_ADDR, tx_addr, NRF24l01_ADDR_WIDTH); 


    //==================================================
    // CONFIG 
    // nrf24l01_config_reg_read(); 
    nrf24l01_config_read(); 
    nrf24l01_data.config.pwr_up = SET_BIT;   // Set to start up the device 
    nrf24l01_config_reg_write(); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, nrf24l01_data.config.config_reg); 
    //==================================================


    // Set CE high to enter back into an active mode 
    nrf24l01_set_ce(GPIO_HIGH); 
}


// Configure a devices PRX settings 
void nrf24l01_prx_config(
    const uint8_t *rx_addr, 
    nrf24l01_data_pipe_t pipe_num)
{
    // Set CE low to exit any active mode 
    nrf24l01_set_ce(GPIO_LOW); 

    // EN_RXADDR - set the data pipe to enable 
    uint8_t en_rxaddr = CLEAR; 
    nrf24l01_reg_read(NRF24L01_REG_EN_RXADDR, &en_rxaddr); 
    nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, 
                            en_rxaddr | (SET_BIT << (uint8_t)pipe_num)); 
    // nrf24l01_reg_byte_write(
    //     NRF24L01_REG_EN_RXADDR, 
    //     nrf24l01_reg_read(NRF24L01_REG_EN_RXADDR) | (SET_BIT << (uint8_t)pipe_num)); 

    // RX_ADDR_PX - set the chosen data pipe address 
    nrf24l01_reg_write(
        NRF24L01_REG_RX_ADDR_P0 + (uint8_t)pipe_num, 
        rx_addr, 
        NRF24l01_ADDR_WIDTH); 

    // RX_PW_PX - set the max number of bytes in RX payload in the chosen data pipe 
    nrf24l01_reg_byte_write(
        (NRF24L01_REG_RX_PW_P0 + (uint8_t)pipe_num), 
        NRF24L01_MAX_PAYLOAD_LEN); 

    // Set CE high to enter back into an active mode 
    nrf24l01_set_ce(GPIO_HIGH); 
}

//=======================================================================================


//=======================================================================================
// User functions 

// Data ready status 
uint8_t nrf24l01_data_ready_status(nrf24l01_data_pipe_t pipe_num)
{
    // Check if there is data in the RX FIFO and if the data belongs to the specified 
    // pipe number. 
    nrf24l01_status_reg_read(); 
    return (nrf24l01_data.status.rx_dr && 
           (nrf24l01_data.status.rx_p_no & (uint8_t)pipe_num)); 
}


// Receive payload 
void nrf24l01_receive_payload(
    uint8_t *read_buff, 
    nrf24l01_data_pipe_t pipe_num)
{
    uint8_t data_len = CLEAR; 
    uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

    if (read_buff == NULL)
    {
        return; 
    }

    // Check FIFO status before attempting a read 
    if (nrf24l01_data.status.rx_dr && 
       (nrf24l01_data.status.rx_p_no & (uint8_t)pipe_num))
    {
        // Read the first byte from the RX FIFO --> This is the data length 
        nrf24l01_receive(NRF24L01_CMD_R_RX_PL, &data_len, BYTE_1); 

        // Use the data length number to read the remaining RX FIFO data 
        nrf24l01_receive(NRF24L01_CMD_R_RX_PL, read_buff, data_len); 

        // // Read the contents from the RX FIFO 
        // nrf24l01_receive(NRF24L01_CMD_R_RX_PL, read_buff, NRF24L01_MAX_PAYLOAD_LEN); 

        // Flush the RX FIFO to ensure old data is not read later 
        nrf24l01_write(NRF24L01_CMD_FLUSH_RX, &buff, BYTE_0); 

        // Clear the RX_DR bit in the STATUS register 
        nrf24l01_status_reg_write(); 
        nrf24l01_status_reg_read(); 
    }
}


// Send payload 
uint8_t nrf24l01_send_payload(const uint8_t *data_buff)
{
    // uint8_t pack_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
    uint8_t data_len = CLEAR; 
    // uint8_t index = NRF24L01_DATA_SIZE_LEN; 
    // uint8_t tx_status = CLEAR; 
    uint8_t tx_status = NRF24L01_OK; 
    uint16_t time_out = NRF24L01_TX_TIMEOUT; 
    uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

    if (data_buff == NULL)
    {
        return tx_status; 
    }


    //==================================================
    // dev 

    data_len = NRF24L01_MAX_PAYLOAD_LEN; 

    // Write the payload to the TX FIFO 
    // nrf24l01_write(NRF24L01_CMD_W_TX_PL, pack_buff, data_len); 
    nrf24l01_write(NRF24L01_CMD_W_TX_PL, data_buff, data_len); 

    // Check to see if the TX FIFO is empty - means data was transmitted. 
    // If data has not been transferred before timeout then it's considered to have failed. 
    do 
    {
        nrf24l01_fifo_status_reg_read(); 
    }
    while(!(nrf24l01_data.fifo_status.tx_empty) && (--time_out)); 

    if (!time_out)
    {
        // Time left on the timer so data has successfully been set. 
        tx_status = NRF24L01_WRITE_FAULT; 
    }

    // Flush the TX FIFO 
    nrf24l01_write(NRF24L01_CMD_FLUSH_TX, &buff, BYTE_0); 

    return tx_status; 

    //==================================================


    // // Fill the packet buffer with the data to be sent. The packet will be capped at a max of 
    // // 30 data bytes with one byte always being saved at the beginning and end of the packet for 
    // // the data length and a NULL termination, respectfully. The following loop counts the data 
    // // length and saves the data/payload into the packet buffer. If the data length is less than 
    // // 30 bytes then the loop ends early. 
    // while (index <= NRF24L01_MAX_DATA_LEN)
    // {
    //     data_len++; 

    //     if (*data_buff == NULL_CHAR)
    //     {
    //         break; 
    //     }

    //     pack_buff[index++] = *data_buff++; 
    // }

    // // Write the data size to the first position of the packet buffer and terminate the payload 
    // pack_buff[0] = data_len; 
    // pack_buff[index] = NULL_CHAR; 
    
    // // Set CE low to exit RX mode 
    // nrf24l01_set_ce(GPIO_LOW); 

    // // Set as a PTX device 
    // nrf24l01_set_data_mode(NRF24L01_TX_MODE); 

    // // Write the payload to the TX FIFO 
    // nrf24l01_write(NRF24L01_CMD_W_TX_PL, pack_buff, data_len); 

    // // Set CE high to enter TX mode and start the transmission. Delay to ensure CE is high long 
    // // enough then set CE low so the device goes back to Standby-1 when done sending. 
    // nrf24l01_set_ce(GPIO_HIGH); 
    // tim_delay_us(nrf24l01_data.timer, NRF24L01_CE_TX_DELAY); 
    // nrf24l01_set_ce(GPIO_LOW); 

    // // Check to see if the TX FIFO is empty - means data was transmitted. 
    // // If data has not been transferred before timeout then it's considered to have failed. 
    // do 
    // {
    //     nrf24l01_fifo_status_reg_read(); 
    // }
    // while(!(nrf24l01_data.fifo_status.tx_empty) && (--time_out)); 

    // if (time_out)
    // {
    //     // Time left on the timer so data has successfully been set. 
    //     tx_status = SET_BIT; 
    // }
    
    // // Flush the TX FIFO 
    // nrf24l01_write(NRF24L01_CMD_FLUSH_TX, &buff, BYTE_0); 

    // // Set to a PRX device 
    // nrf24l01_set_data_mode(NRF24L01_RX_MODE); 

    // // Set CE back high to enter RX mode 
    // nrf24l01_set_ce(GPIO_HIGH); 

    // return tx_status; 
}


//==================================================
// RF_CH register 

// RF_CH register update 
NRF24L01_STATUS nrf24l01_rf_ch_read(void)
{
    return nrf24l01_reg_read(NRF24L01_REG_RF_CH, &nrf24l01_data.rf_ch.rf_ch_reg); 
}


// Get RF_CH channel 
uint8_t nrf24l01_get_rf_ch(void)
{
    // nrf24l01_rf_ch_reg_read(); 
    return nrf24l01_data.rf_ch.rf_ch; 
}


// Set RF_CH channel 
void nrf24l01_set_rf_ch(uint8_t rf_ch_freq)
{
    // // Set CE low to exit any active mode 
    // nrf24l01_set_ce(GPIO_LOW); 

    // Update and write the channel 
    nrf24l01_data.rf_ch.rf_ch = rf_ch_freq & NRF24L01_RF_CH_MASK; 
    // nrf24l01_rf_ch_reg_write(); 

    // // Set CE high to enter back into an active mode 
    // nrf24l01_set_ce(GPIO_HIGH); 
}


// RF_CH register write 
NRF24L01_STATUS nrf24l01_rf_ch_write(void)
{
    return nrf24l01_reg_write_data(NRF24L01_REG_RF_CH, nrf24l01_data.rf_ch.rf_ch_reg); 
}

//==================================================


//==================================================
// RF_SETUP register 

// RF_SETUP register read 
NRF24L01_STATUS nrf24l01_rf_setup_read(void)
{
    return nrf24l01_reg_read(NRF24L01_REG_RF_SET, &nrf24l01_data.rf_setup.rf_set_reg); 
}


// Get RF_SETUP data rate 
nrf24l01_data_rate_t nrf24l01_get_rf_setup_dr(void)
{
    // nrf24l01_rf_setup_reg_read(); 
    return (nrf24l01_data_rate_t)((nrf24l01_data.rf_setup.rf_dr_low << SHIFT_1) | 
                                   nrf24l01_data.rf_setup.rf_dr_high); 
}


// Get RF_SETUP power output 
nrf24l01_rf_pwr_t nrf24l01_get_rf_setup_pwr(void)
{
    // nrf24l01_rf_setup_reg_read(); 
    return (nrf24l01_rf_pwr_t)(nrf24l01_data.rf_setup.rf_pwr); 
}


// Set RF_SETUP data rate 
void nrf24l01_set_rf_setup_dr(nrf24l01_data_rate_t rate)
{
    // // Set CE low to exit any active mode 
    // nrf24l01_set_ce(GPIO_LOW); 

    // Update and write the data rate 
    nrf24l01_data.rf_setup.rf_dr_low = (rate >> SHIFT_1) & NRF24L01_RF_DR_MASK; 
    nrf24l01_data.rf_setup.rf_dr_high = rate & NRF24L01_RF_DR_MASK; 
    // nrf24l01_rf_setup_reg_write(); 

    // // Set CE high to enter back into an active mode 
    // nrf24l01_set_ce(GPIO_HIGH); 
}


// Set RF_SETUP power output 
void nrf24l01_set_rf_setup_pwr(nrf24l01_rf_pwr_t rf_pwr)
{
    // // Set CE low to exit any active mode 
    // nrf24l01_set_ce(GPIO_LOW); 
    
    // Update and write the power output 
    nrf24l01_data.rf_setup.rf_pwr = (uint8_t)rf_pwr; 
    // nrf24l01_rf_setup_reg_write(); 

    // // Set CE high to enter back into an active mode 
    // nrf24l01_set_ce(GPIO_HIGH); 
}


// RF_SETUP register write 
NRF24L01_STATUS nrf24l01_rf_setup_write(void)
{
    return nrf24l01_reg_write_data(NRF24L01_REG_RF_SET, nrf24l01_data.rf_setup.rf_set_reg); 
}

//==================================================


//==================================================
// CONFIG register 

// CONFIG register read 
NRF24L01_STATUS nrf24l01_config_read(void)
{
    return nrf24l01_reg_read(NRF24L01_REG_CONFIG, &nrf24l01_data.config.config_reg); 
}


// Get power mode 
nrf24l01_pwr_mode_t nrf24l01_get_config_pwr_mode(void)
{
    // nrf24l01_config_reg_read(); 
    return (nrf24l01_pwr_mode_t)(nrf24l01_data.config.pwr_up); 
}


// Get active mode 
nrf24l01_mode_select_t nrf24l01_get_config_mode(void)
{
    // nrf24l01_config_reg_read(); 
    return (nrf24l01_mode_select_t)(nrf24l01_data.config.prim_rx); 
}


// Power down 
NRF24L01_STATUS nrf24l01_pwr_down(void)
{
    // Make sure current data transfers are wrapped up. 
    // Set CE=0 to enter standby-1 state. 
    // Set PWR_UP=0 to enter power down state 

    // // Set CE low to exit any active mode 
    // nrf24l01_set_ce(GPIO_LOW); 

    // Set PWR_UP low to go to power down state 
    nrf24l01_data.config.pwr_up = (uint8_t)NRF24L01_PWR_DOWN; 
    // NRF24L01_STATUS status = nrf24l01_config_reg_write(); 
    NRF24L01_STATUS status = nrf24l01_config_write(); 
    nrf24l01_set_ce(GPIO_LOW); 

    if (status != NRF24L01_OK)
    {
        // Write operation failed. Set the CE pin back high 
        nrf24l01_set_ce(GPIO_HIGH); 
    }

    return status; 
}


// Power up 
NRF24L01_STATUS nrf24l01_pwr_up(void)
{   
    // Set PWR_UP high to exit the power down state 
    nrf24l01_data.config.pwr_up = (uint8_t)NRF24L01_PWR_UP; 
    // NRF24L01_STATUS status = nrf24l01_config_reg_write(); 
    NRF24L01_STATUS status = nrf24l01_config_write(); 
    nrf24l01_set_ce(GPIO_LOW); 

    if (status == NRF24L01_OK)
    {
        // Delay to allow for the startup state to pass (~1.5ms) 
        tim_delay_ms(nrf24l01_data.timer, NRF24L01_START_DELAY); 

        // Set CE high to enter back into an active mode 
        nrf24l01_set_ce(GPIO_HIGH); 
    }

    return status; 
}


// CONFIG register write 
NRF24L01_STATUS nrf24l01_config_write(void)
{
    return nrf24l01_reg_write_data(NRF24L01_REG_CONFIG, nrf24l01_data.config.config_reg); 
}

//==================================================

//=======================================================================================


//=======================================================================================
// Configuration functions 

// Set CE pin 
void nrf24l01_set_ce(gpio_pin_state_t state)
{
    gpio_write(nrf24l01_data.gpio_en, nrf24l01_data.en_pin, state); 
}


// Set active mode 
NRF24L01_STATUS nrf24l01_set_data_mode(nrf24l01_mode_select_t mode)
{
    // Write PRIM_RX=mode to the device 
    nrf24l01_data.config.prim_rx = (uint8_t)mode; 
    NRF24L01_STATUS nrf24l01_status = nrf24l01_config_reg_write(); 
    nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, nrf24l01_data.config.config_reg); 

    // Delay to clear the device settling state 
    tim_delay_us(nrf24l01_data.timer, NRF24L01_SETTLE_DELAY); 

    return nrf24l01_status; 
}

//=======================================================================================


//=======================================================================================
// Read and write 

// Receive data from another transceiver 
SPI_STATUS nrf24l01_receive(
    uint8_t cmd, 
    uint8_t *rec_buff, 
    uint8_t data_len)
{
    SPI_STATUS spi_status = SPI_OK; 
    // uint8_t status_reg = CLEAR; 

    // Write the command and read the status back from the slave. Then read the data 
    // from the device. 
    spi_slave_select(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 
    // spi_status |= spi_write_read(nrf24l01_data.spi, cmd, &status_reg, BYTE_1); 
    spi_status |= spi_write_read(nrf24l01_data.spi, cmd, 
                                 &nrf24l01_data.status.status_reg, BYTE_1); 
    spi_status |= spi_write_read(nrf24l01_data.spi, SPI_DUMMY, rec_buff, data_len); 
    spi_slave_deselect(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 

    // // Update the status register data record 
    // nrf24l01_status_reg_update(status_reg); 

    return spi_status; 
}


// Send data to another transceiver 
SPI_STATUS nrf24l01_write(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len)
{
    SPI_STATUS spi_status = SPI_OK; 
    // uint8_t status_reg = CLEAR; 

    // Write the command and read the status back from the slave. Then write the data 
    // to the device. 
    spi_slave_select(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 
    // spi_status |= spi_write_read(nrf24l01_data.spi, cmd, &status_reg, BYTE_1); 
    spi_status |= spi_write_read(nrf24l01_data.spi, cmd, 
                                 &nrf24l01_data.status.status_reg, BYTE_1); 
    spi_status |= spi_write(nrf24l01_data.spi, send_buff, data_len); 
    spi_slave_deselect(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 

    // // Update the status register data record 
    // nrf24l01_status_reg_update(status_reg); 

    return spi_status; 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// CONFIG register write 
NRF24L01_STATUS nrf24l01_config_reg_write(void)
{
    // // Format the data to send 
    // uint8_t config = (nrf24l01_data.config.unused_1    << SHIFT_7) | 
    //                  (nrf24l01_data.config.mask_rx_dr  << SHIFT_6) | 
    //                  (nrf24l01_data.config.mask_tx_ds  << SHIFT_5) | 
    //                  (nrf24l01_data.config.mask_max_rt << SHIFT_4) | 
    //                  (nrf24l01_data.config.en_crc      << SHIFT_3) | 
    //                  (nrf24l01_data.config.crco        << SHIFT_2) | 
    //                  (nrf24l01_data.config.pwr_up      << SHIFT_1) | 
    //                  (nrf24l01_data.config.prim_rx); 

    // Send the data to the CONFIG register 
    // nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, config); 
    
    return nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, nrf24l01_data.config.config_reg); 
}


// // CONFIG register read 
// NRF24L01_STATUS nrf24l01_config_reg_read(void)
// {
//     // // Read the CONFIG register and update the data record 
//     // uint8_t config = nrf24l01_reg_read(NRF24L01_REG_CONFIG); 

//     // nrf24l01_data.config.unused_1    = (config & FILTER_BIT_7) >> SHIFT_7; 
//     // nrf24l01_data.config.mask_rx_dr  = (config & FILTER_BIT_6) >> SHIFT_6; 
//     // nrf24l01_data.config.mask_tx_ds  = (config & FILTER_BIT_5) >> SHIFT_5; 
//     // nrf24l01_data.config.mask_max_rt = (config & FILTER_BIT_4) >> SHIFT_4; 
//     // nrf24l01_data.config.en_crc      = (config & FILTER_BIT_3) >> SHIFT_3; 
//     // nrf24l01_data.config.crco        = (config & FILTER_BIT_2) >> SHIFT_2; 
//     // nrf24l01_data.config.pwr_up      = (config & FILTER_BIT_1) >> SHIFT_1; 
//     // nrf24l01_data.config.prim_rx     = (config & FILTER_BIT_0); 

//     // return config; 

//     return nrf24l01_reg_read(NRF24L01_REG_CONFIG, &nrf24l01_data.config.config_reg); 
// }


// // RF_CH register write 
// NRF24L01_STATUS nrf24l01_rf_ch_reg_write(void)
// {
//     // // Format the data to send 
//     // uint8_t rf_ch = (nrf24l01_data.rf_ch.unused_1 << SHIFT_7) | 
//     //                 (nrf24l01_data.rf_ch.rf_ch); 

//     // // Send the data to the RF_CH register 
//     // nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, rf_ch); 

//     return nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, nrf24l01_data.rf_ch.rf_ch_reg); 
// }


// // RF_CH register read 
// NRF24L01_STATUS nrf24l01_rf_ch_reg_read(void)
// {
//     // // Read the RF_CH register and update the data record 
//     // uint8_t rf_ch = nrf24l01_reg_read(NRF24L01_REG_RF_CH); 

//     // nrf24l01_data.rf_ch.unused_1 = (rf_ch & FILTER_BIT_7) >> SHIFT_7; 
//     // nrf24l01_data.rf_ch.rf_ch    = (rf_ch & FILTER_7_LSB); 

//     // return rf_ch; 

//     return nrf24l01_reg_read(NRF24L01_REG_RF_CH, &nrf24l01_data.rf_ch.rf_ch_reg); 
// }


// // RF_SETUP register write 
// NRF24L01_STATUS nrf24l01_rf_setup_reg_write(void)
// {
//     // // Format the data to send 
//     // uint8_t rf_setup = (nrf24l01_data.rf_setup.cont_wave  << SHIFT_7) | 
//     //                    (nrf24l01_data.rf_setup.unused_1   << SHIFT_6) | 
//     //                    (nrf24l01_data.rf_setup.rf_dr_low  << SHIFT_5) | 
//     //                    (nrf24l01_data.rf_setup.pll_lock   << SHIFT_4) | 
//     //                    (nrf24l01_data.rf_setup.rf_dr_high << SHIFT_3) | 
//     //                    (nrf24l01_data.rf_setup.rf_pwr     << SHIFT_1) | 
//     //                    (nrf24l01_data.rf_setup.unused_2); 

//     // // Send the data to the RF_SET register 
//     // nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, rf_setup); 

//     return nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, nrf24l01_data.rf_setup.rf_set_reg); 
// }


// // RF_SETUP register read 
// NRF24L01_STATUS nrf24l01_rf_setup_reg_read(void)
// {
//     // // Read the RF_CH register and update the data record 
//     // uint8_t rf_setup = nrf24l01_reg_read(NRF24L01_REG_RF_SET); 

//     // nrf24l01_data.rf_setup.cont_wave  = (rf_setup & FILTER_BIT_7) >> SHIFT_7; 
//     // nrf24l01_data.rf_setup.unused_1   = (rf_setup & FILTER_BIT_6) >> SHIFT_6; 
//     // nrf24l01_data.rf_setup.rf_dr_low  = (rf_setup & FILTER_BIT_5) >> SHIFT_5; 
//     // nrf24l01_data.rf_setup.pll_lock   = (rf_setup & FILTER_BIT_4) >> SHIFT_4; 
//     // nrf24l01_data.rf_setup.rf_dr_high = (rf_setup & FILTER_BIT_3) >> SHIFT_3; 
//     // nrf24l01_data.rf_setup.rf_pwr     = (rf_setup & FILTER_3_LSB) >> SHIFT_1; 
//     // nrf24l01_data.rf_setup.unused_2   = (rf_setup & FILTER_BIT_0); 

//     // return rf_setup; 

//     return nrf24l01_reg_read(NRF24L01_REG_RF_SET, &nrf24l01_data.rf_setup.rf_set_reg); 
// }


// STATUS register write 
NRF24L01_STATUS nrf24l01_status_reg_write(void)
{
    // // Format the data to send 
    // uint8_t status_reg = (nrf24l01_data.status.unused_1 << SHIFT_7) | 
    //                      (nrf24l01_data.status.rx_dr    << SHIFT_6) | 
    //                      (nrf24l01_data.status.tx_ds    << SHIFT_5) | 
    //                      (nrf24l01_data.status.max_rt   << SHIFT_4) | 
    //                      (nrf24l01_data.status.rx_p_no  << SHIFT_1) | 
    //                      (nrf24l01_data.status.tx_full); 

    // // Send the data to the STATUS register 
    // nrf24l01_reg_byte_write(NRF24L01_REG_STATUS, status_reg); 

    return nrf24l01_reg_byte_write(NRF24L01_REG_STATUS, nrf24l01_data.status.status_reg); 
}


// STATUS register read 
NRF24L01_STATUS nrf24l01_status_reg_read(void)
{
    // Write a no operation command to the device and the status register will be checked 
    uint8_t buff = CLEAR; 
    SPI_STATUS spi_status = nrf24l01_receive(NRF24L01_CMD_NOP, &buff, BYTE_0); 

    if (spi_status)
    {
        return NRF24L01_READ_FAULT; 
    }

    return NRF24L01_OK; 
}


// // STATUS register state update 
// void nrf24l01_status_reg_update(uint8_t status)
// {
//     // Sort the status register byte into the data record 
//     nrf24l01_data.status.unused_1 = (status & FILTER_BIT_7) >> SHIFT_7; 
//     nrf24l01_data.status.rx_dr    = (status & FILTER_BIT_6) >> SHIFT_6; 
//     nrf24l01_data.status.tx_ds    = (status & FILTER_BIT_5) >> SHIFT_5; 
//     nrf24l01_data.status.max_rt   = (status & FILTER_BIT_4) >> SHIFT_4; 
//     nrf24l01_data.status.rx_p_no  = (status & FILTER_4_LSB) >> SHIFT_1; 
//     nrf24l01_data.status.tx_full  = (status & FILTER_BIT_0); 
// }


// FIFO_STATUS register read 
NRF24L01_STATUS nrf24l01_fifo_status_reg_read(void)
{
    // // Read the FIFO_STATUS register and update the data record 
    // uint8_t fifo_status = nrf24l01_reg_read(NRF24L01_REG_FIFO); 

    // // Store the FIFO status register byte into the data record 
    // nrf24l01_data.fifo_status.unused_1 = (fifo_status & FILTER_BIT_7) >> SHIFT_7; 
    // nrf24l01_data.fifo_status.tx_reuse = (fifo_status & FILTER_BIT_6) >> SHIFT_6; 
    // nrf24l01_data.fifo_status.tx_full  = (fifo_status & FILTER_BIT_5) >> SHIFT_5; 
    // nrf24l01_data.fifo_status.tx_empty = (fifo_status & FILTER_BIT_4) >> SHIFT_4; 
    // nrf24l01_data.fifo_status.unused_2 = (fifo_status & FILTER_4_LSB) >> SHIFT_2; 
    // nrf24l01_data.fifo_status.rx_full  = (fifo_status & FILTER_BIT_1) >> SHIFT_1; 
    // nrf24l01_data.fifo_status.rx_empty = (fifo_status & FILTER_BIT_0); 

    // return fifo_status; 

    return nrf24l01_reg_read(NRF24L01_REG_FIFO, 
                             &nrf24l01_data.fifo_status.fifo_status_reg); 
}


// // Register read 
// uint8_t nrf24l01_reg_read(uint8_t reg_addr)
// {
//     // Read and return the register value 
//     uint8_t reg_read = CLEAR; 
//     nrf24l01_receive(NRF24L01_CMD_R_REG | reg_addr, &reg_read, BYTE_1); 
//     return reg_read; 
// }


// Register read 
NRF24L01_STATUS nrf24l01_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_data)
{
    SPI_STATUS spi_status = nrf24l01_receive(NRF24L01_CMD_R_REG | reg_addr, reg_data, BYTE_1); 

    if (spi_status)
    {
        return NRF24L01_READ_FAULT; 
    }

    return NRF24L01_OK; 
}


// 
NRF24L01_STATUS nrf24l01_reg_write_data(
    uint8_t reg_addr, 
    uint8_t reg_data)
{
    // Set CE low to exit any active mode, write dta to the register, then set CE high 
    // enter back into an active mode. 
    nrf24l01_set_ce(GPIO_LOW); 
    NRF24L01_STATUS status = nrf24l01_reg_byte_write(reg_addr, reg_data); 
    nrf24l01_set_ce(GPIO_HIGH); 

    return status; 
}


// Register byte write 
NRF24L01_STATUS nrf24l01_reg_byte_write(
    uint8_t reg_addr, 
    uint8_t reg_data)
{
    return nrf24l01_reg_write(reg_addr, &reg_data, BYTE_1); 
}


// Register write 
NRF24L01_STATUS nrf24l01_reg_write(
    uint8_t reg_addr, 
    const uint8_t *reg_data, 
    uint8_t reg_size)
{
    SPI_STATUS spi_status = nrf24l01_write(NRF24L01_CMD_W_REG | reg_addr, reg_data, reg_size); 

    if (spi_status)
    {
        return NRF24L01_WRITE_FAULT; 
    }

    return NRF24L01_OK; 
}

//=======================================================================================
