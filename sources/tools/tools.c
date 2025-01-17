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


// Circular buffer parse 
void cb_parse_v2(
    const uint8_t *circular_buff, 
    cb_index_t *cb_index, 
    uint8_t *data_buff)
{
    if ((circular_buff == NULL) || 
        (cb_index == NULL) || 
        (data_buff == NULL) || 
        (cb_index->head > cb_index->cb_size) || 
        (cb_index->tail > cb_index->cb_size))
    {
        return; 
    }

    // Copy the contents of the circular buffer from tail to head into the data buffer. 
    // Once all data within range has been copied or the data buffer has filled up then 
    // stop copying data. If all data gets copied then terminate the data buffer, 
    // otherwise make sure tail matches head so old data is not copied next time. 

    while ((cb_index->tail != cb_index->head) && (data_buff != NULL))
    {
        if (cb_index->tail >= cb_index->cb_size)
        {
            cb_index->tail = CLEAR; 
        }

        *data_buff++ = circular_buff[cb_index->tail++]; 
    }

    if (data_buff != NULL)
    {
        // Tail successfully reached head. 
        *data_buff = NULL_CHAR; 
    }
    else 
    {
        // Data buffer filled up and the index needs to be manually adjusted. 
        cb_index->tail = cb_index->head; 

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
