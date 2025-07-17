/**
 * @file mpu6050_controller.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 IMU controller interface 
 * 
 * @version 0.1
 * @date 2023-01-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _MPU6050_CONTROLLER_H_ 
#define _MPU6050_CONTROLLER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "mpu6050_driver.h"
#include "timers_driver.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Control information 
#define MPU6050_NUM_STATES 7             // Number of controller states 
#define MPU6050_NUM_READS 1              // Number of driver read functions 
#define MPU6050_ST_DELAY 10              // Post self-test delay (ms) 

// Data 
#define MPU6050_RAW_TEMP_MAX 28900       // Max raw temp reading before fault (~ 40 degC) 
#define MPU6050_RAW_TEMP_OFST 27720      // Raw temp reading offset 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief MPU6050 controller states 
 */
typedef enum {
    MPU6050_INIT_STATE,              // State 0: init 
    MPU6050_READ_CONT_STATE,         // State 1: read continuous 
    MPU6050_READ_READY_STATE,        // State 2: read ready 
    MPU6050_LOW_POWER_TRANS_STATE,   // State 3: low power mode transition 
    MPU6050_LOW_POWER_STATE,         // State 4: low power mode 
    MPU6050_FAULT_STATE,             // State 5: fault 
    MPU6050_RESET_STATE              // State 6: reset 
} mpu6050_states_t; 


/**
 * @brief Read states 
 */
typedef enum {
    MPU6050_READ_CONT, 
    MPU6050_READ_READY 
} mpu6050_read_state_t; 


/**
 * @brief MPU6050 sample type 
 * 
 * @details determines which data/sensors to read during the run state 
 */
typedef enum {
    MPU6050_READ_ALL      // Read all data 
} mpu6050_sample_type_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef mpu6050_states_t MPU6050_STATE; 
typedef uint16_t MPU6050_FAULT_CODE; 

//=======================================================================================


//=======================================================================================
// Structures 

// MPU6050 controller trackers 
typedef struct mpu6050_cntrl_data_s 
{
    // Linked list tracking 
    struct mpu6050_cntrl_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripherals 
    TIM_TypeDef *timer;                     // Pointer to timer port used in controller 

    // Device and controller information 
    mpu6050_states_t state;                 // State of the controller 
    uint32_t clk_freq;                      // Timer clock frquency 
    uint32_t sample_period;                 // Time between data samples (us) 
    uint32_t time_cnt_total;                // Time delay counter total count 
    uint32_t time_cnt;                      // Time delay counter instance 
    uint8_t  time_start;                    // Time delay counter start flag 
    // Fault code info 
    // --> bit 0-7: driver faults --> see mpu6050_get_status
    // --> bit 8: over temperature 
    // --> bits 9-15: not used 
    MPU6050_FAULT_CODE fault_code;          // Controller fault code 

    // Trackers 
    mpu6050_sleep_mode_t low_power : 1;     // Low power flag 
    uint8_t reset                  : 1;     // Reset state trigger 
    uint8_t startup                : 1;     // Ensures the init state is run 
    uint8_t read                   : 1;     // Triggers a read in the read ready state 
    uint8_t read_state             : 1;     // Sets which read state to use 
    uint8_t smpl_type              : 3;     // Read function to execute - mpu6050_sample_type_t
}
mpu6050_cntrl_data_t; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief MPU6050 state machine function pointer 
 * 
 * @param mpu6050_device : pointer to device data record 
 */
typedef void (*mpu6050_state_functions_t)(mpu6050_cntrl_data_t *mpu6050_device); 


/**
 * @brief Read function pointer 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 */
typedef MPU6050_STATUS (*mpu6050_read_functions_t)(device_number_t device_num); 

//=======================================================================================


//=======================================================================================
// Control functions 

