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

/*
 * STM32F4 Reference Manual Procedure to Configuring UART
 * 
 * 1. Enable the UART CLOCK and GPIO CLOCK 
 * 2. Configure the UART pins for alternative functions 
 * 3. Enable the USART by writing the UE bit in USART_CR1 register to 1
 *          a. USART_CR1: Control register 1 - Page 550 
 * 4. Program the M bit in USART_CR1 to define the word length 
 * 5. Program the number of stop bits in USART_CR2
 * 6. Select the desired baud rate using the USART_BRR register 
 * 7. Set the TE bit in USART_CR1 to send an idle frame as first transmission
 * 8. Enable the transmitter/reciever by setting the TE and RE bits in USART_CR1 register 
 * 
 */

void uart_init(void)
{
    // UART setup 

    // 1. Enable the UART CLOCK and GPIO CLOCK
    RCC->APB1ENR |= (SET_BIT << SHIFT_17);  // Enable UART2 Clock 
    RCC->APB1ENR |= (SET_BIT << SHIFT_0);   // Enable GPIOA clock for TX and RX pins

    // 2. Configure the UART pins for alternative functions 
 
}



