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
 * @brief Set mode 
 * 
 * @details 
 */
void nrf24l01_set_data_mode(
    nrf24l01_mode_select_t mode); 


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
    uint8_t *reg_write); 


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


/**
 * @brief RF_SETUP register write 
 * 
 * @details 
 */
void nrf24l01_rf_set_reg_write(void); 


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
 */
void nrf24l01_fifo_status_reg_read(void); 

//=======================================================================================


//=======================================================================================
// Initialization 

// Allow for power on reset state of the device to be run (100ms) 

// nRF24L01 initialization 
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer, 
    nrf24l01_data_rate_t rate, 
    uint8_t rf_ch_freq)
{
    //===================================================
    // Initialize data record 

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
    // nrf24l01_driver_data.status_reg.tx_ds = CLEAR_BIT; 
    nrf24l01_driver_data.status_reg.tx_ds = SET_BIT; 
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
    // Configure GPIO pins 

    // SPI slave select pin 
    spi_ss_init(nrf24l01_driver_data.gpio_ss, ss_pin); 

    // Device enable pin 
    gpio_pin_init(
        nrf24l01_driver_data.gpio_en, 
        en_pin, 
        MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 
    
    //===================================================

    //===================================================
    // Configure the device by writing the settings to the device registers 

    // Delay to ensure power on reset state is cleared 
    tim_delay_ms(nrf24l01_driver_data.timer, NRF24L01_PWR_ON_DELAY); 

    //=======================================
    // Test 

    uint8_t buff = CLEAR; 
    // Flush the FIFOs to ensure no leftover data 
    nrf24l01_send(
        NRF24L01_CMD_FLUSH_TX, 
        &buff, 
        BYTE_0); 
    nrf24l01_send(
        NRF24L01_CMD_FLUSH_RX, 
        &buff, 
        BYTE_0); 

    nrf24l01_status_reg_write(); 

    //======================================

    // Configure the RF registers 
    nrf24l01_rf_ch_reg_write(); 
    nrf24l01_rf_set_reg_write(); 

    // Set the CONFIG register and delay for the start up transition state 
    nrf24l01_config_reg_write(); 
    tim_delay_ms(nrf24l01_driver_data.timer, NRF24L01_START_DELAY); 

    //===================================================

    //==================================================
    // Test 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
    //==================================================
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


// Send data to another transceiver 
void nrf24l01_send(
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

//=======================================================================================


//=======================================================================================
// User functions 

// Data ready status 
uint8_t nrf24l01_data_ready_status(void)
{
    nrf24l01_status_reg_read(); 
    return nrf24l01_driver_data.status_reg.rx_dr; 
}


// Receive payload 
void nrf24l01_receive_payload(
    uint8_t *read_buff)
{
    // Local variables 
    uint8_t data_len = CLEAR; 
    uint8_t buff = CLEAR;   // dummy variable to pass to the send function 

    // Check FIFO status before attempting a read 
    if (nrf24l01_driver_data.status_reg.rx_dr)
    {
        // Read the first byte from the RX FIFO --> This is the data length 
        nrf24l01_receive(
            NRF24L01_CMD_R_RX_PL, 
            &data_len, 
            BYTE_1); 

        // Use the data length number to read the remaining RX FIFO data 
        nrf24l01_receive(
            NRF24L01_CMD_R_RX_PL, 
            read_buff, 
            data_len); 

        // Flush the RX FIFO to ensure old data is not read later 
        nrf24l01_send(
            NRF24L01_CMD_FLUSH_RX, 
            &buff, 
            BYTE_0); 

        // Clear the RX_DR bit in the STATUS register 
        // nrf24l01_driver_data.status_reg.rx_dr = SET_BIT; 
        nrf24l01_status_reg_write(); 
        // nrf24l01_driver_data.status_reg.rx_dr = CLEAR_BIT; 
        nrf24l01_status_reg_read(); 
    }
}


// Send payload 
void nrf24l01_send_payload(
    const uint8_t *data_buff)
{
    // Local variables 
    uint8_t pack_buff[NRF24L01_MAX_PACK_LEN]; 
    uint8_t data_len = CLEAR; 
    uint8_t index = NRF24L01_DATA_SIZE_LEN; 
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

    // Write the data size to the packet buffer and terminate the payload 
    pack_buff[0] = data_len; 
    pack_buff[index] = NULL_CHAR; 

    //==================================================
    // Test 

    // Set CE low to exit RX mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    //==================================================

    // Enter TX mode 
    nrf24l01_set_data_mode(NRF24L01_TX_MODE); 

    // Write data to the device 
    nrf24l01_send(
        NRF24L01_CMD_W_TX_PL, 
        pack_buff, 
        data_len); 

    //==================================================
    // Test 

    // Set CE high to start the transmission 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 

    // Delay to ensure CE is high long enough 
    tim_delay_us(nrf24l01_driver_data.timer, 20); 

    // Set CE low 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Wait until transmission is complete 
    do 
    {
        nrf24l01_status_reg_read(); 
    }
    while(!(nrf24l01_driver_data.status_reg.tx_ds | nrf24l01_driver_data.status_reg.max_rt)); 

    // Write TX_DS to 1 to clear the bit 
    // nrf24l01_driver_data.status_reg.tx_ds = SET_BIT; 
    nrf24l01_status_reg_write(); 
    // nrf24l01_driver_data.status_reg.tx_ds = CLEAR_BIT; 
    nrf24l01_status_reg_read(); 

    //==================================================

    // Exit TX mode / Enter RX mode 
    nrf24l01_set_data_mode(NRF24L01_RX_MODE); 

    //==================================================
    // Test 

    // Set CE back high to enter RX mode 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 

    //==================================================

    // // Flush the TX FIFO to ensure no data sits in the FIFO 
    // nrf24l01_send(
    //     NRF24L01_CMD_FLUSH_TX, 
    //     &buff, 
    //     BYTE_0); 
}


// Set frequency channel 
void nrf24l01_set_channel(
    uint8_t rf_ch_freq)
{
    nrf24l01_driver_data.rf_ch.rf_ch = rf_ch_freq & NRF24L01_RF_CH_MASK; 
    nrf24l01_rf_ch_reg_write(); 
}


// Set RF data rate 
void nrf24l01_set_rate(
    nrf24l01_data_rate_t rate)
{
    nrf24l01_driver_data.rf_setup.rf_dr_low = (rate >> SHIFT_1) & NRF24L01_RF_DR_MASK; 
    nrf24l01_driver_data.rf_setup.rf_dr_high = rate & NRF24L01_RF_DR_MASK; 
    nrf24l01_rf_set_reg_write(); 
}


// Set power mode 
void nrf24l01_set_pwr_mode(
    nrf24l01_pwr_mode_t pwr_mode)
{
    // Set CE=0 to enter standby-1 state 
    gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Set PWR_UP=pwr_mode to the desired power mode 
    nrf24l01_driver_data.config.pwr_up = (uint8_t)pwr_mode; 
    nrf24l01_config_reg_write(); 

    // Delay to allow for Startup time (~1.5ms) --> Only needed for exiting power down state 
    tim_delay_ms(nrf24l01_driver_data.timer, NRF24L01_START_DELAY); 
}

//=======================================================================================


//=======================================================================================
// Other functions 

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

// Register write 
void nrf24l01_reg_write(
    uint8_t cmd, 
    uint8_t *reg_write)
{
    nrf24l01_send(cmd, reg_write, BYTE_1); 
}


// Register read 
uint8_t nrf24l01_reg_read(
    uint8_t cmd)
{
    // Read and return the register value 
    uint8_t reg_read = CLEAR; 
    nrf24l01_receive(cmd, &reg_read, BYTE_1); 
    return reg_read; 
}


// CONFIG register write 
void nrf24l01_config_reg_write(void)
{
    // // Set CE=0 to make sure the device is not in TX/RX mode when writing to registers 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

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
    nrf24l01_reg_write(
        NRF24L01_CMD_W_REG | NRF24L01_REG_CONFIG, 
        &config); 

    // // Set CE=1 to put the device into an active mode 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// CONFIG register read 
uint8_t nrf24l01_config_reg_read(void)
{
    return nrf24l01_reg_read(NRF24L01_CMD_R_REG | NRF24L01_REG_CONFIG); 
}


// RF_CH register write 
void nrf24l01_rf_ch_reg_write(void)
{
    // // Set CE=0 to make sure the device is not in TX/RX mode when writing to registers 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Format the data to send 
    uint8_t rf_ch = (nrf24l01_driver_data.rf_ch.unused_1 << SHIFT_7) | 
                    (nrf24l01_driver_data.rf_ch.rf_ch); 

    // Send the data to the RF_CH register 
    nrf24l01_reg_write(
        NRF24L01_CMD_W_REG | NRF24L01_REG_RF_CH, 
        &rf_ch); 

    // // Set CE=1 to put the device back into it's previous data mode 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// RF_SETUP register write 
void nrf24l01_rf_set_reg_write(void)
{
    // // Set CE=0 to make sure the device is not in TX/RX mode when writing to registers 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Format the data to send 
    uint8_t rf_set = (nrf24l01_driver_data.rf_setup.cont_wave  << SHIFT_7) | 
                     (nrf24l01_driver_data.rf_setup.unused_1   << SHIFT_6) | 
                     (nrf24l01_driver_data.rf_setup.rf_dr_low  << SHIFT_5) | 
                     (nrf24l01_driver_data.rf_setup.pll_lock   << SHIFT_4) | 
                     (nrf24l01_driver_data.rf_setup.rf_dr_high << SHIFT_3) | 
                     (nrf24l01_driver_data.rf_setup.rf_pwr     << SHIFT_1) | 
                     (nrf24l01_driver_data.rf_setup.unused_2); 

    // Send the data to the RF_SET register 
    nrf24l01_reg_write(
        NRF24L01_CMD_W_REG | NRF24L01_REG_RF_SET, 
        &rf_set); 

    // // Set CE=1 to put the device back into it's previous data mode 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// STATUS register write 
void nrf24l01_status_reg_write(void)
{
    // // Set CE=0 to make sure the device is not in TX/RX mode when writing to registers 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_LOW); 

    // Format the data to send 
    uint8_t status_reg = (nrf24l01_driver_data.status_reg.unused_1 << SHIFT_7) | 
                         (nrf24l01_driver_data.status_reg.rx_dr    << SHIFT_6) | 
                         (nrf24l01_driver_data.status_reg.tx_ds    << SHIFT_5) | 
                         (nrf24l01_driver_data.status_reg.max_rt   << SHIFT_4) | 
                         (nrf24l01_driver_data.status_reg.rx_p_no  << SHIFT_1) | 
                         (nrf24l01_driver_data.status_reg.tx_full); 

    // Send the data to the STATUS register 
    nrf24l01_reg_write(
        NRF24L01_CMD_W_REG | NRF24L01_REG_STATUS, 
        &status_reg); 

    // // Set CE=1 to put the device back into it's previous data mode 
    // gpio_write(nrf24l01_driver_data.gpio_en, nrf24l01_driver_data.en_pin, GPIO_HIGH); 
}


// STATUS register read 
void nrf24l01_status_reg_read(void)
{
    // Local variables 
    uint8_t buff = CLEAR;   // dummy variable to pass to the receive function 

    // Read the STATUS register and update the data record using a "No Operation" 
    nrf24l01_receive(
        NRF24L01_CMD_NOP, 
        &buff, 
        BYTE_0); 
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
void nrf24l01_fifo_status_reg_read(void)
{
    // Read the FIFO_STATUS register and update the data record 
    uint8_t fifo_status_buff = nrf24l01_reg_read(NRF24L01_CMD_R_REG | NRF24L01_REG_FIFO); 

    // Store the FIFO status register byte into the data record 
    nrf24l01_driver_data.fifo_status.unused_1 = (fifo_status_buff & FILTER_BIT_7) >> SHIFT_7; 
    nrf24l01_driver_data.fifo_status.tx_reuse = (fifo_status_buff & FILTER_BIT_6) >> SHIFT_6; 
    nrf24l01_driver_data.fifo_status.tx_full  = (fifo_status_buff & FILTER_BIT_5) >> SHIFT_5; 
    nrf24l01_driver_data.fifo_status.tx_empty = (fifo_status_buff & FILTER_BIT_4) >> SHIFT_4; 
    nrf24l01_driver_data.fifo_status.unused_2 = (fifo_status_buff & FILTER_4_LSB) >> SHIFT_2; 
    nrf24l01_driver_data.fifo_status.rx_full  = (fifo_status_buff & FILTER_BIT_1) >> SHIFT_1; 
    nrf24l01_driver_data.fifo_status.rx_empty = (fifo_status_buff & FILTER_BIT_0); 
}

//=======================================================================================
