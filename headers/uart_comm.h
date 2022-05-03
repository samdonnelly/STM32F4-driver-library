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
// Enums 

/**
 * @brief UART2 baud rate 
 * 
 * @details 
 * 
 * @see uart2_baud_select
 * 
 */
typedef enum {
    UART2_BAUD_9600,
    UART2_BAUD_115200
} uart2_baud_rate_t;

/**
 * @brief Fraction portion of UART baudrate setup 
 * 
 * @details <br><br>
 *          enum code: UART_(X_1)_(X_2)_FRAC <br>
 *              X_1: PCLK1 frquency (MHz)    <br>
 *              X_2: Baud rate (bps)         <br>
 * 
 * @see uart_mantissa_baud_t 
 * 
 */
typedef enum {
    UART_42_9600_FRAC = 7
} uart_fractional_baud_t;


/**
 * @brief Mantissa portion of UART baudrate setup 
 * 
 * @details <br><br>
 *          enum code: UART_(X_1)_(X_2)_MANT <br>
 *              X_1: PCLK1 frquency (MHz)    <br>
 *              X_2: Baud rate (bps)         <br>
 * 
 * @see uart_fractional_baud_t
 * 
 */
typedef enum {
    UART_42_9600_MANT = 273
} uart_mantissa_baud_t;


/**
 * @brief Number of spaces to send to the serial terminal using UART2
 * 
 * @see uart2_send_spaces
 * 
 */
typedef enum {
    UART2_1_SPACE  = 1,
    UART2_2_SPACES = 2
} uart2_num_spaces_t;


/**
 * @brief Character offsets to produce numbers on the serial terminal 
 * 
 * @details A byte sent to the serial terminal using UART2 is interpreted as a 
 *          character by the terminal and not a number. This means to produce a 
 *          number you must send a byte (integer) that corresponds to a number 
 *          character. The following offset numbers convert the byte that is be 
 *          sent to the terminal into a number character based on the ASCII table. 
 *          <br>
 * 
 * @see uart2_sendchar
 * @see uart2_send_digit
 * @see uart2_send_integer
 * @see uart2_send_spaces
 * 
 */
typedef enum {
    UART2_CHAR_SPACE_OFFSET = 32,
    UART2_CHAR_PLUS_OFFSET  = 43,
    UART2_CHAR_MINUS_OFFSET = 45,
    UART2_CHAR_DIGIT_OFFSET = 48
} uart2_char_offset_t;


/**
 * @brief String formatters for UART2
 * 
 */
typedef enum {
    UART2_STRING_NULL     = 0,  // '\0' == 0
    UART2_STRING_CARRIAGE = 13  // '\r' == 13
} uart2_string_formatters_t;

//=======================================================================================


//=======================================================================================
// UART Initialization 

/**
 * @brief UART2 initialization
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
 * @see uart2_baud_rate_t
 * 
 * @param baud_rate : baud rate to initialize UART2 with - defined by uart2_baud_rate_t
 * 
 */
void uart2_init(uint8_t baud_rate);

//=======================================================================================


//=======================================================================================
// UART2 Send Functions 

/**
 * @brief UART2 send character to serial terminal 
 * 
 * @details Takes a single character and writes it to the data register of UART2. 
 *          Waits until the Transmission Complete (TC) bit (bit 6) in the status
 *          register (USART_SR) is set before exiting the function.
 * 
 * @param character : character written to data register 
 */
void uart2_sendchar(uint8_t character);


/**
 * @brief UART2 send string to serial terminal
 * 
 * @details Iterates through each character of a string and sends each character to the 
 *          serial terminal by calling uart2_sendchar.
 * 
 * @see uart2_sendchar
 * 
 * @param string : string to send using UART2 
 */
void uart2_sendstring(char *string);


/**
 * @brief UART2 send a numeric digit to the serial terminal 
 * 
 * @details Pass a number from 0-9 to print it to the serial terminal. The function 
 *          takes the digit, offsets it to the corresponding character (ex. 9 -> '9')
 *          and sends it to the serial terminal using uart2_sendchar.
 * 
 * @see uart2_sendchar
 * 
 * @param digit : number from 0-9 that gets printed to the serial terminal 
 */
void uart2_send_digit(uint8_t digit);


/**
 * @brief UART2 send an integer to the serial terminal 
 * 
 * @details Takes a signed 16-bit integer, parses the digits and send them to the serial
 *          terminal one at a time using uart2_send_digit along with the correct sign of 
 *          the number. 
 * 
 * @see uart2_send_digit
 * 
 * @param number : signed 16-bit number that gets printed to the serial terminal 
 */
void uart2_send_integer(int16_t integer);


/**
 * @brief UART2 send a desired number of spaces to the serial terminal 
 * 
 * @details Sends a space character to the serial terminal a number of times defined 
 *          by num_spaces. This is usedful for formatting outputs to the serial 
 *          terminal. 
 * 
 * @param num_spaces : number of blank spaces that get sent to the 
 */
void uart2_send_spaces(uint8_t num_spaces);


/**
 * @brief UART2 go to a the beginning of a new line in the serial terminal 
 * 
 * @details When called the serial terminal output will go to the beginning of a new 
 *          line. Useful for formatting outputs to the serial terminal. 
 * 
 */
void uart2_send_new_line(void);

//=======================================================================================


//=======================================================================================
// UART2 Read Functions

/**
 * @brief UART2 get character from serial terminal 
 * 
 * @details Read a byte from the UART2 data register that gets populated by data 
 *          sent from the serial terminal and return the data (single character). 
 * 
 * @return uint8_t : returns character from data register 
 */
uint8_t uart2_getchar(void);


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
 * @param string_to_fill : pointer to string used to store the string input 
 */
void uart2_getstr(char *string_to_fill);

//=======================================================================================


#endif  // _UART_COMM_H_
