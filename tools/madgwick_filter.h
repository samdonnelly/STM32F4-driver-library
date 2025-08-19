/**
 * @file madgwick_filter.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Madgwick filter interface 
 * 
 * @version 0.1
 * @date 2025-08-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _MADGWICK_FILTER_H_
#define _MADGWICK_FILTER_H_

//=======================================================================================
// Includes 

#include "tools.h"
#include <array>

//=======================================================================================


//=======================================================================================
// Classes 

class MadgwickFilter
{
public: 
    
    /**
     * @brief Constructor 
     * 
     * @param beta_setpoint : Madgwick filter weighted adjustment 
     * @param sample_period : time between calculations 
     */
    MadgwickFilter(float beta_setpoint, float sample_period);

    /**
     * @brief Destructor 
     */
    ~MadgwickFilter() = default;

    // Delete copy constructor and assignment operator
    MadgwickFilter(const MadgwickFilter &) = delete;
    MadgwickFilter &operator=(const MadgwickFilter &) = delete;

    // Delete move constructor and assignment operator
    MadgwickFilter(MadgwickFilter &&) = delete;
    MadgwickFilter &operator=(MadgwickFilter &&) = delete;

    enum MadgwickStatus : uint8_t 
    {
        MADGWICK_OK, 
        MADGWICK_NO_ACCEL, 
        MADGWICK_INCOMPLETE
    };

    /**
     * @brief Madgwick filter implementation 
     * 
     * @param gyro : latest 3-axis gyroscope data (deg/s) 
     * @param accel : latest 3-axis accelerometer data (g's) 
     * @param mag : latest 3-axis magnetometer data () 
     * @return MadgwickStatus : status of the calculation 
     */
    MadgwickStatus Madgwick(
        std::array<float, NUM_AXES> &gyro,
        std::array<float, NUM_AXES> &accel,
        std::array<float, NUM_AXES> &mag);

    /**
     * @brief Get Roll to NED frame 
     * 
     * @return float : roll (degrees) 
     */
    float GetRoll(void); 

    /**
     * @brief Get Pitch to NED frame 
     * 
     * @return float : pitch (degrees) 
     */
    float GetPitch(void); 

    /**
     * @brief Get Yaw to NED frame 
     * 
     * @return float : yaw (degrees) 
     */
    float GetYaw(void); 

private: 

    /**
     * @brief Inverse square root calculation 
     * 
     * @param x : value to inverse square root 
     * @return float : result of calculation 
     */
    float invSqrt(float x);

    // Madgwick filter data 
    float beta;				  // Algorithm gain (correction weight) 
    float inv_sample_freq;    // Inverse sample frequency (1 / sample_frequency (Hz) == dt) 
    float q0, q1, q2, q3;	  // Quaternion of sensor frame relative to auxiliary frame 
    float roll, pitch, yaw;   // Orientation 
};

//=======================================================================================

#endif   // _MADGWICK_FILTER_H_ 
