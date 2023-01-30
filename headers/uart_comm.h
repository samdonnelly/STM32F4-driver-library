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
 *          the baud rate of UART2. The baud rates here are defined from 0-X and not 
 *          the actual value of the baud rate. This method is chosen to prevent the need 
 *          to define large numbers for the baud rate. 
 * 
 */
typedef enum {
    UART_BAUD_9600,    // 9600 bits/s 
    UART_BAUD_38400,   // 38400 bits/s 
    UART_BAUD_115200   // 115200 bits/s 
} uart_baud_rate_t;


/**
 * @brief UART clock speed 
 * 
 * @details 
 * 
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
 *          <br><br>
 *          
 *          enum code: UART_(X_1)_(X_2)_FRAC <br>
 *              X_1: PCLK1 frquency (MHz)    <br>
 *              X_2: Baud rate (bps)         <br>
 * 
 * @see uart_mantissa_baud_t 
 * 
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
 *          <br><br>
 *          
 *          enum code: UART_(X_1)_(X_2)_MANT <br>
 *              X_1: PCLK1 frquency (MHz)    <br>
 *              X_2: Baud rate (bps)         <br>
 * 
 * @see uart_fractional_baud_t
 * 
 */
typedef enum {
    UART_42_9600_MANT = 0x111,
    UART_84_9600_MANT = 0x222, 
    UART_84_38400_MANT = 0x88, 
    UART_84_115200_MANT = 0x2D
} uart_mantissa_baud_t;


/**
 * @brief Number of spaces to send to the serial terminal using UART2
 * 
 * @details This enum is used when calling uart2_send_spaces to specify the number of 
 *          spaces to print to ther serial terminal. The purpose of the function and 
 *          this enum is primarily for formatting the serial terminal output. 
 * 
 * @see uart2_send_spaces
 * 
 */
typedef enum {
    UART2_1_SPACE  = 1,
    UART2_2_SPACES = 2,
    UART2_3_SPACES = 3
} uart2_num_spaces_t;


/**
 * @brief Character offsets to produce numbers on the serial terminal 
 * 
 * @details A byte sent to the serial terminal using UART2 is interpreted as a 
 *          character by the terminal and not a number. This means to produce a 
 *          number you must send a byte (integer) that corresponds to a number 
 *          character. The following offset numbers convert the byte (integer) that 
 *          is be sent to the terminal into a number character based on the ASCII 
 *          table. <br>
 * 
 * @see uart2_send_digit
 * @see uart2_send_integer
 * @see uart2_send_spaces
 * 
 */
typedef enum {
    UART_CHAR_SPACE_OFFSET = 32,
    UART_CHAR_PLUS_OFFSET  = 43,
    UART_CHAR_MINUS_OFFSET = 45,
    UART_CHAR_DIGIT_OFFSET = 48
} uart2_char_offset_t;


/**
 * @brief String formatters for UART2
 * 
 * @details These are used in uart2_getstr for reading and formatting strings received
 *          from the serial terminal. Within this function the string has been fully 
 *          read once the code sees a carriage return. A null character is added to the 
 *          end to complete the read string. 
 * 
 * @see uart2_getstr
 * 
 */
typedef enum {
    UART_STR_TERM_NULL     = 0,  // '\0' == 0
    UART_STR_TERM_NL       = 10, // '\n' == 10
    UART_STR_TERM_CARRIAGE = 13  // '\r' == 13
} uart_string_termination_t;

//=======================================================================================


//=======================================================================================
// UART Initialization 

/**
 * @brief UART1 initialization 
 * 
 * @details Configures UART2 which is connected to the serial port of the dev board. 
 *          This allows for communication with the serial terminal. Additional
 *          functionality can be added later to equip USART if desired. <br><br>
 *          
 *          Takes the baud_rate as a parameter do define the communication speed of 
 *          UART2. <br><br>
 *          
 *          Pin information for UART2:  <br>
 *              PA2: TX                 <br>
 *              PA3: RX                 <br>
 * 
 * @param uart : pointer to the UART port 
 * @param baud_rate : 
 * @param clock_speed : 
 */
