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

#define IBUS_PACKET_BYTES 32   // Number of bytes needed to encode an IBUS packet 
#define IBUS_PACKET_ITEMS 16   // Number of items in an IBUS packet - see ibus_packet_index_t 

//=======================================================================================


//=======================================================================================
// Enums 

// IBUS packet index 
typedef enum {
    IBUS_HEADER, 
    IBUS_CH1, 
    IBUS_CH2, 
    IBUS_CH3, 
    IBUS_CH4, 
    IBUS_CH5, 
    IBUS_CH6, 
    IBUS_CH7, 
    IBUS_CH8, 
    IBUS_CH9, 
    IBUS_CH10, 
    IBUS_CH11, 
    IBUS_CH12, 
    IBUS_CH13, 
    IBUS_CH14, 
    IBUS_CHECKSUM 
} ibus_packet_index_t; 

//=======================================================================================


//=======================================================================================
// Structures 

typedef union ibus_PACKET_s 
{
    uint8_t data[IBUS_PACKET_BYTES]; 
    uint16_t items[IBUS_PACKET_ITEMS]; 
}
ibus_packet_t; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief BUS driver init 
 * 
 * @details Wrapper for the UART init function that fixes certain parameters needed for 
 *          IBUS protocol. This can be used instead of the UART init function if using 
 *          the IBUS protocol. 
 * 
 * @param uart : UART port to initialize 
 * @param gpio : GPIO port of UART pins 
 * @param rx_pin : RX pin 
 * @param tx_pin : TX pin 
 * @param tx_dma : TX DMA enable 
 * @param rx_dma : RX DMA enable 
 * @return UART_STATUS : status of the uart initialization 
 */
UART_STATUS ibus_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_param_config_t tx_dma, 
    uart_param_config_t rx_dma); 

//=======================================================================================


//=======================================================================================
// Send data 

/**
 * @brief IBUS send data 
 * 
 * @details Formats the provided IBUS data packet and sends it via UART. The provided 
 *          packet channel values are not modified, only the header and checksum. 
 * 
 * @param uart : UART port to use 
 * @param packet : IBUS packet to send 
 */
void ibus_send_data(
    USART_TypeDef *uart, 
    ibus_packet_t *packet); 

//=======================================================================================


//=======================================================================================
// Read data 

/**
 * @brief IBUS get data 
 * 
 * @details This function is not recommended as it requires polling for data. However, 
 *          if used then this will copy data to the provided packet buffer if there is 
 *          data available to be read. This function does not provide feedback on if 
 *          there was new data read, so the user should check for data availability first 
 *          if they want to be sure. Receivers send IBUS data typically every ~7ms. The 
 *          checksum is calculated and compared to the checksum item in the packets and 
 *          a "UART_BAD_DATA" status will be returned if it does not match. It's the 
 *          users responsibility to ensure that IBUS data will be received on the 
 *          specified UART port. 
 *          
 *          The recommended way to read incoming UART data is via DMA and an interrupt. 
 * 
 * @param uart : UART port to use 
 * @param packet : IBUS packet buffer where packet data will be copied to 
 * @return UART_STATUS : status of the read operation 
 */
UART_STATUS ibus_get_data(
    USART_TypeDef *uart, 
    ibus_packet_t *packet); 

//=======================================================================================


//=======================================================================================
// Data handling 

/**
 * @brief Align IBUS packets 
 * 
 * @details There may be cases where each IBUS packet is not processed/checked right as 
 *          it comes in, but instead packets are collected and checked at an interval 
 *          unrelated to IBUS timing. For example, IBUS data gets sent every 7ms but the 
 *          system may only check packets every 50ms. In this example, the rate at which 
 *          data arrives and the rate at which it's checked are not synced/aligned with 
 *          one another so the collected data may not start and end with complete 
 *          packets. To allow this type of packet collection to occur, this function 
 *          takes the packet data buffer and finds the first start to a packet, then 
 *          returns the location of the packet start in the buffer. 
 * 
 * @param packets : buffer containing packets 
 * @param data_size : size of data in buffer 
 * @return ibus_packet_t : pointer to the start of a packet in the packets buffer 
 */
ibus_packet_t* ibus_packet_align(
    uint8_t *packets, 
    uint16_t data_size); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _IBUS_H_ 
