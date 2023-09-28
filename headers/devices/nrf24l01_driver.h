/**
 * @file nrf24l01_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module driver header 
 * 
 * @version 0.1
 * @date 2023-07-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _NRF24L01_DRIVER_H_ 
#define _NRF24L01_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 
#include "spi_comm.h"
#include "gpio_driver.h"
#include "timers.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Commands 
#define NRF24L01_CMD_R_REG    0x00     // Read command and status registers 
#define NRF24L01_CMD_W_REG    0x20     // Write command and status registers 
#define NRF24L01_CMD_R_RX_PL  0x61     // Read RX payload 
#define NRF24L01_CMD_W_TX_PL  0xA0     // Write TX payload 
#define NRF24L01_CMD_FLUSH_TX 0xE1     // Flush TX FIFO 
#define NRF24L01_CMD_FLUSH_RX 0xE2     // Flush RX FIFO 
#define NRF24L01_CMD_REUSE_TX 0x00     // Reuse TX payload 
#define NRF24L01_CMD_NOP      0xFF     // No operation 

// Register addresses 
#define NRF24L01_REG_CONFIG     0x00   // CONFIG register address 
#define NRF24L01_REG_EN_AA      0x01   // EN_AA register address 
#define NRF24L01_REG_EN_RXADDR  0x02   // EN_RXADDR register address 
#define NRF24L01_REG_SETUP_AW   0x03   // SETUP_AW register address 
#define NRF24L01_REG_SETUP_RETR 0x04   // SETUP_RETR register address 
#define NRF24L01_REG_RF_CH      0x05   // RF_CH register address 
#define NRF24L01_REG_RF_SET     0x06   // RF_SETUP register address 
#define NRF24L01_REG_STATUS     0x07   // STATUS register address 
#define NRF24L01_REG_OBSERVE_TX 0x08   // OBSERVE_TX register address 
#define NRF24L01_REG_RPD        0x09   // RPD register address 
#define NRF24L01_REG_RX_ADDR_P0 0x0A   // RX_ADDR_P0 register address 
#define NRF24L01_REG_RX_ADDR_P1 0x0B   // RX_ADDR_P1 register address 
#define NRF24L01_REG_RX_ADDR_P2 0x0C   // RX_ADDR_P2 register address 
#define NRF24L01_REG_RX_ADDR_P3 0x0D   // RX_ADDR_P3 register address 
#define NRF24L01_REG_RX_ADDR_P4 0x0E   // RX_ADDR_P4 register address 
#define NRF24L01_REG_RX_ADDR_P5 0x0F   // RX_ADDR_P5 register address 
#define NRF24L01_REG_TX_ADDR    0x10   // TX_ADDR register address 
#define NRF24L01_REG_RX_PW_P0   0x11   // RX_PW_P0 register address 
#define NRF24L01_REG_RX_PW_P1   0x12   // RX_PW_P1 register address 
#define NRF24L01_REG_RX_PW_P2   0x13   // RX_PW_P2 register address 
#define NRF24L01_REG_RX_PW_P3   0x14   // RX_PW_P3 register address 
#define NRF24L01_REG_RX_PW_P4   0x15   // RX_PW_P4 register address 
#define NRF24L01_REG_RX_PW_P5   0x16   // RX_PW_P5 register address 
#define NRF24L01_REG_FIFO       0x17   // FIFO_STATUS register address 
#define NRF24L01_REG_DYNPD      0x1C   // DYNPD register address 
#define NRF24L01_REG_FEATURE    0x1D   // FEATURE register address 

// Register reset values (excludes read only bits) 
#define NRF24L01_REG_RESET_CONFIG     0x08   // CONFIG register reset value 
#define NRF24L01_REG_RESET_EN_AA      0x3F   // EN_AA register reset value 
#define NRF24L01_REG_RESET_EN_RXADDR  0x03   // EN_RXADDR register reset value 
#define NRF24L01_REG_RESET_SETUP_AW   0x03   // SETUP_AW register reset value 
#define NRF24L01_REG_RESET_SETUP_RETR 0x03   // SETUP_RETR register reset value 
#define NRF24L01_REG_RESET_RF_CH      0x02   // RF_CH register reset value 
#define NRF24L01_REG_RESET_RF_SET     0x0E   // RF_SETUP register reset value 
#define NRF24L01_REG_RESET_STATUS     0x70   // STATUS register reset value 
#define NRF24L01_REG_RESET_OBSERVE_TX 0x00   // OBSERVE_TX register reset value 
#define NRF24L01_REG_RESET_RPD        0x00   // RPD register reset value 
#define NRF24L01_REG_RESET_RX_ADDR_P0 0xE7   // RX_ADDR_P0 register reset value 
#define NRF24L01_REG_RESET_RX_ADDR_P1 0xC2   // RX_ADDR_P1 register reset value 
#define NRF24L01_REG_RESET_RX_ADDR_P2 0xC3   // RX_ADDR_P2 register reset value 
#define NRF24L01_REG_RESET_RX_ADDR_P3 0xC4   // RX_ADDR_P3 register reset value 
#define NRF24L01_REG_RESET_RX_ADDR_P4 0xC5   // RX_ADDR_P4 register reset value 
#define NRF24L01_REG_RESET_RX_ADDR_P5 0xC6   // RX_ADDR_P5 register reset value 
#define NRF24L01_REG_RESET_TX_ADDR    0xE7   // TX_ADDR register reset value 
#define NRF24L01_REG_RESET_RX_PW_PX   0x00   // RX_PW_PX (X-->0-5) register reset value 
#define NRF24L01_REG_RESET_DYNPD      0x00   // DYNPD register reset value 
#define NRF24L01_REG_RESET_FEATURE    0x00   // FEATURE register reset value 

// Data handling 
#define NRF24L01_RF_CH_MASK 0x7F       // RF channel frequency mask 
#define NRF24L01_RF_DR_MASK 0x01       // RF data rate bit mask 
#define NRF24L01_DUMMY_WRITE 0xFF      // Dummy data for SPI write-read operations 
#define NRF24L01_DATA_SIZE_LEN 1       // Data size indicator length 
#define NRF24L01_MAX_PAYLOAD_LEN 32    // Max data packet size (data size + data) 
#define NRF24L01_MAX_DATA_LEN 30       // Max user data length 
#define NRF24l01_ADDR_WIDTH 5          // Address width 

// Control 
#define NRF24L01_PWR_ON_DELAY 100      // Device power on reset delay (ms) 
#define NRF24L01_START_DELAY 2         // Device start up delay (ms) 
#define NRF24L01_SETTLE_DELAY 130      // Device state settling time delay (us) 
#define NRF24L01_DISABLE_REG 0x00      // Disable settings in a register 

//=======================================================================================


//=======================================================================================
// Enums 
/**
 * @brief Data rate to use 
 * 
 * @details A slower data rate will allow for longer range communication (better receiver 
 *          sensitivity). A higher data rate has lower average current consumption and 
 *          reduced probability of on-air collisions. The transmitter and receiver must 
 *          have the same data rate set in order to communicate with one another, the same 
 *          goes with the channel frequency. 
 */
