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
static constexpr float gravity = 9.81;               // Gravity (m/s^2) 

//=======================================================================================


//=======================================================================================
// Initialization 

NavCalcs::NavCalcs()
    : coordinate_lpf_gain(1.0f), 
      true_north_offset(CLEARF),
      k_dt(CLEARF),
      kg_pos_cur(), kg_pos_prv(),
      kg_vel(),
      kl_pos(), kl_vel(),
      s2_p(), s2_v()
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
float NavCalcs::Heading(
    const float &x, 
    const float &y) const
{
    // Find the heading based on the X and Y axis data. atan2f looks at the value and 
    // sign of X and Y to determine the correct output so axis values don't have to be 
    // checked for potential errors (ex. divide by zero). The provided axis data is 
    // assumed to be in the NWU frame (i.e. X is positive forward and Y is positive left 
    // following right hand rule) which, if correct, will result in the calculated 
    // heading increasing from 0 clockwise relative to North after the result of atan2f 
    // has been shifted into the correct bounds. 
    float heading = atan2f(y, x)*RAD_TO_DEG;

    // atan2f produces a heading in the range -179.9 to 180.0 degrees, however the 
    // returned heading needs to be in the range 0.0 to 359.9 degrees for navigation 
    // purposes. 
    HeadingBoundChecks(heading);

    return heading;
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
    const float
    eqo = true_north_offset*DEG_TO_RAD,
    eq1 = cosf(eqo),
    eq2 = sinf(eqo),
    eq3 = x*eq1,
    eq4 = y*eq2,
    eq5 = x*eq2,
    eq6 = y*eq1;

    x = eq3 - eq4;
    y = eq5 + eq6;
}


// Kalman filter position prediction 
void NavCalcs::KalmanPosePredict(
    const std::array<float, NUM_AXES> &accel_ned,
    const std::array<float, NUM_AXES> &accel_ned_accuracy)
{
    const float dt_2 = k_dt*k_dt;
    const float accel_const = 0.5f*dt_2;
    // const float s2_av_const = gravity*gravity*dt_2;
    // const float s2_ap_const = 2.25f*dt_2;
    constexpr float coordinate_const = earth_radius*KM_TO_M*DEG_TO_RAD;

    // Predict the new state by finding the local change in position and velocity (no 
    // global reference) using the new NED frame acceleration data. 
    const VectorNED accel = 
    {
        .N = accel_ned[X_AXIS]*gravity,
        .E = accel_ned[Y_AXIS]*gravity,
        .D = accel_ned[Z_AXIS]*gravity
    };
    kl_vel.N = kl_vel.N + k_dt*accel.N;
    kl_pos.N = kl_pos.N + kl_vel.N*k_dt + accel_const*accel.N;
    kl_vel.E = kl_vel.E + k_dt*accel.E;
    kl_pos.E = kl_pos.E + kl_vel.E*k_dt + accel_const*accel.E;
    kl_vel.D = kl_vel.D + k_dt*accel.D;
    kl_pos.D = kl_pos.D + kl_vel.D*k_dt + accel_const*accel.D;
    
    // Convert the local changes to a new global position using the last known position 
    // as reference. 
    kg_pos_cur.lat = kg_pos_prv.lat + kl_pos.N / coordinate_const;
    kg_pos_cur.lon = kg_pos_prv.lon + kl_pos.E / (coordinate_const*cosf(kg_pos_cur.lat*DEG_TO_RAD));
    kg_pos_cur.alt = kg_pos_prv.alt + kl_pos.D;

    // Convert the local changes in velocity to a more usable form. 
    kg_vel.sog = sqrtf(kl_vel.N*kl_vel.N + kl_vel.E*kl_vel.E);
    kg_vel.cog = Heading(kl_vel.N, -kl_vel.E);
    kg_vel.vvel = kl_vel.D;

    // Predict the new uncertainty. This must be done each prediction step to account for 
    // error accumulation. 
    const VectorNED accel_uncertainty = 
    {
        .N = accel_ned_accuracy[X_AXIS]*gravity,
        .E = accel_ned_accuracy[Y_AXIS]*gravity,
        .D = accel_ned_accuracy[Z_AXIS]*gravity
    };
    s2_v.N = s2_v.N + k_dt*accel_uncertainty.N;
    s2_p.N = s2_p.N + s2_v.N*k_dt + accel_const*accel_uncertainty.N;
    s2_v.E = s2_v.E + k_dt*accel_uncertainty.E;
    s2_p.E = s2_p.E + s2_v.E*k_dt + accel_const*accel_uncertainty.E;
    s2_v.D = s2_v.D + k_dt*accel_uncertainty.D;
    s2_p.D = s2_p.D + s2_v.D*k_dt + accel_const*accel_uncertainty.D;

    // float 
    // s2_av_N = accel_ned_accuracy[X_AXIS]*s2_av_const,
    // s2_ap_N = s2_av_N*s2_ap_const,
    // s2_av_E = accel_ned_accuracy[Y_AXIS]*s2_av_const,
    // s2_ap_E = s2_av_E*s2_ap_const,
    // s2_av_D = accel_ned_accuracy[Z_AXIS]*s2_av_const,
    // s2_ap_D = s2_av_D*s2_ap_const;

    // s2_v.N = s2_v.N + s2_av_N;
    // s2_p.N = s2_p.N + s2_v.N*dt_2 + s2_ap_N;
    // s2_v.E = s2_v.E + s2_av_E;
    // s2_p.E = s2_p.E + s2_v.E*dt_2 + s2_ap_E;
    // s2_v.D = s2_v.D + s2_av_D;
    // s2_p.D = s2_p.D + s2_v.D*dt_2 + s2_ap_D;
}


