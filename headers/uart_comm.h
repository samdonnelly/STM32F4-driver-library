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
 * @brief Send data over UART2
 * 
 */
void uart2_tx(void);


/**
 * @brief Receive data over UART2 
 * 
 */
void uart2_rx(void);

//=======================================================================================


#endif  // _UART_COMM_H_