typedef enum {
    NRF24L01_DR_1MBPS, 
    NRF24L01_DR_2MBPS, 
    NRF24L01_DR_250KBPS 
} nrf24l01_data_rate_t; 


/**
 * @brief 
 * 
 * @details 
 */
typedef enum {
    NRF24L01_RF_PWR_18DBM, 
    NRF24L01_RF_PWR_12DBM, 
    NRF24L01_RF_PWR_6DBM, 
    NRF24L01_RF_PWR_0DBM
} nrf24l01_rf_pwr_t; 


/**
 * @brief TX/RX mode setter 
 * 
 * @details Used to set the PRIM_RX bit to choose between TX and RX mode. 
 */
typedef enum {
    NRF24L01_TX_MODE, 
    NRF24L01_RX_MODE
} nrf24l01_mode_select_t; 


/**
 * @brief 
 * 
 * @details 
 */
typedef enum {
    NRF24L01_PWR_DOWN, 
    NRF24L01_PWR_UP 
} nrf24l01_pwr_mode_t; 


/**
 * @brief 
 * 
 * @details 
 */
typedef enum {
    NRF24L01_DP_0, 
    NRF24L01_DP_1, 
    NRF24L01_DP_2, 
    NRF24L01_DP_3, 
    NRF24L01_DP_4, 
    NRF24L01_DP_5 
} nrf24l01_data_pipe_t; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief nRF24L01 initialization 
 * 
 * @details 
 *          
 *          NOTE: The device can run onto SPI up to 10Mbps. Ensure the SPI initialized has 
 *                a speed less than or equal to this. 
 *          
 *          NOTE: the timer must be a timer that increments every 1us so that the timer 
 *                delay functions can be used. 
 * 
 * @param spi : 
 * @param gpio_ss : 
 * @param ss_pin : 
 * @param gpio_en : 
 * @param en_pin : 
 * @param timer : 
 */
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer); 

//=======================================================================================


//=======================================================================================
// User configuration functions 

/**
 * @brief Configure a device as PTX 
 * 
 * @details 
 * 
 * @param tx_addr : 
 */
void nrf24l01_ptx_config(
    const uint8_t *tx_addr); 


/**
 * @brief Configure a device as PRX 
 * 
 * @details 
 * 
 * @param rx_addr 
 * @param pipe_num 
 */
void nrf24l01_prx_config(
    const uint8_t *rx_addr, 
    nrf24l01_data_pipe_t pipe_num); 


