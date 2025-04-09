/**
 * @file ibus.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief IBUS driver 
 * 
 * @details IBUS is a specific type of serial protocol. This driver utilizes the UART 
 *          driver for communication. 
 *          
 *          Reference: https://thenerdyengineer.com/ibus-and-arduino/ 
 * 
 * @version 0.1
 * @date 2025-04-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "ibus.h" 

//=======================================================================================


//=======================================================================================
// Initialization 

// IBUS driver init 
void ibus_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_dma_config_t tx_dma, 
    uart_dma_config_t rx_dma)
{
    UART_STATUS uart_status = uart_init(
        uart, 
        gpio, 
        rx_pin, 
        tx_pin, 
        UART_FRAC_84_115200, 
        UART_MANT_84_115200, 
        tx_dma, 
        rx_dma); 

    if (uart_status != UART_OK)
    {
        return; 
    }

    uart_data_frame_config(uart, BYTE_0, BYTE_0, BYTE_2); 
}

//=======================================================================================


//=======================================================================================
// Send data 
//=======================================================================================
