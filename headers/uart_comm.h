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
//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief Configure USART2
 * 
 * @details The function is current equiped to enable UART on pins PA2 and PA3. 
 *          Additional functionality can be added later to equip USART if desired. 
 * 
 */
void uart2_init(void);


/**
 * @brief USART2 Send Character
 * 
 * @details Takes a single character and writes it to the data register of USART2. 
 *          Waits until the Transmission Complete (TC) bit (bit 6) in the status
 *          register (USART_SR) is set before exiting the function.
 * 
 * @param character : Character written to data register 
 */
void uart2_sendchar(uint8_t character);


/**
 * @brief USART2 Send Character 
 * 
 * @details Iterates through a string and calls uart2_sendchar to send characters. 
 * 
 * @see uart2_sendchar
 * 
 * @param string : string to send using USART2 
 */
void uart2_sendstring(char *string);


/**
 * @brief USART2 Get Character 
 * 
 * @details Retreieve character from an external source. 
 * 
 * @return uint8_t 
 */
uint8_t uart2_getchar(void);

//=======================================================================================


#endif  // _UART_COMM_H_
