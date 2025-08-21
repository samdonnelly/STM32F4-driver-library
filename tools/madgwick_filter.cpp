/**
 * @file madgwick_filter.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Madgwick filter 
 * 
 * @details The Madgwick algorithm code implementation is taken from the Madgwick AHRS 
 *          arduino library: https://github.com/arduino-libraries/MadgwickAHRS 
 *          Some changes have been applied to adapt the code to this library. 
 *          
 *          The algorithm is outlined in the research paper called "An efficient 
 *          orientation filter for inertial and inertial/magnetic sensor arrays" by 
 *          Sebastian O.H. Madgwick. 
 *          Source: https://courses.cs.washington.edu/courses/cse466/14au/labs/l4/madgwick_internal_report.pdf 
 * 
 * @version 0.1
 * @date 2025-08-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Includes 

#include "madgwick_filter.h"
#include <math.h>

//=======================================================================================


//=======================================================================================
// Initialization 

MadgwickFilter::MadgwickFilter(float beta_setpoint, float sample_period) 
    : beta(beta_setpoint),
      inv_sample_freq(sample_period),
	  q0(1.0f),
	  q1(0.0f),
	  q2(0.0f),
	  q3(0.0f)
{
}

//=======================================================================================


//=======================================================================================
// User functions 

// Madgwick filter implementation 
MadgwickFilter::MadgwickStatus MadgwickFilter::Madgwick(
    std::array<float, NUM_AXES> &gyro,
    std::array<float, NUM_AXES> &accel,
    std::array<float, NUM_AXES> &mag)
{
    MadgwickStatus status = MADGWICK_OK;
    float gx, gy, gz, ax, ay, az, mx, my, mz;
    float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float hx, hy;
	float _2q0mx, _2q0my, _2q0mz, _2q1mx, 
          _2bx, _2bz, _4bx, _4bz, 
          _2q0, _2q1, _2q2, _2q3, 
          _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    constexpr float _0_0f = 0.0f, _1_0f = 1.0f, _0_5f = 0.5f, _2_0f = 2.0f, _4_0f = 4.0f; 

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if ((mag[X_AXIS] == _0_0f) && (mag[Y_AXIS] == _0_0f) && (mag[Z_AXIS] == _0_0f))
    {
		return MADGWICK_INCOMPLETE;
	}

	// Convert gyroscope degrees/sec to radians/sec
	gx = gyro[X_AXIS] * DEG_TO_RAD;
	gy = gyro[Y_AXIS] * DEG_TO_RAD;
	gz = gyro[Z_AXIS] * DEG_TO_RAD;

	// Rate of change of quaternion from gyroscope
	qDot1 = _0_5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = _0_5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = _0_5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = _0_5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if (!((accel[X_AXIS] == _0_0f) && (accel[Y_AXIS] == _0_0f) && (accel[Z_AXIS] == _0_0f)))
    {
		// Normalise accelerometer measurement
		recipNorm = invSqrt(accel[X_AXIS] * accel[X_AXIS] + 
                            accel[Y_AXIS] * accel[Y_AXIS] + 
                            accel[Z_AXIS] * accel[Z_AXIS]);
		ax = accel[X_AXIS] * recipNorm;
		ay = accel[Y_AXIS] * recipNorm;
		az = accel[Z_AXIS] * recipNorm;

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mag[X_AXIS] * mag[X_AXIS] + 
                            mag[Y_AXIS] * mag[Y_AXIS] + 
                            mag[Z_AXIS] * mag[Z_AXIS]);
		mx = mag[X_AXIS] * recipNorm;
		my = mag[Y_AXIS] * recipNorm;
		mz = mag[Z_AXIS] * recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0mx = _2_0f * q0 * mx;
		_2q0my = _2_0f * q0 * my;
		_2q0mz = _2_0f * q0 * mz;
		_2q1mx = _2_0f * q1 * mx;
		_2q0 = _2_0f * q0;
		_2q1 = _2_0f * q1;
		_2q2 = _2_0f * q2;
		_2q3 = _2_0f * q3;
		_2q0q2 = _2_0f * q0 * q2;
		_2q2q3 = _2_0f * q2 * q3;
		q0q0 = q0 * q0;
		q0q1 = q0 * q1;
		q0q2 = q0 * q2;
		q0q3 = q0 * q3;
		q1q1 = q1 * q1;
		q1q2 = q1 * q2;
		q1q3 = q1 * q3;
		q2q2 = q2 * q2;
		q2q3 = q2 * q3;
		q3q3 = q3 * q3;

		// Reference direction of Earth's magnetic field
		hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
		hy = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
		_2bx = sqrtf(hx * hx + hy * hy);
		_2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
		_4bx = _2_0f * _2bx;
		_4bz = _2_0f * _2bz;

        // Gradient decent algorithm corrective step
		s0 = -_2q2 * (_2_0f * q1q3 - _2q0q2 - ax) + _2q1 * (_2_0f * q0q1 + _2q2q3 - ay) - 
             _2bz * q2 * (_2bx * (_0_5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + 
             (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + 
             _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (_0_5f - q1q1 - q2q2) - mz);
		
        s1 = _2q3 * (_2_0f * q1q3 - _2q0q2 - ax) + _2q0 * (_2_0f * q0q1 + _2q2q3 - ay) - 
             _4_0f * q1 * (_1_0f - _2_0f * q1q1 - _2_0f * q2q2 - az) + _2bz * q3 * 
             (_2bx * (_0_5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + 
             (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + 
             (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (_0_5f - q1q1 - q2q2) - mz);
		
        s2 = -_2q0 * (_2_0f * q1q3 - _2q0q2 - ax) + _2q3 * (_2_0f * q0q1 + _2q2q3 - ay) - 
             _4_0f * q2 * (_1_0f - _2_0f * q1q1 - _2_0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * 
             (_2bx * (_0_5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * 
             (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * 
             (_2bx * (q0q2 + q1q3) + _2bz * (_0_5f - q1q1 - q2q2) - mz);
		
        s3 = _2q1 * (_2_0f * q1q3 - _2q0q2 - ax) + _2q2 * (_2_0f * q0q1 + _2q2q3 - ay) + 
             (-_4bx * q3 + _2bz * q1) * (_2bx * (_0_5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + 
             (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * 
             q1 * (_2bx * (q0q2 + q1q3) + _2bz * (_0_5f - q1q1 - q2q2) - mz);
		
        // Normalise step magnitude
        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}
    else
    {
        status = MADGWICK_NO_ACCEL;
    }

	// Integrate rate of change of quaternion to yield quaternion
	q0 += qDot1 * inv_sample_freq;
	q1 += qDot2 * inv_sample_freq;
	q2 += qDot3 * inv_sample_freq;
	q3 += qDot4 * inv_sample_freq;

	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;

    // Calculate Roll, Pitch and Yaw - NWU 
    roll = atan2f(q0*q1 + q2*q3, _0_5f - q1*q1 - q2*q2);
	pitch = asinf(-_2_0f * (q1*q3 - q0*q2));
	yaw = atan2f(q1*q2 + q0*q3, _0_5f - q2*q2 - q3*q3);

    return status;
}


// Get Roll in radians to NWU frame 
float MadgwickFilter::GetRollRadNWU(void)
{
    return roll; 
}


// Get Pitch in radians to NWU frame 
float MadgwickFilter::GetPitchRadNWU(void)
{
    return pitch; 
}


// Get Yaw in radians to NWU frame 
float MadgwickFilter::GetYawRadNWU(void)
{
    return yaw; 
}


// Get Roll in degrees to NWU frame 
float MadgwickFilter::GetRollDegNWU(void)
{
    return roll * RAD_TO_DEG; 
}


// Get Pitch in degrees to NWU frame 
float MadgwickFilter::GetPitchDegNWU(void)
{
    return pitch * RAD_TO_DEG; 
}


// Get Yaw in degrees to NWU frame 
float MadgwickFilter::GetYawDegNWU(void)
{
    return yaw * RAD_TO_DEG; 
}


// Get Roll in radians to NED frame 
float MadgwickFilter::GetRollRadNED(void)
{
    return roll; 
}


// Get Pitch in radians to NED frame 
float MadgwickFilter::GetPitchRadNED(void)
{
    return -pitch; 
}


// Get Yaw in radians to NED frame 
float MadgwickFilter::GetYawRadNED(void)
{
    return -yaw; 
}


// Get Roll in degrees to NED frame 
float MadgwickFilter::GetRollDegNED(void)
{
    return roll * RAD_TO_DEG; 
}


// Get Pitch in degrees to NED frame 
float MadgwickFilter::GetPitchDegNED(void)
{
    return -pitch * RAD_TO_DEG; 
}


// Get Yaw in degrees to NED frame 
float MadgwickFilter::GetYawDegNED(void)
{
    return -yaw * RAD_TO_DEG; 
}

//=======================================================================================


//=======================================================================================
// Helper functions 

float MadgwickFilter::invSqrt(float x)
{
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;

	i = 0x5f3759df - (i >> 1);
	y = *(float*)&i;
	y *= (1.5f - (halfx * y * y));
	y *= (1.5f - (halfx * y * y));

	return y;
}

//=======================================================================================
