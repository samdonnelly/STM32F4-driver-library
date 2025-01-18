/**
 * @file tools.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief General code tools 
 * 
 * @version 0.1
 * @date 2022-02-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TOOLS_H_ 
#define _TOOLS_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f4x.h" 

// Standard libraries 
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h> 
#include <string.h> 
#include <math.h> 

//=======================================================================================


//=======================================================================================
// Macros 

// Data type 
#define INT16_NUM_DIGITS 5

//==================================================
// Masks 

// AND filters 
#define FILTER_1_LSB 0x01
#define FILTER_2_LSB 0x03
#define FILTER_3_LSB 0x07
#define FILTER_4_LSB 0x0F
#define FILTER_5_LSB 0x1F
#define FILTER_6_LSB 0x3F
#define FILTER_7_LSB 0x7F

#define FILTER_1_MSB 0x80
#define FILTER_2_MSB 0xC0
#define FILTER_3_MSB 0xE0
#define FILTER_4_MSB 0xF0
#define FILTER_5_MSB 0xF8
#define FILTER_6_MSB 0xFC
#define FILTER_7_MSB 0xFE

#define FILTER_BIT_0 0x01 
#define FILTER_BIT_1 0x02 
#define FILTER_BIT_2 0x04 
#define FILTER_BIT_3 0x08 
#define FILTER_BIT_4 0x10 
#define FILTER_BIT_5 0x20 
#define FILTER_BIT_6 0x40 
#define FILTER_BIT_7 0x80 

// Alternate function configuration 
#define AFR_INDEX_PIN_MASK 0x08        // Distinguishes pins 0-7 from pins 8-15 

//==================================================

//==================================================
// Logic setters 

// Logic low - different ways to write zero for readability 
#define CLEAR 0
#define RESET_ZERO 0
#define NONE 0 
#define ZERO 0 

// Logic high 
#define HIGH_8BIT 0xFF 
#define HIGH_16BIT 0xFFFF 
#define HIGH_32BIT 0xFFFFFFFF 

//==================================================

//==================================================
// Characters 

// String formatting 
#define NULL_CHAR       0    // '\0' 
#define NL_CHAR         10   // '\n' 
#define CR_CHAR         13   // '\r' 
#define SPACE_CHAR      32   // ' ' 
#define AST_CHAR        42   // '*' 
#define COMMA_CHAR      44   // ',' 
#define PLUS_CHAR       43   // '+' 
#define MINUS_CHAR      45   // '-' 
#define PERIOD_CHAR     46   // '.' 
#define ZERO_CHAR       48   // '0' 
#define ONE_CHAR        49   // '1' 
#define NINE_CHAR       57   // '9' 
#define A_UP_CHAR       65   // 'A' 
#define E_UP_CHAR       69   // 'E' 
#define F_UP_CHAR       70   // 'F' 
#define N_UP_CHAR       78   // 'N' 
#define S_UP_CHAR       83   // 'S' 
#define W_UP_CHAR       87   // 'W' 
#define Z_UP_CHAR       90   // 'Z' 
#define UNDERSCORE_CHAR 95   // '_' 
#define A_LO_CHAR       97   // 'a' 
#define Z_LO_CHAR       122  // 'z' 

// ASCII formatters 
#define MAX_CHAR_DIGIT 0x09      // Max single digit 
#define NUM_TO_CHAR_OFFSET 0x30  // Hex number (0x0-0x9) to char number ('0'-'9') offset 
#define HEX_TO_LET_CHAR 0x37     // Hex letter (0xA-0xF) to char letter ('A'-'F') offset 

//==================================================

//==================================================
// Scaling 

#define SCALE_10 10 
#define SCALE_100 100 
#define SCALE_1000 1000 
#define SCALE_10000 10000 
#define SCALE_1E5 100000 
#define SCALE_1E6 1000000 

// Modulo operator divisors
#define REMAINDER_10   10
#define REMAINDER_100  100
#define REMAINDER_1000 1000

// Divisors 
#define DIVIDE_10    10
#define DIVIDE_100   100
#define DIVIDE_1000  1000
#define DIVIDE_10000 10000 

//==================================================

//==================================================
// Math 

#define PI 3.141592                    // PI 
#define RAD_TO_DEG 180.0 / PI          // Radians to degrees conversion 
#define DEG_TO_RAD PI / 180.0          // Degrees to radians conversion 

//==================================================

//==================================================
// Physics 

#define KM_TO_M 1000    // kilometers to meters 
#define MIN_TO_DEG 60   // Coordinate minutes to degrees conversion 

//==================================================

//=======================================================================================


//=======================================================================================
// Macro functions 

#define dummy_read(x) ((void)x)  // For clearing registers 

//=======================================================================================


//=======================================================================================
// Enums 

// True and False boolean 
typedef enum {
    FALSE,
    TRUE
} boolean_t;


// General purpose pin selector 
typedef enum {
    PIN_0,
    PIN_1,
    PIN_2,
    PIN_3,
    PIN_4,
    PIN_5,
    PIN_6,
    PIN_7,
    PIN_8,
    PIN_9,
    PIN_10,
    PIN_11,
    PIN_12,
    PIN_13,
    PIN_14,
    PIN_15
} pin_selector_t;


/**
 * @brief Setters 
 * 
 * @details These are mainly used in communication and device drivers during 
 *          initialization in conjunction with bit_shifter_t when setting values 
 *          in registers. However they can be used for setting any value to avoid 
 *          using ghost values. 
 * 
 * @see bit_shifter_t
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
    SET_11,
    SET_12,
    SET_13,
    SET_14,
    SET_15,
    SET_16,
    SET_17,
    SET_18,
    SET_19,
    SET_20,
    SET_21,
    SET_22,
    SET_23,
    SET_24,
    SET_25,
    SET_26,
    SET_27,
    SET_28,
    SET_29,
    SET_30,
    SET_31 
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
    SHIFT_31 
} bit_shifter_t;


// General purpose byte counter for specifying the byte within a data structure 
typedef enum {
    BYTE_0,
    BYTE_1,
    BYTE_2,
    BYTE_3,
    BYTE_4,
    BYTE_5,
    BYTE_6,
    BYTE_7,
    BYTE_8,
    BYTE_9,
    BYTE_10,
    BYTE_11, 
    BYTE_12, 
    BYTE_13, 
    BYTE_14, 
    BYTE_15,
    BYTE_16, 
    BYTE_17 
} byte_num_t;

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Circular buffer indexing 
 * 
 * @details Indexing info used to keep track of data added to a circular buffer. This 
 *          does not provide a circular buffer or a parsed data buffer because their size 
 *          is dependent on the application. The application must provide them as needed. 
 */
