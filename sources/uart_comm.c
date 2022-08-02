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
 * @see uart2_init
 * @see uart_fractional_baud_t
 * @see uart_mantissa_baud_t
 * 
 * @param baud_rate : (bps) communication speed of UART2
 * @param baud_frac : fractional portion of UART2 baud rate setup 
 * @param baud_mant : mantissa portion of UART2 baud rate setup
 */
void uart_baud_select(
    uint8_t baud_rate,
    uint16_t *baud_frac,
    uint16_t *baud_mant);

//=======================================================================================


//=======================================================================================
// Initialization 

//==============================================================
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
//==============================================================


//===================================================
// UART1 initialization 


void uart1_init(uint8_t baud_rate)
{
    // Baud rate setup variables 
    uint16_t baud_frac;
    uint16_t baud_mant;

    //==============================================================
    // Pin information for UART1
    //  PA9:  TX
    //  PA10: RX
    //==============================================================

    // 1. Pin Setup 

    // a) Enable UART1 Clock - RCC_APB2 register, bit 4
    RCC->APB2ENR |= (SET_BIT << SHIFT_4);

    // b) Enable GPIOA clock for TX and RX pins - RCC_AHB1 register, bit 0
    RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

    // c) Configure the UART pins for alternative functions - GPIOA_MODER register 
    GPIOA->MODER |= (SET_2 << SHIFT_18);
    GPIOA->MODER |= (SET_2 << SHIFT_20);

    // d) Set output speed of GPIO pins to high speed - GPIOA_OSPEEDR register 
    GPIOA->OSPEEDR |= (SET_3 << SHIFT_18);
    GPIOA->OSPEEDR |= (SET_3 << SHIFT_20);

    // e) Set the alternative function high ([1]) register for USART1 (AF7)
    GPIOA->AFR[1] |= (SET_7 << SHIFT_4); 
    GPIOA->AFR[1] |= (SET_7 << SHIFT_8);


    // 2. UART Configuration 

    // a) Clear the USART_CR1 register 
    USART1->CR1 = CLEAR;

    // b) Sef the UE bit in the USART_CR1 register 
    USART1->CR1 |= (SET_BIT << SHIFT_13);

    // c) Clear the M bit in the USART_CR1 register for 8-bit data 
    USART1->CR1 &= ~(SET_BIT << SHIFT_12);

    // d) Set the baud rate 
    uart_baud_select(baud_rate, &baud_frac, &baud_mant);
    USART1->BRR |= (baud_frac << SHIFT_0);  // Fractional 
    USART1->BRR |= (baud_mant << SHIFT_4);  // Mantissa 

    // e) Enable the TX/RX by setting the RE and TE bits in USART_CR1 register 
    USART1->CR1 |= (SET_BIT << SHIFT_2);
    USART1->CR1 |= (SET_BIT << SHIFT_3); 

    // f) Clear buffer  
    while (!(USART1->SR & (SET_BIT << SHIFT_6)));
}

//=================================================== // UART1 initialization 


//===================================================
// UART2 initialization 

void uart2_init(uint8_t baud_rate)
{
    // Baud rate setup variables 
    uint16_t baud_frac;
    uint16_t baud_mant;

    //==============================================================
    // Pin information for UART2
    //  PA2: TX
    //  PA3: RX
    //==============================================================

    // 1. Pin Setup 

    // a) Enable UART2 Clock - RCC_APB1 register, bit 17
    RCC->APB1ENR |= (SET_BIT << SHIFT_17);

    // b) Enable GPIOA clock for TX and RX pins - RCC_AHB1 register, bit 0
    RCC->AHB1ENR |= (SET_BIT << SHIFT_0);

    // c) Configure the UART pins for alternative functions - GPIOA_MODER register 
    GPIOA->MODER |= (SET_2 << SHIFT_4);
    GPIOA->MODER |= (SET_2 << SHIFT_6);

    // d) Set output speed of GPIO pins to high speed - GPIOA_OSPEEDR register 
    GPIOA->OSPEEDR |= (SET_3 << SHIFT_4);
    GPIOA->OSPEEDR |= (SET_3 << SHIFT_6);

    // e) Set the alternative function low ([0]) register for USART2 (AF7)
    GPIOA->AFR[0] |= (SET_7 << SHIFT_8); 
    GPIOA->AFR[0] |= (SET_7 << SHIFT_12);


    // 2. UART Configuration 

    // a) Clear the USART_CR1 register 
    USART2->CR1 = CLEAR;

    // b) Write the UE bit in the USART_CR1 register to 1
    USART2->CR1 |= (SET_BIT << SHIFT_13);

    // c) Program the M bit to define the word length - set to 0 for 8-bit word 
    USART2->CR1 &= ~(SET_BIT << SHIFT_12);

    // d) Set the baud rate 
    uart_baud_select(baud_rate, &baud_frac, &baud_mant);
    USART2->BRR |= (baud_frac << SHIFT_0);  // Fractional 
    USART2->BRR |= (baud_mant << SHIFT_4);  // Mantissa 

    // e) Enable the TX/RX by setting the RE and TE bits in USART_CR1 register 
    USART2->CR1 |= (SET_BIT << SHIFT_2);
    USART2->CR1 |= (SET_BIT << SHIFT_3); 

    // f) Clear buffer  
    while (!(USART2->SR & (SET_BIT << SHIFT_6)));
}

//=================================================== // UART2 initialization


