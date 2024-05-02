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


#define NEW_CODE 0 
#define WORKING_CODE 1 
#define ORIGINAL_CODE 0 


// Data handling 
#define NRF24L01_RF_CH_MASK 0x7F       // RF channel frequency mask 
#define NRF24L01_RF_DR_MASK 0x01       // RF data rate bit mask 
#define NRF24L01_DATA_SIZE_LEN 1       // Data size indicator length 
#define NRF24L01_MAX_DATA_LEN 30       // Max user data length 

//==================================================
// Timing 

// Initialization 
#define NRF24L01_PWR_ON_DELAY 100      // Device power on reset delay (ms) 
#define NRF24L01_START_DELAY 2         // Device start up delay (ms) 

// TX 
#define NRF24L01_TX_DELAY 500          // Time before checking for successful transmission (us) 
#define NRF24L01_TX_TIMEOUT 10         // Max times to check for successful transmission 
#define NRF24L01_CE_TX_DELAY 20        // Time CE is held high in TX (us) 

// General 
#define NRF24L01_RW_DELAY 1            // Delay between successive writes/reads (ms) 

//==================================================

// Control 
#define NRF24L01_DISABLE_REG 0x00      // Disable settings in a register 
#define NRF24L01_STATUS_CLEAR 0x70     // STATUS register clear bits 

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
    NRF24L01_REG_CONFIG     = 0x00,   // CONFIG 
    NRF24L01_REG_EN_AA      = 0x01,   // EN_AA 
    NRF24L01_REG_EN_RXADDR  = 0x02,   // EN_RXADDR 
    NRF24L01_REG_SETUP_AW   = 0x03,   // SETUP_AW 
    NRF24L01_REG_SETUP_RETR = 0x04,   // SETUP_RETR 
    NRF24L01_REG_RF_CH      = 0x05,   // RF_CH 
    NRF24L01_REG_RF_SET     = 0x06,   // RF_SETUP 
    NRF24L01_REG_STATUS     = 0x07,   // STATUS 
    NRF24L01_REG_OBSERVE_TX = 0x08,   // OBSERVE_TX 
    NRF24L01_REG_RPD        = 0x09,   // RPD 
    NRF24L01_REG_RX_ADDR_P0 = 0x0A,   // RX_ADDR_P0 
    NRF24L01_REG_RX_ADDR_P1 = 0x0B,   // RX_ADDR_P1 
    NRF24L01_REG_RX_ADDR_P2 = 0x0C,   // RX_ADDR_P2 
    NRF24L01_REG_RX_ADDR_P3 = 0x0D,   // RX_ADDR_P3 
    NRF24L01_REG_RX_ADDR_P4 = 0x0E,   // RX_ADDR_P4 
    NRF24L01_REG_RX_ADDR_P5 = 0x0F,   // RX_ADDR_P5 
    NRF24L01_REG_TX_ADDR    = 0x10,   // TX_ADDR 
    NRF24L01_REG_RX_PW_P0   = 0x11,   // RX_PW_P0 
    NRF24L01_REG_RX_PW_P1   = 0x12,   // RX_PW_P1 
    NRF24L01_REG_RX_PW_P2   = 0x13,   // RX_PW_P2 
    NRF24L01_REG_RX_PW_P3   = 0x14,   // RX_PW_P3 
    NRF24L01_REG_RX_PW_P4   = 0x15,   // RX_PW_P4 
    NRF24L01_REG_RX_PW_P5   = 0x16,   // RX_PW_P5 
    NRF24L01_REG_FIFO       = 0x17,   // FIFO_STATUS 
    NRF24L01_REG_DYNPD      = 0x1C,   // DYNPD 
    NRF24L01_REG_FEATURE    = 0x1D    // FEATURE 
} nrf24l01_reg_addr_t; 


