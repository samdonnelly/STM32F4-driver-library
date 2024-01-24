//=======================================================================================
// Notes 

// - The comparison values used for checking the result of the calculations were 
//   determined externally. 

//=======================================================================================


//=======================================================================================
// Includes 

#include <iostream> 

#include "CppUTest/TestHarness.h"
#include "nav_calcs.h" 

extern "C"
{
	// Add your C-only include files here 
}

//=======================================================================================


//=======================================================================================
// Macros 

#define DEFAULT_GAIN 0.1 
#define DEFAULT_TN_OFFSET 120 

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
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 

    gps_waypoints_t current = { 50.954427, -113.887125 }; 
    gps_waypoints_t target = { 49.334250, -123.028369 }; 

    // int32_t radius = nav_local.gps_radius(
    //     50.954427, 
    //     -113.887125, 
    //     49.334250, 
    //     -123.028369); 
    int32_t radius = nav_local.gps_radius(current, target); 

    std::cout << "\r\nRadius = " << radius << std::endl; 
}

//==================================================


//==================================================
// GPS heading 

// GPS heading calculated correctly 
TEST(nav_calcs_test, nav_calcs_gps_heading_calculation)
{
    nav_calculations nav_local(DEFAULT_GAIN); 
    nav_local.set_tn_offset(DEFAULT_TN_OFFSET); 
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
