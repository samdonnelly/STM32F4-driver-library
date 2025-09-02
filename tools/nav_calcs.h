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

//=======================================================================================
// Includes 

#include "tools.h"
#include <array>

//=======================================================================================


//=======================================================================================
// Structs 

// Stores the latitude and longitude of a GPS coordinate 
typedef struct gps_waypoints_t 
{
    float lat; 
    float lon; 
}
gps_waypoints_t; 

//=======================================================================================


//=======================================================================================
// Classes 

// Navigation calculations 
class NavCalcs 
{
public: 

    /**
     * @brief Constructor 
     * 
     * @param coordinate_gain : low pass filter gain for GPS coordinates 
     * @param tn_offset : magnetic declination (degrees) 
     */
    NavCalcs(float coordinate_gain, float tn_offset); 

    /**
     * @brief Destructor 
     */
    ~NavCalcs() = default; 

    // Delete copy constructor and assignment operator
    NavCalcs(const NavCalcs &) = delete;
    NavCalcs &operator=(const NavCalcs &) = delete;

    // Delete move constructor and assignment operator
    NavCalcs(NavCalcs &&) = delete;
    NavCalcs &operator=(NavCalcs &&) = delete;

    /**
     * @brief Coordinate filter 
     * 
     * @details Uses a low pass filter to reduce noise in GPS coordinate readings. The 
     *          low pass filter equation is as follows: 
     *          
     *          filtered_gps_new = filtered_gps_old + (new_gps - filtered_gps_old)*gain 
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
     * @see SetCoordinateLPFGain 
     * 
     * @param new_data : new location data - most recent coordinates read from GPS 
     * @param filtered_data : filtered coordinates - managed by the application 
     */
    void CoordinateFilter(
        gps_waypoints_t new_data, 
        gps_waypoints_t& filtered_data) const;

    /**
     * @brief Find the distance and heading to the target location 
     * 
     * @details Calculates the initial heading between two GPS coordinates relative to 
     *          true North and the surface distance between them. The target heading 
     *          increases from 0 clockwise relative to true North. The distance has 
     *          no direction, it's just the shortest distance between the current and 
     *          target locations. 
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
     * @param current : current location 
     * @param target : target location 
     * @param target_heading : buffer to store the target heading (degrees) 
     * @param target_distance : buffer to store the target distance (meters) 
     */
    void WaypointError(
        const gps_waypoints_t &current,
        const gps_waypoints_t &target,
        float &target_heading,
        float &target_distance) const;

    /**
     * @brief Magnetic heading 
     * 
     * @details Finds the magnetic heading from magnetometer axis components. The 
     *          returned heading increases from 0 clockwise relative to magnetic North. 
     *          Note that the provided axis data should be calibrated before hand to get 
     *          a more accurate result. Also note that axis units don't matter. Only the 
     *          magnitude relative to each other matters. 
     * 
     * @param mx : magnetometer x-axis data 
     * @param my : magnetometer y-axis data 
     * @return float : magnetic heading (0.0-359.9 degrees) 
     */
    float MagneticHeading(
        const float &mx, 
        const float &my) const;

    /**
     * @brief True North heading 
     * 
     * @details Finds the true North heading using the magnetic heading and the true 
     *          North offset (magnetic declination). The returned heading increases from 
     *          0 clockwise relative to true North. Note that the provided magnetic 
     *          heading should be in the range 0.0-359.9 degrees. 
     *          
     *          This functions uses the true_north_offset that can be set by the user. 
     *          This variable should be set to the magnetic declination of your location 
     *          which can easily be found online. 
     * 
     * @see SetTnOffset 
     * 
     * @param mag_heading : current compass heading relative to magnetic north (0.0-359.9 degrees) 
     * @return float : true north heading (0.0-359.9 degrees) 
     */
    float TrueNorthHeading(const float &mag_heading) const;

    /**
     * @brief Heading error 
     * 
     * @details Determines the error between the current and desired headings. The 
     *          returned error is an angle from -179.9 to +180 degrees relative to the 
     *          current heading.  
     *          
     *          The error will always be the shortest angle between the two headings. 
     *          A positive error indicates a clockwise rotation to get from the current 
     *          to the desired heading. A negative error is a counter clockwise rotation. 
     * 
     * @param current_heading : current heading (0.0-359.9 degrees) 
     * @param target_heading : target/desired heading (0.0-359.9 degrees) 
     * @return float : error between headings (-179.9-180.0 degrees) 
     */
    float HeadingError(
        const float &current_heading, 
        const float &target_heading) const;