/**
 * @brief Set frequency channel 
 * 
 * @details 
 * 
 * @param rf_ch_freq 
 */
void nrf24l01_set_rf_channel(
    uint8_t rf_ch_freq); 


/**
 * @brief RF data rate set 
 * 
 * @details 
 * 
 * @param rate 
 */
void nrf24l01_set_rf_dr(
    nrf24l01_data_rate_t rate); 


/**
 * @brief Set power output 
 * 
 * @param rf_pwr 
 */
void nrf24l01_set_rf_pwr(
    nrf24l01_rf_pwr_t rf_pwr); 


/**
 * @brief Enter low power mode - power down 
 * 
 * @details 
 *          Make sure current data transfers are wrapped up. 
 *          Set CE=0 to enter standby-1 state. 
 *          Set PWR_UP=0 to enter power down state 
 */
void nrf24l01_pwr_down(void); 


/**
 * @brief Exit low power mode - power up 
 * 
 * @details 
 */
void nrf24l01_pwr_up(void); 

//=======================================================================================


//=======================================================================================
// User status functions 

/**
 * @brief Returns data ready status 
 * 
 * @details Data in RX FIFO 
 *          
 *          It's important to read data from the RX FIFO when it's available. Data will fill 
 *          up in the RX FIFO and if the FIFO becomes full then new incoming data will be 
 *          ignored/discarded/lost. 
 * 
 * @param pipe_num : 
 * @return uint8_t : RX FIFO data status 
 */
uint8_t nrf24l01_data_ready_status(
    nrf24l01_data_pipe_t pipe_num); 


/**
 * @brief Get power mode 
 * 
 * @details 
 * 
 * @return nrf24l01_pwr_mode_t 
 */
nrf24l01_pwr_mode_t nrf24l01_get_pwr_mode(void); 


/**
 * @brief Get active mode 
 * 
 * @details 
 * 
 * @return nrf24l01_mode_select_t 
 */
nrf24l01_mode_select_t nrf24l01_get_mode(void); 


/**
 * @brief Get RF channel 
 * 
 * @details 
 * 
 * @return uint8_t 
 */
uint8_t nrf24l01_get_rf_ch(void); 


/**
 * @brief Get RF data rate 
 * 
 * @details 
 * 
 * @return nrf24l01_data_rate_t 
 */
nrf24l01_data_rate_t nrf24l01_get_rf_dr(void); 


/**
 * @brief Get power output 
 * 
 * @details 
 * 
 * @return nrf24l01_rf_pwr_t 
 */
nrf24l01_rf_pwr_t nrf24l01_get_rf_pwr(void); 

//=======================================================================================


//=======================================================================================
// User data functions 

/**
 * @brief Receive payload 
 * 
 * @details 
 *          
 *          NOTE: This function can only be properly used while not in low power mode. 
 *          
 *          NOTE: read_buff must be at least 30 bytes long. This is the longest possible 
 *                data packet that can be received so if read_buff is smaller than this 
 *                some data could be lost. 
 * 
 * @param read_buff : buffer to store the received payload 
 * @param pipe_num : 
 */
void nrf24l01_receive_payload(
    uint8_t *read_buff, 
    nrf24l01_data_pipe_t pipe_num); 


/**
 * @brief Send payload 
 * 
 * @details 
 *          
 *          This function will put the device into TX mode just long enough to send the payload 
 *          out. The device is not supposed to remain in TX mode for longer than 4ms so once a 
 *          single packet has been sent the device is put back into RX mode. 
 *          
 *          The device has 3 separate 32-byte TX FIFOs. This means the data between each FIFO is 
 *          not connected. When sending payloads you can send up to 32 bytes to the device at once 
 *          because that is the capacity of a single FIFO. However, this driver caps the data size 
 *          at 30 bytes to make room for data length and NULL termination bytes. 
 *          
 *          This function determines the length of the payload passed in data_buff so that it 
 *          doesn't have to be specified by the application. However, if the length of the 
 *          payload is too large, not all the data will be sent (see note below). Determining 
 *          payload length is handled here and not left to the application because if this 
 *          device is used to send data that doesn't have a predefined length then the length of 
 *          the data would have to be determined anyway. 
 *          
 *          NOTE: The max data length that can be sent at one time (one call of this function) 
 *                is 30 bytes. The device FIFO supports 32 bytes but the first byte is used to 
 *                store the data length and the data is terminated with a NULL character. 
 *          
 *          NOTE: This function can only be properly used while not in low power mode. 
 * 
 * @param data_buff : buffer that contains data to be sent 
 * @param data_len : 
 * @return uint8_t : 
 */
uint8_t nrf24l01_send_payload(
    const uint8_t *data_buff, 
    uint8_t data_len); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_DRIVER_H_ 