// Select the fractional and mantissa portions of the baud rate setup 
void uart_baud_select(
    uint8_t baud_rate,
    uint16_t *baud_frac,
    uint16_t *baud_mant)
{
    switch (baud_rate)
    {
        case UART_BAUD_9600:
            *baud_frac = UART_42_9600_FRAC;
            *baud_mant = UART_42_9600_MANT;
            break;
        
        case UART_BAUD_115200:
            break;
        
        default:
            break;
    }
}

//=======================================================================================


//=======================================================================================
// Send Data 

//===================================================
// UART1 send 

// UART1 send character to serial terminal 
void uart1_sendchar(uint8_t character)
{
    // Write the data to the data register (USART_DR). 
    USART1->DR = character;

    // Read the Transmission Complete (TC) bit (bit 6) in the status register 
    // (USART_SR) continuously until it is set. 
    while (!(USART1->SR & (SET_BIT << SHIFT_6)));
}


// UART1 send string to serial terminal
void uart1_sendstring(char *string)
{
    // Loop until null character of string is reached. 
    while (*string)
    {
        uart2_sendchar(*string);
        string++;
    }
}


// UART1 send a numeric digit to the serial terminal 
void uart1_send_digit(uint8_t digit)
{
    // Convert the digit into the ASCII character equivalent 
    uart2_sendchar(digit + UART2_CHAR_DIGIT_OFFSET);
}


// UART1 send an integer to the serial terminal 
void uart1_send_integer(int16_t integer)
{
    // Store a digit to print 
    uint8_t digit;

    // Print the sign of the number 
    if (integer < 0)
    {
        // 2's complememt the integer so the correct value is printed
        integer = -(integer);
        uart2_sendchar(UART2_CHAR_MINUS_OFFSET);
    }
    else 
    {
        uart2_sendchar(UART2_CHAR_PLUS_OFFSET);
    }

    // Parse and print each digit
    digit = (uint8_t)((integer / DIVIDE_10000) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_1000) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_100) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_10) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_1) % REMAINDER_10);
    uart2_send_digit(digit);
}

//=================================================== // UART1 send 


//===================================================
// UART2 send 

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
        uart2_sendchar(*string);
        string++;
    }
}


// UART2 send a numeric digit to the serial terminal 
void uart2_send_digit(uint8_t digit)
{
    // Convert the digit into the ASCII character equivalent 
    uart2_sendchar(digit + UART2_CHAR_DIGIT_OFFSET);
}


// UART2 send an integer to the serial terminal 
void uart2_send_integer(int16_t integer)
{
    // Store a digit to print 
    uint8_t digit;

    // Print the sign of the number 
    if (integer < 0)
    {
        // 2's complememt the integer so the correct value is printed
        integer = -(integer);
        uart2_sendchar(UART2_CHAR_MINUS_OFFSET);
    }
    else 
    {
        uart2_sendchar(UART2_CHAR_PLUS_OFFSET);
    }

    // Parse and print each digit
    digit = (uint8_t)((integer / DIVIDE_10000) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_1000) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_100) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_10) % REMAINDER_10);
    uart2_send_digit(digit);

    digit = (uint8_t)((integer / DIVIDE_1) % REMAINDER_10);
    uart2_send_digit(digit);
}


// UART2 send a desired number of spaces to the serial terminal 
void uart2_send_spaces(uint8_t num_spaces)
{
    for (uint8_t i = 0; i < num_spaces; i++)
    {
        uart2_sendchar(UART2_CHAR_SPACE_OFFSET);
    }
}


// UART2 go to a the beginning of a new line in the serial terminal 
void uart2_send_new_line(void)
{
    uart2_sendstring("\r\n");
}

//=================================================== // UART2 send 

//=======================================================================================


//=======================================================================================
// Read Data 

//===================================================
// UART1 read 

// UART1 get character from serial terminal 
uint8_t uart1_getchar(void)
{
    // Read and return data from data register 
    return (uint8_t)(USART1->DR);
}


// UART1 get string from serial terminal
void uart1_getstr(char *string_to_fill)
{
    // Store the character input from uart2_getchar()
    uint8_t input;

    // Run until a carriage return is seen
    do
    {
        // Wait for data to be available then read and store it 
        if (USART1->SR & (SET_BIT << SHIFT_5))
        {
            input = uart2_getchar();
            *string_to_fill = input;
            string_to_fill++;
        }
    } 
    while(input != UART2_STRING_CARRIAGE);

    // Add a null character to the end if the string 
    *string_to_fill = UART2_STRING_NULL;
}

//=================================================== // UART1 read 


//===================================================
// UART2 read 

// UART2 get character from serial terminal 
uint8_t uart2_getchar(void)
{
    // Read and return data from data register 
    return (uint8_t)(USART2->DR);
}


// UART2 get string from serial terminal
void uart2_getstr(char *string_to_fill)
{
    // Store the character input from uart2_getchar()
    uint8_t input;

    // Run until a carriage return is seen
    do
    {
        // Wait for data to be available then read and store it 
        if (USART2->SR & (SET_BIT << SHIFT_5))
        {
            input = uart2_getchar();
            *string_to_fill = input;
            string_to_fill++;
        }
    } 
    while(input != UART2_STRING_CARRIAGE);

    // Add a null character to the end if the string 
    *string_to_fill = UART2_STRING_NULL;
}

//=================================================== // UART2 read 

//=======================================================================================
