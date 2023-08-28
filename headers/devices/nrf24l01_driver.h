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

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief nRF24L01 initialization 
 * 
 * @details 
 * 
 * @param spi : 
 * @param gpio : 
 * @param ss_pin : 
 */
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio, 
    gpio_pin_num_t ss_pin); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_DRIVER_H_ 
