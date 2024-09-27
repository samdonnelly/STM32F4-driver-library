/**
 * @file uart_comm.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief UART driver 
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


//=======================================================================================
// Macros 

#define CURSOR_MOVE_BUFF_SIZE 10 

//=======================================================================================


//=======================================================================================
// Initialization 

//===================================================
// Pin information for UART1
//  PA9:  TX
//  PA10: RX
// 
// Pin information for UART2
//  PA2: TX
//  PA3: RX
// 
// Pin information for UART6 
//  PC6 or PA11: TX 
//  PC7 or PA12: RX 
//===================================================

// UART initialization 
void uart_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant, 
    uart_dma_config_t tx_dma, 
    uart_dma_config_t rx_dma)
{
    //==================================================
    // Enable the UART clock 

    if (uart == USART2)
    {
        // USART2 
        RCC->APB1ENR |= (SET_BIT << SHIFT_17); 
    }
    else 
    {
        // USART1 and USART6 
        RCC->APB2ENR |= (SET_BIT << (SHIFT_4 + (uint8_t)((uint32_t)(uart - USART1) >> SHIFT_10)));
    }
    
    //==================================================

    //==================================================
    // Configure the UART pins for alternative functions 

    // RX pin 
    gpio_pin_init(gpio, rx_pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio, rx_pin, SET_7); 

    // TX pin 
    gpio_pin_init(gpio, tx_pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio, tx_pin, SET_7); 
    
    //==================================================

    //==================================================
    // Configure the UART 

    // Set the baud rate of the UART 
    uart_set_baud_rate(uart, baud_frac, baud_mant); 

    // Configure TX and RX DMA in the USART_CR3 register 
    uart->CR3 |= (tx_dma << SHIFT_7);   // DMAT bit 
    uart->CR3 |= (rx_dma << SHIFT_6);   // DMAR bit 
    
    //==================================================
}


// Change the baud rate of the UART 
void uart_set_baud_rate(
    USART_TypeDef *uart, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant)
{
    // Clear the USART_CR1 register 
    uart->CR1 = CLEAR;
    uart->BRR = CLEAR; 

    // Set the UE bit in the USART_CR1 register 
    uart->CR1 |= (SET_BIT << SHIFT_13);

    // Clear the M bit in the USART_CR1 register for 8-bit data 
    uart->CR1 &= ~(SET_BIT << SHIFT_12);

    // Set the baud rate 
    uart->BRR |= (baud_frac << SHIFT_0);  // Fractional 
    uart->BRR |= (baud_mant << SHIFT_4);  // Mantissa 

    // Enable the TX/RX by setting the RE and TE bits in USART_CR1 register 
    uart->CR1 |= (SET_BIT << SHIFT_2);
    uart->CR1 |= (SET_BIT << SHIFT_3); 

    // Clear the TC and RXNE status bits 
    while (!(uart->SR & (SET_BIT << SHIFT_6)));
    while (uart->SR & (SET_BIT << SHIFT_5)) 
    {
        uart_clear_dr(uart); 
    }
}


// UART interrupt initialization 
void uart_interrupt_init(
    USART_TypeDef *uart, 
    uart_int_config_t peie, 
    uart_int_config_t txeie, 
    uart_int_config_t tcie, 
    uart_int_config_t rxneie, 
    uart_int_config_t idleie, 
    uart_int_config_t cts, 
    uart_int_config_t eie)
{
    // Parity error (PE) interrupt enable 
    uart->CR1 |= (peie << SHIFT_8); 

    // Transmit data register interrupt enable (TXE) 
    uart->CR1 |= (txeie << SHIFT_7); 
    
    // Transmission complete interrupt enable (TCIE) 
    uart->CR1 |= (tcie << SHIFT_6); 
    
    // Read data register interrupt enable (RXNEIE) 
    uart->CR1 |= (rxneie << SHIFT_5); 
    
    // IDLE line detected interrupt enable (IDLEIE) 
    uart->CR1 |= (idleie << SHIFT_4); 
    
    // CTS interrupt enable 
    uart->CR3 |= (cts << SHIFT_10); 
    
    // Error interrupt enable (EIE) 
    uart->CR3 |= (eie << SHIFT_0); 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// Check if data is available for reading 
uint8_t uart_data_ready(USART_TypeDef *uart)
{
    // Check RXNE bit in the status register 
    if (uart->SR & (SET_BIT << SHIFT_5)) 
    {
        return TRUE; 
    }
    
    return FALSE; 
}

//=======================================================================================


//=======================================================================================
// Send Data 

// UART send character  
void uart_sendchar(
    USART_TypeDef *uart, 
    uint8_t character)
{
    // Write the data to the data register (USART_DR). 
    uart->DR = character;

    // Read the Transmission Complete (TC) bit (bit 6) in the status register 
    // (USART_SR) continuously until it is set. 
    while (!(uart->SR & (SET_BIT << SHIFT_6)));
}


// UART send string 
void uart_sendstring(
    USART_TypeDef *uart, 
    const char *string)
{
    // Loop until null character of string is reached. 
    while (*string)
    {
        uart_sendchar(uart, *string++);
    }
}


// UART send a numeric digit 
void uart_send_digit(
    USART_TypeDef *uart, 
    uint8_t digit)
{
    // Convert the digit into the ASCII character equivalent 
    uart_sendchar(uart, digit + ZERO_CHAR); 
}


// UART send an integer 
void uart_send_integer(
    USART_TypeDef *uart, 
    int16_t integer)
{
    // Store a digit to print 
    uint8_t digit;

    // Print the sign of the number 
    if (integer < 0)
    {
        // 2's complememt the integer so the correct value is printed
        integer = -integer;
        uart_sendchar(uart, MINUS_CHAR);
    }
    else 
    {
        uart_sendchar(uart, PLUS_CHAR);
    }

    // Parse and print each digit
    digit = (uint8_t)((integer / DIVIDE_10000) % REMAINDER_10);
    uart_send_digit(uart, digit);

    digit = (uint8_t)((integer / DIVIDE_1000) % REMAINDER_10);
    uart_send_digit(uart, digit);

    digit = (uint8_t)((integer / DIVIDE_100) % REMAINDER_10);
    uart_send_digit(uart, digit);

    digit = (uint8_t)((integer / DIVIDE_10) % REMAINDER_10);
    uart_send_digit(uart, digit);

    digit = (uint8_t)(integer % REMAINDER_10);
    uart_send_digit(uart, digit);
}


// Send a desired number of spaces 
void uart_send_spaces(
    USART_TypeDef *uart, 
    uint8_t num_spaces)
{
    for (uint8_t i = CLEAR; i < num_spaces; i++)
    {
        uart_sendchar(uart, SPACE_CHAR);
    }
}


// Send a carriage return and a new line  
void uart_send_new_line(USART_TypeDef *uart)
{
    uart_sendstring(uart, "\r\n");
}


// Send cursor up the specified number of lines 
void uart_cursor_move(
    USART_TypeDef *uart, 
    uart_cursor_move_t direction, 
    uint8_t num_lines)
{
    char cursor_up_str[CURSOR_MOVE_BUFF_SIZE]; 
    snprintf(cursor_up_str, CURSOR_MOVE_BUFF_SIZE, "\033[%u%c", num_lines, (char)direction); 
    uart_sendstring(uart, cursor_up_str); 
}

//=======================================================================================


//=======================================================================================
// Read Data 

// UART get character 
uint8_t uart_getchar(USART_TypeDef *uart)
{
    return (uint8_t)(uart->DR);  // Read and return data from data register 
}


// UART get string 
UART_STATUS uart_getstr(
    USART_TypeDef *uart, 
    char *str_buff, 
    uint8_t buff_len, 
    uart_str_term_t term_char)
{
    uint8_t input = CLEAR; 
    uint8_t char_count = CLEAR; 
    uint8_t max_char_read = buff_len - UART_BUFF_TERM_OFST; 
    uint16_t timer = UART_GETSTR_TIMEOUT; 

    // Read UART data until string termination, timeout or max character read length 
    // The max character read length is left as one less than the buffer length so 
    // there is a spot for a NULL termination. 
    do
    {
        // Wait for data to be available then read and store it 
        if (uart_data_ready(uart))
        {
            input = uart_getchar(uart);
            *str_buff++ = input;
            timer = UART_GETSTR_TIMEOUT; 
            char_count++; 
        }
    } 
    while((input != term_char) && --timer && (char_count < max_char_read)); 

    // Add a null character to the end of the string 
    *str_buff = UART_STR_TERM_NULL; 

    // Check for timeout 
    if (timer)
    {
        return UART_OK; 
    }

    return UART_TIMEOUT; 
}


// UART clear data register 
void uart_clear_dr(USART_TypeDef *uart)
{
    dummy_read(uart->DR); 
}

//=======================================================================================
