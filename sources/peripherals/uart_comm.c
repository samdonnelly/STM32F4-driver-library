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

#define UART_GET_TIMEOUT 10000        // Max number of times to get for received data 
#define CURSOR_MOVE_BUFF_SIZE 10 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Check for an IDLE line 
 * 
 * @param uart : UART port to use 
 * @return uint8_t : status of the idle line 
 */
uint8_t uart_idle_line_status(USART_TypeDef *uart); 


/**
 * @brief Clear the IDLE line detection bit 
 * 
 * @param uart : UART port to use 
 */
void uart_idle_line_clear(USART_TypeDef *uart); 

//=======================================================================================


//=======================================================================================
// Initialization 

// UART initialization 
UART_STATUS uart_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_param_config_t word_length, 
    uint8_t stop_bits, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant, 
    uart_param_config_t tx_dma, 
    uart_param_config_t rx_dma)
{
    if ((uart == NULL) || (gpio == NULL))
    {
        return UART_INVALID_PTR; 
    }

    // // Enable the UART clock 
    // if (uart == USART2)
    // {
    //     // USART2 
    //     RCC->APB1ENR |= (SET_BIT << SHIFT_17); 
    // }
    // else 
    // {
    //     // USART1 and USART6 
    //     RCC->APB2ENR |= (SET_BIT << (SHIFT_4 + (uint8_t)((uint32_t)(uart - USART1) >> SHIFT_10)));
    // }

    // Enable the UART clock 
    if (uart == USART1)
    {
        RCC->APB2ENR |= (SET_BIT << SHIFT_4); 
    }
    else if (uart == USART2)
    {
        RCC->APB1ENR |= (SET_BIT << SHIFT_17); 
    }
    else if (uart == USART6)
    {
        RCC->APB2ENR |= (SET_BIT << SHIFT_5); 
    }
    else 
    {
        // Not a valid pointer to a USART port 
        return UART_INVALID_PTR; 
    }

    // Configure the UART pins for alternative functions 
    gpio_pin_init(gpio, rx_pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio, rx_pin, SET_7); 
    gpio_pin_init(gpio, tx_pin, MODER_AF, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_afr(gpio, tx_pin, SET_7); 
    
    // Configure the data frame 
    uart_data_frame_config(uart, word_length, stop_bits, baud_frac, baud_mant); 

    // Configure TX and RX DMA in the USART_CR3 register 
    uart->CR3 |= (tx_dma << SHIFT_7);   // DMAT bit 
    uart->CR3 |= (rx_dma << SHIFT_6);   // DMAR bit 

    // Clear the idle line status before data is attempted to be read 
    uart_idle_line_clear(uart); 

    return UART_OK; 
}


// Configure the UART data frame 
void uart_data_frame_config(
    USART_TypeDef *uart, 
    uart_param_config_t word_length, 
    uint8_t stop_bits, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant)
{
    // Clear the USART_CR1 register 
    uart->CR1 = CLEAR;
    uart->BRR = CLEAR; 

    // Set the UE bit in the USART_CR1 register 
    uart->CR1 |= (SET_BIT << SHIFT_13); 

    // Word length 
    uart->CR1 |= (word_length << SHIFT_12); 

    // Stop bits - truncate other bits 
    stop_bits &= SET_3; 
    uart->CR2 |= (stop_bits << SHIFT_12); 

    // Set the baud rate 
    uart->BRR |= (baud_frac << SHIFT_0);   // Fractional 
    uart->BRR |= (baud_mant << SHIFT_4);   // Mantissa 

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
    uart_param_config_t peie, 
    uart_param_config_t txeie, 
    uart_param_config_t tcie, 
    uart_param_config_t rxneie, 
    uart_param_config_t idleie, 
    uart_param_config_t cts, 
    uart_param_config_t eie)
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
    if (uart == NULL)
    {
        return FALSE; 
    }

    // Check RXNE bit in the status register 
    return uart->SR & (SET_BIT << SHIFT_5); 
}


// UART clear data register 
void uart_clear_dr(USART_TypeDef *uart)
{
    if (uart == NULL)
    {
        return; 
    }

    dummy_read(uart->DR); 
}


// Check for an IDLE line 
uint8_t uart_idle_line_status(USART_TypeDef *uart)
{
    // Check IDLE bit in the status register 
    return uart->SR & (SET_BIT << SHIFT_4); 
}


// Clear the IDLE line detection bit 
void uart_idle_line_clear(USART_TypeDef *uart)
{
    dummy_read(uart->SR); 
    dummy_read(uart->DR); 
}

