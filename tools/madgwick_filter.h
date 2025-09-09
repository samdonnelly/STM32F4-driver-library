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
     * @param sample_period : time between calculations (seconds) 
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

    // Driver status 
    enum MadgwickStatus : uint8_t 
    {
        MADGWICK_OK,          // Calculation performed normally 
        MADGWICK_NO_ACCEL,    // No acceleration present ([0,0,0]) so no correction applied 
        MADGWICK_INCOMPLETE   // Calculation not performed - missing data 
    };

    /**
     * @brief Madgwick filter implementation 
     * 
     * @details Performs the Madgwick filter calculations and determines the roll, pitch 
     *          yaw and acceleration in the NWU Earth frame. The gyroscope, accelerometer 
     *          and magnetomter data provided must be measured in the systems body frame 
     *          with forward as positive x, left as positive y and up as positive z. The 
     *          orientation can be retreived in either the NWU or NED frame. 
     * 
     * @note The gyroscope data units matter and must be in deg/s. The accelerometer and 
     *       magnetometer units do not matter as these vectors get normalized (i.e. only 
     *       direction matters). Typical units for the accelerometer is g's and for the 
     *       magnetometer is uT or mG. 
     * 
     * @param gyro : latest 3-axis gyroscope body frame data (deg/s) 
     * @param accel : latest 3-axis accelerometer body frame data 
     * @param mag : latest 3-axis magnetometer body frame data 
     * @return MadgwickStatus : status of the calculation 
     */
    MadgwickStatus Madgwick(
        std::array<float, NUM_AXES> &gyro,
        std::array<float, NUM_AXES> &accel,
        std::array<float, NUM_AXES> &mag);

    /**
     * @brief Get the Roll, Pitch or Yaw in radians to the NWU frame 
     * 
     * @return float : Roll, Pitch or Yaw (radians) 
     */
    float GetRollRadNWU(void) const;
    float GetPitchRadNWU(void) const;
    float GetYawRadNWU(void) const;

    /**
     * @brief Get the Roll, Pitch or Yaw in degrees to the NWU frame 
     * 
     * @return float : Roll, Pitch or Yaw (degrees) 
     */
    float GetRollDegNWU(void) const;
    float GetPitchDegNWU(void) const;
    float GetYawDegNWU(void) const;

    /**
     * @brief Get the Roll, Pitch or Yaw in radians to the NED frame 
     * 
     * @return float : Roll, Pitch or Yaw (radians) 
     */
    float GetRollRadNED(void) const;
    float GetPitchRadNED(void) const;
    float GetYawRadNED(void) const;

    /**
     * @brief Get the Roll, Pitch or Yaw in degrees to the NED frame 
     * 
     * @return float : Roll, Pitch or Yaw (degrees) 
     */
    float GetRollDegNED(void) const;
    float GetPitchDegNED(void) const;
    float GetYawDegNED(void) const;

    /**
     * @brief Get absolute acceleration (no gravity) in the NWU frame 
     * 
     * @details Rotates body frame acceration into NWU Earth frame acceleration using the 
     *          Madgwick quaternion determined during the Madgwick filter calculations. 
     *          This function subtracts gravity in the Up direction. 
     * 
     * @param accel_body : body frame accelerometer data (g's) 
     * @param accel_nwu : buffer to store NWU acceleration values 
     */
    void GetAccelNWU(
        const std::array<float, NUM_AXES> &accel_body,
        std::array<float, NUM_AXES> &accel_nwu) const;

    /**
     * @brief Get absolute acceleration (no gravity) in the NED frame 
     * 
     * @details Rotates body frame acceration into NED Earth frame acceleration using the 
     *          Madgwick quaternion determined during the Madgwick filter calculations. 
     *          This function subtracts gravity in the Down direction. 
     * 
     * @param accel_body : body frame accelerometer data (g's) 
     * @param accel_ned : buffer to store NED acceleration values 
     */
    void GetAccelNED(
        const std::array<float, NUM_AXES> &accel_body,
        std::array<float, NUM_AXES> &accel_ned) const;

    /**
     * @brief Get data in the NWU frame 
     * 
     * @details Rotates body frame data into NWU Earth frame data using the Madgwick 
     *          quaternion determined during the Madgwick filter calculations. The data 
     *          can be any 3-axis body frame data such as gyroscope data or accelerometer 
     *          variance. Note that this function does not account for gravity if passing 
     *          accelerometer values. 
     * 
     * @param data_body : body frame data 
     * @param data_nwu : buffer to store NWU values 
     */
    void GetDataNWU(
        const std::array<float, NUM_AXES> &data_body,
        std::array<float, NUM_AXES> &data_nwu) const;

    /**
     * @brief Get data in the NED frame 
     * 
     * @details Rotates body frame data into NED Earth frame data using the Madgwick 
     *          quaternion determined during the Madgwick filter calculations. The data 
     *          can be any 3-axis body frame data such as gyroscope data or accelerometer 
     *          variance. Note that this function does not account for gravity if passing 
     *          accelerometer values. 
     * 
     * @param data_body : body frame data 
     * @param data_ned : buffer to store NED values 
     */
    void GetDataNED(
        const std::array<float, NUM_AXES> &data_body,
        std::array<float, NUM_AXES> &data_ned) const;

private: 

    /**
     * @brief Inverse square root calculation 
     * 
     * @param x : value to inverse square root 
     * @return float : result of calculation 
     */
    float invSqrt(const float &x) const;

    /**
     * @brief Body frame to Earth frame rotation using Madgwick quaternion 
     * 
     * @param body : body frame data 
     * @param earth : Earth frame data 
     */
    void BodyToEarth(
        const std::array<float, NUM_AXES> &body,
        std::array<float, NUM_AXES> &earth) const;

    // Madgwick filter data 
    float beta;				                             // Algorithm gain (correction weight) 
    float inv_sample_freq;                               // Inverse sample frequency (1 / sample_frequency (Hz) == dt) 
    float q0, q1, q2, q3;	                             // Quaternion of sensor frame relative to auxiliary frame 
    float r11, r12, r13, r21, r22, r23, r31, r32, r33;   // Quaternion rotation matrix elements 
    float roll, pitch, yaw;                              // Orientation 
};

//=======================================================================================

#endif   // _MADGWICK_FILTER_H_ 
