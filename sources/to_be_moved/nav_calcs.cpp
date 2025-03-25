/**
 * @file nav_calcs.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Navigation calculations 
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

//=======================================================================================


//=======================================================================================
// Macros 

#define EARTH_RADIUS 6371         // Average radius of the Earth (km) 
#define MAX_HEADING_DIFF 1800     // Maximum heading difference (+/- 180 degrees * 10) 
#define HEADING_FULL_RANGE 3600   // Full heading range (360 degrees * 10) 

//=======================================================================================


//=======================================================================================
// Global variables 

constexpr double deg_to_rad = DEG_TO_RAD; 
constexpr double rad_to_scaled_deg = RAD_TO_DEG*SCALE_10; 

//=======================================================================================


//=======================================================================================
// Setup and teardown 

// Constructor - Default
nav_calculations::nav_calculations() 
    : coordinate_lpf_gain(CLEAR), 
      true_north_offset(CLEAR) {} 


// Constructor - Specify filter gain 
nav_calculations::nav_calculations(double coordinate_gain) 
    : coordinate_lpf_gain(coordinate_gain), 
      true_north_offset(CLEAR) {} 


// Constructor - Specify true north correction offset 
nav_calculations::nav_calculations(int16_t tn_offset)
    : coordinate_lpf_gain(CLEAR), 
      true_north_offset(tn_offset) {} 


// Constructor - Specify filter gain and true north correction offset 
nav_calculations::nav_calculations(
    double coordinate_gain, 
    int16_t tn_offset)
    : coordinate_lpf_gain(coordinate_gain), 
      true_north_offset(tn_offset) {} 


// Destructor 
nav_calculations::~nav_calculations() {} 

//=======================================================================================


//=======================================================================================
// Calculations 

// Coordinate filter 
void nav_calculations::coordinate_filter(
    gps_waypoints_t new_data, 
    gps_waypoints_t& filtered_data) const
{
    filtered_data.lat += (new_data.lat - filtered_data.lat)*coordinate_lpf_gain; 
    filtered_data.lon += (new_data.lon - filtered_data.lon)*coordinate_lpf_gain; 
}


// GPS radius calculation 
int32_t nav_calculations::gps_radius(
    gps_waypoints_t current, 
    gps_waypoints_t target)
{
    double eq0, eq1, eq2, eq3, eq4, eq5, eq6, eq7, surf_dist; 

    // Convert the coordinates to radians so they're compatible with the math library. 
    current.lat *= deg_to_rad; 
    current.lon *= deg_to_rad; 
    target.lat *= deg_to_rad; 
    target.lon *= deg_to_rad; 

    // Calculate the surface distance (or radius - direction independent) in meters 
    // between coordinates. This distance can also be described as the distance between 
    // coordinates along their great-circle. This calculation comes from the great-circle 
    // navigation equations. Once the distance is found then the returned value is scaled 
    // by 10 (units: meters*10) so its integer representation can hold one decimal place 
    // of accuracy. Equations are structured to not have repeated calculations. 
    eq0 = target.lon - current.lon; 
    eq1 = cos(target.lat); 
    eq2 = cos(current.lat); 
    eq3 = sin(target.lat); 
    eq4 = sin(current.lat); 
    eq5 = eq1*sin(eq0); 
    eq6 = eq1*cos(eq0); 
    eq7 = eq2*eq3 - eq4*eq6; 

    surf_dist = atan2(sqrt((eq7*eq7) + (eq5*eq5)), (eq4*eq3 + eq2*eq6))*EARTH_RADIUS*KM_TO_M; 
    
    return (int32_t)(surf_dist*SCALE_10); 
}


// GPS heading calculation 
int16_t nav_calculations::gps_heading(
    gps_waypoints_t current, 
    gps_waypoints_t target)
{
    int16_t heading; 
    double eq0, eq1, num, den; 

    // Convert the coordinates to radians so they're compatible with the math library. 
    current.lat *= deg_to_rad; 
    current.lon *= deg_to_rad; 
    target.lat *= deg_to_rad; 
    target.lon *= deg_to_rad; 

    // Calculate the initial heading in radians between coordinates relative to true north. 
    // As you move along the path that's the shortest distance between two points on the 
    // globe, your heading relative to true north changes which is why this is just the 
    // instantaneous heading. This calculation comes from the great-circle navigation 
    // equations. Once the heading is found it's converted from radians to degrees and 
    // scaled by 10 (units: degrees*10) so its integer representation can hold one decimal 
    // place of accuracy. 
    eq0 = cos(target.lat); 
    eq1 = target.lon - current.lon; 
    num = eq0*sin(eq1); 
    den = cos(current.lat)*sin(target.lat) - sin(current.lat)*eq0*cos(eq1); 

    heading = (int16_t)(atan(num/den)*rad_to_scaled_deg); 

    // Correct the calculated heading if needed. atan can produce a heading outside the 
    // needed range (0-359.9 degrees) so this correction brings the value back within range. 
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


// True north heading 
int16_t nav_calculations::true_north_heading(int16_t heading) const
{
    // Use the current heading and true north correction offset to get the true north 
    // heading. If the true north heading exceeds acceptable heading bounds (0-359.9deg 
    // or 0-3599 scaled), then shift the heading to be back within bounds. This can be 
    // done because of the circular nature of the heading (ex. 0 degrees is the same 
    // direction as 360 degrees). The returned heading is in degrees*10. 

    int16_t tn_heading = heading + true_north_offset; 

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
    // Calculate the heading error and correct it when the heading crosses the 0/360 
    // degree boundary. For example, if the current heading is 10 degrees and the 
    // target heading is 345 degrees, the error will read as 345-10 = 335 degrees. 
    // Although not technically wrong, it makes more sense to say the error is -25 
    // degrees (-(10 + (360-345))) because that is the smaller angle between the two 
    // headings and the negative sign indicates in what direction this smaller error 
    // happens. So instead of turning 335 degrees clockwise, you can turn 25 degrees 
    // counter clockwise to correct for the error. The inflection point of the error 
    // for this correction is 180 degrees (or 1800 in degrees*10). 

    int16_t heading_error = target_heading - current_heading; 

    if (heading_error > MAX_HEADING_DIFF)
    {
        heading_error -= HEADING_FULL_RANGE; 
    }
    else if (heading_error <= -MAX_HEADING_DIFF)
    {
        heading_error += HEADING_FULL_RANGE; 
    }

    return heading_error; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set the coordinate low pass filter gain 
void nav_calculations::set_coordinate_lpf_gain(double coordinate_gain) 
{
    coordinate_lpf_gain = coordinate_gain; 
}


// Set the true north offset 
void nav_calculations::set_tn_offset(int16_t tn_offset) 
{
    true_north_offset = tn_offset; 
}

//=======================================================================================