// Register reset values 
typedef enum {
    NRF24L01_REG_RESET_CONFIG      = 0x08,   // CONFIG 
    NRF24L01_REG_RESET_EN_AA       = 0x3F,   // EN_AA 
    NRF24L01_REG_RESET_EN_RXADDR   = 0x03,   // EN_RXADDR 
    NRF24L01_REG_RESET_SETUP_AW    = 0x03,   // SETUP_AW 
    NRF24L01_REG_RESET_SETUP_RETR  = 0x03,   // SETUP_RETR 
    NRF24L01_REG_RESET_RF_CH       = 0x02,   // RF_CH 
    NRF24L01_REG_RESET_RF_SETUP    = 0x0E,   // RF_SETUP 
    NRF24L01_REG_RESET_STATUS      = 0x0E,   // STATUS 
    NRF24L01_REG_RESET_OBSERVE_TX  = 0x00,   // OBSERVE_TX 
    NRF24L01_REG_RESET_RPD         = 0x00,   // RPD 
    NRF24L01_REG_RESET_RX_ADDR_P0  = 0xE7,   // RX_ADDR_P0 
    NRF24L01_REG_RESET_RX_ADDR_P1  = 0xC2,   // RX_ADDR_P1 
    NRF24L01_REG_RESET_RX_ADDR_P2  = 0xC3,   // RX_ADDR_P2 
    NRF24L01_REG_RESET_RX_ADDR_P3  = 0xC4,   // RX_ADDR_P3 
    NRF24L01_REG_RESET_RX_ADDR_P4  = 0xC5,   // RX_ADDR_P4 
    NRF24L01_REG_RESET_RX_ADDR_P5  = 0xC6,   // RX_ADDR_P5 
    NRF24L01_REG_RESET_TX_ADDR     = 0xE7,   // TX_ADDR 
    NRF24L01_REG_RESET_RX_PW_PX    = 0x00,   // RX_PW_PX (X-->0-5) 
    NRF24L01_REG_RESET_FIFO_STATUS = 0x11,   // FIFO_STATUS 
    NRF24L01_REG_RESET_DYNPD       = 0x00,   // DYNPD 
    NRF24L01_REG_RESET_FEATURE     = 0x00    // FEATURE 
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
 * @brief CE pin enable 
 * 
 * @details Set CE pin high to enter an active mode 
 */
void nrf24l01_ce_enable(void); 


/**
 * @brief CE pin disable 
 * 
 * @details Set CE pin low to exit an active mode 
 */
void nrf24l01_ce_disable(void); 


/**
 * @brief Set active mode (TX/RX) 
 * 
 * @details Updates the PRIM_RX bit and writes it to the CONFIG register. PRIM_RX 
 *          changes the device between TX (0) and RX (1) mode. This function is used 
 *          primarily when sending a payload where the device will be removed from RX 
 *          mode (default/resting mode) to send data, then put back into RX mode when 
 *          done so the device can look for messages. 
 * 
 * @see nrf24l01_mode_select_t 
 * 
 * @param mode : TX or RX mode 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_set_data_mode(nrf24l01_mode_select_t mode); 


/**
 * @brief CONFIG register write 
 * 
 * @see nrf24l01_config_reg_t 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_config_write(void); 


/**
 * @brief STATUS register write 
 * 
 * @see nrf24l01_status_reg_t 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_status_reg_write(void); 


/**
 * @brief STATUS register read 
 * 
 * @see nrf24l01_status_reg_t 
 * 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_status_reg_read(void); 


/**
 * @brief FIFO_STATUS register read 
 * 
 * @see nrf24l01_fifo_status_reg_t 
 * 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_fifo_status_reg_read(void); 


/**
 * @brief Register read 
 * 
 * @details Reads a single byte register from the device. 
 * 
 * @param reg_addr : address of register to read 
 * @param reg_data : buffer to store the register data 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_data); 


/**
 * @brief Read data from the device 
 * 
 * @details Writes a read command to the device (either a register, RX payload or no 
 *          operation command depending on what's passed to the function) then reads data 
 *          of a specified length back from the device. 
 *          
 *          Note that when writing a command to the device, the device will simultaneously 
 *          write the status register contents back. This means the status register data 
 *          record gets updated every read/write operation. 
 * 
 * @see nrf24l01_cmds_t 
 * 
 * @param cmd : read command 
 * @param rec_buff : buffer to store received data 
 * @param data_len : length of data to read (excluding status) 
 * @return NRF24L01_STATUS : read status 
 */
NRF24L01_STATUS nrf24l01_receive(
    uint8_t cmd, 
    uint8_t *rec_buff, 
    uint8_t data_len); 


/**
 * @brief FIFO flush 
 * 
 * @details Flushes the contents of the TX or RX FIFO by writing the specified flush 
 *          command to the device. This is used to ensure there is no leftover unwanted 
 *          data in the FIFO. 
 *          
 *          Note that this function should only be passed either the NRF24L01_CMD_FLUSH_TX 
 *          or NRF24L01_CMD_FLUSH_RX commands. 
 * 
 * @see nrf24l01_cmds_t 
 * 
 * @param flush_cmd : either TX or RX FIFO flush command 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_fifo_flush(uint8_t flush_cmd); 


/**
 * @brief Register byte write 
 * 
 * @details Writes to a single byte register on the device. 
 * 
 * @param reg_addr : address of register to write/update 
 * @param reg_data : byte to write to the register address 
 * @return NRF24L01_STATUS : status of the write operation 
 */
NRF24L01_STATUS nrf24l01_reg_byte_write(
    uint8_t reg_addr, 
    uint8_t reg_data); 


/**
 * @brief Register write 
 * 
 * @details Writes to device registers of a specified size. 
 * 
 * @see nrf24l01_reg_addr_t 
 * 
 * @param reg_addr : starting address of register to write/update 
 * @param reg_data : data buffer to write starting at the register address 
 * @param reg_size : register size (bytes) 
 * @return NRF24L01_STATUS : status of the write operation 
 */
NRF24L01_STATUS nrf24l01_reg_write(
    uint8_t reg_addr, 
    const uint8_t *reg_data, 
    uint8_t reg_size); 


/**
 * @brief Write data to the device 
 * 
 * @details Writes a write command to the device (either a register, TX payload or flush 
 *          command depending on what's passed to the function) then writes data 
 *          of a specified length to the device. 
 *          
 *          Note that when writing a command to the device, the device will simultaneously 
 *          write the status register contents back. This means the status register data 
 *          record gets updated every read/write operation. 
 * 
 * @param cmd : write command 
 * @param send_buff : data buffer to write 
 * @param data_len : length of data to send (excluding command) 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_write(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len); 

//=======================================================================================


//=======================================================================================
// Initialization 

// nRF24L01 initialization 
#if NEW_CODE 
NRF24L01_STATUS nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer, 
    uint8_t rf_ch_freq, 
    nrf24l01_data_rate_t data_rate, 
    nrf24l01_rf_pwr_t rf_pwr)
#elif WORKING_CODE 
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
#endif 
{
    //==================================================
    // Initialize data 

    // Peripherals 
    nrf24l01_data.spi = spi; 
    nrf24l01_data.gpio_ss = gpio_ss; 
    nrf24l01_data.gpio_en = gpio_en; 
    nrf24l01_data.ss_pin = (gpio_pin_num_t)(SET_BIT << ss_pin); 
    nrf24l01_data.en_pin = (gpio_pin_num_t)(SET_BIT << en_pin); 
    nrf24l01_data.timer = timer; 

    // Register data 
    nrf24l01_data.config.config_reg = NRF24L01_REG_RESET_CONFIG; 
    nrf24l01_data.rf_ch.rf_ch_reg = NRF24L01_REG_RESET_RF_CH; 
    nrf24l01_data.rf_setup.rf_set_reg = NRF24L01_REG_RESET_RF_SETUP; 
    nrf24l01_data.status.status_reg = NRF24L01_REG_RESET_STATUS; 
    nrf24l01_data.fifo_status.fifo_status_reg = NRF24L01_REG_RESET_FIFO_STATUS; 
    
    //==================================================

    //==================================================
    // Configure GPIO pins 

    // SPI slave select pin (CSN) 
    spi_ss_init(nrf24l01_data.gpio_ss, ss_pin); 

    // Device enable pin (CE) 
    gpio_pin_init(
        nrf24l01_data.gpio_en, 
        en_pin, 
        MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    
    //==================================================

#if NEW_CODE 

    NRF24L01_STATUS init_status = NRF24L01_OK; 

    nrf24l01_ce_disable(); 

    //==================================================
    // Device initialization 

    // Configure registers to work for this driver. Registers are either reset to their 
    // default value or diabled. Specific settings for PTX and PRX devices are set 
    // using the PTX and PRX config functions below. 

    // Delay to ensure power on reset state is cleared before accessing the device 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_PWR_ON_DELAY); 

    // Flush the FIFOs to ensure no leftover data 
    init_status |= nrf24l01_fifo_flush(NRF24L01_CMD_FLUSH_TX); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 
    init_status |= nrf24l01_fifo_flush(NRF24L01_CMD_FLUSH_RX); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // CONFIG - leave the device in power down mode for initialization 
    nrf24l01_data.config.config_reg = NRF24L01_REG_RESET_CONFIG; 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, 
                                           nrf24l01_data.config.config_reg); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 
    
    // EN_AA - disable auto acknowledgment (Enhanced Shockburst TM not used) 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_EN_AA, NRF24L01_DISABLE_REG); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_EN_AA, NRF24L01_REG_RESET_EN_AA); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 
    
    // EN_RXADDR - disable data pipes for now - configured separately 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, NRF24L01_DISABLE_REG); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, NRF24L01_REG_RESET_EN_RXADDR); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // SETUP_AW 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_AW, 
                                           NRF24L01_REG_RESET_SETUP_AW); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 
    
    // SETUP_RETR - disable retransmission because auto acknowledgement not used 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_RETR, NRF24L01_DISABLE_REG); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_RETR, NRF24L01_REG_RESET_SETUP_RETR); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RF_CH 
    nrf24l01_data.rf_ch.rf_ch_reg = NRF24L01_REG_RESET_RF_CH; 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, 
                                           nrf24l01_data.rf_ch.rf_ch_reg); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RF_SETUP 
    nrf24l01_data.rf_setup.rf_set_reg = NRF24L01_REG_RESET_RF_SETUP; 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, 
                                           nrf24l01_data.rf_setup.rf_set_reg); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // STATUS - clear all the writeable bits - status gets updated with each write/read 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_STATUS, 
                                           NRF24L01_REG_RESET_STATUS | NRF24L01_STATUS_CLEAR); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // OBSERVE_TX 
    // nrf24l01_reg_byte_write(NRF24L01_REG_OBSERVE_TX, NRF24L01_REG_RESET_OBSERVE_TX); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RPD 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RPD, NRF24L01_REG_RESET_RPD); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_ADDR_P0 
    // nrf24l01_reg_write(NRF24L01_REG_RX_ADDR_P0, p0_addr_buff, NRF24l01_ADDR_WIDTH); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_ADDR_P1 
    // nrf24l01_reg_write(NRF24L01_REG_RX_ADDR_P1, p1_addr_buff, NRF24l01_ADDR_WIDTH); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_ADDR_P2 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P2, NRF24L01_REG_RESET_RX_ADDR_P2); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_ADDR_P3 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P3, NRF24L01_REG_RESET_RX_ADDR_P3); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_ADDR_P4 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P4, NRF24L01_REG_RESET_RX_ADDR_P4); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_ADDR_P5 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P5, NRF24L01_REG_RESET_RX_ADDR_P5); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // TX_ADDR 
    // nrf24l01_reg_write(NRF24L01_REG_TX_ADDR, p0_addr_buff, NRF24l01_ADDR_WIDTH); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_PW_P0 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P0, NRF24L01_REG_RESET_RX_PW_PX); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_PW_P1 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P1, NRF24L01_REG_RESET_RX_PW_PX); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_PW_P2 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P2, NRF24L01_REG_RESET_RX_PW_PX); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_PW_P3 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P3, NRF24L01_REG_RESET_RX_PW_PX); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_PW_P4 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P4, NRF24L01_REG_RESET_RX_PW_PX); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_PW_P5 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RX_PW_P5, NRF24L01_REG_RESET_RX_PW_PX); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // FIFO_STATUS - read the FIFO status register to update the driver data record 
    init_status |= nrf24l01_fifo_status_reg_read(); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // ACK_PLD 

    // TX_PLD 

    // RX_PLD 

    // DYNPD 
    // nrf24l01_reg_byte_write(NRF24L01_REG_DYNPD, NRF24L01_REG_RESET_DYNPD); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // FEATURE 
    // nrf24l01_reg_byte_write(NRF24L01_REG_FEATURE, NRF24L01_REG_RESET_FEATURE); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    //==================================================

    //==================================================
    // User initialization 

    // Configure the settings specified by the user. These can be updated later using 
    // setters if needed. 

    // RF_CH 
    nrf24l01_set_rf_ch(rf_ch_freq); 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, 
                                           nrf24l01_data.rf_ch.rf_ch_reg); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RF_SETUP 
    nrf24l01_set_rf_setup_dr(data_rate); 
    nrf24l01_set_rf_setup_pwr(rf_pwr); 
    init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, 
                                           nrf24l01_data.rf_setup.rf_set_reg); 
    
    //==================================================

    nrf24l01_ce_enable(); 

    return init_status; 

#elif WORKING_CODE 
    
    // Set CE low to prevent entering an active (TX/RX) state 
    nrf24l01_ce_disable(); 

    //==================================================
    // Device initialization 

    // The following registers are modified from their reset value so they will work with 
    // this driver and with how the user needs them set up. Changes here are common 
    // between TX and RX devices. Further changes to TX and RX specific settings are done 
    // using the PTX and PRX config functions below. 

    // Delay to ensure power on reset state is cleared before accessing the device 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_PWR_ON_DELAY); 

    // CONFIG - Power up the device and default to a PRX device. Delay to allow time 
    // for the start up state to pass. 
    // nrf24l01_config_write(); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_START_DELAY); 
    
    // EN_AA - disable auto acknowledgment (Enhanced Shockburst TM not used) 
    nrf24l01_reg_byte_write(NRF24L01_REG_EN_AA, NRF24L01_DISABLE_REG); 
    
    // EN_RXADDR - disable data pipes for now - configured separately 
    nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, NRF24L01_DISABLE_REG); 

    // SETUP_AW 
    // nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_AW, NRF24L01_REG_RESET_SETUP_AW); 
    
    // SETUP_RETR - disable retransmission because auto acknowledgement not used 
    nrf24l01_reg_byte_write(NRF24L01_REG_SETUP_RETR, NRF24L01_DISABLE_REG); 

    // RF_CH 
    nrf24l01_set_rf_ch(rf_ch_freq); 
    nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, nrf24l01_data.rf_ch.rf_ch_reg); 

    // RF_SETUP 
    nrf24l01_set_rf_setup_dr(data_rate); 
    nrf24l01_set_rf_setup_pwr(rf_pwr); 
    nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, nrf24l01_data.rf_setup.rf_set_reg); 

    //==================================================

    //==================================================
    // User initialization 

    // Configure the settings specified by the user. These can be updated later using 
    // setters if needed. 

    // RF_CH 
    // nrf24l01_set_rf_ch(rf_ch_freq); 
    // init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, 
    //                                        nrf24l01_data.rf_ch.rf_ch_reg); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, nrf24l01_data.rf_ch.rf_ch_reg); 
    // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // // RF_SETUP 
    // nrf24l01_set_rf_setup_dr(data_rate); 
    // nrf24l01_set_rf_setup_pwr(rf_pwr); 
    // init_status |= nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, 
    //                                        nrf24l01_data.rf_setup.rf_set_reg); 
    
    //==================================================

    // Set CE high to enter RX mode 
    nrf24l01_ce_enable(); 

#endif 
}


// Configure a devices PTX settings 
void nrf24l01_ptx_config(const uint8_t *tx_addr)
{
    if (tx_addr == NULL)
    {
        return; 
    }

#if NEW_CODE 

    nrf24l01_ce_disable(); 

    // Set TX_ADDR 
    // Don't need to match RX_ADDR_P0 because auto acknowledge is not being used. 
    nrf24l01_reg_write(NRF24L01_REG_TX_ADDR, tx_addr, NRF24l01_ADDR_WIDTH); 

    nrf24l01_ce_enable(); 

#elif WORKING_CODE 

    // Set CE low to exit any active mode 
    nrf24l01_ce_disable(); 

    // //==================================================
    // // RF_CH 
    // // 'nrf24l01_rf_ch_write' is not used here because CE is already set low and must 
    // // remain low until PTX configuration is done. 
    // nrf24l01_set_rf_ch(10); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, nrf24l01_data.rf_ch.rf_ch_reg); 
    // //==================================================

    // Set up TX_ADDR 
    // Don't need to match RX_ADDR_P0 because auto acknowledge is not being used. 
    nrf24l01_reg_write(NRF24L01_REG_TX_ADDR, tx_addr, NRF24l01_ADDR_WIDTH); 

    //==================================================
    // CONFIG 
    // We read first because the config register data record has not been initialized. 
    nrf24l01_config_read(); 
    nrf24l01_data.config.pwr_up = SET_BIT;   // Set to start up the device 
    nrf24l01_config_write(); 
    //==================================================

    // Set CE high to enter back into an active mode 
    nrf24l01_ce_enable(); 

#endif 
}


// Configure a devices PRX settings 
void nrf24l01_prx_config(
    const uint8_t *rx_addr, 
    nrf24l01_data_pipe_t pipe_num)
{
    uint8_t en_rxaddr = CLEAR; 

    if (rx_addr == NULL)
    {
        return; 
    }

#if NEW_CODE 

    nrf24l01_ce_disable(); 

    // EN_RXADDR - set the data pipe to enable 
    nrf24l01_reg_read(NRF24L01_REG_EN_RXADDR, &en_rxaddr); 
    nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, 
                            en_rxaddr | (SET_BIT << (uint8_t)pipe_num)); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_ADDR_PX - set the chosen data pipe address 
    if (pipe_num <= NRF24L01_DP_1)
    {
        nrf24l01_reg_write(
            NRF24L01_REG_RX_ADDR_P0 + (uint8_t)pipe_num, 
            rx_addr, 
            NRF24l01_ADDR_WIDTH); 
    }
    else if (pipe_num <= NRF24L01_DP_5)
    {
        nrf24l01_reg_byte_write(NRF24L01_REG_RX_ADDR_P0 + (uint8_t)pipe_num, *rx_addr); 
    }
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

    // RX_PW_PX - set the max number of bytes in RX payload in the chosen data pipe 
    nrf24l01_reg_byte_write(
        (NRF24L01_REG_RX_PW_P0 + (uint8_t)pipe_num), 
        NRF24L01_MAX_PAYLOAD_LEN); 

    nrf24l01_ce_enable(); 

#elif WORKING_CODE 

    // Set CE low to exit any active mode 
    nrf24l01_ce_disable(); 

    // //==================================================
    // // RF_CH 
    // // 'nrf24l01_rf_ch_write' is not used here because CE is already set low and must 
    // // remain low until PTX configuration is done. 
    // nrf24l01_set_rf_ch(10); 
    // nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, nrf24l01_data.rf_ch.rf_ch_reg); 
    // //==================================================

    // EN_RXADDR - set the data pipe to enable 
    nrf24l01_reg_read(NRF24L01_REG_EN_RXADDR, &en_rxaddr); 
    nrf24l01_reg_byte_write(NRF24L01_REG_EN_RXADDR, 
                            en_rxaddr | (SET_BIT << (uint8_t)pipe_num)); 

    // RX_ADDR_PX - set the chosen data pipe address 
    nrf24l01_reg_write(
        NRF24L01_REG_RX_ADDR_P0 + (uint8_t)pipe_num, 
        rx_addr, 
        NRF24l01_ADDR_WIDTH); 

    // RX_PW_PX - set the max number of bytes in RX payload in the chosen data pipe 
    nrf24l01_reg_byte_write(
        (NRF24L01_REG_RX_PW_P0 + (uint8_t)pipe_num), 
        NRF24L01_MAX_PAYLOAD_LEN); 

    //==================================================
    // CONFIG 
    // We read first because the config register data record has not been initialized. 
    nrf24l01_config_read(); 
    nrf24l01_data.config.pwr_up = SET_BIT;   // Set to start up the device 
    nrf24l01_data.config.prim_rx = SET_BIT;  // Set to RX mode 
    nrf24l01_config_write(); 
    //==================================================

    // Set CE high to enter back into an active mode 
    nrf24l01_ce_enable(); 

#endif 
}

//=======================================================================================


//=======================================================================================
// User functions 

// Data available status 
#if NEW_CODE 
DATA_PIPE nrf24l01_data_ready_status(void)
{
    nrf24l01_status_reg_read(); 
    return (DATA_PIPE)nrf24l01_data.status.rx_p_no; 
}
#elif WORKING_CODE 
uint8_t nrf24l01_data_ready_status(nrf24l01_data_pipe_t pipe_num)
{
    uint8_t data_status = CLEAR; 

    nrf24l01_status_reg_read(); 
    // This pipe check only works if you're using one pipe. It may block additional 
    // pipes. 
    data_status = (nrf24l01_data.status.rx_dr && 
                  (nrf24l01_data.status.rx_p_no == (uint8_t)pipe_num)); 

    return data_status; 

    // // Clear the RX_DR bit in the STATUS register 
    // if (data_status)
    // {
    //     nrf24l01_data.status.rx_dr = CLEAR_BIT; 
    //     nrf24l01_status_reg_write(); 
    // }

    // // Check if there is data in the RX FIFO and if the data belongs to the specified 
    // // pipe number. 
    // nrf24l01_status_reg_read(); 
    // return (nrf24l01_data.status.rx_dr && 
    //        (nrf24l01_data.status.rx_p_no & (uint8_t)pipe_num)); 

}
#endif 


// Receive payload 
void nrf24l01_receive_payload(
    uint8_t *read_buff, 
    nrf24l01_data_pipe_t pipe_num)
{
    if (read_buff == NULL)
    {
        return; 
    }
    
#if NEW_CODE 

    // Check FIFO status before attempting a read 
    // if (nrf24l01_data.status.rx_dr)
    if (nrf24l01_data.status.rx_p_no != NRF24L01_RX_FIFO_EMPTY)
    {
        // Get data 
        nrf24l01_receive(NRF24L01_CMD_R_RX_PL, read_buff, NRF24L01_MAX_PAYLOAD_LEN); 
        // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

        // We don't want to flush in case there is other pipe data still to be read. 
        // // Flush RX FIFO 
        // nrf24l01_fifo_flush(NRF24L01_CMD_FLUSH_RX); 
        // tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 

        // Write RX_DR = 1 to the status register to clear it. It is already equal to 1. 
        // nrf24l01_status_reg_write(); 
    }

#elif WORKING_CODE 

    uint8_t data_len = CLEAR; 
    uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

    // Check FIFO status before attempting a read 
    if (nrf24l01_data.status.rx_dr && 
       (nrf24l01_data.status.rx_p_no == (uint8_t)pipe_num))
    {
        data_len = NRF24L01_MAX_PAYLOAD_LEN; 

        // Get data 
        nrf24l01_receive(NRF24L01_CMD_R_RX_PL, read_buff, data_len); 

        tim_delay_ms(nrf24l01_data.timer, 1); 

        // Flush RX FIFO 
        nrf24l01_write(NRF24L01_CMD_FLUSH_RX, &buff, BYTE_0); 

        tim_delay_ms(nrf24l01_data.timer, 1); 

        // Write RX_DR = 1 to the status register to clear it 
        nrf24l01_status_reg_write(); 
    }

#endif 
}


// Send payload 
uint8_t nrf24l01_send_payload(const uint8_t *data_buff)
{
    if (data_buff == NULL)
    {
        return NRF24L01_INVALID_PTR; 
    }

#if NEW_CODE 

    uint8_t data_len = (uint8_t)sizeof(data_buff); 
    uint8_t tx_status = NRF24L01_OK; 
    uint8_t time_out = NRF24L01_TX_TIMEOUT; 

    if (data_len > NRF24L01_MAX_PAYLOAD_LEN)
    {
        data_len = NRF24L01_MAX_PAYLOAD_LEN; 
    }

    // // Set CE low to exit RX mode, set to TX mode and write the payload to the TX FIFO. 
    nrf24l01_ce_disable(); 
    nrf24l01_set_data_mode(NRF24L01_TX_MODE); 
    nrf24l01_write(NRF24L01_CMD_W_TX_PL, data_buff, data_len); 

    // Enable CE for more than 10us, then disable it so the device will return to 
    // Standy-1 mode when done transmitting. 
    nrf24l01_ce_enable(); 
    tim_delay_us(nrf24l01_data.timer, NRF24L01_CE_TX_DELAY); 
    nrf24l01_ce_disable(); 

    // Check to see if the TX FIFO is empty. If it's empty it means data was transmitted. 
    // There is a short blocking delay before checking the TX FIFO status to give the 
    // device time to send the data. If data has not been transferred before timeout then 
    // the transmission is considered to have failed. 
    tim_delay_us(nrf24l01_data.timer, NRF24L01_TX_DELAY); 
    do 
    {
        nrf24l01_fifo_status_reg_read(); 
        tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 
    }
    while(!(nrf24l01_data.fifo_status.tx_empty) && (--time_out)); 

    // Flush the TX FIFO, set to RX mode and enable CE to go back to an active state. 
    nrf24l01_fifo_flush(NRF24L01_CMD_FLUSH_TX); 
    tim_delay_ms(nrf24l01_data.timer, NRF24L01_RW_DELAY); 
    nrf24l01_set_data_mode(NRF24L01_RX_MODE); 
    nrf24l01_ce_enable(); 

    if (!time_out)
    {
        // No time left on the timer so data failed to send. 
        tx_status = NRF24L01_WRITE_FAULT; 
    }

    return tx_status; 
    
#elif WORKING_CODE 

    // uint8_t pack_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
    uint8_t data_len = NRF24L01_MAX_PAYLOAD_LEN; 
    uint8_t tx_status = NRF24L01_OK; 
    uint16_t time_out = 2; 
    uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

    // Write the payload to the TX FIFO 
    // nrf24l01_write(NRF24L01_CMD_W_TX_PL, pack_buff, data_len); 
    nrf24l01_write(NRF24L01_CMD_W_TX_PL, data_buff, data_len); 

    // Check to see if the TX FIFO is empty. If it's empty it means data was transmitted. 
    // There is a short blocking delay before checking the TX FIFO status to give the 
    // device time to send the data. If data has not been transferred before timeout then 
    // the transmission is considered to have failed. 
    tim_delay_us(nrf24l01_data.timer, 350); 
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

#elif ORIGINAL_CODE 
    
    // uint8_t pack_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
    // uint8_t index = NRF24L01_DATA_SIZE_LEN; 
    // uint8_t tx_status = CLEAR; 
    // uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

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

#endif 
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
    return nrf24l01_data.rf_ch.rf_ch; 
}


// Set RF_CH channel 
void nrf24l01_set_rf_ch(uint8_t rf_ch_freq)
{
    nrf24l01_data.rf_ch.rf_ch = rf_ch_freq & NRF24L01_RF_CH_MASK; 
}


// RF_CH register write 
NRF24L01_STATUS nrf24l01_rf_ch_write(void)
{
    NRF24L01_STATUS status; 

    nrf24l01_ce_disable(); 
    status = nrf24l01_reg_byte_write(NRF24L01_REG_RF_CH, nrf24l01_data.rf_ch.rf_ch_reg); 
    nrf24l01_ce_enable(); 

    return status; 
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
    return (nrf24l01_data_rate_t)((nrf24l01_data.rf_setup.rf_dr_low << SHIFT_1) | 
                                   nrf24l01_data.rf_setup.rf_dr_high); 
}


// Get RF_SETUP power output 
nrf24l01_rf_pwr_t nrf24l01_get_rf_setup_pwr(void)
{
    return (nrf24l01_rf_pwr_t)(nrf24l01_data.rf_setup.rf_pwr); 
}


// Set RF_SETUP data rate 
void nrf24l01_set_rf_setup_dr(nrf24l01_data_rate_t rate)
{
    nrf24l01_data.rf_setup.rf_dr_low = (rate >> SHIFT_1) & NRF24L01_RF_DR_MASK; 
    nrf24l01_data.rf_setup.rf_dr_high = rate & NRF24L01_RF_DR_MASK; 
}


// Set RF_SETUP power output 
void nrf24l01_set_rf_setup_pwr(nrf24l01_rf_pwr_t rf_pwr)
{
    nrf24l01_data.rf_setup.rf_pwr = (uint8_t)rf_pwr; 
}


// RF_SETUP register write 
NRF24L01_STATUS nrf24l01_rf_setup_write(void)
{
    NRF24L01_STATUS status; 

    nrf24l01_ce_disable(); 
    status = nrf24l01_reg_byte_write(NRF24L01_REG_RF_SET, nrf24l01_data.rf_setup.rf_set_reg); 
    nrf24l01_ce_enable(); 

    return status; 
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
    return (nrf24l01_pwr_mode_t)(nrf24l01_data.config.pwr_up); 
}


// Get active mode 
nrf24l01_mode_select_t nrf24l01_get_config_mode(void)
{
    return (nrf24l01_mode_select_t)(nrf24l01_data.config.prim_rx); 
}


// Power down 
NRF24L01_STATUS nrf24l01_pwr_down(void)
{
    NRF24L01_STATUS status; 

    nrf24l01_ce_disable(); 

    // Set PWR_UP low to go to power down state 
    nrf24l01_data.config.pwr_up = (uint8_t)NRF24L01_PWR_DOWN; 
    status = nrf24l01_config_write(); 

    if (status != NRF24L01_OK)
    {
        // Write operation failed. Set the CE pin back high 
        nrf24l01_ce_enable(); 
    }

    return status; 
}


// Power up 
NRF24L01_STATUS nrf24l01_pwr_up(void)
{
    NRF24L01_STATUS status; 

    // Set PWR_UP and PRIM_RX high to exit the power down state and default to RX mode 
    nrf24l01_ce_disable(); 
    nrf24l01_data.config.pwr_up = (uint8_t)NRF24L01_PWR_UP; 
    status = nrf24l01_set_data_mode(NRF24L01_RX_MODE); 
    // nrf24l01_data.config.prim_rx = (uint8_t)NRF24L01_RX_MODE; 
    // status = nrf24l01_config_write(); 

    if (status == NRF24L01_OK)
    {
        // Delay to allow for the startup state to pass (~1.5ms) 
        tim_delay_ms(nrf24l01_data.timer, NRF24L01_START_DELAY); 

        nrf24l01_ce_enable(); 
    }

    return status; 
}

//==================================================

//=======================================================================================


//=======================================================================================
// Configuration functions 

// Set CE pin high to enter an active mode 
void nrf24l01_ce_enable(void)
{
    gpio_write(nrf24l01_data.gpio_en, nrf24l01_data.en_pin, GPIO_HIGH); 
}


// Set CE pin low to exit an active mode 
void nrf24l01_ce_disable(void)
{
    gpio_write(nrf24l01_data.gpio_en, nrf24l01_data.en_pin, GPIO_LOW); 
}


// Set active mode 
NRF24L01_STATUS nrf24l01_set_data_mode(nrf24l01_mode_select_t mode)
{
    // Write PRIM_RX=mode to the device 
    nrf24l01_data.config.prim_rx = (uint8_t)mode; 
    return nrf24l01_config_write(); 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// CONFIG register write 
NRF24L01_STATUS nrf24l01_config_write(void)
{
    return nrf24l01_reg_byte_write(NRF24L01_REG_CONFIG, nrf24l01_data.config.config_reg); 
}


// STATUS register write 
NRF24L01_STATUS nrf24l01_status_reg_write(void)
{
    return nrf24l01_reg_byte_write(NRF24L01_REG_STATUS, nrf24l01_data.status.status_reg); 
}


// STATUS register read 
NRF24L01_STATUS nrf24l01_status_reg_read(void)
{
    // Write a no operation command to the device and the status register will be checked 
    uint8_t buff = CLEAR; 
    return nrf24l01_receive(NRF24L01_CMD_NOP, &buff, BYTE_0); 
}


// FIFO_STATUS register read 
NRF24L01_STATUS nrf24l01_fifo_status_reg_read(void)
{
    return nrf24l01_reg_read(NRF24L01_REG_FIFO, &nrf24l01_data.fifo_status.fifo_status_reg); 
}

//=======================================================================================


//=======================================================================================
// Read and write 

// Register read 
NRF24L01_STATUS nrf24l01_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_data)
{
    return nrf24l01_receive(NRF24L01_CMD_R_REG | reg_addr, reg_data, BYTE_1); 
}


// Read data from the device 
NRF24L01_STATUS nrf24l01_receive(
    uint8_t cmd, 
    uint8_t *rec_buff, 
    uint8_t data_len)
{
    SPI_STATUS spi_status = SPI_OK; 

    // Write the command and read the status back from the slave. Then read the data 
    // from the device. 
    spi_slave_select(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 
    spi_status |= spi_write_read(nrf24l01_data.spi, cmd, 
                                 &nrf24l01_data.status.status_reg, BYTE_1); 
    spi_status |= spi_write_read(nrf24l01_data.spi, SPI_DUMMY, rec_buff, data_len); 
    spi_slave_deselect(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 

    if (spi_status)
    {
        return NRF24L01_READ_FAULT; 
    }

    return NRF24L01_OK; 
}


// FIFO flush 
NRF24L01_STATUS nrf24l01_fifo_flush(uint8_t flush_cmd)
{
    uint8_t buff = CLEAR; 
    return nrf24l01_write(flush_cmd, &buff, BYTE_0); 
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
    return nrf24l01_write(NRF24L01_CMD_W_REG | reg_addr, reg_data, reg_size); 
}


// Write data to the device 
NRF24L01_STATUS nrf24l01_write(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len)
{
    SPI_STATUS spi_status = SPI_OK; 

    // Write the command and read the status back from the slave. Then write the data 
    // to the device. 
    spi_slave_select(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 
    spi_status |= spi_write_read(nrf24l01_data.spi, cmd, 
                                 &nrf24l01_data.status.status_reg, BYTE_1); 
    spi_status |= spi_write(nrf24l01_data.spi, send_buff, data_len); 
    spi_slave_deselect(nrf24l01_data.gpio_ss, nrf24l01_data.ss_pin); 

    if (spi_status)
    {
        return NRF24L01_WRITE_FAULT; 
    }

    return NRF24L01_OK; 
}

//=======================================================================================
