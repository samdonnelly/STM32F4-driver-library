//=======================================================================================
// Notes 

// - The comparison values used for checking the result of the calculations were 
//   determined externally. 

//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

#include "nav_calcs.h" 
#include "tools.h" 
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
#define MAX_HEADING_ERROR 5     // 0.5 degrees (or 5 degrees*10 - scaled value) 

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
// Helper functions 

// Update the indexing used to navigation the test coordinate info 
void nav_calcs_test_coordinate_info_indexing(
    uint8_t& i0, 
    uint8_t& i1)
{
    // Global and local waypoints require different indexing beacuse they're not 
    // evaluated together. 

    // Global waypoints 
    if (i0 < NUM_GLOBAL_TEST_WAYPOINTS)
    {
        if (i1 >= NUM_GLOBAL_TEST_WAYPOINTS)
        {
            if (++i0 < NUM_GLOBAL_TEST_WAYPOINTS)
            {
                i1 = CLEAR; 
            }
        }
        
        if ((i0 == (NUM_GLOBAL_TEST_WAYPOINTS - 1)) && (i0 == i1))
        {
            i0++; 
            i1++; 
        }
    }
    // Localized waypoints 
    else 
    {
        if (i1 >= NUM_TEST_WAYPOINTS)
        {
            if (++i0 < NUM_TEST_WAYPOINTS)
            {
                i1 = NUM_GLOBAL_TEST_WAYPOINTS; 
            }
        }
    }

    if (i0 == i1)
    {
        i1++; 
    }
}

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
    int32_t radii[NUM_TEST_INFO]; 
    uint8_t i0 = CLEAR, i1 = CLEAR; 

    // Calculate the radius/distance between each combination of waypoints. 
    for (uint8_t i = CLEAR; i < NUM_TEST_INFO; i++)
    {
        nav_calcs_test_coordinate_info_indexing(i0, i1); 
        radii[i] = nav_local.gps_radius(waypoints_test[i0], waypoints_test[i1++]); 
    }

    // // Evaluate the radius/distance calculations. Global and local values require 
    // // different evalution (see the notes below). 
    // for (uint8_t i = CLEAR; i < NUM_TEST_INFO; i++)
    // {
    //     // Global waypoints 
    //     if (i < NUM_GLOBAL_TEST_INFO)
    //     {
    //         // The global coordinates are spaced very far apart and the results are compared to 
    //         // values calculated external to this code which means some error is expected. For 
    //         // these coordinates specifically, the result of the calculation in this code is 
    //         // checked to be within a maximum error of the external value. 
    //         LONGS_EQUAL(true, abs(waypoint_info_test[i].radius - radii[i]) < MAX_RADIUS_ERROR); 
    //     }
    //     // Localized waypoints 
    //     else 
    //     {
    //         // The local coordinates are close enough together that error is minimal so the 
    //         // excat value can be checked. 
    //         LONGS_EQUAL(waypoint_info_test[i].radius, radii[i]); 
    //     }
    // }

    LONGS_EQUAL(true, abs(waypoint_info_test[0].radius - radii[0]) < MAX_RADIUS_ERROR); 
    LONGS_EQUAL(true, abs(waypoint_info_test[1].radius - radii[1]) < MAX_RADIUS_ERROR); 
    LONGS_EQUAL(true, abs(waypoint_info_test[2].radius - radii[2]) < MAX_RADIUS_ERROR); 
    LONGS_EQUAL(true, abs(waypoint_info_test[3].radius - radii[3]) < MAX_RADIUS_ERROR); 
    LONGS_EQUAL(true, abs(waypoint_info_test[4].radius - radii[4]) < MAX_RADIUS_ERROR); 
}

//==================================================


//==================================================
// GPS heading 

// GPS heading calculated correctly 
TEST(nav_calcs_test, nav_calcs_gps_heading_calculation)
{
    nav_calculations nav_local; 
    int16_t headings[NUM_TEST_INFO]; 
    uint8_t i0 = CLEAR, i1 = CLEAR; 

    // Calculate the heading between each combination of waypoints. 
    for (uint8_t i = CLEAR; i < NUM_TEST_INFO; i++)
    {
        nav_calcs_test_coordinate_info_indexing(i0, i1); 
        headings[i] = nav_local.gps_heading(waypoints_test[i0], waypoints_test[i1++]); 
    }

    LONGS_EQUAL(true, abs(waypoint_info_test[0].heading - headings[0]) < MAX_HEADING_ERROR); 
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
