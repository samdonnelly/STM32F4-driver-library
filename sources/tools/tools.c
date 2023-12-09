/**
 * @file tools.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief General code tools 
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
    const char *ref_msg, 
    const char *msg, 
    uint8_t msg_start)
{
    uint8_t str_len = strlen(ref_msg); 

    if ((ref_msg == NULL) || (msg == NULL) || (str_len == NONE)) 
    {
        return FALSE; 
    } 

    // Move to the first comparison character 
    msg += msg_start; 

    for (uint8_t i = CLEAR; i < str_len; i++)
    {
        if (*msg++ != *ref_msg++)
        {
            return FALSE; 
        }
    }

    return TRUE; 
}


// Circular buffer parse 
void cb_parse(
    uint8_t *circ_buff, 
    uint8_t *msg_buff, 
    uint8_t *buff_index,
    uint8_t max_buff_size)
{
    // Copy the new contents in the circular buffer to the user input buffer 
    for (uint8_t i = CLEAR; i < max_buff_size; i++)
    {
        // Reset the circular buffer index if needed 
        if (*buff_index >= max_buff_size)
        {
            *buff_index = CLEAR; 
        }

        // Populate the user input buffer - terminate the input at the end 
        if (circ_buff[*buff_index] == CR_CHAR)
        {
            msg_buff[i] = NULL_CHAR; 
            circ_buff[*buff_index] = NULL_CHAR; 
            *buff_index += 1; 
            break; 
        }
        else 
        {
            msg_buff[i] = circ_buff[*buff_index]; 
            *buff_index += 1; 
        }
    }
}


// Character to scaled integer 
uint32_t char_to_int(
    uint8_t num_char, 
    uint8_t place)
{
    return ((uint32_t)(num_char - NUM_TO_CHAR_OFFSET))*((uint32_t)pow(SCALE_10, (double)place)); 
}

//=======================================================================================
