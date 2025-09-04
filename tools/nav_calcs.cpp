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
// Global variables 

static constexpr float earth_radius = 6371.0;        // Average radius of the Earth (km) 
static constexpr float heading_north = 0.0;          // Heading when point North (0 degrees) 
static constexpr float max_heading_diff = 180.0;     // Maximum heading difference (+/- 180 degrees * 10) 
static constexpr float heading_full_range = 360.0;   // Full heading range (360 degrees * 10) 

//=======================================================================================


//=======================================================================================
// Initialization 

NavCalcs::NavCalcs()
    : coordinate_lpf_gain(1.0), 
      true_north_offset(CLEAR),
      k_dt(CLEAR),
      k_pos_cur(), k_pos_prv(),
      k_vel_cur(), k_vel_prv()
{
}

//=======================================================================================


//=======================================================================================
// User functions 

// Coordinate filter 
void NavCalcs::CoordinateFilter(
    Position new_data, 
    Position& filtered_data) const
{
    filtered_data.lat += (new_data.lat - filtered_data.lat)*coordinate_lpf_gain; 
    filtered_data.lon += (new_data.lon - filtered_data.lon)*coordinate_lpf_gain; 
}


// Find the distance and heading to the target location 
void NavCalcs::WaypointError(
    const Position &current,
    const Position &target,
    float &target_heading,
    float &target_distance) const
{
    float current_lat, current_lon, target_lat, target_lon; 
    float eq0, eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8; 
    float num, den; 

    // Convert the coordinates to radians so they're compatible with the math library. 
    current_lat = current.lat * DEG_TO_RAD; 
    current_lon = current.lon * DEG_TO_RAD; 
    target_lat = target.lat * DEG_TO_RAD; 
    target_lon = target.lon * DEG_TO_RAD; 

    // Break the calculations down into smaller chunks to make it more readable. This 
    // also prevents from the same calculations from being done twice. 
    eq0 = target_lon - current_lon; 
    eq1 = cos(target_lat); 
    eq2 = cos(current_lat); 
    eq3 = sin(target_lat); 
    eq4 = sin(current_lat); 
    eq5 = eq1*sin(eq0); 
    eq6 = eq1*cos(eq0); 
    eq7 = eq2*eq3; 
    eq8 = eq7 - eq4*eq6; 
    
    // Calculate the initial heading in radians between coordinates relative to true 
    // North. As you move along the path that's the shortest distance between two points 
    // on the globe, your heading relative to true North changes which is why this is 
    // just the instantaneous heading. This calculation comes from the great-circle 
    // navigation equations. Once the heading is found it's converted from radians to 
    // degrees and scaled by 10 (units: degrees*10) so its integer representation can 
    // hold one decimal place of accuracy. 
    num = eq1*sin(eq0); 
    den = eq7 - eq4*eq1*cos(eq0); 
    target_heading = atan(num/den)*RAD_TO_DEG;
    
    // Correct the calculated heading if needed. atan can produce a heading outside the 
    // needed range (0-359.9 degrees) so this correction brings the value back within 
    // range. 
    if (den < 0)
    {
        target_heading += max_heading_diff; 
    }
    else if (num < 0)
    {
        target_heading += heading_full_range; 
    }

    // Calculate the surface distance (or radius - direction independent) in meters 
    // between the current and target coordinates. This distance can also be described as 
    // the distance between coordinates along their great-circle. This calculation comes 
    // from the great-circle navigation equations. 
    target_distance = atan2(sqrt((eq8*eq8) + (eq5*eq5)), (eq4*eq3 + eq2*eq6))*earth_radius*KM_TO_M; 
}


// Magnetic heading 
float NavCalcs::MagneticHeading(
    const float &mx, 
    const float &my) const
{
    // Find the magnetic heading based on the magnetometer X and Y axis data. atan2f 
    // looks at the value and sign of X and Y to determine the correct output so axis 
    // values don't have to be checked for potential errors (ex. divide by zero). The 
    // provided magnetometer axis data is assumed to be in the NWU frame (i.e. X is 
    // positive forward and Y is positive left following right hand rule) which, if 
    // correct, will result in the calculated heading increasing from 0 clockwise with 
    // reference to magnetic North. 
    float mag_heading = atan2f(my, mx)*RAD_TO_DEG;

    // The magnetic heading is calculated within the range -179.9 to 180.0 degrees, 
    // however the returned heading needs to be in the range 0.0 to 359.9 degrees 
    // for navigation purposes. 
    HeadingBoundChecks(mag_heading);

    return mag_heading;
}


