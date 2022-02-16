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

//=======================================================================================


//=======================================================================================
// Enums 

// TODO create a helper header file that contains general purpose enums such as below

/**
 * @brief 
 * 
 */
typedef enum {
    CLEAR_BIT,
    SET_BIT
} bit_setter_t;


/**
 * @brief 
 * 
 */
typedef enum {
    SHIFT_0,
    SHIFT_1,
    SHIFT_2,
    SHIFT_3,
    SHIFT_4,
    SHIFT_5,
    SHIFT_6,
    SHIFT_7,
    SHIFT_8,
    SHIFT_9,
    SHIFT_10,
    SHIFT_11,
    SHIFT_12,
    SHIFT_13,
    SHIFT_14,
    SHIFT_15,
    SHIFT_16,
    SHIFT_17,
    SHIFT_18,
    SHIFT_19,
    SHIFT_20,
    SHIFT_21,
    SHIFT_22,
    SHIFT_23,
    SHIFT_24,
    SHIFT_25,
    SHIFT_26,
    SHIFT_27,
    SHIFT_28,
    SHIFT_29,
    SHIFT_30,
    SHIFT_31,
} bit_shifter_t;


//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief 
 * 
 */
void uart_init(void);

//=======================================================================================


#endif  // _UART_COMM_H_