/**
 * @brief MPU6050 controller initialization 
 * 
 * @details Initializes controller parameters. The timer specified in the arguments is used 
 *          to ensure the data gets sampled at the period (in us) specified by sample_period. 
 *          
 *          In this function a data record is created for the device specified by device_num. 
 *          If this function is called multiple times with the same device number, it will not 
 *          create a duplicate record but rather overwrite the existing record data. If a 
 *          valid data record fails to be created then the function will return without having 
 *          created and intialized a data record. 
 * 
 * @param device_num : device number - used for creating the correct data record 
 * @param timer : pointer to timer port to use for sample period timing 
 * @param sample_period : time (us) between samples 
 */
void mpu6050_controller_init(
    device_number_t device_num, 
    TIM_TypeDef *timer, 
    uint32_t sample_period); 


/**
 * @brief MPU6050 controller 
 * 
 * @details Contains the state machine that dictates the control of the device. device_num 
 *          is used to check for the existance of a data record created during the controller 
 *          initialization function. If this does not exist then the control will not be 
 *          carried out. This is also true for all the setters and getters. 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 */
void mpu6050_controller(device_number_t device_num); 

//=======================================================================================


//=======================================================================================
// Setters 

/**
 * @brief Set low power flag 
 * 
 * @details This flag triggers the low power state which puts the device in sleep mode. 
 *          Clearing the flag, setting the reset flag or a fault conditon being set will 
 *          cause exit of the low power state and this setter will need to be called again 
 *          to get back to low power mode. 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 */
void mpu6050_set_low_power(device_number_t device_num); 


/**
 * @brief Clear low power flag 
 * 
 * @details Clears the low power flag to bring the device out of low power mode. Note that 
 *          triggering a reset will automatically clear this flag. 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 */
void mpu6050_clear_low_power(device_number_t device_num); 


/**
 * @brief Set the data sample type 
 * 
 * @details Used to update the sample type in the data record which is used to determine 
 *          which data to sample while in the run state 
 * 
 * @see mpu6050_sample_type_t 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 * @param type : 
 */
void mpu6050_set_smpl_type(
    device_number_t device_num, 
    mpu6050_sample_type_t type); 


/**
 * @brief Set the read state 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 * @param read_type 
 */
void mpu6050_set_read_state(
    device_number_t device_num, 
    mpu6050_read_state_t read_type); 


/**
 * @brief Set the read flag 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 */
void mpu6050_set_read_flag(device_number_t device_num); 


/**
 * @brief MPU6050 set reset flag 
 * 
 * @details This flag triggers a controller reset. It is used in the event of a fault. The 
 *          flag clears automatically after being called. 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 */
void mpu6050_set_reset_flag(device_number_t device_num); 

//=======================================================================================


//=======================================================================================
// Getters 

/**
 * @brief Get the controller state 
 * 
 * @details Returns the current state of the controller. 
 * 
 * @see mpu6050_states_t
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 * @return MPU6050_STATE : state of the controller 
 */
MPU6050_STATE mpu6050_get_state(device_number_t device_num); 


/**
 * @brief Get the controller fault code 
 * 
 * @details Returns the fault code for the controller. Note that the fault code is a 
 *          combination of the driver and controller faults. Fault code definition: 
 *          - bit 0  : I2C comms error 
 *          - bit 1  : driver init status (who_am_i register read) 
 *          - bit 2  : self-test result - accelerometer x-axis 
 *          - bit 3  : self-test result - accelerometer y-axis 
 *          - bit 4  : self-test result - accelerometer z-axis 
 *          - bit 5  : self-test result - gyroscope x-axis 
 *          - bit 6  : self-test result - gyroscope y-axis 
 *          - bit 7  : self-test result - gyroscope z-axis 
 *          - bit 8  : invalid data record pointer 
 *          - bit 9  : over-temperature 
 *          - bit 10 : not used 
 *          - bit 11 : not used 
 *          - bit 12 : not used 
 *          - bit 13 : not used 
 *          - bit 14 : not used 
 *          - bit 15 : not used 
 * 
 * @param device_num : device number - used for retrieving the correct data record 
 * @return MPU6050_FAULT_CODE : controller fault code 
 */
MPU6050_FAULT_CODE mpu6050_get_fault_code(device_number_t device_num); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _MPU6050_CONTROLLER_H_ 
