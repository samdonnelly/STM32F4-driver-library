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
#include "timers.h"

//=======================================================================================


//=======================================================================================
// Finction Prototypes 

//==============================================================
// Note: These functions are for internal driver use only and 
//       are therefore not included in the header file. 
//==============================================================

/**
 * @brief Select the fractional and mantissa portions of the baud rate setup 
 * 
 * @details Uses the baud_rate to determine the fractional and mantissa values used in 
 *          the initialization of the baud rate in uart2_init. This function is made 
 *          to make uart2_init easier to use by allowing for a baud rate to be passsed 
 *          when initializing UART2 as opposed to fractional and mantissa parameters. 
 * 
 * @see uart1_init
 * @see uart2_init
 * @see uart_fractional_baud_t
 * @see uart_mantissa_baud_t
 * 
 * @param baud_rate : (bps) communication speed of UART2
 * @param baud_frac : fractional portion of UART2 baud rate setup 
 * @param baud_mant : mantissa portion of UART2 baud rate setup
 */
void uart_baud_select(
    uart_baud_rate_t baud_rate,
    uart_clock_speed_t clock_speed, 
    uart_fractional_baud_t *baud_frac,
    uart_mantissa_baud_t *baud_mant);

//=======================================================================================


//=======================================================================================
// Initialization 

//===================================================
// UART Setup Steps 
//  1. Configure the pins 
//     a) Enable the UART Clock - RCC_APB1 register
//     b) Enable GPIOX clock for TX and RX pins - RCC_AHB1 register
//     c) Configure the UART pins for alternative functions - GPIOX_MODER register 
//     d) Set output speed of GPIO pins to high speed - GPIOX_OSPEEDR register 
//     e) Set the alternative function register for USART
// 
//  2. Configure the UART 
//     a) Clear the USART_CR1 register 
//     b) Write the UE bit in the USART_CR1 register to 1
//     c) Program the M bit to define the word length
//     d) Set the baud rate 
//     e) Enable the TX/RX by setting the TE and RE bits in USART_CR1 register 
//     f) Clear buffer 
//===================================================


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

void uart_init(
    USART_TypeDef *uart, 
    uart_baud_rate_t baud_rate,
    uart_clock_speed_t clock_speed)
{
    // UART specific information 
    if (uart == USART1)
    {
        // Enable UART1 Clock - RCC_APB2 register, bit 4
        RCC->APB2ENR |= (SET_BIT << SHIFT_4);

        // Enable GPIOA clock for TX and RX pins - RCC_AHB1 register, bit 0
        RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

        // Configure the UART pins for alternative functions - GPIOA_MODER register 
        GPIOA->MODER |= (SET_2 << SHIFT_18);
        GPIOA->MODER |= (SET_2 << SHIFT_20);

        // Set output speed of GPIO pins to high speed - GPIOA_OSPEEDR register 
        GPIOA->OSPEEDR |= (SET_3 << SHIFT_18);
        GPIOA->OSPEEDR |= (SET_3 << SHIFT_20);

        // Set the alternative function high ([1]) register for USART1 (AF7)
        GPIOA->AFR[1] |= (SET_7 << SHIFT_4); 
        GPIOA->AFR[1] |= (SET_7 << SHIFT_8);
    }
    else if (uart == USART2)
    {
        // Enable UART2 Clock - RCC_APB1 register, bit 17
        RCC->APB1ENR |= (SET_BIT << SHIFT_17);

        // Enable GPIOA clock for TX and RX pins - RCC_AHB1 register, bit 0
        RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

        // Configure the UART pins for alternative functions - GPIOA_MODER register 
        GPIOA->MODER |= (SET_2 << SHIFT_4);
        GPIOA->MODER |= (SET_2 << SHIFT_6);

        // Set output speed of GPIO pins to high speed - GPIOA_OSPEEDR register 
        GPIOA->OSPEEDR |= (SET_3 << SHIFT_4);
        GPIOA->OSPEEDR |= (SET_3 << SHIFT_6);

        // Set the alternative function low ([0]) register for USART2 (AF7)
        GPIOA->AFR[0] |= (SET_7 << SHIFT_8); 
        GPIOA->AFR[0] |= (SET_7 << SHIFT_12);
    }
    else if (uart == USART6)
    {
        // Not currently supported 
    }

    // UART Configuration 
    uart_set_baud_rate(uart, baud_rate, clock_speed); 
}


