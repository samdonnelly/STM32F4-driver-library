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

#define DEFAULT_GAIN 0.1        // Low pass filter gain 
#define DEFAULT_TN_OFFSET 120   // 12 degrees (or 120 degrees*10 - scaled value) 
#define MAX_RADIUS_ERROR 50     // 5 meters (or 50 meters*10 - scaled value) 
#define MAX_HEADING_ERROR 2     // 0.2 degrees (or 2 degrees*10 - scaled value) 

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
    double coordinate_filter_gain = DEFAULT_GAIN; 
    nav_calculations nav_local(coordinate_filter_gain); 
}


// Coordinate filter gain updated successfully 
TEST(nav_calcs_test, nav_calcs_coordinate_filter_gain_update)
{
    double coordinate_filter_gain = DEFAULT_GAIN; 
    nav_calculations nav_local(coordinate_filter_gain); 
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

    // Evaluate the radius/distance calculations. Global and local values require 
    // different evalution (see the notes below). 
    for (uint8_t i = CLEAR; i < NUM_TEST_INFO; i++)
    {
        // Global waypoints 
        if (i < NUM_GLOBAL_TEST_INFO)
        {
            // The global coordinates are spaced very far apart and the results are 
            // compared to values calculated external to this code which means some error 
            // is expected. For these coordinates specifically, the result of the 
            // calculation in this code is checked to be within a maximum error of the 
            // external value. 
            LONGS_EQUAL(true, abs(waypoint_info_test[i].radius - radii[i]) < MAX_RADIUS_ERROR); 
        }
        // Localized waypoints 
        else 
        {
            // The local coordinates are close enough together that error is minimal so 
            // the exact value can be checked. 
            LONGS_EQUAL(waypoint_info_test[i].radius, radii[i]); 
        }
    }
}

//==================================================


//==================================================
// GPS heading 

// GPS heading calculated correctly 
TEST(nav_calcs_test, nav_calcs_gps_heading_calculation)
{
    nav_calculations nav_local; 
    int16_t gps_headings[NUM_TEST_INFO]; 
    uint8_t i0 = CLEAR, i1 = CLEAR; 

    // Calculate the heading between each combination of waypoints. 
    for (uint8_t i = CLEAR; i < NUM_TEST_INFO; i++)
    {
        nav_calcs_test_coordinate_info_indexing(i0, i1); 
        gps_headings[i] = nav_local.gps_heading(waypoints_test[i0], waypoints_test[i1++]); 
    }

    // Evaluate the heading calculations. A small amount of error is expected due to the 
    // comparison values coming from an external source so the calculated value is checked 
    // to be within a maximum error of the external value. 
    for (uint8_t i = CLEAR; i < NUM_TEST_INFO; i++)
    {
        LONGS_EQUAL(
            true, abs(waypoint_info_test[i].heading - gps_headings[i]) < MAX_HEADING_ERROR); 
    }
}

//==================================================


//==================================================
// True north heading 

// True north heading calculated and adjusted correctly 
TEST(nav_calcs_test, nav_calcs_tn_heading_calculation)
{
    // Note that only valid initial heading values are used. The heading would be read from 
    // a compass (magnetometer) and that heading should be within range (0-359.9 degrees). 
    // Each test case below tries one heading that will remain in range and one that will go 
    // out of range when the true north offset is accounted for. 3600 is a full heading 
    // angle rotation (degrees*10) so it's used to correct the expected value when it goes 
    // out of range. 

    int16_t tn_offset = DEFAULT_TN_OFFSET; 
    nav_calculations nav_local(tn_offset); 

    // Start with a positive true north offset (set in the constructor). 
    LONGS_EQUAL(headings[3] + tn_offset, nav_local.true_north_heading(headings[3])); 
    LONGS_EQUAL(headings[4] + tn_offset - 3600, nav_local.true_north_heading(headings[4])); 

    // Rerun the tests with the true north offset equal to zero. 
    nav_local.set_tn_offset(CLEAR); 
    LONGS_EQUAL(headings[0], nav_local.true_north_heading(headings[0])); 
    LONGS_EQUAL(headings[5] - 3600, nav_local.true_north_heading(headings[5])); 

    // Rerun the tests with the true north offset as a negative value. 
    nav_local.set_tn_offset(-tn_offset); 
    LONGS_EQUAL(headings[2] - tn_offset, nav_local.true_north_heading(headings[2])); 
    LONGS_EQUAL(headings[1] - tn_offset + 3600, nav_local.true_north_heading(headings[1])); 
}

//==================================================


//==================================================
// Heading error 

// Heading error calculated and adjusted correctly 
TEST(nav_calcs_test, nav_calcs_heading_error_calculation)
{
    nav_calculations nav_local; 
}

//==================================================

//=======================================================================================
