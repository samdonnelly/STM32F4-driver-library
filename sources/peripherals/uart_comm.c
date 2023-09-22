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


//=======================================================================================
// Finction Prototypes 

// /**
//  * @brief Select the fractional and mantissa portions of the baud rate setup 
//  * 
//  * @details Uses the baud_rate to determine the fractional and mantissa values used in 
//  *          the initialization of the baud rate in uart2_init. This function is made 
//  *          to make uart2_init easier to use by allowing for a baud rate to be passsed 
//  *          when initializing UART2 as opposed to fractional and mantissa parameters. 
//  * 
//  * @see uart1_init
//  * @see uart2_init
//  * @see uart_fractional_baud_t
//  * @see uart_mantissa_baud_t
//  * 
//  * @param baud_rate : (bps) communication speed of UART2
//  * @param baud_frac : fractional portion of UART2 baud rate setup 
//  * @param baud_mant : mantissa portion of UART2 baud rate setup
//  */
// void uart_baud_select(
//     uart_baud_rate_t baud_rate,
//     uart_clock_speed_t clock_speed, 
//     uart_fractional_baud_t *baud_frac,
//     uart_mantissa_baud_t *baud_mant);

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
// void uart_init(
//     USART_TypeDef *uart, 
//     GPIO_TypeDef *gpio, 
//     pin_selector_t rx_pin, 
//     pin_selector_t tx_pin, 
//     uart_baud_rate_t baud_rate,
//     uart_clock_speed_t clock_speed, 
//     uart_dma_tx_t tx_dma, 
//     uart_dma_rx_t rx_dma)
void uart_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant, 
    uart_dma_tx_t tx_dma, 
    uart_dma_rx_t rx_dma)
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

    // uart_set_baud_rate(uart, baud_rate, clock_speed); 
    uart_set_baud_rate(uart, baud_frac, baud_mant); 

    // Configure TX and RX DMA in the USART_CR3 register 
    uart->CR3 |= (tx_dma << SHIFT_7);   // DMAT bit 
    uart->CR3 |= (rx_dma << SHIFT_6);   // DMAR bit 
    
    //==================================================
}


// Change the baud rate of the UART 
// void uart_set_baud_rate(
//     USART_TypeDef *uart, 
//     uart_baud_rate_t baud_rate,
//     uart_clock_speed_t clock_speed)
void uart_set_baud_rate(
    USART_TypeDef *uart, 
    uart_fractional_baud_t baud_frac, 
    uart_mantissa_baud_t baud_mant)
{
    // Baud rate setup variables 
    // uart_fractional_baud_t baud_frac;
    // uart_mantissa_baud_t baud_mant;

    // Clear the USART_CR1 register 
    uart->CR1 = CLEAR;
    uart->BRR = CLEAR; 

    // Set the UE bit in the USART_CR1 register 
    uart->CR1 |= (SET_BIT << SHIFT_13);

    // Clear the M bit in the USART_CR1 register for 8-bit data 
    uart->CR1 &= ~(SET_BIT << SHIFT_12);

    // Set the baud rate 
    // uart_baud_select(baud_rate, clock_speed, &baud_frac, &baud_mant);
    // uart->BRR |= (baud_frac << SHIFT_0);  // Fractional 
    // uart->BRR |= (baud_mant << SHIFT_4);  // Mantissa 
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


// // Select the fractional and mantissa portions of the baud rate setup 
// void uart_baud_select(
//     uart_baud_rate_t baud_rate,
//     uart_clock_speed_t clock_speed, 
//     uart_fractional_baud_t *baud_frac,
//     uart_mantissa_baud_t *baud_mant)
// {
//     switch (clock_speed)
//     {
//         case UART_CLOCK_42:
//             switch (baud_rate)
//             {
//                 case UART_BAUD_9600:
//                     *baud_frac = UART_FRAC_42_9600;
//                     *baud_mant = UART_MANT_42_9600;
//                     break;
                
//                 default:
//                     break;
//             }
//             break;

//         case UART_CLOCK_84:
//             switch (baud_rate)
//             {
//                 case UART_BAUD_9600:
//                     *baud_frac = UART_FRAC_84_9600;
//                     *baud_mant = UART_MANT_84_9600;
//                     break;
                
//                 case UART_BAUD_38400:
//                     *baud_frac = UART_FRAC_84_38400;
//                     *baud_mant = UART_MANT_84_38400;
//                     break;
                
//                 case UART_BAUD_115200:
//                     *baud_frac = UART_FRAC_84_115200;
//                     *baud_mant = UART_MANT_84_115200;
//                     break;
                
//                 default:
//                     break;
//             }
//             break;
        
//         default:
//             break;
//     }
// }

//=======================================================================================


//=======================================================================================
// Register functions 

// Check if data is available for reading 
uint8_t uart_data_ready(
    USART_TypeDef *uart)
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
        uart_sendchar(uart, *string);
        string++;
    }
}


// UART send a numeric digit 
void uart_send_digit(
    USART_TypeDef *uart, 
    uint8_t digit)
{
    // Convert the digit into the ASCII character equivalent 
    uart_sendchar(uart, digit + UART_CHAR_DIGIT_OFFSET); 
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
        integer = -(integer);
        uart_sendchar(uart, UART_CHAR_MINUS_OFFSET);
    }
    else 
    {
        uart_sendchar(uart, UART_CHAR_PLUS_OFFSET);
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
    for (uint8_t i = 0; i < num_spaces; i++)
    {
        uart_sendchar(uart, UART_CHAR_SPACE_OFFSET);
    }
}


// Send a carriage return and a new line  
void uart_send_new_line(USART_TypeDef *uart)
{
    uart_sendstring(uart, "\r\n");
}

//=======================================================================================


//=======================================================================================
// Read Data 

// UART get character 
uint8_t uart_getchar(
    USART_TypeDef *uart)
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
    // Local variables 
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
void uart_clear_dr(
    USART_TypeDef *uart)
{
    dummy_read(uart->DR); 
}

//=======================================================================================
