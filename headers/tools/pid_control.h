/**
 * @file pid_control.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief PID controller calculation interface 
 * 
 * @version 0.1
 * @date 2024-01-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _PID_CONTROL_H_ 
#define _PID_CONTROL_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "stm32f411xe.h" 

//=======================================================================================


//=======================================================================================
// Classes 

// PID controller 
class pid_controller 
{
private:   // Private variables 
    int16_t KP, KI, KD, error_prev, error_sum, error_max, error_min; 

public:   // Setup and teardown 

    // Constructor 
    pid_controller(
        int16_t kp, 
        int16_t ki, 
        int16_t kd, 
        int16_t max_error, 
        int16_t min_error); 

    // Destructor 
    ~pid_controller(); 

public:   // Controller 
    
    /**
     * @brief PID error response calculation 
     * 
     * @details Takes an error and calculates the PID output based on PID gains (KP, KI, 
     *          KD) and previous errors. The PID output can be used to dictate the output 
     *          of the control system using this function (such as motor RPM). 
     * 
     * @param error : current control system error 
     * @return int16_t : PID controller output 
     */
    int16_t pid_calc(int16_t error); 

public:   // Setters 

    /**
     * @brief Set proportional gain (KP) 
     * 
     * @param kp : new proportional gain 
     */
    void set_kp(int16_t kp); 


    /**
     * @brief Set integral gain (KI) 
     * 
     * @param ki : new integral gain 
     */
    void set_ki(int16_t ki);


    /**
     * @brief Set derivative gain (KD) 
     * 
     * @param kd : new derivative gain 
     */
    void set_kd(int16_t kd);


    /**
     * @brief Set maximum error sum 
     * 
     * @param max_error : new maximum error sum 
     */
    void set_max_error_sum(int16_t max_error);


    /**
     * @brief Set minimum error sum 
     * 
     * @param min_error : new minimum error sum 
     */
    void set_min_error_sum(int16_t min_error);


    /**
     * @brief Reset the PID controller error 
     */
    void clear_error(void); 
}; 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _PID_CONTROL_H_ 
