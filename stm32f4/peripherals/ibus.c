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
 *          References: 
 *          - https://thenerdyengineer.com/ibus-and-arduino/ 
 *          - https://forum.arduino.cc/t/ibus-protocol-decoded/1073658 
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
// Macros 

#define IBUS_PACKET_HEADER 0x4020 
#define IBUS_PACKET_HEADER_HI 0x40 
#define IBUS_PACKET_HEADER_LO 0x20 

//=======================================================================================


//=======================================================================================
// Initialization 

// IBUS driver init 
UART_STATUS ibus_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_param_config_t tx_dma, 
    uart_param_config_t rx_dma)
{
    UART_STATUS uart_status = uart_init(
        uart, 
        gpio, 
        rx_pin, 
        tx_pin, 
        UART_PARAM_DISABLE, 
        SET_2, 
        UART_FRAC_84_115200, 
        UART_MANT_84_115200, 
        tx_dma, 
        rx_dma); 

    return uart_status; 
}

//=======================================================================================


//=======================================================================================
// Send data 

// IBUS send data 
void ibus_send_data(
    USART_TypeDef *uart, 
    ibus_packet_t *packet)
{
    if ((uart == NULL) || (packet == NULL))
    {
        return; 
    }

    uint16_t checksum = HIGH_16BIT - IBUS_PACKET_HEADER; 
    packet->items[IBUS_HEADER] = IBUS_PACKET_HEADER; 

    for (uint8_t i = IBUS_CH1; i <= IBUS_CH14; i++)
    {
        checksum -= packet->items[i]; 
    }

    packet->items[IBUS_CHECKSUM] = checksum; 

    uart_send_data(uart, packet->data, IBUS_PACKET_BYTES); 
}

//=======================================================================================


//=======================================================================================
// Read data 

// IBUS get data 
UART_STATUS ibus_get_data(
    USART_TypeDef *uart, 
    ibus_packet_t *packet)
{
    if ((uart == NULL) || (packet == NULL))
    {
        return UART_INVALID_PTR; 
    }

    uint16_t checksum = HIGH_16BIT; 
    UART_STATUS status = uart_get_data(uart, packet->data); 

    if (status == UART_OK)
    {
        for (uint8_t i = IBUS_HEADER; i < IBUS_CHECKSUM; i++)
        {
            checksum -= packet->items[i]; 
        }

        if (checksum != packet->items[IBUS_CHECKSUM])
        {
            status = UART_BAD_DATA; 
        }
    }

    return status; 
}

//=======================================================================================


//=======================================================================================
// Data handling 

// Align IBUS packets 
ibus_packet_t* ibus_packet_align(
    uint8_t *packets, 
    uint16_t data_size)
{
    ibus_packet_t *packet_start = NULL; 

    for (uint16_t i = CLEAR; i < (data_size - 1); i++)
    {
        // Search for the packet header 
        if ((packets[i] == IBUS_PACKET_HEADER_LO) && 
            (packets[i + 1] == IBUS_PACKET_HEADER_HI))
        {
            packet_start = (ibus_packet_t *)&packets[i]; 
            break; 
        }
    }

    return packet_start; 
}

//=======================================================================================
