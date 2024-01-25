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

// Coordinates 
#define NUM_TEST_WAYPOINTS 10 
#define NUM_GLOBAL_TEST_WAYPOINTS 6 

// Coordinate info 
#define NUM_TEST_INFO 42 
#define NUM_GLOBAL_TEST_INFO 30 

//=======================================================================================


//=======================================================================================
// Structs 

// Radius and heading info between coordinates 
typedef struct inter_coordinate_info_s 
{
    int32_t radius;    // units: meters*10 
    int16_t heading;   // units: degrees*10 
}
inter_coordinate_info_t; 

//=======================================================================================


//=======================================================================================
// Coordinates 

extern const gps_waypoints_t waypoints_test[NUM_TEST_WAYPOINTS]; 

//=======================================================================================


//=======================================================================================
// Coordinate info 

extern const inter_coordinate_info_t waypoint_info_test[NUM_TEST_INFO]; 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _GPS_COORDINATES_TEST_H_
