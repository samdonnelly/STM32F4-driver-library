/**
 * @file lsm303agr_config_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR  configuration file test implementation 
 * 
 * @version 0.1
 * @date 2024-02-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_config_test.h" 

//=======================================================================================


//=======================================================================================
// Config messages 

// Zero offsets used for calibrating the device 
const int16_t lsm303agr_calibrate_offsets[LSM303AGR_M_NUM_DIR] = { 0, 0, 0, 0, 0, 0, 0, 0}; 

// Directional offsets to correct for heading errors (units: degrees*10) 
const int16_t lsm303agr_config_dir_offsets[LSM303AGR_M_NUM_DIR] = 
{
    -160,     // N  (0/360deg) 
    32,       // NE (45deg) 
    215,      // E  (90deg) 
    385,      // SE (135deg) 
    435,      // S  (180deg) 
    20,       // SW (225deg) 
    -450,     // W  (270deg) 
    -365      // NW (315deg) 
}; 

//=======================================================================================


//=======================================================================================
// Sample axis data 

// Data range (int16_t): 
// +32,767 (0x7FFF) 
// -1      (0xFFFF) 
// -32,768 (0x8000) 

const uint8_t axis_bytes_0[BYTE_6] = 
{
    0xC1, 0x4D,   // 0x4DC1 --> 19,905 ---> 19,905 * 1.5 = 29,857 
    0x32, 0x8A,   // 0x8A32 --> -30,158 --> -30,158 * 1.5 = -45,237 
    0x57, 0x04    // 0x0457 --> 1,111 ----> 1,111 * 1.5 = 1,666 
}; 

const uint8_t axis_bytes_1[BYTE_6] = 
{
    0xC1, 0x4D,   // 0x4DC1 --> 19,905 ---> 19,905 * 1.5 = 29,857 
    0x32, 0x8A,   // 0x8A32 --> -30,158 --> -30,158 * 1.5 = -45,237 
    0x57, 0x04    // 0x0457 --> 1,111 ----> 1,111 * 1.5 = 1,666 
}; 

const uint8_t axis_bytes_2[BYTE_6] = 
{
    0xC1, 0x4D,   // 0x4DC1 --> 19,905 ---> 19,905 * 1.5 = 29,857 
    0x32, 0x8A,   // 0x8A32 --> -30,158 --> -30,158 * 1.5 = -45,237 
    0x57, 0x04    // 0x0457 --> 1,111 ----> 1,111 * 1.5 = 1,666 
}; 

const uint8_t axis_bytes_3[BYTE_6] = 
{
    0xC1, 0x4D,   // 0x4DC1 --> 19,905 ---> 19,905 * 1.5 = 29,857 
    0x32, 0x8A,   // 0x8A32 --> -30,158 --> -30,158 * 1.5 = -45,237 
    0x57, 0x04    // 0x0457 --> 1,111 ----> 1,111 * 1.5 = 1,666 
}; 

//=======================================================================================
