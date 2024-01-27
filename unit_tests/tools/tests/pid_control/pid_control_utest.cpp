//=======================================================================================
// Notes 
// - Since the behavior of a control system is highly dependent on the system itself and 
//   how the error is used, it's impossible to properly test this PID control driver 
//   through unit testing. For this reason, only the calculation of each individual PID 
//   element (P, I and D) is checked. 
//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

#include "pid_control.h" 
#include "tools.h" 

extern "C"
{
	// Add your C-only include files here 
}

//=======================================================================================


//=======================================================================================
// Macros 

// Gains 
#define GAIN_0 1 
#define GAIN_1 2 

// Errors 
#define MAX_ERROR_0 20 
#define MAX_ERROR_1 30 
#define ERROR_0 5 
#define ERROR_1 15 

//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(pid_control_test)
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
// P control 

TEST(pid_control_test, test0)
{
    pid_controller pid_local(GAIN_0, CLEAR, CLEAR, CLEAR, CLEAR); 

    // Test the proportional output only 
    LONGS_EQUAL(ERROR_0, pid_local.pid_calc(ERROR_0)); 

    // Retest the proportional output with a new gain 
    pid_local.set_kp(GAIN_1); 
    LONGS_EQUAL(GAIN_1*ERROR_0, pid_local.pid_calc(ERROR_0)); 
}

//==================================================


//==================================================
// I control 

TEST(pid_control_test, test1)
{
    pid_controller pid_local(CLEAR, GAIN_0, CLEAR, MAX_ERROR_0, -MAX_ERROR_0); 

    // Test the integral output only 
    LONGS_EQUAL(ERROR_1, pid_local.pid_calc(ERROR_1)); 
    LONGS_EQUAL(4*ERROR_0, pid_local.pid_calc(ERROR_1)); 

    // Retest the integral output with a new gain and error threshold 
    pid_local.clear_error(); 
    pid_local.set_ki(GAIN_1); 
    pid_local.set_max_error_sum(MAX_ERROR_1); 
    pid_local.set_min_error_sum(-MAX_ERROR_1); 
    LONGS_EQUAL(-2*ERROR_1, pid_local.pid_calc(-ERROR_1)); 
    LONGS_EQUAL(-4*ERROR_1, pid_local.pid_calc(-ERROR_1)); 
    LONGS_EQUAL(-4*ERROR_1, pid_local.pid_calc(-ERROR_1)); 
}

//==================================================


//==================================================
// D control 

TEST(pid_control_test, test2)
{
    pid_controller pid_local(CLEAR, CLEAR, GAIN_0, CLEAR, CLEAR); 

    // Test the derivative output only 
    LONGS_EQUAL(ERROR_0, pid_local.pid_calc(ERROR_0)); 
    LONGS_EQUAL(2*ERROR_0, pid_local.pid_calc(ERROR_1)); 

    // Retest the derivative output with a new gain 
    pid_local.clear_error(); 
    pid_local.set_kd(GAIN_1); 
    LONGS_EQUAL(2*ERROR_0, pid_local.pid_calc(ERROR_0)); 
    LONGS_EQUAL(4*ERROR_0, pid_local.pid_calc(ERROR_1)); 
}

//==================================================

//=======================================================================================