    /**
     * @brief True North acceleration 
     * 
     * @details Takes the Earth frame acceleration relative to magnetic North and rotates 
     *          it to point towards true North using the true North offset (magnetic 
     *          declination). The result can be used to estimate global position which 
     *          can be fused with GPS position for more accurate results. 
     *          
     *          This rotation calculation supports both NED and NWU frame orientations 
     *          but the order in which directions are provided matters. For NED frames, 
     *          the North axis should be x and the East axis should be y. For NWU frames, 
     *          the West axis should be x and the North axis should be y. If this is not 
     *          followed then the rotation will likely come out incorrect. 
     *          
     *          This functions uses the true_north_offset that can be set by the user. 
     *          This variable should be set to the magnetic declination of your location 
     *          which can easily be found online. 
     * 
     * @param x : x element of Earth frame acceleration to be rotated - see description 
     * @param y : y element of Earth frame acceleration to be rotated - see description 
     */
    void TrueNorthAccel(
        float &x,
        float &y) const;

    /**
     * @brief Kalman filter position prediction 
     * 
     * @details Kalman filter prediction step for determining the global position of a 
     *          system. This function: 
     *          - Takes in the systems acceleration in the NED frame and uses that to 
     *            predict the new position based on the last measured GPS location. 
     *          - Should be called each time accelration in the NED frame is updated and 
     *            the time between acceleration updates is set either in the constructor 
     *            or using the setter function. 
     *          - Is linked to the Kalman filter update function which will update the 
     *            reference position when it's called. 
     *          
     *          After this function is called, the estimated position and velocity can 
     *          be retrieved from the getter. 
     * 
     * @see KalmanPoseUpdate
     * @see GetKalmanPose
     * 
     * @param accel_ned : acceleration of the system in the NED frame (g's) 
     */
    void KalmanPosePredict(std::array<float, NUM_AXES> &accel_ned);

    /**
     * @brief Kalman filter position update 
     * 
     * @details Kalman filter update/correction step for determining the global position 
     *          of a system. This function: 
     *          - Takes the systems measured position (lititude, longitude, altitude) and 
     *            velocity (in North, East and Down) from a GPS device and uses that along 
     *            with the predicted position and velocity to find the best estimate of the 
     *            systems true position and velocity. 
     *          - Should be called each time new GPS data is received. 
     *          - Is linked to the Kalman filter prediction function which will provide 
     *            the predicited position and velocity which is fused with the measured 
     *            data. This function will update the last known position and reset the 
     *            predicition as needed. 
     *          
     *          After this function is called, the determined position and velocity can 
     *          be retrieved from the getter. 
     * 
     * @see KalmanPosePredict
     * @see GetKalmanPose
     * 
     * @param gps_pose : GPS measured position: lat (deg), lon (deg) and altitude (m) 
     * @param gps_vel : GPS measured velocity: North, East and Down (m/s) 
     */
    void KalmanPoseUpdate(
        std::array<float, NUM_AXES> &gps_pose,
        std::array<float, NUM_AXES> &gps_vel);

    /**
     * @brief Set the GPS coordinate low pass filter gain 
     * 
     * @see CoordinateFilter 
     * 
     * @param coordinate_gain : coordinate low pass filter gain 
     */
    void SetCoordinateLPFGain(float coordinate_gain); 

    /**
     * @brief Set the true North correction offset 
     * 
     * @see TrueNorthHeading 
     * 
     * @param tn_offset : offset between magnetic and true north (degrees*10) 
     */
    void SetTnOffset(int16_t tn_offset);

    /**
     * @brief Get the Kalman filter position and velocity 
     * 
     * @details Returns the determined position and velocity from the Kalman filter 
     *          in the provided buffers. These values will be updated every time the 
     *          prediction or update step is called for the pose Kalman filter. 
     * 
     * @see KalmanPosePredict
     * @see KalmanPoseUpdate
     * 
     * @param pose : Global position: lat (deg), lon (deg), atitude(m) 
     * @param vel : NED velocity: North, East, down (m/s) 
     */
    void GetKalmanPose(
        std::array<float, NUM_AXES> &pose,
        std::array<float, NUM_AXES> &vel);

private: 

    /**
     * @brief Correct the heading if it exceeds acceptable bounds 
     * 
     * @details Makes sure the provided heading angle is within 0.0-359.9 degrees. This 
     *          is for headings that may have exceeded the upper or lower bounds of the 
     *          desired heading range. Note that only up to 360 degrees of heading 
     *          correction can be applied. 
     * 
     * @param heading : potentially out of bounds heading (degrees) 
     * @return float : heading within bounds (0.0-359.9 degrees) 
     */
    void HeadingBoundChecks(float &heading) const;

    float coordinate_lpf_gain;   // Low pass filter gain for GPS coordinates 
    float true_north_offset;     // True north offset from magnetic north 
}; 

//=======================================================================================

#endif   // _NAV_CALCs_H_ 
