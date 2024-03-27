/**
 * @file active_object_framework.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Active Object Framework interface 
 * 
 * @version 0.1
 * @date 2024-03-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _ACTIVE_OBJECT_FRAMEWORK_H_ 
#define _ACTIVE_OBJECT_FRAMEWORK_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f411xe.h" 

#include "FreeRTOS.h" 
#include "cmsis_os2.h" 
#include "queue.h" 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef uint8_t Event; 

//=======================================================================================


//=======================================================================================
// Data 

// Thread Event Info 
typedef struct 
{
    osThreadAttr_t attr;              // Thread attributes 
    uint8_t event;                    // Event index 
    QueueHandle_t ThreadEventQueue;   // Queue 
    void (*dispatch)(Event event);    // Dispatch function 
} 
ThreadEventData; 

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief Common event loop task function for all threads (FreeRTOS format) 
 * 
 * @param thread_info : void pointer to a 'ThreadEventData' object (see above) 
 */
void eventLoop(void *thread_info); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _ACTIVE_OBJECT_FRAMEWORK_H_ 
