/**
 * @file linked_list_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Linked list driver 
 * 
 * @version 0.1
 * @date 2023-02-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "linked_list_driver.h" 

//=======================================================================================


//=======================================================================================
// Function prototypes 
//=======================================================================================


//=======================================================================================
// Functions 

// Create linked list entry 
void *create_linked_list_entry(
    device_number_t device_num, 
    void **list_ptr, 
    uint32_t record_size)
{
    // Check is the data record already exists 
    linked_list_struct_t *data_record = 
        (linked_list_struct_t *)get_linked_list_entry(device_num, *list_ptr); 

    // If pointer is NULL then create entry, otherwise skip and return the existing entry 
    if (data_record == NULL)
    {
        // Allocate new heap memeory for the list entry 
        data_record = (linked_list_struct_t *)malloc(record_size); 

        // Update the linked list data for the new entry 
        memset((void *)data_record, CLEAR, record_size); 
        data_record->device_num = device_num; 
        data_record->next_ptr = NULL; 

        // Update the previous node pointer to the new node 
        if (*list_ptr == NULL)
        {
            // Update the root pointer if not already done 
            *list_ptr = (void *)data_record; 
        }
        else 
        {
            // Need to traverse to the previous node from the beginning since this is 
            // a singly linked list and out pointer is currently at the new node 
            linked_list_struct_t *previous_node = (linked_list_struct_t *)*list_ptr; 

            while (previous_node->next_ptr != NULL)
            {
                previous_node = previous_node->next_ptr; 
            }

            previous_node->next_ptr = data_record; 
        }
    }

    return (void *)data_record; 
}


// Get linked list entry 
void *get_linked_list_entry(
    device_number_t device_num, 
    void *list_ptr)
{
    // Cast to a common structure type to get the device number 
    linked_list_struct_t *data_record = (linked_list_struct_t *)list_ptr; 

    // Check if the pointer is NULL 
    while (data_record != NULL)
    {
        // Check if the device numbers match 
        if (data_record->device_num == device_num)
        {
            // Device numbers match so we can return the data record pointer 
            break; 
        }

        // Device numbers don't match, check the next entry 
        data_record = (linked_list_struct_t *)data_record->next_ptr; 
    }

    return (void *)data_record; 
}

//=======================================================================================
