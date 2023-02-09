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

// Drivers 
#include "mpu6050_driver.h"
#include "timers.h"

// Test 
#include "uart_comm.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define MPU6050_NUM_STATES 6         // Number of controller states 

#define MPU6050_MAX_TEMP 1180        // Max raw temp reading before fault (~40 degC) 

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
// Datatypes 

typedef mpu6050_states_t MPU6050_STATE; 
typedef uint16_t MPU6050_FAULT_CODE; 

//=======================================================================================


// Fault code Definition 
// bit 0  : driver init status (who am i read) 
// bit 1  : I2C comms error (timeout) 
// bit 2  : self-test accelerometer x-axis 
// bit 3  : self-test accelerometer y-axis 
// bit 4  : self-test accelerometer z-axis 
// bit 5  : self-test gyroscope x-axis 
// bit 6  : self-test gyroscope y-axis 
// bit 7  : self-test gyroscope z-axis 
// bit 8  : high temperature 
// bit 9  : not used 
// bit 10 : not used 
// bit 11 : not used 
// bit 12 : not used 
// bit 13 : not used 
// bit 14 : not used 
// bit 15 : not used 


//=======================================================================================
// Structures 

// MPU6050 controller trackers 
typedef struct mpu6050_trackers_s 
{
    // Peripherals 
    TIM_TypeDef *timer;                     // Pointer to timer port used in controller 

    // Device and controller information 
    mpu6050_states_t state;                 // State of the controller 
    MPU6050_FAULT_CODE fault_code;          // Controller fault code 
    uint32_t time_cnt_total;                // Time delay counter total count 
    uint32_t time_cnt;                      // Time delay counter instance 
    uint8_t  time_start;                    // Time delay counter start flag 
    uint32_t sample_period;                 // Time between data samples (us) 

    // State trackers 
    mpu6050_sleep_mode_t low_power : 1;     // Low power flag 
    uint8_t reset                  : 1;     // Reset state trigger 
    uint8_t startup                : 1;     // Ensures the init state is run 
}
mpu6050_trackers_t; 

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
 * @param timer : 
 */
void mpu6050_controller_init(
    TIM_TypeDef *timer, 
    uint32_t sample_period); 


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


/**
 * @brief Set low power flag 
 */
void mpu6050_set_low_power(void); 


/**
 * @brief Clear low power flag 
 */
void mpu6050_clear_low_power(void); 

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


/**
 * @brief Get the controller fault code 
 * 
 * @return MPU6050_FAULT_CODE 
 */
MPU6050_FAULT_CODE mpu6050_get_fault_code(void); 

//=======================================================================================

#endif   // _MPU6050_CONTROLLER_H_ 
