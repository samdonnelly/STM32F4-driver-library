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

#define NUM_TEST_DIRS 16 

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

//==================================================
// Sample data 

extern const uint8_t axis_bytes_0[BYTE_6]; 

//==================================================

//==================================================
// True directions 

extern const uint8_t axis_bytes_1[BYTE_6];    // 0 degrees 
extern const uint8_t axis_bytes_2[BYTE_6];    // 30 degrees 
extern const uint8_t axis_bytes_3[BYTE_6];    // 45 degrees 
extern const uint8_t axis_bytes_4[BYTE_6];    // 60 degrees 
extern const uint8_t axis_bytes_5[BYTE_6];    // 90 degrees 
extern const uint8_t axis_bytes_6[BYTE_6];    // 120 degrees 
extern const uint8_t axis_bytes_7[BYTE_6];    // 135 degrees 
extern const uint8_t axis_bytes_8[BYTE_6];    // 150 degrees 
extern const uint8_t axis_bytes_9[BYTE_6];    // 180 degrees 
extern const uint8_t axis_bytes_10[BYTE_6];   // 210 degrees 
extern const uint8_t axis_bytes_11[BYTE_6];   // 225 degrees 
extern const uint8_t axis_bytes_12[BYTE_6];   // 240 degrees 
extern const uint8_t axis_bytes_13[BYTE_6];   // 270 degrees 
extern const uint8_t axis_bytes_14[BYTE_6];   // 300 degrees 
extern const uint8_t axis_bytes_15[BYTE_6];   // 315 degrees 
extern const uint8_t axis_bytes_16[BYTE_6];   // 330 degrees 

//==================================================

//==================================================
// Offset directions that correlate to true directions 

extern const uint8_t axis_bytes_17[BYTE_6];   // 16 degrees (0 degrees) 
extern const uint8_t axis_bytes_18[BYTE_6];   // 33.2 degrees (30 degrees) 
extern const uint8_t axis_bytes_19[BYTE_6];   // 41.8 degrees (45 degrees) 
extern const uint8_t axis_bytes_20[BYTE_6];   // 50.7 degrees (60 degrees) 
extern const uint8_t axis_bytes_21[BYTE_6];   // 68.5 degrees (90 degrees) 
extern const uint8_t axis_bytes_22[BYTE_6];   // 87.2 degrees (120 degrees) 
extern const uint8_t axis_bytes_23[BYTE_6];   // 96.5 degrees (135 degrees) 
extern const uint8_t axis_bytes_24[BYTE_6];   // 109.9 degrees (150 degrees) 
extern const uint8_t axis_bytes_25[BYTE_6];   // 136.5 degrees (180 degrees) 
extern const uint8_t axis_bytes_26[BYTE_6];   // 194.2 degrees (210 degrees) 
extern const uint8_t axis_bytes_27[BYTE_6];   // 223 degrees (225 degrees) 
extern const uint8_t axis_bytes_28[BYTE_6];   // 253.7 degrees (240 degrees) 
extern const uint8_t axis_bytes_29[BYTE_6];   // 315 degrees (270 degrees) 
extern const uint8_t axis_bytes_30[BYTE_6];   // 339.3 degrees (300 degrees) 
extern const uint8_t axis_bytes_31[BYTE_6];   // 351.5 degrees (315 degrees) 
extern const uint8_t axis_bytes_32[BYTE_6];   // 359.7 degrees (330 degrees) 

//==================================================

//=======================================================================================


//=======================================================================================
// Sample heading checks 

extern const int16_t heading_checks[NUM_TEST_DIRS]; 

//=======================================================================================

#endif   // _LSM303AGR_CONFIG_TEST_H_ 
