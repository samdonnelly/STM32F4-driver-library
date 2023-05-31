/**
 * @file uart_comm.h
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

#ifndef _UART_COMM_H_
#define _UART_COMM_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define UART_DR_CLEAR_TIMER 10    // Timer used while clearing the data register during init 

#define UART_GETSTR_TIMEOUT 10000 // uart_getstr timeout 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief UART baud rate 
 * 
 * @details Passed as an argument to uart_baud_select that allows the user to define 
 *          the baud rate of UART. The baud rates here are defined from 0-X and not 
 *          the actual value of the baud rate. This method is chosen to prevent the need 
 *          to define large numbers for the baud rate. 
 */
typedef enum {
    UART_BAUD_9600,    // 9600 bits/s 
    UART_BAUD_38400,   // 38400 bits/s 
    UART_BAUD_115200   // 115200 bits/s 
} uart_baud_rate_t;


/**
 * @brief UART clock speed 
 * 
 * @details Specifies the speed of the clock for a given UART port. This is used as an 
 *          argument in the uart init function. Specifying the clock speed of the UART 
 *          being initialized helps with correctly setting the baud rate. The clock 
 *          speeds shown below are speeds that have been implemented already, however 
 *          others can be added. 
 * 
 * @see uart_fractional_baud_t
 * @see uart_mantissa_baud_t
 */
typedef enum {
    UART_CLOCK_42,   // APBX clock speed = 42 MHz 
    UART_CLOCK_84    // APBX clock speed = 84 MHz 
} uart_clock_speed_t; 

/**
 * @brief Fractional portion of UART baud rate setup 
 * 
 * @details UART baud rate is a function of a variable called USARTDIV. After picking a 
 *          baud rate, USARTDIV can be calculated as an unsigned fixed point number. This 
 *          value needs to be set in the USART_BRR register in order to set the baud rate
 *          and we define USART_DIV in two parts. The first is the mantissa which defines 
 *          the integer portion of the value and the second is the fraction which defines 
 *          the decimal places. This enum defines the fraction portion for a given UART 
 *          clock speed and baud rate. Refer to the reference manual for more information.
 *          
 *          enum code: UART_(X_1)_(X_2)_FRAC 
 *              X_1: PCLK1 frquency (MHz) 
 *              X_2: baud rate (bps) 
 * 
 * @see uart_mantissa_baud_t 
 */
typedef enum {
    UART_42_9600_FRAC = 0x7,
    UART_84_9600_FRAC = 0xE,
    UART_84_38400_FRAC = 0xB, 
    UART_84_115200_FRAC = 0x9
} uart_fractional_baud_t;


/**
 * @brief Mantissa portion of UART baud rate setup 
 * 
 * @details UART baud rate is a function of a variable called USARTDIV. After picking a 
 *          baud rate, USARTDIV can be calculated as an unsigned fixed point number. This 
 *          value needs to be set in the USART_BRR register in order to set the baud rate
 *          and we define USART_DIV in two parts. The first is the mantissa which defines 
 *          the integer portion of the value and the second is the fraction which defines 
 *          the decimal places. This enum defines the mantissa portion for a given UART 
 *          clock speed and baud rate. Refer to the reference manual for more information.
 *          
 *          enum code: UART_(X_1)_(X_2)_MANT 
 *              X_1: PCLK1 frquency (MHz)    
 *              X_2: Baud rate (bps)         
 * 
 * @see uart_fractional_baud_t
 */
typedef enum {
    UART_42_9600_MANT = 0x111,
    UART_84_9600_MANT = 0x222, 
    UART_84_38400_MANT = 0x88, 
    UART_84_115200_MANT = 0x2D
} uart_mantissa_baud_t;


/**
 * @brief Number of spaces to send over UART 
 * 
 * @details This enum is used when calling uart_send_spaces to specify the number of 
 *          spaces to send. The purpose of this enum is purly for formatting outputs. 
 * 
 * @see uart_send_spaces
 */
typedef enum {
    UART_SPACE_1 = 1,
    UART_SPACE_2,
    UART_SPACE_3
} uart_num_spaces_t;


/**
 * @brief Character offsets to produce numbers on the serial terminal 
 * 
 * @details A byte sent to the serial terminal using UART is interpreted as a 
 *          character by the terminal and not a number. This means to produce a 
 *          number you must send a byte (integer) that corresponds to a number 
 *          character. The following offset numbers convert the byte (integer) that 
 *          is be sent to the terminal into a number character based on the ASCII 
 *          table. 
 * 
 * @see uart_send_digit
 * @see uart_send_integer
 * @see uart_send_spaces
 */
typedef enum {
    UART_CHAR_SPACE_OFFSET = 32,
    UART_CHAR_PLUS_OFFSET  = 43,
    UART_CHAR_MINUS_OFFSET = 45,
    UART_CHAR_DIGIT_OFFSET = 48
} uart_char_offset_t;


/**
 * @brief String formatters for UART
 * 
 * @details These are used in uart_getstr for reading and formatting strings received
 *          from the serial terminal. Within this function the string has been fully 
 *          read once the code sees a carriage return. A null character is added to the 
 *          end to complete the read string. 
 * 
 * @see uart_getstr
 */
typedef enum {
    UART_STR_TERM_NULL = 0,        // '\0' == 0
    UART_STR_TERM_NL = 10,         // '\n' == 10
    UART_STR_TERM_CARRIAGE = 13    // '\r' == 13
} uart_string_termination_t;

//=======================================================================================


//=======================================================================================
// UART Initialization 

/**
 * @brief UART initialization 
 * 
 * @details Initializes the specified UART port so it can be used to send and receive data. 
 *          Currently this function is set up to support UART1 and UART2. UART6 is not yet 
 *          supported. 
 * 
 * @param uart : pointer to the UART port 
 * @param gpio : 
 * @param rx_pin : 
 * @param tx_pin : 
 * @param baud_rate : communication speed of the UART (bps) 
 * @param clock_speed : clock speed of the uart port being initialized 
 */
