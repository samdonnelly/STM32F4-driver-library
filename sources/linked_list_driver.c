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
// - Check for NULL pointer --> If it is NULL then we create the first entry in the list, 
//   if it is not NULL then we create the next entry in the list 
void *create_linked_list_entry(
    device_number_t device_num, 
    void *list_ptr, 
    uint32_t record_size)
{
    // Generic list 
    linked_list_struct_t *data_record = (linked_list_struct_t *)list_ptr; 

    // If pointer is NULL then create entry 
    if (data_record == NULL)
    {
        data_record = malloc(record_size); 

        // Assign a number to the entry 
        data_record->device_num = device_num; 

        // Assign the next entry pointer (there is none) 
        data_record->next_ptr = NULL; 
    }
    // If the pointer is not NULL then create a new entry 
    else 
    {
        // 
    }

    // return (void *)data_record; 

    // Check if the entry already exists 
    // - We do this by calling get_linked_list_entry and checking the return pointer. 
    // - The return pointer will be NULL if it does not exist. 

    // If it exists then return the existing entry 
    // - This is done by casting the return pointer from get_linked_list_entry to void 
    //   and return it. 

    // If it does not exist then allocate memory for the new entry 
    // - This is done using malloc 

    // Assign the pointer of the new entry to be NULL and the device instance to 
    // be the device_num 

    // Return a void pointer (cast to it) to the new entry 
}

// Get linked list entry 
// - Look up the entry based on the device number. Check that the entry exists before 
//   returning anything. 
void *get_linked_list_entry(
    device_number_t device_num, 
    void *list_ptr, 
    uint32_t record_size)
{
    // 
}

//=======================================================================================
