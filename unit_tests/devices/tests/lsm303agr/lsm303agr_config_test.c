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
// - +32,767 (0x7FFF) 
// - -1      (0xFFFF) 
// - -32,768 (0x8000) 
// 
// The orientation of the device is the same as a typical xy-plane where the positive 
// x-axis is the forward direction. The heading calculated using the device axis data 
// is in the range 0-359.9 degrees where 0 degrees is the positive x-axis and the 
// the heading increases by travelling in the clockwise direction. 
// 
// The below axis byte data is organized as: 
// - X-axis 
// - Y-axis 
// - Z-axis 

const uint8_t axis_bytes_0[BYTE_6] = 
{
    0xC1, 0x4D,   // 0x4DC1 --> +19,905 --> +19,905 * 1.5 = 29,857 
    0x32, 0x8A,   // 0x8A32 --> -30,158 --> -30,158 * 1.5 = -45,237 
    0x57, 0x04    // 0x0457 --> +1,111 ---> +1,111 * 1.5 = 1,666 
}; 

//==================================================
// 

// 0 degrees 
const uint8_t axis_bytes_1[BYTE_6] = 
{
    0x00, 0x7D,   // 0x7D00 --> +32,000 
    0x00, 0x00,   // 0x0000 --> 0 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 30 degrees 
const uint8_t axis_bytes_2[BYTE_6] = 
{
    // 0x41, 0x6C,   // 0xA79D --> +27,713 
    // 0x80, 0x3E,   // 0xA79D --> +16,000 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 45 degrees 
const uint8_t axis_bytes_3[BYTE_6] = 
{
    // 0x63, 0x58,   // 0x5863 --> +22,627 
    // 0x63, 0x58,   // 0x5863 --> +22,627 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 60 degrees 
const uint8_t axis_bytes_4[BYTE_6] = 
{
    // 0x80, 0x3E,   // 0xA79D --> +16,000 
    // 0x41, 0x6C,   // 0xA79D --> +27,713 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 90 degrees 
const uint8_t axis_bytes_5[BYTE_6] = 
{
    // 0x00, 0x00,   // 0x0000 --> 0 
    // 0x63, 0x58,   // 0x5863 --> +22,627 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x00, 0x00,   // 0x0000 --> 0 
    0x00, 0x83,   // 0x8300 --> -32,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 120 degrees 
const uint8_t axis_bytes_6[BYTE_6] = 
{
    // 0x80, 0xC1,   // 0xA79D --> -16,000 
    // 0x41, 0x6C,   // 0xA79D --> +27,713 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 135 degrees 
const uint8_t axis_bytes_7[BYTE_6] = 
{
    // 0x9D, 0xA7,   // 0xA79D --> -22,627 
    // 0x63, 0x58,   // 0x5863 --> +22,627 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 150 degrees 
const uint8_t axis_bytes_8[BYTE_6] = 
{
    // 0xBF, 0x93,   // 0xA79D --> -27,713 
    // 0x80, 0x3E,   // 0xA79D --> +16,000 
    // 0x00, 0x00    // 0x0000 --> 0 
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 180 degrees 
const uint8_t axis_bytes_9[BYTE_6] = 
{
    // 0x9D, 0xA7,   // 0xA79D --> -22,627 
    // 0x00, 0x00,   // 0x0000 --> 0 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x00, 0x83,   // 0x8300 --> -32,000 
    0x00, 0x00,   // 0x0000 --> 0 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 210 degrees 
const uint8_t axis_bytes_10[BYTE_6] = 
{
    // 0xBF, 0x93,   // 0xA79D --> -27,713 
    // 0x80, 0xC1,   // 0xA79D --> -16,000 
    // 0x00, 0x00    // 0x0000 --> 0 
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 225 degrees 
const uint8_t axis_bytes_11[BYTE_6] = 
{
    // 0x9D, 0xA7,   // 0xA79D --> -22,627 
    // 0x9D, 0xA7,   // 0xA79D --> -22,627 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 240 degrees 
const uint8_t axis_bytes_12[BYTE_6] = 
{
    // 0x80, 0xC1,   // 0xA79D --> -16,000 
    // 0xBF, 0x93,   // 0xA79D --> -27,713 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 270 degrees 
const uint8_t axis_bytes_13[BYTE_6] = 
{
    // 0x00, 0x00,   // 0x0000 --> 0 
    // 0x9D, 0xA7,   // 0xA79D --> -22,627 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x00, 0x00,   // 0x0000 --> 0 
    0x00, 0x7D,   // 0x7D00 --> +32,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 300 degrees 
const uint8_t axis_bytes_14[BYTE_6] = 
{
    // 0x80, 0x3E,   // 0xA79D --> +16,000 
    // 0xBF, 0x93,   // 0xA79D --> -27,713 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 315 degrees 
const uint8_t axis_bytes_15[BYTE_6] = 
{
    // 0x63, 0x58,   // 0x5863 --> +22,627 
    // 0x9D, 0xA7,   // 0xA79D --> -22,627 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 330 degrees 
const uint8_t axis_bytes_16[BYTE_6] = 
{
    // 0x41, 0x6C,   // 0xA79D --> +27,713 
    // 0x80, 0xC1,   // 0xA79D --> -16,000 
    // 0x00, 0x00    // 0x0000 --> 0 
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

//==================================================

// //==================================================
// // 

// // 0 degrees 
// const uint8_t axis_bytes_17[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_18[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_19[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_20[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_21[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_22[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_23[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_24[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_25[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_26[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_27[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_28[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_29[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_30[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_31[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// // 0 degrees 
// const uint8_t axis_bytes_32[BYTE_6] = 
// {
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x9D, 0xA7,   // 0xA79D --> -22,627 
//     0x00, 0x00    // 0x0000 --> 0 
// }; 

// //==================================================

//=======================================================================================


//=======================================================================================
// Sample heading checks 

// Axis data range (int16_t): -32,768 - +32,767 
// Z-axis assumed to be zero because it is not currently used in the heading calculation. 
// The number of each heading below correlates to the axis bytes number above. 

// Steps to determine the below headings: 
// - No divide by zero 
// - atan2 
// - reverse sign 
// - adjust to correct range 
// - add offset (not used here) 
// - Adjust range if needed after offset (not used here) 
// - Low pass filter (not used here) 
// - Adjust range if needed after updated heading (not used here) 

const int16_t heading_checks[NUM_TEST_DIRS] = 
{
    0,      // 1: 
    300,    // 2: 
    450,    // 3: 
    600,    // 4: 
    900,    // 5: 
    1200,   // 6: 
    1350,   // 7: 
    1500,   // 8: 
    1800,   // 9: 
    2100,   // 10: 
    2250,   // 11: 
    2400,   // 12: 
    2700,   // 13: 
    3000,   // 14: 
    3150,   // 15: 
    3300    // 16: 
}; 


// const int16_t offset_heading_checks[NUM_TEST_DIRS] = 
// {
//     160,    // 0 
//     332,    // 300 
//     418,    // 450 
//     507,    // 600 
//     685,    // 900 
//     872,    // 1200 
//     965,    // 1350 
//     1099,   // 1500 
//     1365,   // 1800 
//     1942,   // 2100 
//     2230,   // 2250 
//     2537,   // 2400 
//     3150,   // 2700 
//     3393,   // 3000 
//     3515,   // 3150 
//     3597    // 3300 
// }; 

//=======================================================================================
