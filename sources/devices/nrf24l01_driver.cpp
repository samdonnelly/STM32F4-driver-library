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
// Function prototypes 

/**
 * @brief Set active mode (TX/RX) 
 * 
 * @details 
 */
void nrf24l01_set_data_mode(
    nrf24l01_mode_select_t mode); 


/**
 * @brief CONFIG register write 
 * 
 * @details 
 */
void nrf24l01_config_reg_write(void); 


/**
 * @brief CONFIG register read 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t nrf24l01_config_reg_read(void); 


/**
 * @brief RF_CH register write 
 * 
 * @details 
 */
void nrf24l01_rf_ch_reg_write(void); 


/**
 * @brief RF_CH register read 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t nrf24l01_rf_ch_reg_read(void); 


/**
 * @brief RF_SETUP register write 
 * 
 * @details 
 */
void nrf24l01_rf_setup_reg_write(void); 


/**
 * @brief RF_SETUP register read 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t nrf24l01_rf_setup_reg_read(void); 


/**
 * @brief STATUS register write 
 * 
 * @details 
 */
void nrf24l01_status_reg_write(void); 


/**
 * @brief STATUS register read 
 * 
 * @details 
 */
void nrf24l01_status_reg_read(void); 


/**
 * @brief STATUS register state update 
 * 
 * @details 
 * 
 * @param status 
 */
void nrf24l01_status_reg_update(
    uint8_t status); 


/**
 * @brief FIFO_STATUS register read 
 * 
 * @details 
 * 
 * @return uint8_t : 
 */
uint8_t nrf24l01_fifo_status_reg_read(void); 


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
 * @brief Register read 
 * 
 * @details 
 * 
 * @param cmd 
 * @return uint8_t : 
 */
uint8_t nrf24l01_reg_read(
    uint8_t cmd); 


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
void nrf24l01_write(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len); 


/**
 * @brief Register write 
 * 
 * @details 
 * 
 * @param cmd 
 * @param reg_write 
 */
void nrf24l01_reg_write(
    uint8_t cmd, 
    uint8_t reg_write); 


/**
 * @brief Read all device registers to verify values 
 * 
 * @details 
 */
void nrf24l01_read_all_reg(void); 

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
    TIM_TypeDef *timer; 

    // Status info 
    // 'status' --> bit 0: spi status 
    //          --> bits 1-7: not used 
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
// Initialization 

