/**
 * @file pid_control.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief PID controller calculation implementation 
 * 
 * @version 0.1
 * @date 2024-01-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "pid_control.h" 
#include "tools.h" 

//=======================================================================================


//=======================================================================================
// Setup and teardown 

// Constructor 
pid_controller::pid_controller(
    int16_t kp, 
    int16_t ki, 
    int16_t kd, 
    int16_t max_error, 
    int16_t min_error) 
    : KP(kp), 
      KI(ki), 
      KD(kd), 
      error_sum(CLEAR), 
      error_max(max_error), 
      error_min(min_error), 
      error_prev(CLEAR) {} 


// Destructor 
pid_controller::~pid_controller() {} 

//=======================================================================================


//=======================================================================================
// Controller 

// PID error response calculation 
int16_t pid_controller::pid_calc(int16_t error)
{
    int16_t proportional, integral, derivative; 

    //==================================================
    // Proportional 

    proportional = KP*error; 
    
    //==================================================

    //==================================================
    // Integral 

    // Integrate/sum the error and cap it if it gets too large 
    error_sum += error; 

    if (error_sum > error_max)
    {
        error_sum = error_max; 
    }
    else if (error_sum < error_min)
    {
        error_sum = error_min; 
    }

    integral = KI*error_sum; 
    
    //==================================================

    //==================================================
    // Derivative 

    derivative = KD*(error - error_prev); 
    error_prev = error; 
    
    //==================================================

    return proportional + integral + derivative; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// Set proportional gain (KP) 
void pid_controller::set_kp(int16_t kp) { KP = kp; }


// Set integral gain (KI) 
void pid_controller::set_ki(int16_t ki) { KI = ki; }


// Set derivative gain (KD) 
void pid_controller::set_kd(int16_t kd) { KD = kd; }


// Set maximum error sum 
void pid_controller::set_max_error_sum(int16_t max_error) { error_max = max_error; }


// Set minimum error sum 
void pid_controller::set_min_error_sum(int16_t min_error) { error_min = min_error; }


// Reset the PID controller error 
void pid_controller::clear_error(void) 
{
    error_prev = CLEAR; 
    error_sum = CLEAR; 
}

//=======================================================================================
