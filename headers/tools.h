/**
 * @file tools.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2022-02-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TOOLS_H_
#define _TOOLS_H_

//=======================================================================================
// Includes 
//=======================================================================================


//=======================================================================================
// Macros 

// Different ways to write zero for readability 
#define CLEAR 0
#define RESET 0

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief 
 * 
 */
typedef enum {
    CLEAR_BIT,
    SET_BIT,
    SET_2,
    SET_3,
    SET_4,
    SET_5,
    SET_6,
    SET_7,
    SET_8
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


/**
 * @brief 
 * 
 */
typedef enum {
    USART_50MHZ_115200_FRAC = 2,
    USART_50MHZ_9600_FRAC = 8
} usart_fractional_baud_t;


/**
 * @brief 
 * 
 */
typedef enum {
    USART_50MHZ_115200_MANT = 27,
    USART_50MHZ_9600_MANT = 325
} usart_mantissa_baud_t;

//=======================================================================================


//=======================================================================================
// Structures 
//=======================================================================================


#endif  // _TOOL_H_