// nRF24L01 initialization 
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer)
{
    // Local variables 
    uint8_t reg_buff = CLEAR; 
    uint8_t p0_addr_buff[NRF24l01_ADDR_WIDTH]; 
    uint8_t p1_addr_buff[NRF24l01_ADDR_WIDTH]; 

    //===================================================
    // Initialize data 

    // Default addresses 
    memset((void *)p0_addr_buff, NRF24L01_REG_RESET_RX_ADDR_P0, sizeof(p0_addr_buff)); 
    memset((void *)p1_addr_buff, NRF24L01_REG_RESET_RX_ADDR_P1, sizeof(p1_addr_buff)); 

    // Peripherals 
    nrf24l01_driver_data.spi = spi; 
    nrf24l01_driver_data.gpio_ss = gpio_ss; 
    nrf24l01_driver_data.gpio_en = gpio_en; 
    nrf24l01_driver_data.ss_pin = (gpio_pin_num_t)(SET_BIT << ss_pin); 
    nrf24l01_driver_data.en_pin = (gpio_pin_num_t)(SET_BIT << en_pin); 
    nrf24l01_driver_data.timer = timer; 

    // Driver status 
    nrf24l01_driver_data.driver_status = CLEAR; 

    // CONFIG register 
    nrf24l01_driver_data.config.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.config.mask_rx_dr = CLEAR_BIT; 
    nrf24l01_driver_data.config.mask_tx_ds = CLEAR_BIT; 
    nrf24l01_driver_data.config.mask_max_rt = CLEAR_BIT; 
    nrf24l01_driver_data.config.en_crc = CLEAR_BIT; 
    nrf24l01_driver_data.config.crco = CLEAR_BIT; 
    nrf24l01_driver_data.config.pwr_up = SET_BIT;   // Set to start up the device 
    nrf24l01_driver_data.config.prim_rx = SET_BIT;  // Default to RX mode 

    // RF_CH register 
    nrf24l01_driver_data.rf_ch.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.rf_ch.rf_ch = NRF24L01_REG_RESET_RF_CH; 

    // RF_SETUP register 
    nrf24l01_driver_data.rf_setup.cont_wave = CLEAR_BIT; 
    nrf24l01_driver_data.rf_setup.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.rf_setup.rf_dr_low = 
        ((uint8_t)NRF24L01_DR_2MBPS >> SHIFT_1) & NRF24L01_RF_DR_MASK;   // Set to default 
    nrf24l01_driver_data.rf_setup.pll_lock = CLEAR_BIT; 
    nrf24l01_driver_data.rf_setup.rf_dr_high = 
        (uint8_t)NRF24L01_DR_2MBPS & NRF24L01_RF_DR_MASK;                // Set to default 
    nrf24l01_driver_data.rf_setup.rf_pwr = SET_3;                        // Set to default 
    nrf24l01_driver_data.rf_setup.unused_2 = CLEAR_BIT; 

    // STATUS register 
    nrf24l01_driver_data.status_reg.unused_1 = CLEAR_BIT; 
    nrf24l01_driver_data.status_reg.rx_dr = SET_BIT;    // Write 1 to clear 
    nrf24l01_driver_data.status_reg.tx_ds = SET_BIT;    // Write 1 to clear 
    nrf24l01_driver_data.status_reg.max_rt = SET_BIT;   // Write 1 to clear 
    nrf24l01_driver_data.status_reg.rx_p_no = CLEAR; 
    nrf24l01_driver_data.status_reg.tx_full = CLEAR_BIT; 
    
    //===================================================

    //===================================================
    // Configure GPIO pins 

    // SPI slave select pin (CSN) 
    spi_ss_init(nrf24l01_driver_data.gpio_ss, ss_pin); 

    // Device enable pin (CE) 
    gpio_pin_init(
        nrf24l01_driver_data.gpio_en, 
        en_pin, 
        MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    
    //===================================================

    //==================================================
    // Prep the device for initialization 
    
    // Set CE low to prevent entering an active (TX/RX) state 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Delay to ensure power on reset state is cleared before accessing the device 
    tim_delay_ms(nrf24l01_driver_data.timer, NRF24L01_PWR_ON_DELAY); 

    // Flush the FIFOs to ensure no leftover data 
    nrf24l01_write(NRF24L01_CMD_FLUSH_TX, &reg_buff, BYTE_0); 
    nrf24l01_write(NRF24L01_CMD_FLUSH_RX, &reg_buff, BYTE_0); 

    //==================================================

    //==================================================
    // Set register values to their reset/default value 

    nrf24l01_reg_write(NRF24L01_REG_CONFIG, NRF24L01_REG_RESET_CONFIG); 
    nrf24l01_reg_write(NRF24L01_REG_EN_AA, NRF24L01_REG_RESET_EN_AA); 
    nrf24l01_reg_write(NRF24L01_REG_EN_RXADDR, NRF24L01_REG_RESET_EN_RXADDR); 
    nrf24l01_reg_write(NRF24L01_REG_SETUP_AW, NRF24L01_REG_RESET_SETUP_AW); 
    nrf24l01_reg_write(NRF24L01_REG_SETUP_RETR, NRF24L01_REG_RESET_SETUP_RETR); 
    nrf24l01_reg_write(NRF24L01_REG_RF_CH, NRF24L01_REG_RESET_RF_CH); 
    nrf24l01_reg_write(NRF24L01_REG_RF_SET, NRF24L01_REG_RESET_RF_SET); 
    nrf24l01_reg_write(NRF24L01_REG_STATUS, NRF24L01_REG_RESET_STATUS); 
    nrf24l01_reg_write(NRF24L01_REG_OBSERVE_TX, NRF24L01_REG_RESET_OBSERVE_TX); 
    nrf24l01_reg_write(NRF24L01_REG_RPD, NRF24L01_REG_RESET_RPD); 
    nrf24l01_write(NRF24L01_CMD_W_REG | NRF24L01_REG_RX_ADDR_P0, p0_addr_buff, NRF24l01_ADDR_WIDTH); 
    nrf24l01_write(NRF24L01_CMD_W_REG | NRF24L01_REG_RX_ADDR_P1, p1_addr_buff, NRF24l01_ADDR_WIDTH); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P2, NRF24L01_REG_RESET_RX_ADDR_P2); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P3, NRF24L01_REG_RESET_RX_ADDR_P3); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P4, NRF24L01_REG_RESET_RX_ADDR_P4); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P5, NRF24L01_REG_RESET_RX_ADDR_P5); 
    nrf24l01_write(NRF24L01_CMD_W_REG | NRF24L01_REG_TX_ADDR, p0_addr_buff, NRF24l01_ADDR_WIDTH); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P0, NRF24L01_REG_RESET_RX_PW_PX); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P1, NRF24L01_REG_RESET_RX_PW_PX); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P2, NRF24L01_REG_RESET_RX_PW_PX); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P3, NRF24L01_REG_RESET_RX_PW_PX); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P4, NRF24L01_REG_RESET_RX_PW_PX); 
    nrf24l01_reg_write(NRF24L01_REG_RX_PW_P5, NRF24L01_REG_RESET_RX_PW_PX); 
    nrf24l01_reg_write(NRF24L01_REG_DYNPD, NRF24L01_REG_RESET_DYNPD); 
    nrf24l01_reg_write(NRF24L01_REG_FEATURE, NRF24L01_REG_RESET_FEATURE); 

    //==================================================

    //==================================================
    // Configure registers to work for this driver 

    // These settings are common between both PRX and PTX devices. Registers not changed here remain 
    // at their default value or are changed selectively by setters. 

    // CONFIG - power up the device and default to a PRX device - delay for start up state 
    nrf24l01_config_reg_write(); 
    tim_delay_ms(nrf24l01_driver_data.timer, NRF24L01_START_DELAY); 
    
    // EN_AA - disable auto acknowledgment (Enhanced Shockburst TM not used) - set 0 
    nrf24l01_reg_write(NRF24L01_REG_EN_AA, NRF24L01_DISABLE_REG); 
    
    // EN_RXADDR - disable data pipes for now - configured separately - set 0 
    nrf24l01_reg_write(NRF24L01_REG_EN_RXADDR, NRF24L01_DISABLE_REG); 
    
    // SETUP_RETR - disable retransmission because auto acknowledgement not used - set 0 
    nrf24l01_reg_write(NRF24L01_REG_SETUP_RETR, NRF24L01_DISABLE_REG); 

    // FIFO_STATUS - read the FIFO status register to update the driver data record 
    nrf24l01_fifo_status_reg_read(); 

    //==================================================

    // Set CE high to enter RX mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}