void uart_init(
    USART_TypeDef *uart, 
    GPIO_TypeDef *gpio, 
    pin_selector_t rx_pin, 
    pin_selector_t tx_pin, 
    uart_baud_rate_t baud_rate, 
    uart_clock_speed_t clock_speed);


/**
 * @brief Set the UART baud rate 
 * 
 * @details Allows for changing the baud rate of the UART port. This is used by the init 
 *          function but can also be called independently if the rate needs to change. 
 * 
 * @param uart : pointer to the UART port 
 * @param baud_rate : communication speed of the UART (bps) 
 * @param clock_speed : clock speed of the uart port being initialized 
 */
void uart_set_baud_rate(
    USART_TypeDef *uart, 
    uart_baud_rate_t baud_rate,
    uart_clock_speed_t clock_speed); 

//=======================================================================================


//=======================================================================================
// Register functions 

/**
 * @brief Check if data is available for reading 
 * 
 * @details Reads the UART status register to check the status of the read data register. 
 *          If new data is available then the function will return true and it indicates 
 *          the data register can be read. 
 * 
 * @param uart : pointer to the UART port 
 * @return uint8_t : read data register status 
 */
uint8_t uart_data_ready(
    USART_TypeDef *uart); 

//=======================================================================================


//=======================================================================================
// Send Functions 

/**
 * @brief UART send character 
 * 
 * @details Takes a single character and writes it to the data register of the specified 
 *          UART. Waits until the Transmission Complete (TC) bit (bit 6) in the status
 *          register (USART_SR) is set before exiting the function.
 * 
 * @param uart : pointer to the UART port 
 * @param character : character written to data register 
 */
void uart_sendchar(
    USART_TypeDef *uart, 
    uint8_t character);


/**
 * @brief UART send string 
 * 
 * @details Sends the characters of a data buffer one at a time until a NUL character is 
 *          seen. Utilizes uart_sendchar to send each character. 
 * 
 * @see uart_sendchar
 * 
 * @param uart : pointer to the UART port 
 * @param string : pointer to buffer containing string to send 
 */
void uart_sendstring(
    USART_TypeDef *uart, 
    char *string);


/**
 * @brief UART send digit 
 * 
 * @details Pass a number from 0-9 to send via UART. The function takes the digit, offsets 
 *          it to the corresponding character (ex. 9 -> '9') and sends it using uart_sendchar. 
 * 
 * @see uart_sendchar
 * 
 * @param uart : pointer to the UART port 
 * @param digit : single numeric digit to send 
 */
void uart_send_digit(
    USART_TypeDef *uart, 
    uint8_t digit);


/**
 * @brief UART send an integer 
 * 
 * @details Takes a signed 16-bit integer, parses the digits (including the integer sign) 
 *          and sends each integer digit using send uart_send_digit. Note that this function 
 *          performs division for each digit so it relatively expensive. 
 * 
 * @see uart_send_digit
 * 
 * @param uart : pointer to the UART port 
 * @param number : signed 16-bit integer to send 
 */
void uart_send_integer(
    USART_TypeDef *uart, 
    int16_t integer);


/**
 * @brief UART send spaces 
 * 
 * @details Sends space characters a number of times defined by num_spaces. This is usedful 
 *          for formatting visual/user outputs. 
 * 
 * @param uart : pointer to the UART port 
 * @param num_spaces : number of blank spaces that get sent to the 
 */
void uart_send_spaces(
    USART_TypeDef *uart, 
    uint8_t num_spaces);


/**
 * @brief UART new line 
 * 
 * @details Sends new line and carriage return characters. This is mainly usedful for when 
 *          the UART is configured for the serial terminal and you want to format the output. 
 * 
 * @param uart : pointer to the UART port 
 */
void uart_send_new_line(
    USART_TypeDef *uart);

//=======================================================================================


//=======================================================================================
// Read Functions

/**
 * @brief UART read character 
 * 
 * @details Read the contents of the UART data register. This is a single byte of data. 
 *          uart_data_ready can be used to check when new data is available to be read. 
 *          Reading the data register will clear the status returned by uart_data_ready. 
 * 
 * @see uart_data_ready
 * 
 * @param uart : pointer to the UART port 
 * @return uint8_t : contents of the data register 
 */
uint8_t uart_getchar(
    USART_TypeDef *uart);


/**
 * @brief UART get string 
 * 
 * @details Read a string of data until the specified termination character is seen. 
 *          uart_getchar is used to read individual characters of the string. Ensure the 
 *          buffer used to store the string is large enough to accomodate the string. 
 *          
 *          If reading from PuTTy, PuTTy will add a carriage return character to the end 
 *          of the string so ensure to set the termination character to "\r". If new data 
 *          isn't seen soon enough or the termination character isn't seen then the 
 *          function will time out and return. 
 * 
 * @see uart_getchar
 * 
 * @param uart : pointer to the UART port 
 * @param string_to_fill : buffer used to store the string input 
 * @param end_of_string : character, that once seen, will end the read sequence 
 */
void uart_getstr(
    USART_TypeDef *uart, 
    char *string_to_fill,
    uart_string_termination_t end_of_string);

//=======================================================================================


//=======================================================================================
// Misc functions 

/**
 * @brief UART clear data register 
 * 
 * @details Clears the data register. This can be used to ensure a false read isn't trigger 
 *          over old data. 
 * 
 * @param uart : pointer to the UART port 
 */
void uart_clear_dr(
    USART_TypeDef *uart); 

//=======================================================================================

#endif  // _UART_COMM_H_
