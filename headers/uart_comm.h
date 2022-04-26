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

#define UART_NULL 0       // '\0' == 0
#define UART_CARRIAGE 13  // '\r' == 13

#define UART2_NUM_SIGN_MASK 32768  // 0x8000

#define UART2_2S_COMP_OFFSET 1

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Fraction portion of UART baudrate setup 
 * 
 */
typedef enum {
    USART_42MHZ_9600_FRAC = 7
} usart_fractional_baud_t;


/**
 * @brief Mantissa portion of UART baudrate setup 
 * 
 */
typedef enum {
    USART_42MHZ_9600_MANT = 273
} usart_mantissa_baud_t;


/**
 * @brief 
 * 
 */
typedef enum {
    UART2_1_SPACE  = 1,
    UART2_2_SPACES = 2
} uart2_num_spaces_t;


/**
 * @brief 
 * 
 */
typedef enum {
    UART2_NUM_POSITIVE,
    UART2_NUM_NEGATIVE
} uart2_num_sign_t;



typedef enum {
    UART2_CHAR_SPACE_OFFSET = 32,
    UART2_CHAR_PLUS_OFFSET  = 43,
    UART2_CHAR_MINUS_OFFSET = 45,
    UART2_CHAR_DIGIT_OFFSET = 48
} uart2_char_offset_t;

//=======================================================================================


//=======================================================================================
// UART Initialization 

/**
 * @brief Configure UART2
 * 
 * @details The function is current equiped to enable UART on pins PA2 and PA3. 
 *          Additional functionality can be added later to equip USART if desired.
 *          
 *          Pin information for USART2: 
 *              PA2: TX
 *              PA3: RX
 * 
 */
void uart2_init(void);

//=======================================================================================


//=======================================================================================
// UART2 Send Functions 

/**
 * @brief UART2 send character to serial terminal 
 * 
 * @details Takes a single character and writes it to the data register of USART2. 
 *          Waits until the Transmission Complete (TC) bit (bit 6) in the status
 *          register (USART_SR) is set before exiting the function.
 * 
 * @param character character written to data register 
 */
void uart2_sendchar(uint8_t character);


/**
 * @brief UART2 send string to serial terminal
 * 
 * @details Iterates through a string and calls uart2_sendchar to send characters. 
 * 
 * @see uart2_sendchar
 * 
 * @param string string to send using USART2 
 */
void uart2_sendstring(char *string);


/**
 * @brief 
 * 
 * @details Input a number from 0-9 to print it to the terminal. The function 
 *          takes the digit, offsets it to the corresponding character (ex. 9 -> '9')
 *          and sends it to the serial terminal using uart2_sendchar.
 * 
 * @see uart2_sendchar
 * 
 * @param digit 
 */
void uart2_send_digit(uint8_t digit);


/**
 * @brief 
 * 
 * @details 
 * 
 * @see 
 * 
 * @param number 
 */
void uart2_send_integer(int16_t integer);


/**
 * @brief Print a desired number of blank spaces to the serial terminal 
 * 
 * @details 
 * 
 * @param num_spaces 
 */
void uart2_send_spaces(uint8_t num_spaces);


/**
 * @brief Print a new line on the serial terminal 
 * 
 * @details 
 * 
 */
void uart2_send_new_line(void);

//=======================================================================================


//=======================================================================================
// UART2 Read Functions

/**
 * @brief UART2 get character from serial terminal 
 * 
 * @details Read a character from the UART data register that gets populated from
 *          the serial terminal. Returns a single character. Typically this is 
 *          called from uart2_getstr instead of called directly. 
 * 
 * @see uart2_getstr
 * 
 * @return uint8_t : returns character from data register 
 */
uint8_t uart2_getchar(void);


/**
 * @brief UART2 get string from serial terminal
 * 
 * @details Read a string from the serial terminal using uart2_getchar. The string
 *          read from the terminal is recorded into string_to_fill. Ensure 
 *          string_to_fill is big enough to accomidate the side of string you want 
 *          to read. 
 *          
 *          I'm using PuTTy to send and receive data. When inputing information into 
 *          PuTTy and sending it to the device the final character sent is a carriage
 *          return (\r). This function is only called once there is data available 
 *          to be read, but once there is data the function will continually wait 
 *          for all the data until the full string is read. Once a carriage return 
 *          is seen the function then adds a null termination to the string and returns. 
 * 
 * @see uart2_getchar
 * 
 * @param string_to_fill pointer to string used to store the string input 
 */
void uart2_getstr(char *string_to_fill);

//=======================================================================================


#endif  // _UART_COMM_H_
