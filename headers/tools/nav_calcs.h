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

    // Constructor - Default 
    nav_calculations(); 

    // Constructor - Specify coordinate filter gain 
    nav_calculations(double coordinate_gain); 

    // Constructor - Specify true north correction offset 
    nav_calculations(int16_t tn_offset); 

    // Constructor - Specify coordinate filter gain and true north correction offset 
    nav_calculations(
        double coordinate_gain, 
        int16_t tn_offset); 

    // Destructor 
    ~nav_calculations(); 

public:   // Calculations 

    /**
     * @brief Coordinate filter 
     * 
     * @details Uses a low pass filter to reduce noise in GPS coordinate readings. The 
     *          low pass filter equation is as follows: 
     *          
     *          filtered_gps_new = filtered_gps_old + (current_gps - filtered_gps_old)*gain 
     *          
     *          where 'gain' is 'coordinate_lpf_gain'. A smaller gain will filter more noise 
     *          but require more calculations/updates to converge on the current value. 
     *          The opposite is true for a larger gain. For this filter to work as intended, 
     *          the gain must be between 0 and 1. The needed gain will depend on the 
     *          application. 
     *          
     *          Coordinates passed to this function must be expressed entirely in degrees 
     *          (i.e. no minutes or seconds representation). They must also be within the 
     *          following range: 
     *                            -90deg <= latitude  <= +90deg 
     *                           -180deg <  longitude <= +180deg 
     * 
     * @see set_coordinate_lpf_gain 
     * 
     * @param current : current location - most recent coordinates read from GPS 
     * @param filtered : filtered coordinates - managed by the application 
     */
    void coordinate_filter(
        gps_waypoints_t current, 
        gps_waypoints_t& filtered); 


    /**
     * @brief GPS coordinate radius calculation 
     * 
     * @details Calculates the surface distance (or radius because it's direction 
     *          independent) between two coordinates. This distance can also be described 
     *          as the length of the arc along the great circle that connects these two 
     *          points. The two coordinates are the current and target locations and the 
     *          returned distance is expressed in meters*10 to provide one decimal place 
     *          of accuracy while still being an integer. 
     *          
     *          Coordinates passed to this function must be expressed entirely in degrees 
     *          (i.e. no minutes or seconds representation). They must also be within the 
     *          following range: 
     *                            -90deg <= latitude  <= +90deg 
     *                           -180deg <  longitude <= +180deg 
     * 
     * @param current : current location (read from a GPS device) 
     * @param target : target location 
     * @return int32_t : GPS radius (meters*10) 
     */
    int32_t gps_radius(
        gps_waypoints_t current, 
        gps_waypoints_t target); 


    /**
     * @brief GPS heading calculation 
     * 
     * @details Calculates the initial heading between two GPS coordinates relative to 
     *          true north. The heading is an angle from 0 to 359.9 degrees rotating 
     *          clockwise starting from the true north direction. The two coordinates 
     *          are the current and target locations and the returned heading is 
     *          expressed in degrees*10 to provide one decimal place of accuracy while 
     *          still being an integer. 
     *          
     *          As you move along the great circle path between two coordinates (i.e. 
     *          the most direct path), your heading relative to true north changes 
     *          which is why this function calculates the instantaneous heading. This 
     *          function must be called repeatedly to keep the heading up to date. 
     *          
     *          Coordinates passed to this function must be expressed entirely in degrees 
     *          (i.e. no minutes or seconds representation). They must also be within the 
     *          following range: 
     *                            -90deg <= latitude  <= +90deg 
     *                           -180deg <  longitude <= +180deg 
     * 
     * @param current : current location (read from a GPS device) 
     * @param target : target/desired location 
     * @return int16_t : GPS heading (degrees*10) 
     */
    int16_t gps_heading(
        gps_waypoints_t current, 
        gps_waypoints_t target); 


    /**
     * @brief True north heading 
     * 
     * @details Takes a heading relative to magnetic north and determines the heading 
     *          relative to true north. The returned heading is an angle from 0 to 359.9 
     *          degrees rotating clockwise starting from the true north direction, and 
     *          it's expressed in degrees*10 to provide a decimal place of accuracy. A 
     *          heading relative to magnetic north typically comes from a digital compass. 
     *          
     *          This function uses 'true_north_offset' to get the true north heading. 
     *          It can be either positive or negative and its value is how many degrees 
     *          you must rotate from true north to get to magnetic north where clockwise 
     *          rotation is positive. The offset between magnetic and true north changes 
     *          depending on your location on Earth which means this value must be updated 
     *          accordingly. 
     * 
     * @see set_tn_offset 
     * 
     * @param heading : current compass heading relative to magnetic north (degrees*10) 
     * @return int16_t : true north heading (degrees*10) 
     */
    int16_t true_north_heading(int16_t heading); 


    /**
     * @brief Heading error 
     * 
     * @details Determines the error between the current and desired headings. The 
     *          returned error is an angle from -179.9 to +180 degrees relative to the 
     *          current heading and expressed in degrees*10 to provide one decimal 
     *          place of accuracy. 
     *          
     *          The error will always be the shortest angle between the two headings. 
     *          A positive error indicates a clockwise rotation to get from the current 
     *          to the desired heading. A negative error is a counter clockwise rotation. 
     * 
     * @param current_heading : current heading (degrees*10) 
     * @param target_heading : target/desired heading (degrees*10) 
     * @return int16_t : signed error between headings (degrees*10) 
     */
    int16_t heading_error(
        int16_t current_heading, 
        int16_t target_heading); 

public:   // Setters 

    /**
     * @brief Set the GPS coordinate low pass filter gain 
     * 
     * @see coordinate_filter 
     * 
     * @param coordinate_gain : coordinate low pass filter gain 
     */
    void set_coordinate_lpf_gain(double coordinate_gain); 


    /**
     * @brief Set the true north correction offset 
     * 
     * @see true_north_heading 
     * 
     * @param tn_offset : offset between magnetic and true north (degrees*10) 
     */
    void set_tn_offset(int16_t tn_offset); 
}; 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NAV_CALCs_H_ 
