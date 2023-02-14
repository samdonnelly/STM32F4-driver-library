/**
 * @file linked_list_driver.h
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

#ifndef _LINKED_LIST_DRIVER_H_ 
#define _LINKED_LIST_DRIVER_H_ 

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h" 

// Libraries 
#include <stdlib.h>

//=======================================================================================


//=======================================================================================
// Macros 

#define NULL_PTR_RETURN 0        // Use for NULL node pointer return 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Device number used to index the linked list data records 
 */
typedef enum {
    DEVICE_ONE = 1, 
    DEVICE_TWO, 
    DEVICE_THREE 
} device_number_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 
//=======================================================================================


//=======================================================================================
// Structure 


typedef struct linked_list_struct_s 
{
    struct linked_list_struct_s *next_ptr; 
    device_number_t device_num; 
}
linked_list_struct_t; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Create a linked list entry object 
 * 
 * @details 
 *          
 *          NOTE: A double pointer is needed for list_ptr because we update the value of 
 *                this pointer within the function. In order to update the value of a 
 *                pointer we need to use double pointers. 
 * 
 * @param device_num 
 * @param list_ptr 
 * @param record_size 
 * @return void* 
 */
void *create_linked_list_entry(
    device_number_t device_num, 
    void **list_ptr, 
    uint32_t record_size); 


/**
 * @brief Get linked list entry 
 * 
 * @details 
 * 
 * @param device_num 
 * @param list_ptr 
 * @return void* 
 */
void *get_linked_list_entry(
    device_number_t device_num, 
    void *list_ptr); 

//=======================================================================================

#endif   // _LINKED_LIST_DRIVER_H_ 
