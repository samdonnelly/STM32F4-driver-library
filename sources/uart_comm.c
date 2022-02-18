/**
 * @file uart_comm.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver for UART communication 
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "uart_comm.h"

//=======================================================================================


// UART2 setup 
void uart2_init(void)
{
    // Enable UART on pins PA2 and PA3 

    // Pin Setup 

    // Enable UART2 Clock - RCC_APB1 register, bit 17
    RCC->APB1ENR |= (SET_BIT << SHIFT_17);

    // Enable GPIOA clock for TX and RX pins - RCC_AHB! register, bit 0
    RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

    // Configure the UART pins for alternative functions - GPIOA_MODER register 
    GPIOA->MODER |= (SET_2 << SHIFT_4);   // pin PA2
    GPIOA->MODER |= (SET_2 << SHIFT_6);   // pin PA3

    // Set output speed of GPIO pins to high speed - GPIOA_OSPEEDR register 
    GPIOA->OSPEEDR |= (SET_3 << SHIFT_4);   // pin PA2
    GPIOA->OSPEEDR |= (SET_3 << SHIFT_6);   // pin PA3

    // Set the alternative function low ([0]) register for USART2 (AF7)
    GPIOA->AFR[0] |= (SET_7 << SHIFT_8);    // pin PA2
    GPIOA->AFR[0] |= (SET_7 << SHIFT_12);   // pin PA3

    // UART Configuration 

    // Clear the USART_CR1 register 
    USART2->CR1 = CLEAR;

    // Write the UE bit in the USART_CR1 register to 1
    USART2->CR1 |= (SET_BIT << SHIFT_13);

    // Program the M bit to define the word length
    USART2->CR1 &= ~(SET_BIT << SHIFT_12);  //  Set to zero for 8 bit word length

    // Program the number of stop bits in the USART_CR2 register 
    USART2->CR2 &= ~(SET_2 << SHIFT_12);  // Set to 0 for 1 stop bit 

    // Set the baud rate 
    USART2->BRR |= (USART_50MHZ_9600_FRAC << SHIFT_0);  // Fractional 
    USART2->BRR |= (USART_50MHZ_9600_MANT << SHIFT_4);  // Mantissa 

    // Enable the TX/RX by setting the TE and RE bits in USART_CR1 register 
    USART2->CR1 |= (SET_BIT << SHIFT_2);  // Set RE
    USART2->CR1 |= (SET_BIT << SHIFT_3);  // Set TE
}


// UART2 Transmit 
void uart2_tx(void)
{
    // 
}


// UART2 Receive
void uart2_rx(void)
{
    // 
}


