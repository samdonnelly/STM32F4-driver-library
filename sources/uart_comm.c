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
// Initialization 

// UART2 setup 
void uart2_init(void)
{
    //==============================================================
    // Pin information for I2C1
    //  PA2: TX
    //  PA3: RX
    //==============================================================

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
    USART2->CR1 = 0x00;  // CLEAR;

    // Write the UE bit in the USART_CR1 register to 1
    USART2->CR1 |= (SET_BIT << SHIFT_13);

    // Program the M bit to define the word length
    USART2->CR1 &= ~(SET_BIT << SHIFT_12);  //  Set to zero for 8 bit word length 

    // Set the baud rate 
    USART2->BRR |= (USART_42MHZ_9600_FRAC << SHIFT_0);  // Fractional 
    USART2->BRR |= (USART_42MHZ_9600_MANT << SHIFT_4);  // Mantissa 

    // Enable the TX/RX by setting the TE and RE bits in USART_CR1 register 
    USART2->CR1 |= (SET_BIT << SHIFT_2);  // Set RE
    USART2->CR1 |= (SET_BIT << SHIFT_3);  // Set TE

    // Clear buffer 

    // Read the Transmission Complete (TC) bit (bit 6) in the status register 
    // (USART_SR) continuously until it is set. If this is not done then some
    // characters can be skipped in transmission on reset. 
    while (!(USART2->SR & (SET_BIT << SHIFT_6)));
}

//=======================================================================================


//=======================================================================================
// UART2 Send

// UART2 send character to serial terminal 
void uart2_sendchar(uint8_t character)
{
    // Write the data to the data register (USART_DR). 
    USART2->DR = character;

    // Read the Transmission Complete (TC) bit (bit 6) in the status register 
    // (USART_SR) continuously until it is set. 
    while (!(USART2->SR & (SET_BIT << SHIFT_6)));
}


// UART2 send string to serial terminal
void uart2_sendstring(char *string)
{
    // Loop until null character of string is reached. 
    while (*string)
    {
        // Send characters one at a time. 
        uart2_sendchar(*string);

        // Increment to next character. 
        string++;
    }
}


// 
void uart2_send_digit(uint8_t digit)
{
    uart2_sendchar(digit + UART2_CHAR_DIGIT_OFFSET);
}


// 
void uart2_send_integer(int16_t integer)
{
    // Print sign 
    integer = uart2_send_sign(integer);

    // separate digits
}


// 
int16_t uart2_send_sign(int16_t integer)
{
    // Get sign 
    uint8_t sign = (uint8_t)((integer & UART2_NUM_SIGN_MASK) >> SHIFT_15);

    // Print sign 
    switch(sign)
    {
        case UART2_NUM_POSITIVE:
            uart2_sendchar(sign + UART2_CHAR_PLUS_OFFSET);
            break;
        case UART2_NUM_NEGATIVE: 
            uart2_sendchar(sign + UART2_CHAR_MINUS_OFFSET);
            integer = ~(integer) + UART2_2S_COMP_OFFSET;    // Eliminate negative
            break;
        default:
            break;
    }

    return integer;
}


// Print a desired number of spaces 
void uart2_send_spaces(uint8_t num_spaces)
{
    for (uint8_t i = 0; i < num_spaces; i++)
    {
        uart2_sendchar(" ");
    }
}


// Go to a new line in the serial terminal 
void uart2_send_new_line(void)
{
    uart2_sendstring("\r\n");
}

//=======================================================================================


//=======================================================================================
// UART2 read

// UART2 get character from serial terminal 
uint8_t uart2_getchar(void)
{
    // Store the character input from the data register
    static uint8_t input = 0;

    // Read data from data register 
    input = USART2->DR;

    return input;
}


// UART2 get string from serial terminal
void uart2_getstr(char *string_to_fill)
{
    // Store the character input from uart2_getchar()
    static uint8_t input = 0;

    // Run until a carriage return is seen
    do
    {
        // Wait for data to be available
        if (USART2->SR & (SET_BIT << SHIFT_5))
        {
            input = uart2_getchar();

            // Record character and increment to the next space in memeory
            *string_to_fill = input;
            string_to_fill++;
        }
    } 
    while(input != UART_CARRIAGE);

    // Add a null character to the end if the string 
    *string_to_fill = UART_NULL;
}

//=======================================================================================
