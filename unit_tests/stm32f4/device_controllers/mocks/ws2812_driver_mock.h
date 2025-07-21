/**
 * @file ws2812_driver_mock.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 driver mock interface 
 * 
 * @version 0.1
 * @date 2024-04-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _WS2812_DRIVER_MOCK_H_ 
#define _WS2812_DRIVER_MOCK_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "ws2812_driver.h" 

//=======================================================================================


//=======================================================================================
// Mock functions 

/**
 * @brief Initialize driver mock 
 */
void ws2812_mock_init(void); 


/**
 * @brief Get colour data 
 * 
 * @param led_colours 
 */
void ws2812_mock_get_colour_data(uint32_t *led_colours); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _WS2812_DRIVER_MOCK_H_ 