void uart_init(
    USART_TypeDef *uart, 
    uart_baud_rate_t baud_rate, 
    uart_clock_speed_t clock_speed);


/**
 * @brief Set the UART baud rate 
 * 
 * @details 
 * 
 * @param baud_rate : 
 * @param uart : 
 * @param clock_speed : 
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
 * @details 
 * 
 * @param uart : pointer to the UART port used 
 * @return uint8_t : Status of available data 
 */
uint8_t uart_data_ready(
    USART_TypeDef *uart); 

//=======================================================================================


//=======================================================================================
// Send Functions 

/**
 * @brief 
 * 
 * @details Takes a single character and writes it to the data register of UART2. 
 *          Waits until the Transmission Complete (TC) bit (bit 6) in the status
 *          register (USART_SR) is set before exiting the function.
 * 
 * @param character : character written to data register 
 */
void uart_sendchar(
    USART_TypeDef *uart, 
    uint8_t character);


/**
 * @brief 
 * 
 * @details Iterates through each character of a string and sends each character to the 
 *          serial terminal by calling uart2_sendchar.
 * 
 * @param string : string to send using UART2 
 */
void uart_sendstring(
    USART_TypeDef *uart, 
    char *string);


/**
 * @brief 
 * 
 * @details Pass a number from 0-9 to print it to the serial terminal. The function 
 *          takes the digit, offsets it to the corresponding character (ex. 9 -> '9')
 *          and sends it to the serial terminal using uart2_sendchar.
 * 
 * @param digit : number from 0-9 that gets printed to the serial terminal 
 */
void uart_send_digit(
    USART_TypeDef *uart, 
    uint8_t digit);


/**
 * @brief 
 * 
 * @details Takes a signed 16-bit integer, parses the digits and send them to the serial
 *          terminal one at a time using uart2_send_digit along with the correct sign of 
 *          the number. 
 * 
 * @param number : signed 16-bit number that gets printed to the serial terminal 
 */
void uart_send_integer(
    USART_TypeDef *uart, 
    int16_t integer);


/**
 * @brief UART2 send a desired number of spaces to the serial terminal 
 * 
 * @details Sends a space character to the serial terminal a number of times defined 
 *          by num_spaces. This is usedful for formatting outputs to the serial 
 *          terminal. 
 * 
 * @param uart 
 * @param num_spaces : number of blank spaces that get sent to the 
 */
void uart_send_spaces(
    USART_TypeDef *uart, 
    uint8_t num_spaces);


/**
 * @brief UART2 go to a the beginning of a new line in the serial terminal 
 * 
 * @details When called the serial terminal output will go to the beginning of a new 
 *          line. Useful for formatting outputs to the serial terminal. 
 * 
 * @param uart 
 */
void uart_send_new_line(USART_TypeDef *uart);

//=======================================================================================


//=======================================================================================
// Read Functions

/**
 * @brief UART2 get character from serial terminal 
 * 
 * @details Read a byte from the UART2 data register that gets populated by data 
 *          sent from the serial terminal and return the data (single character). 
 * 
 * @param uart 
 * @return uint8_t : returns character from data register 
 */
uint8_t uart_getchar(USART_TypeDef *uart);


/**
 * @brief UART2 get string from serial terminal
 * 
 * @details Read a string from the serial terminal one character at a time by repeatedly
 *          calling uart2_getchar. The string data gets stored into string_to_fill which 
 *          is a char pointer passed to the function. Ensure string_to_fill is big enough
 *          to accomidate the size of string you want to read. <br><br>
 *          
 *          PuTTy was used to test sending and receiving data. When inputing information 
 *          into PuTTy and sending it to the device the final character sent is a
 *          carriage return (\r). This function is only called once there is data
 *          available to be read, but once there is data the function will continually
 *          wait for all the data until the full string is read. Once a carriage return
 *          is seen the function then adds a null termination to the string and returns.
 * 
 * @see uart2_getchar
 * 
 * @param uart 
 * @param string_to_fill : pointer to string used to store the string input 
 * @param end_of_string 
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
 * @details 
 * 
 * @param uart 
 */
void uart_clear_dr(USART_TypeDef *uart); 

//=======================================================================================

#endif  // _UART_COMM_H_