// Change the baud rate of the UART 
void uart_set_baud_rate(
    USART_TypeDef *uart, 
    uart_baud_rate_t baud_rate,
    uart_clock_speed_t clock_speed)
{
    // Baud rate setup variables 
    uart_fractional_baud_t baud_frac;
    uart_mantissa_baud_t baud_mant;

    // Clear the USART_CR1 register 
    uart->CR1 = CLEAR;
    uart->BRR = CLEAR; 

    // Set the UE bit in the USART_CR1 register 
    uart->CR1 |= (SET_BIT << SHIFT_13);

    // Clear the M bit in the USART_CR1 register for 8-bit data 
    uart->CR1 &= ~(SET_BIT << SHIFT_12);

    // Set the baud rate 
    uart_baud_select(baud_rate, clock_speed, &baud_frac, &baud_mant);
    uart->BRR |= (baud_frac << SHIFT_0);  // Fractional 
    uart->BRR |= (baud_mant << SHIFT_4);  // Mantissa 

    // Enable the TX/RX by setting the RE and TE bits in USART_CR1 register 
    uart->CR1 |= (SET_BIT << SHIFT_2);
    uart->CR1 |= (SET_BIT << SHIFT_3); 

    // Clear buffers  
    while (!(uart->SR & (SET_BIT << SHIFT_6)));
    while(uart->SR & (SET_BIT << SHIFT_5)) 
    {
        uart_getchar(uart);
        tim9_delay_ms(UART_DR_CLEAR_TIMER); 
    }
}


// Select the fractional and mantissa portions of the baud rate setup 
void uart_baud_select(
    uart_baud_rate_t baud_rate,
    uart_clock_speed_t clock_speed, 
    uart_fractional_baud_t *baud_frac,
    uart_mantissa_baud_t *baud_mant)
{
    switch (clock_speed)
    {
        case UART_CLOCK_42:
            switch (baud_rate)
            {
                case UART_BAUD_9600:
                    *baud_frac = UART_42_9600_FRAC;
                    *baud_mant = UART_42_9600_MANT;
                    break;
                
                default:
                    break;
            }
            break;

        case UART_CLOCK_84:
            switch (baud_rate)
            {
                case UART_BAUD_9600:
                    *baud_frac = UART_84_9600_FRAC;
                    *baud_mant = UART_84_9600_MANT;
                    break;
                
                case UART_BAUD_38400:
                    *baud_frac = UART_84_38400_FRAC;
                    *baud_mant = UART_84_38400_MANT;
                    break;
                
                case UART_BAUD_115200:
                    *baud_frac = UART_84_115200_FRAC;
                    *baud_mant = UART_84_115200_MANT;
                    break;
                
                default:
                    break;
            }
            break;
        
        default:
            break;
    }
}

//=======================================================================================


//=======================================================================================
// Send Data 

// UART send character to serial terminal 
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


// UART send string to serial terminal
void uart_sendstring(
    USART_TypeDef *uart, 
    char *string)
{
    // Loop until null character of string is reached. 
    while (*string)
    {
        uart_sendchar(uart, *string);
        string++;
    }
}


// UART send a numeric digit to the serial terminal 
void uart_send_digit(
    USART_TypeDef *uart, 
    uint8_t digit)
{
    // Convert the digit into the ASCII character equivalent 
    uart_sendchar(uart, digit + UART_CHAR_DIGIT_OFFSET);
}


// UART send an integer to the serial terminal 
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

    digit = (uint8_t)((integer / DIVIDE_1) % REMAINDER_10);
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

// UART get character from serial terminal 
uint8_t uart_getchar(USART_TypeDef *uart)
{
    return (uint8_t)(uart->DR);  // Read and return data from data register 
}


// UART get string 
void uart_getstr(
    USART_TypeDef *uart, 
    char *string_to_fill, 
    uart_string_termination_t end_of_string)
{
    // Store the character input from uart_getchar()
    uint8_t input = 0;

    // Run until the end of string character is seen 
    do
    {
        // Wait for data to be available then read and store it 
        if (uart->SR & (SET_BIT << SHIFT_5))
        {
            input = uart_getchar(uart);
            *string_to_fill = input;
            string_to_fill++;
        }
    } 
    while(input != end_of_string);

    // Add a null character to the end of the string 
    *string_to_fill = UART_STR_TERM_NULL;
}

//=======================================================================================


//=======================================================================================
// Misc functions 

// UART clear data register 
void uart_clear_dr(USART_TypeDef *uart)
{
    dummy_read(uart->DR); 
}

//=======================================================================================