typedef struct cb_index_s
{
    uint8_t cb_size;   // Size of the circular buffer 
    uint8_t head;      // Index to read up to in the circular buffer 
    uint8_t tail;      // Index where the new data in the circular buffer begins 
}
cb_index_t; 

//=======================================================================================


//=======================================================================================
// String functions 

/**
 * @brief String comparison 
 * 
 * @details Compares two strings to see if they match. "msg" gets compared to "ref_msg" 
 *          starting at "msg_start" (index of "msg" to start at) up until the length of 
 *          "ref_msg". Each string character of interest is checked individually. If along 
 *          the way there is a mismatch then false is returned. If there are no mismatches 
 *          then the function returns true. 
 * 
 *          NOTE: If the reference message matches the section of the main message being 
 *                compared, but the main message has mismatching characters before or after 
 *                the section being compared, the function will still return true. 
 * 
 * @param ref_msg : reference message to compare against 
 * @param msg : message of interest to compare to the reference message 
 * @param msg_start : index of "msg" from where to start the comparison 
 * @return uint8_t : result of the comparison - true if there is a match, false otherwise 
 */
uint8_t str_compare(
    const char *ref_msg, 
    const char *msg, 
    uint8_t msg_start); 


/**
 * @brief Circular buffer parse 
 * 
 * @details Copies the contents of the provided circular buffer from index "tail" to 
 *          index "head" into the provided data buffer. The head and tail indexes must 
 *          be within range of the circular buffer size or else no data will be copied. 
 *          The tail index will be updated to match the head index once data is done 
 *          being copied. It's the responsibility of the user to maintain the value of 
 *          the tail index between function calls and to provide an accurate head index 
 *          based on the data received in the circular buffer by the application. It's 
 *          also the users responsibility to provide a data buffer that is large enough 
 *          to handle whatever data the circular buffer contains (i.e the data buffer 
 *          should be at least as large as the circular buffer size). If the data buffer 
 *          is too small then the data buffer will be filled and the remaining data will 
 *          be ignored and lost. If data received into the circular buffer is greater 
 *          than the circular buffer size, data will be overwritten and this function 
 *          can't work properly. 
 * 
 * @see cb_index_t 
 * 
 * @param circular_buff : circular buffer containing data to parse 
 * @param cb_index : circular buffer indexing info - see cb_index_t 
 * @param data_buff : buffer to store parsed circular buffer data bewteen tail and head 
 */
void cb_parse(
    const uint8_t *circular_buff, 
    cb_index_t *cb_index, 
    uint8_t *data_buff); 


/**
 * @brief Character to scaled integer 
 * 
 * @details Takes a single character ('0' to '9') and converts it to its equivalent 
 *          integer value at whatever digit place the character is in. 
 * 
 * @param num_char : character that represents a number 
 * @param place : digit place of of character 
 * @return uint32_t : converted integer 
 */
uint32_t char_to_int(
    uint8_t num_char, 
    uint8_t place); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _TOOLS_H_ 
