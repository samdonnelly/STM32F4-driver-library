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

//=======================================================================================


//=======================================================================================
// Macros 
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
// Function prototypes 

/**
 * @brief Create a linked list entry object 
 * 
 * @param device_num 
 * @param list_ptr 
 * @param record_size 
 * @return void* 
 */
void *create_linked_list_entry(
    device_number_t device_num, 
    void *list_ptr, 
    uint32_t record_size); 

//=======================================================================================

#endif   // _LINKED_LIST_DRIVER_H_ 
