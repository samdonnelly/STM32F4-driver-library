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

//=======================================================================================


//=======================================================================================
// Macros 

// Commands 
#define NRF24L01_CMD_R_REG 0x00     // Read command and status registers 
#define NRF24L01_CMD_W_REG 0x20     // Write command and status registers 
#define NRF24L01_CMD_R_RX_PL 0x61   // Read RX payload 
#define NRF24L01_CMD_W_RX_PL 0xA0   // Write TX payload 
#define NRF24L01_CMD_FLUSH_TX 0xE1  // Flush TX FIFO 
#define NRF24L01_CMD_FLUSH_RX 0xE2  // Flush RX FIFO 
#define NRF24L01_CMD_REUSE_TX 0x00  // Reuse TX payload 
#define NRF24L01_CMD_NOP 0xFF       // No operation 

// Register addresses 
#define NRF24L01_REG_CONFIG 0x00   // CONFIG register address 
#define NRF24L01_REG_RF_CH 0x05    // RF_CH register address 
#define NRF24L01_REG_RF_SET 0x06   // RF_SETUP register address 
#define NRF24L01_REG_STATUS 0x07   // STATUS register address 
#define NRF24L01_REG_FIFO 0x17     // FIFO_STATUS register address 

// Data handling 
#define NRF24L01_RF_CH_MASK 0x7F   // RF channel frequency mask 
#define NRF24L01_RF_DR_MASK 0x01   // RF data rate bit mask 
#define NRF24L01_DUMMY_WRITE 0xFF  // Dummy data for SPI write-read operations 

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

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief nRF24L01 initialization 
 * 
 * @details 
 * 
 * @param spi : 
 * @param gpio_ss : 
 * @param ss_pin : 
 * @param gpio_en : 
 * @param en_pin : 
 * @param rate : 
 * @param rf_ch_freq : 
 */
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    gpio_pin_num_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    gpio_pin_num_t en_pin, 
    nrf24l01_data_rate_t rate, 
    uint8_t rf_ch_freq); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Receive payload 
 * 
 * @details 
 */
void nrf24l01_receive_payload(void); 


/**
 * @brief Send payload 
 * 
 * @details 
 */
void nrf24l01_send_payload(void); 


/**
 * @brief Set frequency channel 
 * 
 * @details 
 */
void nrf24l01_set_channel(void); 


/**
 * @brief RF data rate set 
 * 
 * @details 
 */
void nrf24l01_set_rate(void); 


/**
 * @brief Status read --> non-operation write 
 * 
 * @details 
 */
void nrf24l01_get_status(void); 


/**
 * @brief Set RX mode 
 * 
 * @details 
 *          - Set PRIM_RX=1 
 *          - Set CE=1 
 */
void nrf24l01_set_rx_mode(void); 


/**
 * @brief Set TX mode 
 * 
 * @details 
 *          - Set PRIM_RX=0 
 *          - Set CE=1 
 */
void nrf24l01_set_tx_mode(void); 


/**
 * @brief Low power mode 
 * 
 * @details 
 *          Make sure current data transfers are wrapped up. 
 *          Set CE=0 to enter standby-1 state. 
 *          Set PWR_UP=0 to enter power down state 
 */
void nrf24l01_set_low_pwr(void); 


/**
 * @brief Standby mode 
 * 
 * @details 
 */
void nrf24l01_set_standby(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_DRIVER_H_ 