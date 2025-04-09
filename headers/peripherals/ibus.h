/**
 * @file ibus.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief IBUS driver interface 
 * 
 * @details IBUS is a specific type of serial protocol. This driver utilizes the UART 
 *          driver for communication. 
 * 
 * @version 0.1
 * @date 2025-04-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _IBUS_H_ 
#define _IBUS_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f411xe.h" 
#include "tools.h" 
#include "uart_comm.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define IBUS_DATA_BYTES 28   // Number of bytes needed to encode all RC channels 
#define IBUS_DATA_ITEMS 14   // Number of RC channels supported 

//=======================================================================================


//=======================================================================================
// Structures 

typedef union ibus_data_s 
{
    uint8_t data[IBUS_DATA_BYTES]; 
    uint16_t items[IBUS_DATA_ITEMS]; 
}
ibus_data_t; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief BUS driver init 
 * 
 * @param uart : UART port to initialize 
 * @param gpio : GPIO port of UART pins 
 * @param rx_pin : RX pin 
 * @param tx_pin : TX pin 
 * @param tx_dma : TX DMA enable 
 * @param rx_dma : RX DMA enable 
 */
void ibus_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_dma_config_t tx_dma, 
    uart_dma_config_t rx_dma); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _IBUS_H_ 
