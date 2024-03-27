/**
 * @file active_object_framework.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Active Object Framework 
 * 
 * @version 0.1
 * @date 2024-03-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "active_object_framework.h" 
#include "tools.h" 

//=======================================================================================


//=======================================================================================
// Functions 

// Common event loop shared by all threads 
void eventLoop(void *thread_info)
{
    ThreadEventData *thread = (ThreadEventData *)thread_info; 

    // Event loop 
    while (1)
    {
        thread->event = CLEAR; 
        xQueueReceive(thread->ThreadEventQueue, (void *)&thread->event, portMAX_DELAY); 
        thread->dispatch(thread->event); 
    }

    vTaskDelete(NULL); 
}

//=======================================================================================
