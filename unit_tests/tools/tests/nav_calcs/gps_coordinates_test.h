/**
 * @file gps_coordinates_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPS coordinates test file interface 
 * 
 * @version 0.1
 * @date 2024-01-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _GPS_COORDINATES_TEST_H_ 
#define _GPS_COORDINATES_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "nav_calcs.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define NUM_TEST_WAYPOINTS 10 

//=======================================================================================


//=======================================================================================
// Coordinates 

extern const gps_waypoints_t waypoints_test[NUM_TEST_WAYPOINTS]; 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _GPS_COORDINATES_TEST_H_
