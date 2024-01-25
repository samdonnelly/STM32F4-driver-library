//=======================================================================================
// Notes 

// - The comparison values used for checking the result of the calculations were 
//   determined externally. 

//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

#include "nav_calcs.h" 
#include "gps_coordinates_test.h" 

#include <iostream> 

extern "C"
{
	// Add your C-only include files here 
}

//=======================================================================================


//=======================================================================================
// Macros 

#define DEFAULT_GAIN 0.1 
#define DEFAULT_TN_OFFSET 120 
#define MAX_RADIUS_ERROR 50     // 5 meters (or 50 meters*10 - scaled value) 

//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(nav_calcs_test)
{
    // Global test group variables 

    // Constructor 
    void setup()
    {
        // 
    }

    // Destructor 
    void teardown()
    {
        // 
    }
}; 

//=======================================================================================


//=======================================================================================
// Tests 

//==================================================
// Coordinate filtering 

// Coordinate filtering works as expected 
TEST(nav_calcs_test, nav_calcs_coordinate_filter_works)
{
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 
}


// Coordinate filter gain updated successfully 
TEST(nav_calcs_test, nav_calcs_coordinate_filter_gain_update)
{
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 
}

//==================================================


//==================================================
// GPS radius 

// GPS radius calculated correctly 
TEST(nav_calcs_test, nav_calcs_gps_radius_calculation)
{
    nav_calculations nav_local; 
    int32_t radii[9]; 

    radii[0] = nav_local.gps_radius(waypoints_test[0], waypoints_test[1]); 
    radii[1] = nav_local.gps_radius(waypoints_test[0], waypoints_test[2]); 
    radii[2] = nav_local.gps_radius(waypoints_test[0], waypoints_test[3]); 
    radii[3] = nav_local.gps_radius(waypoints_test[0], waypoints_test[4]); 
    radii[4] = nav_local.gps_radius(waypoints_test[0], waypoints_test[5]); 
    radii[5] = nav_local.gps_radius(waypoints_test[6], waypoints_test[7]); 
    radii[6] = nav_local.gps_radius(waypoints_test[7], waypoints_test[8]); 
    radii[7] = nav_local.gps_radius(waypoints_test[8], waypoints_test[9]); 
    radii[8] = nav_local.gps_radius(waypoints_test[9], waypoints_test[6]); 

    // The first 6 coordinates are spaced very far apart and the results are compared to 
    // values calculated external to this code which means some error is expected. For 
    // these coordinates specifically, the result of the calculation in this code is 
    // checked to be within a maximum error of the external value. 
    LONGS_EQUAL(true, abs(110004589 - radii[0]) < MAX_RADIUS_ERROR); 
}

//==================================================


//==================================================
// GPS heading 

// GPS heading calculated correctly 
TEST(nav_calcs_test, nav_calcs_gps_heading_calculation)
{
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 

    int32_t heading = nav_local.gps_heading(waypoints_test[0], waypoints_test[1]); 

    LONGS_EQUAL(true, abs(1437 - heading) < 5); 
}

//==================================================


//==================================================
// True north heading 

// True north heading calculated and adjusted correctly 
TEST(nav_calcs_test, nav_calcs_tn_heading_calculation)
{
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 
}


// True north offset update successfully 
TEST(nav_calcs_test, nav_calcs_tn_offset_update)
{
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 
}

//==================================================


//==================================================
// Heading error 

// Heading error calculated and adjusted correctly 
TEST(nav_calcs_test, nav_calcs_heading_error_calculation)
{
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 
}

//==================================================

//=======================================================================================
