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

    /**
     * @brief Madgwick filter implementation 
     * 
     * @param wx 
     * @param wy 
     * @param wz 
     * @param ax 
     * @param ay 
     * @param az 
     * @param mx 
     * @param my 
     * @param mz 
     */
    void Madgwick(float wx, float wy, float wz, float ax, float ay, float az, float mx, float my, float mz);

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

    // Data 
    float beta;				  // Algorithm gain 
    float inv_sample_freq;    // Inverse sample frequency (1 / sample_frequency (Hz) == dt) 
    float q0, q1, q2, q3;	  // quaternion of sensor frame relative to auxiliary frame 
    float roll, pitch, yaw;   // Orientation 
};

//=======================================================================================

#endif   // _MADGWICK_FILTER_H_ 
