/**
 * @file tools.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief General code tools used across files 
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
// Macros 

// Different ways to write zero for readability 
#define CLEAR 0
#define RESET_ZERO 0

// Data type 
#define INT16_NUM_DIGITS 5

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief True and False boolean for readability
 * 
 */
typedef enum {
    FALSE,
    TRUE
} boolean_t;


/**
 * @brief Setters 
 * 
 * @details These are mainly used in communication and device drivers during 
 *          initialization in conjunction with bit_shifter_t when setting values 
 *          in registers. However they can be used for setting any value to avoid 
 *          using ghost values. 
 * 
 * @see bit_shifter_t
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
    SET_8,
    SET_9,
    SET_10,
} bit_setter_t;


/**
 * @brief Bit shifters 
 * 
 * @details These are mainly used in communication and device drivers during 
 *          initialization in conjunction with bit_setter_t when setting values 
 *          in registers. However they can be used for any bit shifting operation 
 *          that is needed. 
 * 
 * @see bit_setter_t
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
 * @brief Modulo operator divisors
 * 
 * @details These are used during mathematical remainders (modulo) expressions to divide
 *          a particular number by multiples of 10 and get the remainder. 
 * 
 */
typedef enum {
    REMAINDER_10   = 10,
    REMAINDER_100  = 100,
    REMAINDER_1000 = 1000
} remainder_decimal_place_t;


/**
 * @brief Divisors 
 * 
 * @details Used to divide values by multiples of 10. 
 * 
 */
typedef enum {
    DIVIDE_1     = 1,
    DIVIDE_10    = 10,
    DIVIDE_100   = 100,
    DIVIDE_1000  = 1000,
    DIVIDE_10000 = 10000
} divide_decimal_place_t;

//=======================================================================================

#endif  // _TOOL_H_
