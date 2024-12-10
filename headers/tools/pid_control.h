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

//=======================================================================================
// Includes 

#include "tools.h" 

//=======================================================================================


//=======================================================================================
// Classes 

// PID controller 
class pid_controller 
{
private:   // Private variables 
    int16_t KP, KI, KD;                                    // Controller gains 
    int16_t error_sum, error_max, error_min, error_prev;   // Error tracking 

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
     * @details Uses system error, error history and controller gains (KP, KI, KD) to 
     *          calculate a PID controller output. Individual parts of the controller 
     *          (K, I or D) can be ignored by setting the respective gain to zero. 
     *          For example, to have only a PI controller, set KD to zero. Gains, error 
     *          and controller output are all integers to avoid floating point math. 
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
     * @brief Cap the accumulated error to this upper bound 
     * 
     * @param max_error : new maximum error sum 
     */
    void set_max_error_sum(int16_t max_error);


    /**
     * @brief Cap the accumulated error to this lower bound 
     * 
     * @param min_error : new minimum error sum 
     */
    void set_min_error_sum(int16_t min_error);


    /**
     * @brief Reset the PID controller error history 
     */
    void clear_error(void); 
}; 

//=======================================================================================

#endif   // _PID_CONTROL_H_ 
