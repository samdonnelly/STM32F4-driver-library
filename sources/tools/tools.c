/**
 * @file tools.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief General code tools used across files 
 * 
 * @version 0.1
 * @date 2022-09-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "tools.h"

//=======================================================================================


//=======================================================================================
// String functions 

// String comparison 
uint8_t str_compare(
    char *ref_msg, 
    char *msg, 
    uint8_t msg_start)
{
    // Local variables 
    uint8_t str_length = strlen(ref_msg); 

    // Check message ID 
    msg += msg_start; 
    for (uint8_t i = 0; i < str_length; i++)
    {
        if (*msg++ != *ref_msg++) return FALSE; 
    }

    return TRUE; 
}


// Character to scaled integer 
uint32_t char_to_int(
    uint8_t num_char, 
    uint8_t place)
{
    return ((uint32_t)(num_char - NUM_TO_CHAR_OFFSET))*((uint32_t)pow(SCALE_10, (double)place)); 
}

//=======================================================================================
