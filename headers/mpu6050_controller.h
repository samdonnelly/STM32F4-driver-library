/**
 * @file mpu6050_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU6050 controller 
 * 
 * @version 0.1
 * @date 2023-01-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _MPU6050_CONTROLLER_H_ 
#define _MPU6050_CONTROLLER_H_ 

//=======================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "mpu6050_driver.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define MPU6050_NUM_STATES 6           // Number of controller states 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief 
 * 
 */
typedef enum {
    MPU6050_INIT_STATE, 
    MPU6050_RUN_STATE, 
    MPU6050_LOW_POWER_STATE, 
    MPU6050_LOW_POWER_TRANS_STATE, 
    MPU6050_FAULT_STATE, 
    MPU6050_RESET_STATE 
} mpu6050_states_t; 

//=======================================================================================


//=======================================================================================
// Structures 

// MPU6050 controller trackers 
typedef struct mpu6050_trackers_s 
{
    // Controller information 
    mpu6050_states_t state;               // State of the controller 
    uint8_t fault_code;                   // Controller fault code 

    // State trackers 
    uint8_t low_power : 1;                // Low power flag 
    uint8_t reset     : 1;                // Reset state trigger 
    uint8_t startup   : 1;                // Ensures the init state is run 
}
mpu6050_trackers_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef mpu6050_states_t MPU6050_STATE; 
typedef uint8_t MPU6050_FAULT_CODE; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief 
 */
typedef void (*mpu6050_state_functions_t)(
    mpu6050_trackers_t *mpu6050_device); 

//=======================================================================================


//=======================================================================================
// Control functions 

/**
 * @brief 
 * 
 */
void mpu6050_controller_init(void); 


/**
 * @brief 
 * 
 */
void mpu6050_controller(void); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief MPU6050 set reset flag 
 */
void mpu6050_set_reset_flag(void); 

//=======================================================================================


//=======================================================================================
// Getters 

/**
 * @brief Get the controller state 
 * 
 * @details 
 * 
 * @return MPU6050_STATE 
 */
MPU6050_STATE mpu6050_get_state(void); 

//=======================================================================================

#endif   // _MPU6050_CONTROLLER_H_ 
