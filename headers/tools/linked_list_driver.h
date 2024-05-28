/**
 * @file linked_list_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Linked list driver interface 
 * 
 * @version 0.1
 * @date 2023-02-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _LINKED_LIST_DRIVER_H_ 
#define _LINKED_LIST_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

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
// Structure 

/**
 * @brief General linked list data structure 
 * 
 * @details This structure allows for different structures to be used with the linked list 
 *          driver. External struct pointers passed to the linked list functions are 
 *          cast with this general purpose struct so that the device number and next node 
 *          pointer can be checked and updated as needed. The only requirement from 
 *          external structs is that their first two entries match the entries in this 
 *          general linked list struct. 
 */
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
 * @details Creates a new linked list entry if it does not already exist. The existance of 
 *          the list entry is checked first using the double pointer to the first entry in 
 *          the list and the device number. If it exists then no new entry is created. 
 *          Otherwise, memory is allocated and a new list entry is made. The pointer to the 
 *          new node is updated and the new node pointer is set to NULL. The device number 
 *          for the node is also assigned so it can be searched for when checking for the 
 *          existance of a device data record. 
 *          
 *          NOTE: A double pointer is needed for list_ptr because we update the value of 
 *                this pointer within the function. In order to update the value of a 
 *                pointer we need to use double pointers. 
 * 
 * @see device_number_t
 * @see linked_list_struct_t
 * @see get_linked_list_entry
 * 
 * @param device_num : number assigned to device data record in the linked list 
 * @param list_ptr : double pointer to the first entry of the data record linked list 
 * @param record_size : size of the struct passed to the function 
 * @return void* : void pointer to data record or NULL if it doesn't exist 
 */
void *create_linked_list_entry(
    device_number_t device_num, 
    void **list_ptr, 
    uint32_t record_size); 


/**
 * @brief Get linked list entry object 
 * 
 * @details This function checks for the existance of a device data record. A pointer to the 
 *          first entry of a linked list is passed to the function and the list is traversed 
 *          while checking the nodes device number against the device_num. If there is a match 
 *          before getting to the end of the list, then a pointer to the data record is 
 *          returned. If there is no match or the first linked list pointer is invalid, then 
 *          a NULL pointer is returned. 
 * 
 * @see device_number_t
 * @see linked_list_struct_t
 * 
 * @param device_num : number assigned to device data record in the linked list 
 * @param list_ptr : pointer to the first entry of the data record linked list 
 * @return void* : void pointer to data record or NULL if it doesn't exist 
 */
void *get_linked_list_entry(
    device_number_t device_num, 
    void *list_ptr); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _LINKED_LIST_DRIVER_H_ 