//=======================================================================================


//=======================================================================================
// Send Data 

// UART send character 
void uart_send_char(
    USART_TypeDef *uart, 
    uint8_t character)
{
    if (uart == NULL)
    {
        return; 
    }

    // Write the data to the data register then read the Transmission Complete (TC) bit 
    // in the status register continuously until it is set. 
    uart->DR = character; 
    while (!(uart->SR & (SET_BIT << SHIFT_6))); 
}


// UART send string 
void uart_send_str(
    USART_TypeDef *uart, 
    const char *string)
{
    if ((uart == NULL) || (string == NULL))
    {
        return; 
    }

    while ((*string != NULL_CHAR) && (string != NULL))
    {
        uart_send_char(uart, *string++); 
    }
}


// UART send data 
void uart_send_data(
    USART_TypeDef *uart, 
    const uint8_t *data, 
    uint16_t data_len)
{
    if ((uart == NULL) || (data == NULL))
    {
        return; 
    }

    for (uint16_t i = CLEAR; (i < data_len) && (data != NULL); i++)
    {
        uart_send_char(uart, *data++); 
    }
}


// UART send a numeric digit 
void uart_send_digit(
    USART_TypeDef *uart, 
    uint8_t digit)
{
    if (uart == NULL)
    {
        return; 
    }

    // Convert the digit into the ASCII character equivalent 
    uart_send_char(uart, digit + ZERO_CHAR); 
}


// UART send an integer 
void uart_send_integer(
    USART_TypeDef *uart, 
    int16_t integer)
{
    if (uart == NULL)
    {
        return; 
    }

    uint8_t digit = CLEAR;

    // Print the sign of the number 
    if (integer < 0)
    {
        // 2's complememt the integer so the correct value is printed
        integer = -integer;
        uart_send_char(uart, MINUS_CHAR);
    }
    else 
    {
        uart_send_char(uart, PLUS_CHAR);
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
    if (uart == NULL)
    {
        return; 
    }

    for (uint8_t i = CLEAR; i < num_spaces; i++)
    {
        uart_send_char(uart, SPACE_CHAR);
    }
}


// Send a carriage return and a new line  
void uart_send_new_line(USART_TypeDef *uart)
{
    if (uart == NULL)
    {
        return; 
    }

    uart_send_str(uart, "\r\n");
}


// Send cursor up the specified number of lines 
void uart_cursor_move(
    USART_TypeDef *uart, 
    uart_cursor_move_t dir, 
    uint8_t num_units)
{
    if (uart == NULL)
    {
        return; 
    }

    char cursor_move_str[CURSOR_MOVE_BUFF_SIZE]; 
    snprintf(cursor_move_str, CURSOR_MOVE_BUFF_SIZE, "\033[%u%c", num_units, (char)dir); 
    uart_send_str(uart, cursor_move_str); 
}

//=======================================================================================


//=======================================================================================
// Read Data 

// UART get character 
uint8_t uart_get_char(USART_TypeDef *uart)
{
    if (uart == NULL)
    {
        return NULL_CHAR; 
    }

    return (uint8_t)(uart->DR); 
}


// UART get data 
UART_STATUS uart_get_data(
    USART_TypeDef *uart, 
    uint8_t *data_buff)
{
    if ((uart == NULL) || (data_buff == NULL))
    {
        return UART_INVALID_PTR; 
    }

    uint16_t timer = UART_GET_TIMEOUT; 
    
    // Read from the UART data register as long as there is data available and the 
    // provided buffer is not full. If an idle line is detected then no more data is 
    // coming so the loop is terminated. A timeout is included to make sure the loop 
    // doesn't get stuck. 
    while ((data_buff != NULL) && --timer)
    {
        if (uart_data_ready(uart))
        {
            *data_buff++ = uart_get_char(uart); 
            timer = UART_GET_TIMEOUT; 
        }
        else if (uart_idle_line_status(uart))
        {
            uart_idle_line_clear(uart); 
            break; 
        }
    }

    // Make sure the data register is empty. If the loop above exits without having 
    // read all the available data then we don't want old data to trigger a new read. 
    while (uart_data_ready(uart))
    {
        uart_clear_dr(uart); 
    }

    // Terminate the data buffer to signify the end of the received data. The received 
    // data may already be terminated but this is added just in case. 
    if (data_buff != NULL)
    {
        *data_buff = NULL_CHAR; 
    }

    // Check for a timeout 
    if (timer == ZERO)
    {
        return UART_TIMEOUT; 
    }

    return UART_OK; 
}

//=======================================================================================
