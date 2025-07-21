/**
 * @file timer_mock.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Mock timers driver interface - for unit testing 
 * 
 * @version 0.1
 * @date 2024-01-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _TIMERS_MOCK_H_
#define _TIMERS_MOCK_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Mock functions 

// Set the timer comparison return value 
void tim_mock_set_compare_state(
    uint8_t tim_compare_status); 

//=======================================================================================

#endif   // _TIMERS_MOCK_H_
