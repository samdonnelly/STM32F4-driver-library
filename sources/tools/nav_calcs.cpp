/**
 * @file nav_calcs.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Navigation calculations implementation 
 * 
 * @version 0.1
 * @date 2023-12-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "nav_calcs.h" 
#include "tools.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define EARTH_RADIUS 6371         // Average radius of the Earth (km) 
#define MAX_HEADING_DIFF 1800     // Maximum heading difference (+/- 180 degrees * 10) 
#define HEADING_FULL_RANGE 3600   // Full heading range (360 degrees * 10) 

//=======================================================================================


//=======================================================================================
// Global variables 

const double deg_to_rad = DEG_TO_RAD; 
const double pi_over_2 = PI_OVER_2; 

//=======================================================================================


//=======================================================================================
// Setup and teardown 

// Constructor 
nav_calculations::nav_calculations(
    double radius_lpf_gain, 
    double heading_lpf_gain) 
    : radius_gain(radius_lpf_gain), 
      heading_gain(heading_lpf_gain), 
      true_north_offset(CLEAR) {} 


// Destructor 
nav_calculations::~nav_calculations() {} 

//=======================================================================================


//=======================================================================================
// Calculations 

// GPS radius calculation 
int32_t nav_calculations::gps_radius(
    double lat_cur, 
    double lon_cur, 
    double lat_tar, 
    double lon_tar)
{
    // Local variables 
    double eq1, eq2, eq3, eq4, eq5; 
    static double surf_dist = CLEAR; 

    // Convert coordinates to radians 
    lat_cur *= deg_to_rad; 
    lon_cur *= deg_to_rad; 
    lat_tar *= deg_to_rad; 
    lon_tar *= deg_to_rad; 

    eq1 = cos(pi_over_2 - lat_tar)*sin(lon_tar - lon_cur); 
    eq2 = cos(pi_over_2 - lat_cur)*sin(pi_over_2 - lat_tar); 
    eq3 = sin(pi_over_2 - lat_cur)*cos(pi_over_2 - lat_tar)*cos(lon_tar - lon_cur); 
    eq4 = sin(pi_over_2 - lat_cur)*sin(pi_over_2 - lat_tar); 
    eq5 = cos(pi_over_2 - lat_cur)*cos(pi_over_2 - lat_tar)*cos(lon_tar - lon_cur); 

    // atan2 is used because it produces an angle between +/-180 (pi). The central angle 
    // should always be positive and never greater than 180. 
    // Calculate the radius using a low pass filter to smooth the data. 
    surf_dist += ((atan2(sqrt((eq2 - eq3)*(eq2 - eq3) + (eq1*eq1)), (eq4 + eq5)) * 
                 EARTH_RADIUS*KM_TO_M) - surf_dist)*radius_gain; 

    return (int32_t)(surf_dist*SCALE_10); 
}


// GPS heading calculation 
int16_t nav_calculations::gps_heading(
    double lat_cur, 
    double lon_cur, 
    double lat_tar, 
    double lon_tar)
{
    // Local variables 
    int16_t heading = CLEAR; 
    double num, den; 
    static double heading_temp = CLEAR; 

    // Convert the coordinates to radians and calculate the numerator and denominator 
    // of the initial heading equation. 
    lat_cur *= deg_to_rad; 
    lon_cur *= deg_to_rad; 
    lat_tar *= deg_to_rad; 
    lon_tar *= deg_to_rad; 

    num = cos(lat_tar)*sin(lon_tar-lon_cur); 
    den = cos(lat_cur)*sin(lat_tar) - sin(lat_cur)*cos(lat_tar)*cos(lon_tar-lon_cur); 

    // Calculate the initial heading between coordinates. This calculation comes from the 
    // great-circle navigation equations. A low pass filter is added to smooth the data. 
    heading_temp += (atan(num/den) - heading_temp)*heading_gain; 

    // Convert the calculated heading to degrees*10. 
    heading = (int16_t)(heading_temp*SCALE_10/deg_to_rad); 

    // Correct the calculated heading if needed. atan can produce a heading outside 
    // the needed range (0-359.9 degrees) so this correction brings the value back 
    // within range. 
    if (den < 0)
    {
        heading += MAX_HEADING_DIFF; 
    }
    else if (num < 0)
    {
        heading += HEADING_FULL_RANGE; 
    }

    return heading; 
}


// True North heading 
int16_t nav_calculations::true_north_heading(int16_t heading)
{
    int16_t tn_heading = CLEAR; 

    // Get the magnetometer heading and add the true North correction 
    tn_heading = heading + true_north_offset; 

    // Adjust the true North heading if the corrected headed exceeds heading bounds 
    if (true_north_offset >= 0)
    {
        if (tn_heading >= HEADING_FULL_RANGE)
        {
            tn_heading -= HEADING_FULL_RANGE; 
        }
    }
    else 
    {
        if (tn_heading < 0)
        {
            tn_heading += HEADING_FULL_RANGE; 
        }
    }

    return tn_heading; 
}


// Heading error 
int16_t nav_calculations::heading_error(
    int16_t current_heading, 
    int16_t target_heading)
{
    int16_t heading_error = CLEAR; 

    // Calculate the heading error 
    heading_error = target_heading - current_heading; 

    // Correct the error for when the heading crosses the 0/360 degree boundary 
    if (heading_error > MAX_HEADING_DIFF)
    {
        heading_error -= HEADING_FULL_RANGE; 
    }
    else if (heading_error < -MAX_HEADING_DIFF)
    {
        heading_error += HEADING_FULL_RANGE; 
    }

    return heading_error; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set the True North offset 
void nav_calculations::set_tn_offset(int16_t tn_offset) { true_north_offset = tn_offset; }

//=======================================================================================
