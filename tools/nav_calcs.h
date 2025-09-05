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

#ifndef _NAV_CALCS_H_ 
#define _NAV_CALCS_H_ 

//=======================================================================================
// Includes 

#include "tools.h"
#include <array>

//=======================================================================================


//=======================================================================================
// Classes 

// Navigation calculations 
class NavCalcs 
{
public: 

    /**
     * @brief Constructor 
     */
    NavCalcs(); 

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

    // Position 
    struct Position
    {
        float lat, lon, alt;   // Latitude, longitude, altitude 
    };

    // Velocity 
    struct Velocity
    {
        float sog, cog, vvel;   // Speed over ground, course over ground, vertical velocity 
    };

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
        Position new_data, 
        Position& filtered_data) const;

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
        const Position &current,
        const Position &target,
        float &target_heading,
        float &target_distance) const;

    /**
     * @brief Heading 
     * 
     * @details Finds the heading or course within a 2D plane. The returned heading is in 
     *          the range 0.0-359.9 degrees clockwise relative to North. Applications of 
     *          this function include finding magnetic heading (relative to magnetic 
     *          North) from magnetometer axis data or determining the course of an object 
     *          using velocity along axes. This function assumes that X is positive 
     *          forward and Y is positive left (following right hand rule for NWU frame). 
     *          If this convention is not followed then results may come out incorrect. 
     *          Note that axis units don't matter, only the magnitude relative to each 
     *          other matters. 
     * 
     * @param x : x-axis data 
     * @param y : y-axis data 
     * @return float : heading (0.0-359.9 degrees) 
     */
    float Heading(
        const float &x, 
        const float &y) const;

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
     *          current heading. Make sure both of the provided headings are relative to 
     *          either magnetic North or true North, not a combination of the two. 
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
     * @brief True North Earth frame acceleration 
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
    void TrueNorthEarthAccel(
        float &x,
        float &y) const;

    /**
     * @brief Kalman filter position prediction 
     * 
     * @details Kalman filter prediction step for determining the global position of a 
     *          system. This function: 
     *          - Takes in the systems acceleration in the NED frame and uses that to 
     *            predict the new position based on the last measured GPS location. 
     *          - Should be called each time acceleration in the NED frame is updated and 
     *            the time between acceleration updates is set either in the constructor 
     *            or using the setter function. 
     *          - Is linked to the Kalman filter update function which will update the 
     *            reference position when it's called. 
     *          
     *          After this function is called, the estimated position and velocity can 
     *          be retrieved from the getter. 
     * 
     * @note The provided 3-axis NED acceleration is defined as follows: 
     *       - North: index 0 / X-axis 
     *       - East:  index 1 / Y-axis 
     *       - Down:  index 2 / Z-axis 
     * 
     * @see KalmanPoseUpdate
     * @see GetKalmanPose
     * 
     * @param accel_ned : 3-axis acceleration of the system in the NED frame (g's) 
     */
    void KalmanPosePredict(const std::array<float, NUM_AXES> &accel_ned);

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
     * @param gps_position : GPS measured position: lat (deg), lon (deg) and altitude (m) 
     * @param gps_velocity : GPS measured velocity: SOG (m/s), COG (deg), vvel (m/s) 
     */
    void KalmanPoseUpdate(
        const Position &gps_position,
        const Velocity &gps_velocity);

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
     * @brief Set the Kalman Pose Data objectSet Kalman filter data 
     * 
     * @details This setter must be called before the Kalman filter for pose can be used. 
     *          Not doing so will likely produce incorrect results. Other Kalman filter 
     *          data not specified here is initialized to zero. 
     * 
     * @param predict_delta : time between calls to the prediction step function (s) 
     * @param initial_position : initial coordinates and altitude of the system 
     * @param accel_pos_variance : accelerometer position data variance 
     * @param accel_vel_variance : accelerometer velocity data variance 
     * @param gps_pos_variance : GPS position data variance 
     * @param gps_vel_variance : GPS velocity data variance 
     */
    void SetKalmanPoseData(
        float predict_delta,
        Position initial_position,
        float accel_pos_variance,
        float accel_vel_variance,
        float gps_pos_variance,
        float gps_vel_variance);

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
     * @param kalman_position : Global position: lat (deg), lon (deg), atitude(m) 
     * @param kalman_velocity : NED velocity: North, East, down (m/s) 
     */
    void GetKalmanPose(
        Position &kalman_position,
        Velocity &kalman_velocity);

private: 

    // 3-axis vector in the NED frame 
    struct VectorNED
    {
        float N, E, D;
    };

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

    // General class data 
    float coordinate_lpf_gain;   // Low pass filter gain for GPS coordinates 
    float true_north_offset;     // True north offset from magnetic north 

    // Kalman filter pose data 
    float k_dt;                         // Predicition step calculation interval (s) 
    Position kg_pos_cur, kg_pos_prv;    // Kalman filter global position - current and previous 
    Velocity kg_vel;                    // Kalman filter global velocity 
    VectorNED kl_pos, kl_vel;           // Kalman filter local position and velocity 
    VectorNED s2_p, s2_v;               // Variance in Kalman filter position and velocity 
    float s2_ap, s2_av;                 // Process (accelerometer) variance for position and velocity 
    float s2_gp, s2_gv;                 // Measurement (GPS) variance for position and velocity 
}; 

//=======================================================================================

#endif   // _NAV_CALCS_H_ 