// True north heading 
float NavCalcs::TrueNorthHeading(const float &mag_heading) const
{
    // Find the true North heading by adding the true North heading offset to the 
    // magnetic heading. After this is done, the bounds are checked to make sure the 
    // offset didn't put the heading value outside its acceptable range. 
    float tn_heading = mag_heading + true_north_offset;
    HeadingBoundChecks(tn_heading);

    return tn_heading; 
}


// Heading error 
float NavCalcs::HeadingError(
    const float &current_heading, 
    const float &target_heading) const
{
    // Calculate the heading error and correct it when the heading crosses the 0/360 
    // degree boundary. For example, if the current heading is 10 degrees and the 
    // target heading is 345 degrees, the error will read as 345-10 = 335 degrees. 
    // Although not technically wrong, it makes more sense to say the error is -25 
    // degrees (-(10 + (360-345))) because that is the smaller angle between the two 
    // headings and the negative sign indicates in what direction this smaller error 
    // happens. So instead of turning 335 degrees clockwise, you can turn 25 degrees 
    // counter clockwise to correct for the error. The inflection point of the error 
    // for this correction is 180 degrees. 

    float heading_error = target_heading - current_heading; 

    if (heading_error > max_heading_diff)
    {
        heading_error -= heading_full_range; 
    }
    else if (heading_error <= -max_heading_diff)
    {
        heading_error += heading_full_range; 
    }

    return heading_error; 
}


// True North Earth frame acceleration 
void NavCalcs::TrueNorthEarthAccel(
    float &x,
    float &y) const
{
    // Use a 2D rotation matrix to rotate the North and East/West axes about the 
    // down/up axis. 
    float
    eq1 = cosf(true_north_offset),
    eq2 = sinf(true_north_offset),
    eq3 = x*eq1,
    eq4 = y*eq2,
    eq5 = x*eq2,
    eq6 = y*eq1;
    x = eq3 - eq4;
    y = eq5 + eq6;
}


// Kalman filter position prediction 
void NavCalcs::KalmanPosePredict(const std::array<float, NUM_AXES> &accel_ned)
{
    // State 
    k_pos_cur.N = k_pos_prv.N + k_vel_prv.N*k_dt + 0.5*k_dt*k_dt*accel_ned[X_AXIS];
    k_vel_cur.N = k_vel_prv.N + k_dt*accel_ned[X_AXIS];
    k_pos_cur.E = k_pos_prv.E + k_vel_prv.E*k_dt + 0.5*k_dt*k_dt*accel_ned[Y_AXIS];
    k_vel_cur.E = k_vel_prv.E + k_dt*accel_ned[Y_AXIS];
    k_pos_cur.D = k_pos_prv.D + k_vel_prv.D*k_dt + 0.5*k_dt*k_dt*accel_ned[Z_AXIS];
    k_vel_cur.D = k_vel_prv.D + k_dt*accel_ned[Z_AXIS];

    kalman_pos.lat;
    kalman_pos.lon;
    kalman_pos.alt;
}


// Kalman filter position update 
void NavCalcs::KalmanPoseUpdate(
    const Position &gps_position,
    const Velocity &gps_velocity)
{
    float K_N11, K_N22, K_E11, K_E22, K_D11, K_D22;

    // Kalman gain 

    // Update prediciton data 
}


// Set the coordinate low pass filter gain 
void NavCalcs::SetCoordinateLPFGain(float coordinate_gain)
{
    coordinate_lpf_gain = coordinate_gain; 
}


// Set the true north offset 
void NavCalcs::SetTnOffset(int16_t tn_offset)
{
    true_north_offset = tn_offset; 
}


// Set the time between Kalman pose prediction calculations 
void NavCalcs::SetKalmanDT(float dt)
{
    k_dt = dt;
}


// Get the Kalman filter position and velocity 
void NavCalcs::GetKalmanPose(
    Position &kalman_position,
    Velocity &kalman_velocity)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// Correct the heading if it exceeds acceptable bounds 
void NavCalcs::HeadingBoundChecks(float &heading) const
{
    if (heading < heading_north)
    {
        heading += heading_full_range;
    }
    else if (heading >= heading_full_range)
    {
        heading -= heading_full_range;
    }
}

//=======================================================================================