//=======================================================================================


//=======================================================================================
// User configuration functions 

// Configure a devices PTX settings 
void nrf24l01_ptx_config(
    const uint8_t *tx_addr)
{
    // Set CE low to exit any active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Set RF_CH - handled as a separate setter and in the init 
    
    // Set up TX_ADDR - don't need to match RX_ADDR_P0 as not using auto acknowledge 
    nrf24l01_write(NRF24L01_CMD_W_REG | NRF24L01_REG_TX_ADDR, tx_addr, NRF24l01_ADDR_WIDTH); 

    // Set CE high to enter back into an active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// Configure a devices PRX settings 
void nrf24l01_prx_config(
    const uint8_t *rx_addr, 
    nrf24l01_data_pipe_t pipe_num)
{
    // Set CE low to exit any active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Set RF_CH - handled as a separate setter and in the init 

    // EN_RXADDR - set the data pipe to enable 
    nrf24l01_reg_write(
        NRF24L01_REG_EN_RXADDR, 
        nrf24l01_reg_read(NRF24L01_REG_EN_RXADDR) | (SET_BIT << (uint8_t)pipe_num)); 

    // RX_ADDR_PX - set the chosen data pipe address 
    // TODO this will only work for pipes 0 and 1 --> 2-5 are one byte 
    //      Could have another function that gets called from here but only for 2-5 and if 2-5 is 
    //      requested then the first 4 bytes of 1 are also updated. 
    nrf24l01_write(
        NRF24L01_CMD_W_REG | (NRF24L01_REG_RX_ADDR_P0 + (uint8_t)pipe_num), 
        rx_addr, 
        NRF24l01_ADDR_WIDTH); 

    // RX_PW_PX - set the max number of bytes in RX payload in the chosen data pipe 
    nrf24l01_reg_write(
        (NRF24L01_REG_RX_PW_P0 + (uint8_t)pipe_num), 
        NRF24L01_MAX_PAYLOAD_LEN); 

    // Set CE high to enter back into an active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// Set RF channel 
void nrf24l01_set_rf_channel(
    uint8_t rf_ch_freq)
{
    // Set CE low to exit any active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Update and write the channel 
    nrf24l01_driver_data.rf_ch.rf_ch = rf_ch_freq & NRF24L01_RF_CH_MASK; 
    nrf24l01_rf_ch_reg_write(); 

    // Set CE high to enter back into an active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// Set data rate 
void nrf24l01_set_rf_dr(
    nrf24l01_data_rate_t rate)
{
    // Set CE low to exit any active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Update and write the data rate 
    nrf24l01_driver_data.rf_setup.rf_dr_low = (rate >> SHIFT_1) & NRF24L01_RF_DR_MASK; 
    nrf24l01_driver_data.rf_setup.rf_dr_high = rate & NRF24L01_RF_DR_MASK; 
    nrf24l01_rf_setup_reg_write(); 

    // Set CE high to enter back into an active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// Set power output 
void nrf24l01_set_rf_pwr(
    nrf24l01_rf_pwr_t rf_pwr)
{
    // Set CE low to exit any active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 
    
    // Update and write the power output 
    nrf24l01_driver_data.rf_setup.rf_pwr = (uint8_t)rf_pwr; 
    nrf24l01_rf_setup_reg_write(); 

    // Set CE high to enter back into an active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// Power down 
void nrf24l01_pwr_down(void)
{
    // Set CE low to exit any active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Set PWR_UP low to go to power down state 
    nrf24l01_driver_data.config.pwr_up = (uint8_t)NRF24L01_PWR_DOWN; 
    nrf24l01_config_reg_write(); 
}


// Power up 
void nrf24l01_pwr_up(void)
{   
    // Set PWR_UP high to exit the power down state 
    nrf24l01_driver_data.config.pwr_up = (uint8_t)NRF24L01_PWR_UP; 
    nrf24l01_config_reg_write(); 

    // Delay to allow for the startup state to pass (~1.5ms) 
    tim_delay_ms(nrf24l01_driver_data.timer, NRF24L01_START_DELAY); 

    // Set CE high to enter back into an active mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}

//=======================================================================================


//=======================================================================================
// User getter functions 

// Data ready status 
uint8_t nrf24l01_data_ready_status(
    nrf24l01_data_pipe_t pipe_num)
{
    // Check if there is data in the RX FIFO and if the data belongs to the specified pipe 
    // number. 
    nrf24l01_status_reg_read(); 
    return (nrf24l01_driver_data.status_reg.rx_dr && 
           (nrf24l01_driver_data.status_reg.rx_p_no & (uint8_t)pipe_num)); 
}


// Get power mode 
nrf24l01_pwr_mode_t nrf24l01_get_pwr_mode(void)
{
    nrf24l01_config_reg_read(); 
    return (nrf24l01_pwr_mode_t)(nrf24l01_driver_data.config.pwr_up); 
}


// Get active mode 
nrf24l01_mode_select_t nrf24l01_get_mode(void)
{
    nrf24l01_config_reg_read(); 
    return (nrf24l01_mode_select_t)(nrf24l01_driver_data.config.prim_rx); 
}


// Get RF channel 
uint8_t nrf24l01_get_rf_ch(void)
{
    nrf24l01_rf_ch_reg_read(); 
    return nrf24l01_driver_data.rf_ch.rf_ch; 
}


// Get RF data rate 
nrf24l01_data_rate_t nrf24l01_get_rf_dr(void)
{
    nrf24l01_rf_setup_reg_read(); 
    return (nrf24l01_data_rate_t)((nrf24l01_driver_data.rf_setup.rf_dr_low << SHIFT_1) | 
                                   nrf24l01_driver_data.rf_setup.rf_dr_high); 
}


// Get power output 
nrf24l01_rf_pwr_t nrf24l01_get_rf_pwr(void)
{
    nrf24l01_rf_setup_reg_read(); 
    return (nrf24l01_rf_pwr_t)(nrf24l01_driver_data.rf_setup.rf_pwr); 
}

//=======================================================================================


//=======================================================================================
// User data functions 

// Receive payload 
void nrf24l01_receive_payload(
    uint8_t *read_buff, 
    nrf24l01_data_pipe_t pipe_num)
{
    // Local variables 
    uint8_t data_len = CLEAR; 
    uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

    // Check FIFO status before attempting a read 
    if (nrf24l01_driver_data.status_reg.rx_dr && 
       (nrf24l01_driver_data.status_reg.rx_p_no & (uint8_t)pipe_num))
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
uint8_t nrf24l01_send_payload(
    const uint8_t *data_buff)
{
    // Local variables 
    uint8_t pack_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
    uint8_t data_len = CLEAR; 
    uint8_t index = NRF24L01_DATA_SIZE_LEN; 
    uint8_t tx_status = CLEAR; 
    uint16_t time_out = NRF24L01_TX_TIMEOUT; 
    uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

    // Fill the packet buffer with the data to be sent. The packet will be capped at a max of 
    // 30 data bytes with one byte always being saves at the beginning and end of the packet for 
    // the data length and a NULL termination, respectfully. The following loop counts the data 
    // length and saves the data/payload into the packet buffer. If the data length is less than 
    // 30 bytes then the loop ends early. 
    while (index <= NRF24L01_MAX_DATA_LEN)
    {
        data_len++; 

        if (*data_buff == NULL_CHAR)
        {
            break; 
        }

        pack_buff[index++] = *data_buff++; 
    }

    // Write the data size to the first position of the packet buffer and terminate the payload 
    pack_buff[0] = data_len; 
    pack_buff[index] = NULL_CHAR; 
    
    // Set CE low to exit RX mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Set as a PTX device 
    nrf24l01_set_data_mode(NRF24L01_TX_MODE); 

    // Write the payload to the TX FIFO 
    nrf24l01_write(NRF24L01_CMD_W_TX_PL, pack_buff, data_len); 

    // Set CE high to enter TX mode and start the transmission. Delay to ensure CE is high long 
    // enough then set CE low so the device goes back to Standby-1 when done sending. 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
    tim_delay_us(nrf24l01_driver_data.timer, NRF24L01_CE_TX_DELAY); 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Check to see if the TX FIFO is empty - means data was transmitted. 
    // If data has not been transferred before timeout then it's considered to have failed. 
    do 
    {
        nrf24l01_fifo_status_reg_read(); 
    }
    while(!(nrf24l01_driver_data.fifo_status.tx_empty) && (--time_out)); 

    if (time_out)
    {
        // Time left on the timer so data has successfully been set. 
        tx_status = SET_BIT; 
    }

    // Flush the TX FIFO 
    nrf24l01_write(NRF24L01_CMD_FLUSH_TX, &buff, BYTE_0); 

    // Set to a PRX device 
    nrf24l01_set_data_mode(NRF24L01_RX_MODE); 

    // Set CE back high to enter RX mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 

    return tx_status; 
}

//=======================================================================================


//=======================================================================================
// Configuration functions 

// Set mode 
void nrf24l01_set_data_mode(
    nrf24l01_mode_select_t mode)
{
    // Write PRIM_RX=mode to the device 
    nrf24l01_driver_data.config.prim_rx = (uint8_t)mode; 
    nrf24l01_config_reg_write(); 

    // Delay to clear the device settling state 
    tim_delay_us(nrf24l01_driver_data.timer, NRF24L01_SETTLE_DELAY); 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// CONFIG register write 
void nrf24l01_config_reg_write(void)
{
    // Format the data to send 
    uint8_t config = (nrf24l01_driver_data.config.unused_1    << SHIFT_7) | 
                     (nrf24l01_driver_data.config.mask_rx_dr  << SHIFT_6) | 
                     (nrf24l01_driver_data.config.mask_tx_ds  << SHIFT_5) | 
                     (nrf24l01_driver_data.config.mask_max_rt << SHIFT_4) | 
                     (nrf24l01_driver_data.config.en_crc      << SHIFT_3) | 
                     (nrf24l01_driver_data.config.crco        << SHIFT_2) | 
                     (nrf24l01_driver_data.config.pwr_up      << SHIFT_1) | 
                     (nrf24l01_driver_data.config.prim_rx); 

    // Send the data to the CONFIG register 
    nrf24l01_reg_write(NRF24L01_REG_CONFIG, config); 
}


// CONFIG register read 
uint8_t nrf24l01_config_reg_read(void)
{
    // Read the CONFIG register and update the data record 
    uint8_t config = nrf24l01_reg_read(NRF24L01_REG_CONFIG); 

    nrf24l01_driver_data.config.unused_1    = (config & FILTER_BIT_7) >> SHIFT_7; 
    nrf24l01_driver_data.config.mask_rx_dr  = (config & FILTER_BIT_6) >> SHIFT_6; 
    nrf24l01_driver_data.config.mask_tx_ds  = (config & FILTER_BIT_5) >> SHIFT_5; 
    nrf24l01_driver_data.config.mask_max_rt = (config & FILTER_BIT_4) >> SHIFT_4; 
    nrf24l01_driver_data.config.en_crc      = (config & FILTER_BIT_3) >> SHIFT_3; 
    nrf24l01_driver_data.config.crco        = (config & FILTER_BIT_2) >> SHIFT_2; 
    nrf24l01_driver_data.config.pwr_up      = (config & FILTER_BIT_1) >> SHIFT_1; 
    nrf24l01_driver_data.config.prim_rx     = (config & FILTER_BIT_0); 

    return config; 
}


// RF_CH register write 
void nrf24l01_rf_ch_reg_write(void)
{
    // Format the data to send 
    uint8_t rf_ch = (nrf24l01_driver_data.rf_ch.unused_1 << SHIFT_7) | 
                    (nrf24l01_driver_data.rf_ch.rf_ch); 

    // Send the data to the RF_CH register 
    nrf24l01_reg_write(NRF24L01_REG_RF_CH, rf_ch); 
}


// RF_CH register read 
uint8_t nrf24l01_rf_ch_reg_read(void)
{
    // Read the RF_CH register and update the data record 
    uint8_t rf_ch = nrf24l01_reg_read(NRF24L01_REG_RF_CH); 

    nrf24l01_driver_data.rf_ch.unused_1 = (rf_ch & FILTER_BIT_7) >> SHIFT_7; 
    nrf24l01_driver_data.rf_ch.rf_ch    = (rf_ch & FILTER_7_LSB); 

    return rf_ch; 
}


// RF_SETUP register write 
void nrf24l01_rf_setup_reg_write(void)
{
    // Format the data to send 
    uint8_t rf_setup = (nrf24l01_driver_data.rf_setup.cont_wave  << SHIFT_7) | 
                       (nrf24l01_driver_data.rf_setup.unused_1   << SHIFT_6) | 
                       (nrf24l01_driver_data.rf_setup.rf_dr_low  << SHIFT_5) | 
                       (nrf24l01_driver_data.rf_setup.pll_lock   << SHIFT_4) | 
                       (nrf24l01_driver_data.rf_setup.rf_dr_high << SHIFT_3) | 
                       (nrf24l01_driver_data.rf_setup.rf_pwr     << SHIFT_1) | 
                       (nrf24l01_driver_data.rf_setup.unused_2); 

    // Send the data to the RF_SET register 
    nrf24l01_reg_write(NRF24L01_REG_RF_SET, rf_setup); 
}


// RF_SETUP register read 
uint8_t nrf24l01_rf_setup_reg_read(void)
{
    // Read the RF_CH register and update the data record 
    uint8_t rf_setup = nrf24l01_reg_read(NRF24L01_REG_RF_SET); 

    nrf24l01_driver_data.rf_setup.cont_wave  = (rf_setup & FILTER_BIT_7) >> SHIFT_7; 
    nrf24l01_driver_data.rf_setup.unused_1   = (rf_setup & FILTER_BIT_6) >> SHIFT_6; 
    nrf24l01_driver_data.rf_setup.rf_dr_low  = (rf_setup & FILTER_BIT_5) >> SHIFT_5; 
    nrf24l01_driver_data.rf_setup.pll_lock   = (rf_setup & FILTER_BIT_4) >> SHIFT_4; 
    nrf24l01_driver_data.rf_setup.rf_dr_high = (rf_setup & FILTER_BIT_3) >> SHIFT_3; 
    nrf24l01_driver_data.rf_setup.rf_pwr     = (rf_setup & FILTER_3_LSB) >> SHIFT_1; 
    nrf24l01_driver_data.rf_setup.unused_2   = (rf_setup & FILTER_BIT_0); 

    return rf_setup; 
}


// STATUS register write 
void nrf24l01_status_reg_write(void)
{
    // Format the data to send 
    uint8_t status_reg = (nrf24l01_driver_data.status_reg.unused_1 << SHIFT_7) | 
                         (nrf24l01_driver_data.status_reg.rx_dr    << SHIFT_6) | 
                         (nrf24l01_driver_data.status_reg.tx_ds    << SHIFT_5) | 
                         (nrf24l01_driver_data.status_reg.max_rt   << SHIFT_4) | 
                         (nrf24l01_driver_data.status_reg.rx_p_no  << SHIFT_1) | 
                         (nrf24l01_driver_data.status_reg.tx_full); 

    // Send the data to the STATUS register 
    nrf24l01_reg_write(NRF24L01_REG_STATUS, status_reg); 
}


// STATUS register read 
void nrf24l01_status_reg_read(void)
{
    // Write a no operation command to the device and the status register will be checked 
    uint8_t buff = CLEAR; 
    nrf24l01_receive(NRF24L01_CMD_NOP, &buff, BYTE_0); 
}


// STATUS register state update 
void nrf24l01_status_reg_update(
    uint8_t status)
{
    // Sort the status register byte into the data record 
    nrf24l01_driver_data.status_reg.unused_1 = (status & FILTER_BIT_7) >> SHIFT_7; 
    nrf24l01_driver_data.status_reg.rx_dr    = (status & FILTER_BIT_6) >> SHIFT_6; 
    nrf24l01_driver_data.status_reg.tx_ds    = (status & FILTER_BIT_5) >> SHIFT_5; 
    nrf24l01_driver_data.status_reg.max_rt   = (status & FILTER_BIT_4) >> SHIFT_4; 
    nrf24l01_driver_data.status_reg.rx_p_no  = (status & FILTER_4_LSB) >> SHIFT_1; 
    nrf24l01_driver_data.status_reg.tx_full  = (status & FILTER_BIT_0); 
}


// FIFO_STATUS register read 
uint8_t nrf24l01_fifo_status_reg_read(void)
{
    // Read the FIFO_STATUS register and update the data record 
    uint8_t fifo_status = nrf24l01_reg_read(NRF24L01_REG_FIFO); 

    // Store the FIFO status register byte into the data record 
    nrf24l01_driver_data.fifo_status.unused_1 = (fifo_status & FILTER_BIT_7) >> SHIFT_7; 
    nrf24l01_driver_data.fifo_status.tx_reuse = (fifo_status & FILTER_BIT_6) >> SHIFT_6; 
    nrf24l01_driver_data.fifo_status.tx_full  = (fifo_status & FILTER_BIT_5) >> SHIFT_5; 
    nrf24l01_driver_data.fifo_status.tx_empty = (fifo_status & FILTER_BIT_4) >> SHIFT_4; 
    nrf24l01_driver_data.fifo_status.unused_2 = (fifo_status & FILTER_4_LSB) >> SHIFT_2; 
    nrf24l01_driver_data.fifo_status.rx_full  = (fifo_status & FILTER_BIT_1) >> SHIFT_1; 
    nrf24l01_driver_data.fifo_status.rx_empty = (fifo_status & FILTER_BIT_0); 

    return fifo_status; 
}

//=======================================================================================


//=======================================================================================
// Read and write 

// Receive data from another transceiver 
void nrf24l01_receive(
    uint8_t cmd, 
    uint8_t *rec_buff, 
    uint8_t data_len)
{
    // Local variables 
    uint8_t spi_status = SPI_OK; 
    uint8_t status_reg = CLEAR; 

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
    nrf24l01_status_reg_update(status_reg); 

    // Update the driver status 
    nrf24l01_driver_data.driver_status |= spi_status; 
}


// Register read 
uint8_t nrf24l01_reg_read(
    uint8_t cmd)
{
    // Read and return the register value 
    uint8_t reg_read = CLEAR; 
    nrf24l01_receive(NRF24L01_CMD_R_REG | cmd, &reg_read, BYTE_1); 
    return reg_read; 
}


// Send data to another transceiver 
void nrf24l01_write(
    uint8_t cmd, 
    const uint8_t *send_buff, 
    uint8_t data_len)
{
    // Local variables 
    uint8_t spi_status = SPI_OK; 
    uint8_t status_reg = CLEAR; 

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
    nrf24l01_status_reg_update(status_reg); 

    // Update the driver status 
    nrf24l01_driver_data.driver_status |= spi_status; 
}


// Register write 
void nrf24l01_reg_write(
    uint8_t cmd, 
    uint8_t reg_write)
{
    nrf24l01_write(NRF24L01_CMD_W_REG | cmd, &reg_write, BYTE_1); 
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Read all device registers to verify values 
void nrf24l01_read_all_reg(void)
{
    // Local variables 
    uint8_t reg_buff = CLEAR; 
    uint8_t addr_buff[NRF24l01_ADDR_WIDTH]; 

    // CONFIG register read 
    reg_buff = nrf24l01_config_reg_read(); 
    
    // EN_AA register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_EN_AA, &reg_buff, BYTE_1); 
    
    // EN_RXADDR register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_EN_RXADDR, &reg_buff, BYTE_1); 
    
    // SETUP_AW register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_SETUP_AW, &reg_buff, BYTE_1); 
    
    // SETUP_RETR register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_SETUP_RETR, &reg_buff, BYTE_1); 
    
    // RF_CH register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RF_CH, &reg_buff, BYTE_1); 
    
    // RF_SETUP register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RF_SET, &reg_buff, BYTE_1); 
    
    // STATUS register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_STATUS, &reg_buff, BYTE_1); 
    
    // OBSERVE_TX register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_OBSERVE_TX, &reg_buff, BYTE_1); 
    
    // RPD register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RPD, &reg_buff, BYTE_1); 
    
    // RX_ADDR_P0 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_ADDR_P0, addr_buff, BYTE_5); 
    
    // RX_ADDR_P1 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_ADDR_P1, addr_buff, BYTE_5); 
    
    // RX_ADDR_P2 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_ADDR_P2, &reg_buff, BYTE_1); 
    
    // RX_ADDR_P3 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_ADDR_P3, &reg_buff, BYTE_1); 
    
    // RX_ADDR_P4 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_ADDR_P4, &reg_buff, BYTE_1); 
    
    // RX_ADDR_P5 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_ADDR_P5, &reg_buff, BYTE_1); 
    
    // TX_ADDR register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_TX_ADDR, addr_buff, BYTE_5); 
    
    // RX_PW_P0 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_PW_P0, &reg_buff, BYTE_1); 
    
    // RX_PW_P1 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_PW_P1, &reg_buff, BYTE_1); 
    
    // RX_PW_P2 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_PW_P2, &reg_buff, BYTE_1); 
    
    // RX_PW_P3 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_PW_P3, &reg_buff, BYTE_1); 
    
    // RX_PW_P4 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_PW_P4, &reg_buff, BYTE_1); 
    
    // RX_PW_P5 register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_RX_PW_P5, &reg_buff, BYTE_1); 
    
    // FIFO_STATUS register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_FIFO, &reg_buff, BYTE_1); 
    
    // DYNPD register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_DYNPD, &reg_buff, BYTE_1); 
    
    // FEATURE register read 
    nrf24l01_receive(NRF24L01_CMD_R_REG | NRF24L01_REG_FEATURE, &reg_buff, BYTE_1); 
}

//=======================================================================================
