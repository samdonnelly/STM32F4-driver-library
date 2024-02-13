/**
 * @file lsm303agr_config_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR configuration file test interface 
 * 
 * @version 0.1
 * @date 2024-02-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _LSM303AGR_CONFIG_TEST_H_ 
#define _LSM303AGR_CONFIG_TEST_H_ 

//=======================================================================================
// Includes 

#include "lsm303agr_driver.h" 

//=======================================================================================


//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Config messages 

// Zero offsets used for calibrating the device 
extern const int16_t lsm303agr_calibrate_offsets[LSM303AGR_M_NUM_DIR]; 

// Directional offsets to correct for heading errors (units: degrees*10) 
extern const int16_t lsm303agr_config_dir_offsets[LSM303AGR_M_NUM_DIR]; 

//=======================================================================================


//=======================================================================================
// Sample axis data 

extern const uint8_t axis_bytes_0[BYTE_6]; 
extern const uint8_t axis_bytes_1[BYTE_6]; 
extern const uint8_t axis_bytes_2[BYTE_6]; 
extern const uint8_t axis_bytes_3[BYTE_6]; 

//=======================================================================================

#endif   // _LSM303AGR_CONFIG_TEST_H_ 
