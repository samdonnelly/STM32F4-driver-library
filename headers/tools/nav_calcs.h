/**
 * @file nav_calcs.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Navigation calculations interface 
 * 
 * @version 0.1
 * @date 2023-12-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _NAV_CALCs_H_ 
#define _NAV_CALCs_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f411xe.h" 

//=======================================================================================


//=======================================================================================
// Structs 

// Stores the latitude and longitude of a GPS coordinate 
typedef struct gps_waypoints_t 
{
    double lat; 
    double lon; 
}
gps_waypoints_t; 

//=======================================================================================


//=======================================================================================
// Classes 

// Navigation calculations 
class nav_calculations 
{
private:   // Private variables 
    double coordinate_lpf_gain;  // Low pass filter gain for GPS coordinates 
    int16_t true_north_offset;   // True north offset from magnetic north 

public:   // Setup and teardown 

    // Constructor 
    nav_calculations(double coordinate_lpf_gain); 

    // Destructor 
    ~nav_calculations(); 

public:   // Calculations 

    /**
     * @brief Coordinate filter 
     * 
     * @param coordinates : coordinates to filter 
     * @param new_lat 
     * @param new_lon 
     */
    void coordinate_filter(
        gps_waypoints_t& coordinates, 
        double new_lat, 
        double new_lon); 


    /**
     * @brief GPS radius calculation 
     * 
     * @details Calculates the Earths surface distance (arc distance) between the current 
     *          GPS location and the target GPS location and returns the distance expressed 
     *          as meters*10. The great-circle navigation equations are used in this 
     *          function to determine the distance between the points. 
     *          
     *          An example use case for this information is knowing when an object has "hit" 
     *          its desired location, meaning the calculated distance is below some 
     *          threshold. Note that this function does not compare the distance against a 
     *          threshold, that is left to the application to interpret. 
     *          
     *          This distance is referred to as a radius because even though coordinates are 
     *          used to find the distance, the result has no directional significance. If 
     *          the target location is assumed to be the center of a circle, all the 
     *          locations in a circle around the target have the same calculated distance, 
     *          or radius. 
     *          
     *          The radius is calculated using a low pass filter to smooth out GPS noise 
     *          and inaccuracy. The amount of filtering depends on the gain chosen (argument 
     *          in the constructor). The gain must be greater than 0 but less than or equal 
     *          to 1 (0 < gain <= 1). As the gain approaches 0, the filtering is stronger 
     *          but requires more calls to this function to reach the "true" value. If the 
     *          gain is 1 then no filtering takes place. 
     * 
     * @param current : current location 
     * @param target : target location 
     * @return int32_t : GPS radius (meters*10) 
     */
    int32_t gps_radius(
        gps_waypoints_t current, 
        gps_waypoints_t target); 


    /**
     * @brief GPS heading calculation 
     * 
     * @details Calculates the initial heading between the current location and the target 
     *          location. The heading is an angle between 0-359.9 degrees clockwise relative 
     *          to True North and the returned heading is expressed as degrees*10. The 
     *          great-circle navigation equations are used in this function to find the 
     *          initial heading. 
     *          
     *          This information identifies the direction the object must travel at that 
     *          given moment in time in order to go directly towards the target location. 
     *          This can be compared to an objects current heading to know the error 
     *          between the current and target heading. 
     *          
     *          The heading is calculated using a low pass filter to smooth out GPS noise 
     *          and inaccuracy. The amount of filtering depends on the gain chosen (argument 
     *          in the constructor). The gain must be greater than 0 but less than or equal 
     *          to 1 (0 < gain <= 1). As the gain approaches 0, the filtering is stronger 
     *          but requires more calls to this function to reach the "true" value. If the 
     *          gain is 1 then no filtering takes place. 
     * 
     * @param lat_cur : current device latitude 
     * @param lon_cur : current device longitude 
     * @param lat_tar : target waypoint latitude 
     * @param lon_tar : target waypoint longitude 
     * @return int16_t : GPS True North heading heading (degrees*10) 
     */
    int16_t gps_heading(
        double lat_cur, 
        double lon_cur, 
        double lat_tar, 
        double lon_tar); 


    /**
     * @brief True North heading 
     * 
     * @details Reads the heading from the magnetometer and adds the true north heading offset 
     *          stored in 'mag_tn_correction' (global variable below). After the offset is 
     *          added the heading is checked to see if it is outside the acceptable heading 
     *          range (0-359.9 degrees) and if it is then it's corrected to be withing range 
     *          (ex. 365 degrees gets corrected to 5 degrees which is the same direction). 
     * 
     * @param heading : current compass heading (degrees*10) 
     * @return int16_t : true north heading (degrees*10) 
     */
    int16_t true_north_heading(int16_t heading); 


    /**
     * @brief Heading error 
     * 
     * @details Difference between the current and target heading. If the 0/360deg 
     *          heading point is between the two headings then the error is adjusted. 
     *          Returned error is within +/-180deg. 
     * 
     * @param heading_desired 
     * @param heading_current 
     * @return int16_t 
     */
    int16_t heading_error(
        int16_t current_heading, 
        int16_t target_heading); 

public:   // Setters 

    /**
     * @brief Set the GPS coordinate low pass filter gain 
     * 
     * @details Show equation for the low pass filter 
     * 
     * @param coordinate_gain : coordinate low pass filter gain 
     */
    void set_coordinate_lpf_gain(double coordinate_gain); 


    /**
     * @brief Set the true north correction offset 
     * 
     * @see true_north_heading 
     * 
     * @param tn_offset : true north offset 
     */
    void set_tn_offset(int16_t tn_offset); 
}; 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NAV_CALCs_H_ 
