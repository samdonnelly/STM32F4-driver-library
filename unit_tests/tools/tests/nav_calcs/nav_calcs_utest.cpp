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

extern "C"
{
	// Add your C-only include files here 
}

//=======================================================================================


//=======================================================================================
// Macros 

// Driver parameters 
#define LPF_GAIN_0 0.1           // Low pass filter gain 
#define LPF_GAIN_1 0.5           // Low pass filter gain 
#define DEFAULT_TN_OFFSET 120    // 12 degrees (or 120 degrees*10 - scaled value) 

// Calculation allowable errors 
#define MAX_GPS_ERROR 0.0001     // Max difference between coordinates when filtering 
#define MAX_RADIUS_ERROR 50      // 5 meters (or 50 meters*10 - scaled value) 
#define MAX_HEADING_ERROR 2      // 0.2 degrees (or 2 degrees*10 - scaled value) 

// Data 
#define LPF0_FILTER_ATTEMPT 45   // Number of filter attempts needed for LPF_GAIN_0 
#define LPF1_FILTER_ATTEMPT 7    // Number of filter attempts needed for LPF_GAIN_1 
#define HEADING_RANGE 3600       // 360 degrees (or 3600 degrees*10 - scaled value) 

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
    // This test shows that the coordinate filter function needs to be called less 
    // times for a larger gain. A caveat to a larger gain is that it will filter less 
    // noise. Noise depends on the system and can't be tested here. 

    double coordinate_filter_gain = LPF_GAIN_0; 
    nav_calculations nav_local(coordinate_filter_gain); 
    gps_waypoints_t gps_current = { waypoints_test[8].lat, waypoints_test[8].lon }; 
    gps_waypoints_t gps_filtered = { waypoints_test[7].lat, waypoints_test[7].lon }; 

    // Call the coordinate filter function until the filtered coordinate is within 
    // an acceptable error of the current location. 
    for (uint8_t i = CLEAR; i < LPF0_FILTER_ATTEMPT; i++)
    {
        nav_local.coordinate_filter(gps_current, gps_filtered); 
    }
    LONGS_EQUAL(true, abs(gps_current.lat - gps_filtered.lat) < MAX_GPS_ERROR); 
    LONGS_EQUAL(true, abs(gps_current.lon - gps_filtered.lon) < MAX_GPS_ERROR); 

    // Retest the filter with a larger gain. The increased gain will allow for the 
    // filtered coordinates to update more quickly. 
    gps_filtered = { waypoints_test[7].lat, waypoints_test[7].lon }; 
    nav_local.set_coordinate_lpf_gain(LPF_GAIN_1); 

    for (uint8_t i = CLEAR; i < LPF1_FILTER_ATTEMPT; i++)
    {
        nav_local.coordinate_filter(gps_current, gps_filtered); 
    }
    LONGS_EQUAL(true, abs(gps_current.lat - gps_filtered.lat) < MAX_GPS_ERROR); 
    LONGS_EQUAL(true, abs(gps_current.lon - gps_filtered.lon) < MAX_GPS_ERROR); 
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
    LONGS_EQUAL(headings[4] + tn_offset, nav_local.true_north_heading(headings[4])); 
    LONGS_EQUAL(headings[5] + tn_offset - HEADING_RANGE, 
                nav_local.true_north_heading(headings[5])); 

    // Rerun the tests with the true north offset equal to zero. 
    nav_local.set_tn_offset(CLEAR); 
    LONGS_EQUAL(headings[0], nav_local.true_north_heading(headings[0])); 
    LONGS_EQUAL(headings[6] - HEADING_RANGE, 
                nav_local.true_north_heading(headings[6])); 

    // Rerun the tests with the true north offset as a negative value. 
    nav_local.set_tn_offset(-tn_offset); 
    LONGS_EQUAL(headings[2] - tn_offset, nav_local.true_north_heading(headings[2])); 
    LONGS_EQUAL(headings[1] - tn_offset + HEADING_RANGE, 
                nav_local.true_north_heading(headings[1])); 
}

//==================================================


//==================================================
// Heading error 

// Heading error calculated and adjusted correctly 
TEST(nav_calcs_test, nav_calcs_heading_error_calculation)
{
    // Heading error can be either positive or negative. Positive indicates that the 
    // shortest angle between the current and target headings is in the clockwise 
    // direction. Negative indicates the shortest angle is in the counter clockwise 
    // direction. 

    nav_calculations nav_local; 

    // Negative error. The target and current headings produce a positive error greater 
    // than 180deg so it's changed to be a negative error that is less than 180deg. 
    LONGS_EQUAL(headings[5] - headings[2] - HEADING_RANGE, 
                nav_local.heading_error(headings[2], headings[5])); 
    
    // Positive error. The target and current headings produce a negative error greater 
    // than 180deg so it's changed to be a positive error that is less than 180deg. 
    LONGS_EQUAL(headings[2] - headings[4] + HEADING_RANGE, 
                nav_local.heading_error(headings[4], headings[2])); 
    
    // Negative error. The target and current headings produce a negative error less 
    // than 180deg. 
    LONGS_EQUAL(headings[3] - headings[0], 
                nav_local.heading_error(headings[0], headings[3])); 
    
    // Positive error. The target and current headings produce a positive error less 
    // than 180deg. 
    LONGS_EQUAL(headings[5] - headings[3], 
                nav_local.heading_error(headings[3], headings[5])); 
}

//==================================================

//=======================================================================================
