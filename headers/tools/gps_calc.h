/**
 * @file gps_calc.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPS calculations interface 
 * 
 * @version 0.1
 * @date 2023-12-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _GPS_CALC_H_ 
#define _GPS_CALC_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "tools.h" 

//=======================================================================================


//=======================================================================================
// Classes 

// GPS calculations 
class gps_calcs 
{
private:   // Private variables 
    double radius_gain;     // Low pass filter gain of the radius calculation 
    double heading_gain;    // Low pass filter gain of the heading calculation 

public: 
    // Constructor 
    gps_calcs(
        double radius_lpf_gain, 
        double heading_lpf_gain); 

    // Destructor 
    ~gps_calcs(); 

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
     * @param lat_cur : current device latitude 
     * @param lon_cur : current device longitude 
     * @param lat_tar : target waypoint latitude 
     * @param lon_tar : target waypoint longitude 
     * @return int16_t : GPS radius (meters*10) 
     */
    int32_t gps_radius(
        double lat_cur, 
        double lon_cur, 
        double lat_tar, 
        double lon_tar); 


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
}; 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _GPS_CALC_H_ 