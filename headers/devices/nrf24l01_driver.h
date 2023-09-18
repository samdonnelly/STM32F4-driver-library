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
#define NRF24L01_CMD_R_REG 0x00     // Read command and status registers 
#define NRF24L01_CMD_W_REG 0x20     // Write command and status registers 
#define NRF24L01_CMD_R_RX_PL 0x61   // Read RX payload 
#define NRF24L01_CMD_W_TX_PL 0xA0   // Write TX payload 
#define NRF24L01_CMD_FLUSH_TX 0xE1  // Flush TX FIFO 
#define NRF24L01_CMD_FLUSH_RX 0xE2  // Flush RX FIFO 
#define NRF24L01_CMD_REUSE_TX 0x00  // Reuse TX payload 
#define NRF24L01_CMD_NOP 0xFF       // No operation 

// Register addresses 
#define NRF24L01_REG_CONFIG 0x00    // CONFIG register address 
#define NRF24L01_REG_RF_CH 0x05     // RF_CH register address 
#define NRF24L01_REG_RF_SET 0x06    // RF_SETUP register address 
#define NRF24L01_REG_STATUS 0x07    // STATUS register address 
#define NRF24L01_REG_FIFO 0x17      // FIFO_STATUS register address 

// Data handling 
#define NRF24L01_RF_CH_MASK 0x7F    // RF channel frequency mask 
#define NRF24L01_RF_DR_MASK 0x01    // RF data rate bit mask 
#define NRF24L01_DUMMY_WRITE 0xFF   // Dummy data for SPI write-read operations 
#define NRF24L01_DATA_SIZE_LEN 2    // Data size indicator length 
#define NRF24L01_MAX_DATA_LEN 30    // Max data size that can be sent in one transaction 
#define NRF24L01_MAX_PACK_LEN 32    // Max data packet size (data size + data) 

// Control 
#define NRF24L01_PWR_ON_DELAY 100   // Device power on reset delay (ms) 
#define NRF24L01_START_DELAY 2      // Device start up delay (ms) 
#define NRF24L01_SETTLE_DELAY 130   // Device state settling time delay (us) 

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
    NRF24L01_PWR_UP, 
    NRF24L01_PWR_DOWN 
} nrf24l01_pwr_mode_t; 

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
 * @param rate : 
 * @param rf_ch_freq : 
 */
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer, 
    nrf24l01_data_rate_t rate, 
    uint8_t rf_ch_freq); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Returns data ready status 
 * 
 * @details Data in RX FIFO 
 */
uint8_t nrf24l01_data_ready_status(void); 


/**
 * @brief Receive payload 
 * 
 * @details 
 * 
 * @param read_buff 
 */
void nrf24l01_receive_payload(
    const uint8_t *read_buff); 


/**
 * @brief Send payload 
 * 
 * @details 
 * 
 * @param data_buff 
 * @param buff_len 
 */
void nrf24l01_send_payload(
    const uint8_t *data_buff, 
    uint8_t buff_len); 


/**
 * @brief Set frequency channel 
 * 
 * @details 
 * 
 * @param rf_ch_freq 
 */
void nrf24l01_set_channel(
    uint8_t rf_ch_freq); 


/**
 * @brief RF data rate set 
 * 
 * @details 
 * 
 * @param rate 
 */
void nrf24l01_set_rate(
    nrf24l01_data_rate_t rate); 


/**
 * @brief Low power mode 
 * 
 * @details 
 *          Make sure current data transfers are wrapped up. 
 *          Set CE=0 to enter standby-1 state. 
 *          Set PWR_UP=0 to enter power down state 
 * 
 * @param pwr_mode 
 */
void nrf24l01_set_pwr_mode(
    nrf24l01_pwr_mode_t pwr_mode); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_DRIVER_H_ 
