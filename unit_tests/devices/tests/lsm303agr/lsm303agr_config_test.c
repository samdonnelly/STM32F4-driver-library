/**
 * @file lsm303agr_config_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR configuration file test implementation 
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
const int16_t lsm303agr_calibrate_offsets_test[LSM303AGR_M_NUM_DIR] = 
{
    0, 0, 0, 0, 0, 0, 0, 0
}; 

// Directional offsets to correct for heading errors (units: degrees*10) 
const int16_t lsm303agr_config_dir_offsets_test[LSM303AGR_M_NUM_DIR] = 
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
// The below axis byte data is organized as: 
// - X-axis 
// - Y-axis 
// - Z-axis 
// 
// Z-axis assumed to be zero because it is not currently used in the heading calculation. 
// The number of each heading below correlates to the axis bytes number above. 
// 
// The orientation of the device is the same as a typical xy-plane where the positive 
// x-axis is the forward direction. The heading calculated using the device axis data 
// is in the range 0-359.9 degrees where 0 degrees is the positive x-axis and the 
// the heading increases by travelling in the clockwise direction. 
// 
// Below there are two main sets of directions: true and offset. The true directions 
// are incremental headings between 0 and 360 degrees such as 0, 30, 45, 60, 90, 120, etc. 
// These are used as tests for the heading calculation when no error/offset correction 
// is applied. The offset directions are sample directions read from the device that 
// correspond to each of the true directions. The device has errors in its reading and 
// these are accounted for through device calibration which generates correction 
// equations based on how far off the device reading is from true directions (see the 
// driver interface function comments for further details). During the unit tests, the 
// offset directions are used to see if the driver will correct them to the 
// corresponding true direction. 

//==================================================
// Random sample 

const uint8_t axis_bytes_0[BYTE_6] = 
{
    0xC1, 0x4D,   // 0x4DC1 --> +19,905 --> +19,905 * 1.5 = 29,857 
    0x32, 0x8A,   // 0x8A32 --> -30,158 --> -30,158 * 1.5 = -45,237 
    0x57, 0x04    // 0x0457 --> +1,111 ---> +1,111 * 1.5 = 1,666 
}; 

//==================================================

//==================================================
// True directions 

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
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 45 degrees 
const uint8_t axis_bytes_3[BYTE_6] = 
{
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 60 degrees 
const uint8_t axis_bytes_4[BYTE_6] = 
{
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 90 degrees 
const uint8_t axis_bytes_5[BYTE_6] = 
{
    0x00, 0x00,   // 0x0000 --> 0 
    0x00, 0x83,   // 0x8300 --> -32,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 120 degrees 
const uint8_t axis_bytes_6[BYTE_6] = 
{
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 135 degrees 
const uint8_t axis_bytes_7[BYTE_6] = 
{
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 150 degrees 
const uint8_t axis_bytes_8[BYTE_6] = 
{
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 180 degrees 
const uint8_t axis_bytes_9[BYTE_6] = 
{
    0x00, 0x83,   // 0x8300 --> -32,000 
    0x00, 0x00,   // 0x0000 --> 0 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 210 degrees 
const uint8_t axis_bytes_10[BYTE_6] = 
{
    0xBF, 0x93,   // 0xA79D --> -27,713 
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 225 degrees 
const uint8_t axis_bytes_11[BYTE_6] = 
{
    0x9D, 0xA7,   // 0xA79D --> -22,627 
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 240 degrees 
const uint8_t axis_bytes_12[BYTE_6] = 
{
    0x80, 0xC1,   // 0xA79D --> -16,000 
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 270 degrees 
const uint8_t axis_bytes_13[BYTE_6] = 
{
    0x00, 0x00,   // 0x0000 --> 0 
    0x00, 0x7D,   // 0x7D00 --> +32,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 300 degrees 
const uint8_t axis_bytes_14[BYTE_6] = 
{
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 315 degrees 
const uint8_t axis_bytes_15[BYTE_6] = 
{
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 330 degrees 
const uint8_t axis_bytes_16[BYTE_6] = 
{
    0x41, 0x6C,   // 0xA79D --> +27,713 
    0x80, 0x3E,   // 0xA79D --> +16,000 
    0x00, 0x00    // 0x0000 --> 0 
}; 

//==================================================

//==================================================
// Offset directions that correlate to true directions 

// 16 degrees (0 degrees) 
const uint8_t axis_bytes_17[BYTE_6] = 
{
    0x28, 0x78,   // 0x7828 --> +30,760 
    0x8C, 0xDD,   // 0xDD8C --> -8,820 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 33.2 degrees (30 degrees) 
const uint8_t axis_bytes_18[BYTE_6] = 
{
    0x98, 0x68,   // 0x6898 --> +26,776 
    0x8E, 0xBB,   // 0xBB8E --> -17,522 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 41.8 degrees (45 degrees) 
const uint8_t axis_bytes_19[BYTE_6] = 
{
    0x2F, 0x5D,   // 0x5D2F --> +23,855 
    0xAF, 0xAC,   // 0xACAF --> -21,329 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 50.7 degrees (60 degrees) 
const uint8_t axis_bytes_20[BYTE_6] = 
{
    0x2C, 0x4F,   // 0x4F2C --> +20,268 
    0x45, 0x9F,   // 0x9F45 --> -24,763 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 68.5 degrees (90 degrees) 
const uint8_t axis_bytes_21[BYTE_6] = 
{
    0xD0, 0x2D,   // 0x2DD0 --> +11,728 
    0xB3, 0x8B,   // 0x8BB3 --> -29,773 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 87.2 degrees (120 degrees) 
const uint8_t axis_bytes_22[BYTE_6] = 
{
    0x1B, 0x06,   // 0x061B --> +1,563 
    0x26, 0x83,   // 0x8326 --> -31,962 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 96.5 degrees (135 degrees) 
const uint8_t axis_bytes_23[BYTE_6] = 
{
    0xD9, 0xF1,   // 0xF1D9 --> -3,623 
    0xCE, 0x83,   // 0x83CE --> -31,794 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 109.9 degrees (150 degrees) 
const uint8_t axis_bytes_24[BYTE_6] = 
{
    0x74, 0xD5,   // 0xD574 --> -10,892 
    0x77, 0x8A,   // 0x8A77 --> -30,089 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 136.5 degrees (180 degrees) 
const uint8_t axis_bytes_25[BYTE_6] = 
{
    0x54, 0xA5,   // 0xA554 --> -23,212 
    0xF5, 0xA9,   // 0xA9F5  --> -22,027 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 194.2 degrees (210 degrees) 
const uint8_t axis_bytes_26[BYTE_6] = 
{
    0xD2, 0x86,   // 0x86D2 --> -31,022 
    0xAA, 0x1E,   // 0x1EAA --> +7,850 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 223 degrees (225 degrees) 
const uint8_t axis_bytes_27[BYTE_6] = 
{
    0x95, 0xA4,   // 0xA495 --> -23,403 
    0x40, 0x55,   // 0x5540 --> +21,824 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 253.7 degrees (240 degrees) 
const uint8_t axis_bytes_28[BYTE_6] = 
{
    0xEB, 0xDC,   // 0xDCEB --> -8,981 
    0xFA, 0x77,   // 0x77FA --> +30,714 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 315 degrees (270 degrees) 
const uint8_t axis_bytes_29[BYTE_6] = 
{
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x63, 0x58,   // 0x5863 --> +22,627 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 339.3 degrees (300 degrees) 
const uint8_t axis_bytes_30[BYTE_6] = 
{
    0xEE, 0x74,   // 0x74EE --> +29,934 
    0x2F, 0x2C,   // 0x2C2F --> +11,311 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 351.5 degrees (315 degrees) 
const uint8_t axis_bytes_31[BYTE_6] = 
{
    0xA1, 0x7B,   // 0x7BA1 --> +31,649 
    0x7A, 0x12,   // 0x127A --> +4,730 
    0x00, 0x00    // 0x0000 --> 0 
}; 

// 359.7 degrees (330 degrees) 
const uint8_t axis_bytes_32[BYTE_6] = 
{
    0xFF, 0x7C,   // 0x7CFF --> +31,999 
    0xA8, 0x00,   // 0x00A8 --> +168 
    0x00, 0x00    // 0x0000 --> 0 
}; 

//==================================================

//=======================================================================================


//=======================================================================================
// Sample heading checks 

const int16_t heading_checks[NUM_TEST_DIRS] = 
{
    0, 300, 450, 600, 900, 1200, 1350, 1500, 1800, 2100, 2250, 2400, 2700, 3000, 3150, 3300  
}; 

//=======================================================================================
