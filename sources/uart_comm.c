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

void uart2_init(void)
{
    // UART setup 

    // Pin Setup 

    // Enable UART2 Clock - RCC_APB1 register, bit 17
    RCC->APB1ENR |= (SET_BIT << SHIFT_17);

    // Enable GPIOA clock for TX and RX pins - RCC_AHB! register, bit 0
    RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

    // Configure the UART pins for alternative functions - GPIOA_MODER register 
    GPIOA->MODER |= (SET_TWO << SHIFT_4);   // pin PA2
    GPIOA->MODER |= (SET_TWO << SHIFT_6);   // pin PA3

    // Set output speed of GPIO pins to high speed - GPIOA_OSPEEDR register 
    GPIOA->OSPEEDR |= (SET_THREE << SHIFT_4);   // pin PA2
    GPIOA->OSPEEDR |= (SET_THREE << SHIFT_6);   // pin PA3

    // Set the alternative function low ([0]) register for USART2 (AF7)
    GPIOA->AFR[0] |= (SET_SEVEN << SHIFT_8);    // pin PA2
    GPIOA->AFR[0] |= (SET_SEVEN << SHIFT_12);   // pin PA3

    // UART Configuration 

    // 
}