// Kalman filter position update 
void NavCalcs::KalmanPoseUpdate(
    const Position &gps_position,
    const Position &gps_position_accuracy,
    const Velocity &gps_velocity,
    const Velocity &gps_velocity_accuracy)
{
    constexpr float _0_0f = 0.0f, _1_0f = 1.0f;
    const float cog = gps_velocity.cog*DEG_TO_RAD;

    // Find the Kalman gains for position and velocity in each NED axis. 
    const float
    K_N11 = s2_p.N / (s2_p.N + gps_position_accuracy.lat),
    K_N22 = s2_v.N / (s2_v.N + gps_velocity_accuracy.sog),
    K_E11 = s2_p.E / (s2_p.E + gps_position_accuracy.lon),
    K_E22 = s2_v.E / (s2_v.E + gps_velocity_accuracy.sog),
    K_D11 = s2_p.D / (s2_p.D + gps_position_accuracy.alt),
    K_D22 = s2_v.D / (s2_v.D + gps_velocity_accuracy.vvel);

    // Update the uncertainty of the estimation. 
    s2_p.N = (_1_0f - K_N11)*s2_p.N;
    s2_v.N = (_1_0f - K_N22)*s2_v.N;
    s2_p.E = (_1_0f - K_E11)*s2_p.E;
    s2_v.E = (_1_0f - K_E22)*s2_v.E;
    s2_p.D = (_1_0f - K_D11)*s2_p.D;
    s2_v.D = (_1_0f - K_D22)*s2_v.D;

    // Update the state position 
    kg_pos_cur.lat = kg_pos_cur.lat + (gps_position.lat - kg_pos_cur.lat)*K_N11;
    kg_pos_cur.lon = kg_pos_cur.lon + (gps_position.lon - kg_pos_cur.lon)*K_E11;
    kg_pos_cur.alt = kg_pos_cur.alt + (gps_position.alt - kg_pos_cur.alt)*K_D11;
    
    // Update the state velocity. The local velocity for each axis is updated so velocity 
    // can continue to be estimated during the prediction step. The determined velocity 
    // is then converted to its global format for the user to fetch. 
    kl_vel.N = kl_vel.N + (gps_velocity.sog*cosf(cog) - kl_vel.N)*K_N22;
    kl_vel.E = kl_vel.E + (gps_velocity.sog*sinf(cog) - kl_vel.E)*K_E22;
    kl_vel.D = kl_vel.D + (gps_velocity.vvel - kl_vel.D)*K_D22;
    kg_vel.sog = sqrtf(kl_vel.N*kl_vel.N + kl_vel.E*kl_vel.E);
    kg_vel.cog = Heading(kl_vel.N, -kl_vel.E);
    kg_vel.vvel = kl_vel.D;

    // Save the global position so the prediction step can add local position estimates 
    // to it to estimate global position. Reset the local position estimates as the 
    // distance from last known position is now zero. 
    kg_pos_prv = kg_pos_cur;
    kl_pos = { _0_0f, _0_0f, _0_0f };
}


// Set the coordinate low pass filter gain 
void NavCalcs::SetCoordinateLPFGain(float coordinate_gain)
{
    coordinate_lpf_gain = coordinate_gain; 
}


// Set the true north offset 
void NavCalcs::SetTnOffset(float tn_offset)
{
    true_north_offset = tn_offset; 
}


// Set the Kalman Pose Data objectSet Kalman filter data 
void NavCalcs::SetKalmanPoseData(
    float predict_delta,
    Position initial_position)
{
    k_dt = predict_delta;
    kg_pos_prv = initial_position;
}


// Get the Kalman filter position and velocity 
void NavCalcs::GetKalmanPose(
    Position &kalman_position,
    Velocity &kalman_velocity)
{
    kalman_position = kg_pos_cur;
    kalman_velocity = kg_vel;
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
