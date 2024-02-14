/**
 * @file lsm303agr_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR 6-axis magnetometer and accelerometer driver implementation 
 * 
 * @version 0.1
 * @date 2023-06-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_driver.h"

//=======================================================================================


//=======================================================================================
// Notes: 
// - Procedures start on page 38 of the datasheet 
// - Low power mode can achieved by setting the low power bit but also putting the device 
//   into idle mode as well 
// - The magnetometer works best in the XY plane (board orientation is level with the 
//   ground). For this reason, for the magnetometer driver to work correctly, the board 
//   must be oriented in the XY plane. 
//=======================================================================================


//=======================================================================================
// Macros 

// Magnetometer data 
#define LSM303AGR_M_SENS 3              // Magnetometer sensitivity numerator (3/2 == 1.5) 
#define LSM303AGR_M_HEAD_SCALE 10             // Heading scaling factor (to remove decimals) 
#define LSM303AGR_M_N 0                       // North direction heading - scaled 
#define LSM303AGR_M_NE 450                    // North-East direction heading - scaled 
#define LSM303AGR_M_E 900                     // East direction heading - scaled 
#define LSM303AGR_M_SE 1350                   // South-East direction heading - scaled 
#define LSM303AGR_M_S 1800                    // South direction heading - scaled 
#define LSM303AGR_M_SW 2250                   // South-West direction heading - scaled 
#define LSM303AGR_M_W 2700                    // West direction heading - scaled 
#define LSM303AGR_M_NW 3150                   // North-West direction heading - scaled 
#define LSM303AGR_M_GAIN 0.1                  // Magnetometer filter gain 


//==================================================
// Dev 

// I2C addresses (datasheet page 39) 
#define LSM303AGR_A_ADDR 0x32           // Accelerometer (7-bit addr (0x19<<1) + Write bit) 
#define LSM303AGR_M_ADDR 0x3C           // Magnetometer (7-bit addr (0x1E<<1) + Write bit) 

// Data tools 
#define LSM303AGR_BIT_MASK 0x01         // Mask to filter out status bits 
#define LSM303AGR_ADDR_INC 0x80         // Register address increment mask for r/w 

// Magnetometer register addresses 
#define LSM303AGR_M_WHO_AM_I 0x4F       // WHO AM I 
#define LSM303AGR_M_CFG_A 0x60          // Configuration register A 
#define LSM303AGR_M_CFG_B 0x61          // Configuration register B 
#define LSM303AGR_M_CFG_C 0x62          // Configuration register C 
#define LSM303AGR_M_STATUS 0x67         // Status register 
#define LSM303AGR_M_X_L 0x68            // X component of magnetic field (first data reg) 

// Magnetometer data 
#define LSM303AGR_M_ID 0x40             // Value returned from the WHO_AM_I register 
#define LSM303AGR_M_SENS_NUM 3          // Magnetometer sensitivity numerator (3/2) 
#define LSM303AGR_M_SENS_DEN 2          // Magnetometer sensitivity denominator (3/2) 
#define LSM303AGR_M_HEAD_MAX 3600       // Max heading value - scaled (360deg * 10)
#define LSM303AGR_M_HEAD_DIFF 1800      // Heading different threshold for filtering 
#define LSM303AGR_M_DIR_OFFSET 450      // 45deg (*10) - heading sections (ex. N-->NE) 

//==================================================

//=======================================================================================


//=======================================================================================
// Enums 

// Device axis index 
typedef enum {
    X_AXIS, 
    Y_AXIS, 
    Z_AXIS, 
    NUM_AXES 
} lsm303agr_axis_t; 

//=======================================================================================


//=======================================================================================
// Register data 

//==================================================
// Dev 

// Magnetometer data 
typedef union lsm303agr_m_data_s_dev 
{
    // Axis data is a 16-bit value but broken down into low and high byte registers. 
    uint8_t m_axis_bytes[BYTE_2]; 
    int16_t m_axis; 
}
lsm303agr_m_data_t_dev; 


// Magnetometer configuration register A 
typedef union lsm303agr_m_cfga_s_dev 
{
    // Configuration register A bits 
    struct {
        uint8_t md           : 2;      // Mode select 
        uint8_t odr          : 2;      // Output data rate 
        uint8_t lp           : 1;      // Low-power mode enable 
        uint8_t soft_rst     : 1;      // Config and user register reset 
        uint8_t reboot       : 1;      // Reboot memory contents 
        uint8_t comp_temp_en : 1;      // Temperature compensation 
    }; 

    // CFG-A register byte 
    uint8_t cfga_reg; 
}
lsm303agr_m_cfga_t_dev; 


// Magnetometer configuration register B 
typedef union lsm303agr_m_cfgb_s_dev 
{
    // Configuration register C bits 
    struct {
        uint8_t lpf               : 1;      // Low pass filter enable 
        uint8_t off_canc          : 1;      // Enable offset cancellation 
        uint8_t set_freq          : 1;      // Frequency of set pulse 
        uint8_t int_on_dataoff    : 1;      // Interrupt check after hard-ron correction 
        uint8_t off_canc_one_shot : 1;      // Offset cancellation - single measurement mode 
        uint8_t unused_1          : 3;      // Not used 
    }; 

    // CFG-B register byte 
    uint8_t cfgb_reg; 
}
lsm303agr_m_cfgb_t_dev; 


// Magnetometer configuration register C 
typedef union lsm303agr_m_cfgc_s_dev 
{
    // Configuration register C bits 
    struct {
        uint8_t int_mag     : 1;      // DRDY pin configured as digital output 
        uint8_t self_test   : 1;      // Self-test enable 
        uint8_t unused_2    : 1;      // Bit not used 
        uint8_t ble         : 1;      // High and low data inversion 
        uint8_t bdu         : 1;      // Asynchronous read data protection 
        uint8_t i2c_dis     : 1;      // I2C disable 
        uint8_t int_mag_pin : 1;      // Interrupt on INT_MAG_PIN enable 
        uint8_t unused_1    : 1;      // Bit not used 
    }; 

    // CFG-C register byte 
    uint8_t cfgc_reg; 
}
lsm303agr_m_cfgc_t_dev; 


// Magnetometer status register 
typedef union lsm303agr_m_status_s_dev 
{
    // Status register bits 
    struct {
        uint8_t x_da   : 1;      // X-axis new data available 
        uint8_t y_da   : 1;      // Y-axis new data available 
        uint8_t z_da   : 1;      // Z-axis new data available 
        uint8_t zyx_da : 1;      // XYZ-axis new data available 
        uint8_t x_or   : 1;      // X-axis data overrun 
        uint8_t y_or   : 1;      // Y-axis data overrun 
        uint8_t z_or   : 1;      // Z-axis data overrun 
        uint8_t zyx_or : 1;      // XYZ-axis data overrun 
    }; 

    // Status register byte 
    uint8_t status_reg; 
}
lsm303agr_m_status_t_dev; 

//==================================================


// Magnetometer data 
typedef struct lsm303agr_m_data_s 
{
    int16_t m_x;      // X-axis magnetometer data 
    int16_t m_y;      // Y-axis magnetometer data 
    int16_t m_z;      // Z-axis magnetometer data 
}
lsm303agr_m_data_t; 


// Magnetometer configuration register A 
typedef struct lsm303agr_m_cfga_s 
{
    uint8_t comp_temp_en : 1;      // Temperature compensation 
    uint8_t reboot       : 1;      // Reboot memory contents 
    uint8_t soft_rst     : 1;      // Config and user register reset 
    uint8_t lp           : 1;      // Low-power mode enable 
    uint8_t odr1         : 1;      // Output data rate - high bit 
    uint8_t odr0         : 1;      // Output data rate - low bit 
    uint8_t md1          : 1;      // Mode select - high bit 
    uint8_t md0          : 1;      // Mode select - low bit 
}
lsm303agr_m_cfga_t; 


// Magnetometer configuration register B 
typedef struct lsm303agr_m_cfgb_s 
{
    uint8_t unused_1          : 3;      // Not used 
    uint8_t off_canc_one_shot : 1;      // Offset cancellation - single measurement mode 
    uint8_t int_on_dataoff    : 1;      // Interrupt check after hard-ron correction 
    uint8_t set_freq          : 1;      // Frequency of set pulse 
    uint8_t off_canc          : 1;      // Enable offset cancellation 
    uint8_t lpf               : 1;      // Low pass filter enable 
}
lsm303agr_m_cfgb_t; 


// Magnetometer configuration register C 
typedef struct lsm303agr_m_cfgc_s 
{
    uint8_t unused_1    : 1;      // Bit not used 
    uint8_t int_mag_pin : 1;      // Interrupt on INT_MAG_PIN enable 
    uint8_t i2c_dis     : 1;      // I2C disable 
    uint8_t bdu         : 1;      // Asynchronous read data protection 
    uint8_t ble         : 1;      // High and low data inversion 
    uint8_t unused_2    : 1;      // Bit not used 
    uint8_t self_test   : 1;      // Self-test enable 
    uint8_t int_mag     : 1;      // DRDY pin configured as digital output 
}
lsm303agr_m_cfgc_t; 


// Magnetometer status register 
typedef struct lsm303agr_m_status_s 
{
    uint8_t zyx_or : 1;      // XYZ-axis data overrun 
    uint8_t z_or   : 1;      // Z-axis data overrun 
    uint8_t y_or   : 1;      // Y-axis data overrun 
    uint8_t x_or   : 1;      // X-axis data overrun 
    uint8_t zyx_da : 1;      // XYZ-axis new data available 
    uint8_t z_da   : 1;      // Z-axis new data available 
    uint8_t y_da   : 1;      // Y-axis new data available 
    uint8_t x_da   : 1;      // X-axis new data available 
}
lsm303agr_m_status_t; 

//=======================================================================================


//=======================================================================================
// Data record 

// Magnetometer calculated heading correction equation components 
typedef struct lsm303agr_m_heading_offset_s
{
    double slope;            // Slope of linear offset equation 
    double intercept;        // Intercept of linear offset equation 
}
lsm303agr_m_heading_offset_t; 


// Data record structure 
typedef struct lsm303agr_driver_data_s 
{
    //==================================================
    // Dev 

    // Peripherals 
    I2C_TypeDef *i2c; 

    // Device info 
    uint8_t m_addr; 
    uint8_t a_addr; 

    // Magnetometer register data 
    lsm303agr_m_data_t_dev m_data_dev[NUM_AXES]; 
    lsm303agr_m_cfga_t_dev m_cfga_dev; 
    lsm303agr_m_cfgb_t_dev m_cfgb_dev; 
    lsm303agr_m_cfgc_t_dev m_cfgc_dev; 
    lsm303agr_m_status_t_dev m_status_dev; 

    // Magnetometer calculation info 
    lsm303agr_m_heading_offset_t heading_offset_eqns[LSM303AGR_M_NUM_DIR]; 
    double heading_gain; 

    // Accelerometer register data 
    
    //==================================================

    // Magnetometer register data 
    lsm303agr_m_data_t m_data; 
    lsm303agr_m_cfga_t m_cfga; 
    lsm303agr_m_cfgb_t m_cfgb; 
    lsm303agr_m_cfgc_t m_cfgc; 
    lsm303agr_m_status_t m_status; 

    // Magnetometer heading 
    int16_t heading;   // Filtered heading 

    // Status info 
    // 'status' --> bit 0: i2c status (see i2c_status_t) 
    //          --> bit 1: init status (WHO_AM_I) 
    //          --> bits 2-7: self test results 
    uint8_t status; 
}
lsm303agr_driver_data_t; 


// Driver data record instance 
static lsm303agr_driver_data_t lsm303agr_driver_data; 

//=======================================================================================


//=======================================================================================
// Datatypes 
//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Magnetometer heading offset equation generation 
 * 
 * @details 
 * 
 * @param offset_eqn 
 * @param dir1_offset 
 * @param dir2_offset 
 * @param dir1_heading 
 */
void lsm303agr_m_head_offset_eqn(
    lsm303agr_m_heading_offset_t *offset_eqn, 
    int16_t dir1_offset, 
    int16_t dir2_offset, 
    int16_t dir1_heading); 


/**
 * @brief Read from register 
 * 
 * @details 
 * 
 * @param i2c_addr 
 * @param reg_addr 
 * @param lsm303agr_reg_value 
 * @param lsm303agr_reg_size 
 */
void lsm303agr_read(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_reg_size); 


/**
 * @brief Write to register 
 * 
 * @details 
 * 
 * @param i2c_addr 
 * @param reg_addr 
 * @param lsm303agr_reg_value 
 * @param lsm303agr_reg_size 
 */
void lsm303agr_write(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_reg_size); 


/**
 * @brief Magnetometer WHO AM I read 
 * 
 * @details 
 * 
 * @return uint8_t : contents of the WHO AM I register 
 */
uint8_t lsm303agr_m_whoami_read(void); 


/**
 * @brief Magnetometer configuration register A write/read 
 * 
 * @details 
 */
void lsm303agr_m_cfga_write(void); 


/**
 * @brief Magnetometer configuration register B write/read 
 * 
 * @details 
 */
void lsm303agr_m_cfgb_write(void); 


/**
 * @brief Magnetometer configuration register C write/read 
 * 
 * @details 
 */
void lsm303agr_m_cfgc_write(void); 


/**
 * @brief Magnetometer status register read 
 * 
 * @details 
 */
void lsm303agr_m_status_read(void); 


//==================================================
// Dev 

/**
 * @brief Read from register 
 * 
 * @details 
 * 
 * @param i2c_addr 
 * @param reg_addr 
 * @param lsm303agr_reg_value 
 * @param lsm303agr_data_size 
 * @return I2C_STATUS 
 */
I2C_STATUS lsm303agr_read_dev(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size); 


/**
 * @brief Write to register 
 * 
 * @details 
 * 
 * @param i2c_addr 
 * @param reg_addr 
 * @param lsm303agr_reg_value 
 * @param lsm303agr_data_size 
 * @return I2C_STATUS 
 */
I2C_STATUS lsm303agr_write_dev(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size); 


/**
 * @brief Magnetometer register write 
 * 
 * @details 
 * 
 * @param reg_addr 
 * @param reg_data 
 * @return LSM303AGR_STATUS 
 */
LSM303AGR_STATUS lsm303agr_m_reg_write_dev(
    uint8_t reg_addr, 
    uint8_t *reg_data); 


/**
 * @brief Magnetometer register read 
 * 
 * @details 
 * 
 * @param reg_addr 
 * @param reg_buff 
 * @param data_size : 
 * @return LSM303AGR_STATUS 
 */
LSM303AGR_STATUS lsm303agr_m_reg_read_dev(
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    byte_num_t data_size); 

//==================================================

//=======================================================================================


//=======================================================================================
// Initialization 

// Initialization 
void lsm303agr_init(
    I2C_TypeDef *i2c, 
    const int16_t *offsets, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag)
{
    // Local variables 
    const int16_t *offset1 = offsets;           // Copy of offsets array 
    const int16_t *offset2 = offsets + 1;       // Copy of offsets array at next address 

    // Initialize data record 
    // Peripherals 
    lsm303agr_driver_data.i2c = i2c; 
    // Device info 
    lsm303agr_driver_data.m_addr = LSM303AGR_M_ADDR; 
    // Status info 
    lsm303agr_driver_data.status = CLEAR;  

    // Generate magnetometer heading correction equations 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[0], *offset1++, *offset2++, LSM303AGR_M_N); 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[1], *offset1++, *offset2++, LSM303AGR_M_NE); 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[2], *offset1++, *offset2++, LSM303AGR_M_E); 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[3], *offset1++, *offset2++, LSM303AGR_M_SE); 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[4], *offset1++, *offset2++, LSM303AGR_M_S); 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[5], *offset1++, *offset2++, LSM303AGR_M_SW); 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[6], *offset1++, *offset2++, LSM303AGR_M_W); 
    lsm303agr_m_head_offset_eqn(
        &lsm303agr_driver_data.heading_offset_eqns[7], *offset1, *offsets, LSM303AGR_M_NW); 

    lsm303agr_driver_data.heading = CLEAR; 

    // Magnetometer parameters 
    lsm303agr_driver_data.m_cfga.comp_temp_en = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfga.reboot = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfga.soft_rst = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfga.lp = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfga.odr0 = (uint8_t)m_odr & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_cfga.odr1 = ((uint8_t)m_odr >> SHIFT_1) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_cfga.md0 = (uint8_t)m_mode & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_cfga.md1 = ((uint8_t)m_mode >> SHIFT_1) & LSM303AGR_BIT_MASK; 

    lsm303agr_driver_data.m_cfgb.unused_1 = CLEAR; 
    lsm303agr_driver_data.m_cfgb.off_canc_one_shot = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgb.int_on_dataoff = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgb.set_freq = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgb.off_canc = (uint8_t)m_off_canc; 
    lsm303agr_driver_data.m_cfgb.lpf = (uint8_t)m_lpf; 

    lsm303agr_driver_data.m_cfgc.unused_1 = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgc.int_mag_pin = (uint8_t)m_int_mag_pin; 
    lsm303agr_driver_data.m_cfgc.i2c_dis = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgc.bdu = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgc.ble = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgc.unused_2 = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgc.self_test = CLEAR_BIT; 
    lsm303agr_driver_data.m_cfgc.int_mag = (uint8_t)m_int_mag; 
    
    // Check ID and configure device 
    // Check WHO AM I 
    if (lsm303agr_m_whoami_read() != LSM303AGR_M_ID)
    {
        lsm303agr_driver_data.status |= (SET_BIT << SHIFT_1); 
        return; 
    }

    // Config magnetometer 
    lsm303agr_m_cfga_write(); 
    lsm303agr_m_cfgb_write(); 
    lsm303agr_m_cfgc_write(); 
}


// Magnetometer heading offset equation generation 
void lsm303agr_m_head_offset_eqn(
    lsm303agr_m_heading_offset_t *offset_eqn, 
    int16_t dir1_offset, 
    int16_t dir2_offset, 
    int16_t dir1_heading)
{
    // Calculate the slope and intercept of the linear equation used to correct calculated 
    // magnetometer headings between two directions 45deg apart (ex. between N and NE). 
    offset_eqn->slope = (double)(dir2_offset - dir1_offset) / LSM303AGR_M_DIR_OFFSET; 
    offset_eqn->intercept = (double)dir1_offset - (offset_eqn->slope * (double)dir1_heading); 
}


//==================================================
// Dev 

// Magnetometer initialization 
LSM303AGR_STATUS lsm303agr_m_init_dev(
    I2C_TypeDef *i2c, 
    const int16_t *offsets, 
    double heading_lpf_gain, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag)
{
    if ((i2c == NULL) || (offsets == NULL))
    {
        return LSM303AGR_INVALID_PTR; 
    }

    LSM303AGR_STATUS init_status = LSM303AGR_OK; 
    uint8_t whoami_status = CLEAR; 

    // Initialize driver data 
    lsm303agr_driver_data.i2c = i2c; 
    lsm303agr_driver_data.m_addr = LSM303AGR_M_ADDR; 
    lsm303agr_driver_data.heading_gain = heading_lpf_gain; 
    // Configuration register A 
    lsm303agr_driver_data.m_cfga_dev.cfga_reg = CLEAR; 
    lsm303agr_driver_data.m_cfga_dev.md = m_mode; 
    lsm303agr_driver_data.m_cfga_dev.odr = m_odr; 
    // Configuration register B 
    lsm303agr_driver_data.m_cfgb_dev.cfgb_reg = CLEAR; 
    lsm303agr_driver_data.m_cfgb_dev.lpf = m_lpf; 
    lsm303agr_driver_data.m_cfgb_dev.off_canc = m_off_canc; 
    // Configuration register C 
    lsm303agr_driver_data.m_cfgc_dev.cfgc_reg = CLEAR; 
    lsm303agr_driver_data.m_cfgc_dev.int_mag = m_int_mag; 
    lsm303agr_driver_data.m_cfgc_dev.int_mag_pin = m_int_mag_pin; 

    // Generate heading offset equations 
    lsm303agr_m_heading_calibration_dev(offsets); 
    
    // Identify the device 
    init_status |= lsm303agr_m_reg_read_dev(LSM303AGR_M_WHO_AM_I, &whoami_status, BYTE_1); 
    
    if (whoami_status != LSM303AGR_M_ID)
    {
        init_status = LSM303AGR_WHOAMI; 
    }
    
    // Configure magnetometer 
    init_status |= lsm303agr_m_reg_write_dev(
                        LSM303AGR_M_CFG_A, &lsm303agr_driver_data.m_cfga_dev.cfga_reg); 
    init_status |= lsm303agr_m_reg_write_dev(
                        LSM303AGR_M_CFG_B, &lsm303agr_driver_data.m_cfgb_dev.cfgb_reg); 
    init_status |= lsm303agr_m_reg_write_dev(
                        LSM303AGR_M_CFG_C, &lsm303agr_driver_data.m_cfgc_dev.cfgc_reg); 

    return init_status; 
}


// Calibrate the magnetometer heading - generate heading offset equations 
void lsm303agr_m_heading_calibration_dev(const int16_t *offsets)
{
    if (offsets == NULL)
    {
        return; 
    }

    lsm303agr_m_heading_offset_t *offset_eqn = lsm303agr_driver_data.heading_offset_eqns; 
    double delta1, delta2, heading = LSM303AGR_M_N; 
    const double dir_spacing = LSM303AGR_M_DIR_OFFSET; 
    uint8_t last = LSM303AGR_M_NUM_DIR - 1; 

    // Calculate the slope and intercept of the linear equation used to adjust calculated 
    // magnetometer headings between two directions. 
    for (uint8_t i = CLEAR; i < last; i++)
    {
        delta1 = offsets[i]; 
        delta2 = offsets[i + 1]; 
        
        offset_eqn[i].slope = dir_spacing / (dir_spacing + delta1 - delta2); 
        offset_eqn[i].intercept = heading - offset_eqn[i].slope*(heading - delta1); 

        heading += dir_spacing; 
    }

    // The heading loops back to 0/360 degrees so the final offset direction is also the 
    // first (index 0). 
    offset_eqn[last].slope = dir_spacing / (dir_spacing + delta2 - offsets[BYTE_0]); 
    offset_eqn[last].intercept = heading - offset_eqn[last].slope*(heading - delta2); 
}

//==================================================

//=======================================================================================


//=======================================================================================
// User functions 

// Magnetometer data read 
void lsm303agr_m_read(void)
{
    // Local variables 
    uint8_t mag_data[BYTE_6]; 

    // Read the magnetometer data 
    // The MSB of the register address is set to 1 so the register address will 
    // automatically be incremented between each byte read 
    lsm303agr_read(lsm303agr_driver_data.m_addr, LSM303AGR_M_X_L | LSM303AGR_ADDR_INC, mag_data, BYTE_6); 

    // Combine the return values into signed integers 
    // These values are raw magnetometer axis readings and have units of milli-gauss 
    lsm303agr_driver_data.m_data.m_x = (int16_t)((mag_data[1] << SHIFT_8) | (mag_data[0]));
    lsm303agr_driver_data.m_data.m_y = (int16_t)((mag_data[3] << SHIFT_8) | (mag_data[2]));
    lsm303agr_driver_data.m_data.m_z = (int16_t)((mag_data[5] << SHIFT_8) | (mag_data[4]));
}


// Get magnetometer data 
void lsm303agr_m_get_data(
    int16_t *m_x_data, 
    int16_t *m_y_data, 
    int16_t *m_z_data)
{
    // Takes the raw axis reading and scale it by the magnetometer sensitivity (1.5) 
    *m_x_data = (lsm303agr_driver_data.m_data.m_x * LSM303AGR_M_SENS) >> SHIFT_1; 
    *m_y_data = (lsm303agr_driver_data.m_data.m_y * LSM303AGR_M_SENS) >> SHIFT_1; 
    *m_z_data = (lsm303agr_driver_data.m_data.m_z * LSM303AGR_M_SENS) >> SHIFT_1; 
}


// Heading calculation (compass) 
int16_t lsm303agr_m_get_heading(void)
{
    // Local variables 
    int16_t m_x_data, m_y_data, m_z_data; 
    int16_t heading = CLEAR; 
    double heading_temp = CLEAR; 
    int16_t *heading_ptr = NULL; 
    int16_t inflection = CLEAR; 
    uint8_t eqn_index = CLEAR; 
    int16_t eqn_check = CLEAR; 

    // Get the magnetometer data 
    lsm303agr_m_get_data(&m_x_data, &m_y_data, &m_z_data); 

    // Check for potential divide by zero errors. 
    // If an axis is zero we don't want to assume a direction and return the result because 
    // the device data is not 100% accurate. 
    if (m_x_data == 0)
    {
        m_x_data++; 
    }

    // Calculate the heading based on the Y and X components read from the magnetometer. For 
    // this to work properly, the X-axis (long edge of the lsm303agr board) must be oriented 
    // in the forward direction. The heading is calculated using atan2 which is the inverse 
    // tangent of Y/X. atan2 accounts of Y and X signs so the proper quadrant is used and 
    // therefore the proper angle us returned. The returned angle us between +/-pi but this 
    // gets converted to degrees and scaled to eliminate decimal place values. 
    heading = (int16_t)((atan2((double)m_y_data, 
                               (double)m_x_data) * RAD_TO_DEG) * LSM303AGR_M_HEAD_SCALE); 

    // atan2 produces an angle that is positive in the counter clockwise direction (starts 
    // counting up from 0 degrees / forward axis counter clockwise). However, in this 
    // scenario (typical compass headings), the angle is positive in the clockwise 
    // direction so to correct this we invert the sign on the calculated heading. 
    heading = ~heading + 1; 

    // For navigation we want a 0-360 degree heading as opposed to a +/-180 degree 
    // heading so if the calculated heading is negative then we correct it to be positive. 
    if (heading < 0)
    {
        heading += LSM303AGR_M_HEAD_MAX; 
    }

    // Correct the calculated heading to align with the true magnetic heading. The number of 
    // times 45 degrees goes into the heading is found to know which correction equation to 
    // use. 
    while (eqn_check < heading)
    {
        eqn_index++; 
        eqn_check += LSM303AGR_M_DIR_OFFSET; 
    }

    heading_temp = (double)heading; 
    heading += (int16_t)(lsm303agr_driver_data.heading_offset_eqns[eqn_index-1].slope*heading_temp + 
                         lsm303agr_driver_data.heading_offset_eqns[eqn_index-1].intercept); 

    // After the heading has been corrected for offsets there is again a possibility the 
    // heading is negative so we must correct it to make sure the range is 0-360 degrees. 
    // If the heading corrections were set up around +/-180 degrees then this step would 
    // only have to be run once here and not before the correction as well. 
    if (heading < 0)
    {
        heading += LSM303AGR_M_HEAD_MAX; 
    }

    // Low pass filter the heading signal 
    // The heading provides circular data meaning it goes from 0 up to 359.9 then back to 
    // 0 again. Going directly from 0 to 359.9 (counter clockwise rotation) or vice versa 
    // will make a filter think there was a spike in the data which is not desired. To 
    // correct for this, the difference between the new sample and old filtered sample is 
    // checked. If it's magnitude is greater than 1800 (180 degrees) then we add 3600 
    // (360 degrees) before filtering (this assumes the device will not properly rotate 
    // more than 180 degrees between two consecutive samples). Adding 3600 tricks the 
    // filter into adjusting the heading in the correct direction. After filtering the 3600 
    // is removed if the filtered heading is above the 360 degree limit. 
    
    inflection = heading - lsm303agr_driver_data.heading; 

    if ((inflection > LSM303AGR_M_HEAD_DIFF) || (inflection < -LSM303AGR_M_HEAD_DIFF))
    {
        // Adjust the appropriate heading (new or old filtered) 
        heading_ptr = (heading < lsm303agr_driver_data.heading) ? 
                       &heading : &lsm303agr_driver_data.heading; 
        *heading_ptr += LSM303AGR_M_HEAD_MAX; 
    }

    // Find the new filtered heading 
    lsm303agr_driver_data.heading += 
        (int16_t)(((double)(heading - lsm303agr_driver_data.heading))*LSM303AGR_M_GAIN); 

    // If the filtered heading is outside the acceptable heading range then revert 
    // the adjustment 
    if (lsm303agr_driver_data.heading >= LSM303AGR_M_HEAD_MAX)
    {
        lsm303agr_driver_data.heading -= LSM303AGR_M_HEAD_MAX; 
    }

    return lsm303agr_driver_data.heading; 
}


//==================================================
// Dev 

// Get the most recent magnetometer data 
LSM303AGR_STATUS lsm303agr_m_update_dev(void)
{
    // Read the magnetometer axis data (units: milli-gauss). The LSM303AGR_ADDR_INC mask 
    // is used to increment to the next register address after each byte read. 
    return lsm303agr_m_reg_read_dev(LSM303AGR_M_X_L | LSM303AGR_ADDR_INC, 
                                    lsm303agr_driver_data.m_data_dev[X_AXIS].m_axis_bytes, 
                                    BYTE_6); 
}


// Get magnetometer axis data 
void lsm303agr_m_get_axis_data_dev(int16_t *m_axis_data)
{
    if (m_axis_data == NULL)
    {
        return; 
    }

    memcpy((void *)m_axis_data, (void *)lsm303agr_driver_data.m_data_dev, BYTE_6); 
}


// Get magnetometer applied magnetic field reading for each axis 
void lsm303agr_m_get_field_dev(int32_t *m_field_data)
{
    if (m_field_data == NULL)
    {
        return; 
    }

    lsm303agr_m_data_t_dev *m_data = lsm303agr_driver_data.m_data_dev; 
    const int32_t sens_num = LSM303AGR_M_SENS_NUM; 
    const int32_t sens_den = LSM303AGR_M_SENS_DEN; 

    // Divide the axis data by the magnetometer sensitivity to get the applied magnetic 
    // field on each axis. Per the datasheet, the magnetic sensitivity after factory 
    // calibration test and trim is 1.5 +/-7% mgauss/LSB. See the datasheet for further 
    // details on sensitivity. 
    m_field_data[X_AXIS] = ((int32_t)m_data[X_AXIS].m_axis * sens_num) / sens_den; 
    m_field_data[Y_AXIS] = ((int32_t)m_data[Y_AXIS].m_axis * sens_num) / sens_den; 
    m_field_data[Z_AXIS] = ((int32_t)m_data[Z_AXIS].m_axis * sens_num) / sens_den; 
}


// Get magnetometer (compass) heading 
int16_t lsm303agr_m_get_heading_dev(void)
{
    static int16_t heading_stored = CLEAR; 
    int16_t heading, heading_check = CLEAR, heading_diff; 
    double atan2_calc, slope = CLEAR, intercept = CLEAR; 

    // If the x-axis is zero then it's adjusted by 0.1 degrees to prevent a divide by zero 
    // error and the heading calculation is done anyway. This adjustment is considered 
    // negligible and this way the filtered heading is still updated. 
    if (!lsm303agr_driver_data.m_data_dev[X_AXIS].m_axis)
    {
        lsm303agr_driver_data.m_data_dev[X_AXIS].m_axis++; 
    }

    // printf("\r\nY-axis: %d", lsm303agr_driver_data.m_data_dev[Y_AXIS].m_axis); 
    // printf("\r\nX-axis: %d\r\n", lsm303agr_driver_data.m_data_dev[X_AXIS].m_axis); 

    // Using the Y and X axis data read from the magnetometer, the heading relative to 
    // magnetic north is calculated. For this to work properly, the X-axis must be 
    // oriented in the forward direction and the Z-axis must be vertical. The heading is 
    // calculated using atan2 which is the inverse tangent that accounts for the sign of 
    // Y and X. The returned angle is a double in the range +/-pi but 0-359.9 degrees is 
    // required so it gets converted to degrees, scaled by 10 to maintain one decimal 
    // place of accuracy and shifted to be a positive value if less than zero. The 
    // returned angle is also positive in the counter clockwise direction, however the 
    // opposite is needed so the sign is inverted. 
    atan2_calc = atan2((double)lsm303agr_driver_data.m_data_dev[Y_AXIS].m_axis, 
                       (double)lsm303agr_driver_data.m_data_dev[X_AXIS].m_axis); 
    
    // printf("\r\natan2: %f\r\n", atan2_calc); 

    // heading = -(int16_t)(atan2_calc * RAD_TO_DEG * SCALE_10); 
    heading = (int16_t)(atan2_calc * RAD_TO_DEG * SCALE_10); 
    
    // printf("\r\nheading: %d\r\n", heading); 

    if (heading < 0)
    {
        heading += LSM303AGR_M_HEAD_MAX; 
    }

    // printf("\r\nheading: %d\r\n", heading); 

    // To account for errors in the data read from the device, an offset is added to the 
    // calculated heading based on the direction it's pointing (see the 
    // lsm303agr_m_heading_calibration function). The offset is a linear interpolation 
    // between each 45 degree heading interval (ex. 0, 45, 90, etc.). To find the region 
    // the calculated heading is in, the number of times 45 degrees goes into the heading 
    // if found which then provides the index of the equation to use. 
    for (uint8_t i = CLEAR; i < LSM303AGR_M_NUM_DIR; i++)
    {
        heading_check += LSM303AGR_M_DIR_OFFSET; 

        if (heading < heading_check)
        {
            slope = lsm303agr_driver_data.heading_offset_eqns[i].slope; 
            intercept = lsm303agr_driver_data.heading_offset_eqns[i].intercept; 
            break; 
        }
    }

    // heading += (int16_t)(slope*(double)heading + intercept); 
    heading = (int16_t)(slope*(double)heading + intercept); 

    // printf("\r\nslope: %f", slope); 
    // printf("\r\nintercept: %f", intercept); 
    // printf("\r\nheading: %d\r\n", heading); 

    // The calculated heading is put through a low pass filter because the data read 
    // from the device has lots of noise (the gain of the filter can be set during init). 
    // The heading data is circular meaning it increments from 359.9 to 0 degrees after 
    // one full rotation. This jump in data will make the filter think there was a sudden 
    // change in direction which is not true so this must be addressed before filtering 
    // the heading. To correct for this, the difference between the new and previously 
    // calculated headings is checked. If the magnitude exceeds +/-180 degrees then 
    // -/+360 degrees is added to find the true heading change. After the heading is 
    // filtered/updated, if it falls outside of the acceptable range then it's moved 
    // back within range (while still maintaining the same heading). 
    heading_diff = heading - heading_stored; 

    if (heading_diff > LSM303AGR_M_HEAD_DIFF)
    {
        heading_diff -= LSM303AGR_M_HEAD_MAX; 
    }
    else if (heading_diff < -LSM303AGR_M_HEAD_DIFF)
    {
        heading_diff += LSM303AGR_M_HEAD_MAX; 
    }

    heading_stored += (int16_t)((double)heading_diff*lsm303agr_driver_data.heading_gain); 

    if (heading_stored >= LSM303AGR_M_HEAD_MAX)
    {
        heading_stored -= LSM303AGR_M_HEAD_MAX; 
    }
    else if (heading_stored < 0)
    {
        heading_stored += LSM303AGR_M_HEAD_MAX; 
    }

    return heading_stored; 
}

//==================================================

//=======================================================================================


//=======================================================================================
// Read and write 

// Read from register 
void lsm303agr_read(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    // Local variables 
    uint8_t i2c_status = I2C_OK; 

    // Generate a start condition 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the slave address with a write bit 
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          i2c_addr + I2C_W_OFFSET); 
    i2c_clear_addr(lsm303agr_driver_data.i2c); 

    // Send the register address that is going to be read 
    i2c_status |= (uint8_t)i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1);

    // Create another start signal 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the LSM303AGR address with a read offset 
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          i2c_addr + I2C_R_OFFSET);
    
    // Read the data sent by the MPU6050 
    i2c_status |= (uint8_t)i2c_read(lsm303agr_driver_data.i2c, 
                                    lsm303agr_reg_value, 
                                    lsm303agr_data_size);

    // Generate a stop condition 
    i2c_stop(lsm303agr_driver_data.i2c); 

    // Update the driver status 
    lsm303agr_driver_data.status |= i2c_status; 
}


// Write to register 
void lsm303agr_write(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    // Local variables 
    uint8_t i2c_status = I2C_OK; 
    
    // Create start condition to initiate master mode 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the MPU6050 address with a write offset
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          i2c_addr + I2C_W_OFFSET);
    i2c_clear_addr(lsm303agr_driver_data.i2c);

    // Send the register address that is going to be written to 
    i2c_status |= (uint8_t)i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1);

    // Write the data to the MPU6050 
    i2c_status |= (uint8_t)i2c_write(lsm303agr_driver_data.i2c, 
                                     lsm303agr_reg_value, 
                                     lsm303agr_data_size);

    // Create a stop condition
    i2c_stop(lsm303agr_driver_data.i2c); 

    // Update the driver status 
    lsm303agr_driver_data.status |= i2c_status; 
}


//==================================================
// Dev 

// Read from register 
I2C_STATUS lsm303agr_read_dev(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Generate a start condition, send the slave address with a write offset, then sned 
    // the slave's register address to read. 
    i2c_status |= i2c_start(lsm303agr_driver_data.i2c); 
    i2c_status |= i2c_write_addr(lsm303agr_driver_data.i2c, i2c_addr + I2C_W_OFFSET); 
    i2c_clear_addr(lsm303agr_driver_data.i2c); 
    i2c_status |= i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1); 

    // Generate another start condition, send the slave address with a read offset and 
    // finally read the device data before stopping the transaction. 
    i2c_status |= i2c_start(lsm303agr_driver_data.i2c); 
    i2c_status |= i2c_write_addr(lsm303agr_driver_data.i2c, i2c_addr + I2C_R_OFFSET);
    i2c_status |= i2c_read(lsm303agr_driver_data.i2c, lsm303agr_reg_value, lsm303agr_data_size); 
    i2c_stop(lsm303agr_driver_data.i2c); 

    return i2c_status; 
}


// Write to register 
I2C_STATUS lsm303agr_write_dev(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 
    
    // Generate a start condition, send the slave address with a write offset, then sned 
    // the slave's register address to write to. 
    i2c_status |= i2c_start(lsm303agr_driver_data.i2c); 
    i2c_status |= i2c_write_addr(lsm303agr_driver_data.i2c, i2c_addr + I2C_W_OFFSET);
    i2c_clear_addr(lsm303agr_driver_data.i2c);
    i2c_status |= i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1); 

    // Write the data to the device then stop the transaction. 
    i2c_status |= i2c_write(lsm303agr_driver_data.i2c, lsm303agr_reg_value, lsm303agr_data_size);
    i2c_stop(lsm303agr_driver_data.i2c); 

    return i2c_status; 
}

//==================================================

//=======================================================================================


//=======================================================================================
// Register functions 

// Magnetometer WHO AM I read 
uint8_t lsm303agr_m_whoami_read(void)
{
    // Local variables 
    uint8_t who_am_i; 

    // Read and return the value of the WHO AM I register 
    lsm303agr_read(lsm303agr_driver_data.m_addr, LSM303AGR_M_WHO_AM_I, &who_am_i, BYTE_1); 

    return who_am_i; 
}


// Magnetometer configuration register A write/read 
void lsm303agr_m_cfga_write(void)
{
    // Format the data 
    uint8_t cfga = (lsm303agr_driver_data.m_cfga.comp_temp_en << SHIFT_7) | 
                   (lsm303agr_driver_data.m_cfga.reboot       << SHIFT_6) | 
                   (lsm303agr_driver_data.m_cfga.soft_rst     << SHIFT_5) | 
                   (lsm303agr_driver_data.m_cfga.lp           << SHIFT_4) | 
                   (lsm303agr_driver_data.m_cfga.odr1         << SHIFT_3) | 
                   (lsm303agr_driver_data.m_cfga.odr0         << SHIFT_2) | 
                   (lsm303agr_driver_data.m_cfga.md1          << SHIFT_1) | 
                   (lsm303agr_driver_data.m_cfga.md0); 

    // Write the formatted data to the device 
    lsm303agr_write(lsm303agr_driver_data.m_addr, LSM303AGR_M_CFG_A, &cfga, BYTE_1); 
}


// Magnetometer configuration register B write/read 
void lsm303agr_m_cfgb_write(void)
{
    // Format the data 
    uint8_t cfgb = (lsm303agr_driver_data.m_cfgb.unused_1          << SHIFT_5) | 
                   (lsm303agr_driver_data.m_cfgb.off_canc_one_shot << SHIFT_4) | 
                   (lsm303agr_driver_data.m_cfgb.int_on_dataoff    << SHIFT_3) | 
                   (lsm303agr_driver_data.m_cfgb.set_freq          << SHIFT_2) | 
                   (lsm303agr_driver_data.m_cfgb.off_canc          << SHIFT_1) | 
                   (lsm303agr_driver_data.m_cfgb.lpf); 

    // Write the formatted data to the device 
    lsm303agr_write(lsm303agr_driver_data.m_addr, LSM303AGR_M_CFG_B, &cfgb, BYTE_1); 
}


// Magnetometer configuration register C write/read 
void lsm303agr_m_cfgc_write(void)
{
    // Format the data 
    uint8_t cfgc = (lsm303agr_driver_data.m_cfgc.unused_1    << SHIFT_7) | 
                   (lsm303agr_driver_data.m_cfgc.int_mag_pin << SHIFT_6) | 
                   (lsm303agr_driver_data.m_cfgc.i2c_dis     << SHIFT_5) | 
                   (lsm303agr_driver_data.m_cfgc.bdu         << SHIFT_4) | 
                   (lsm303agr_driver_data.m_cfgc.ble         << SHIFT_3) | 
                   (lsm303agr_driver_data.m_cfgc.unused_2    << SHIFT_2) | 
                   (lsm303agr_driver_data.m_cfgc.self_test   << SHIFT_1) | 
                   (lsm303agr_driver_data.m_cfgc.int_mag); 

    // Write the formatted data to the device 
    lsm303agr_write(lsm303agr_driver_data.m_addr, LSM303AGR_M_CFG_C, &cfgc, BYTE_1); 
}


// Magnetometer status register read 
void lsm303agr_m_status_read(void)
{
    // Local variables 
    uint8_t status; 

    // Read the magnetometer status 
    lsm303agr_read(lsm303agr_driver_data.m_addr, LSM303AGR_M_STATUS, &status, BYTE_1); 

    // Parse the data 
    lsm303agr_driver_data.m_status.zyx_or = (status >> SHIFT_7) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_status.z_or   = (status >> SHIFT_6) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_status.y_or   = (status >> SHIFT_5) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_status.x_or   = (status >> SHIFT_4) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_status.zyx_da = (status >> SHIFT_3) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_status.z_da   = (status >> SHIFT_2) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_status.y_da   = (status >> SHIFT_1) & LSM303AGR_BIT_MASK; 
    lsm303agr_driver_data.m_status.x_da   = (status) & LSM303AGR_BIT_MASK; 
}


//==================================================
// Dev 

// Magnetometer register write 
LSM303AGR_STATUS lsm303agr_m_reg_write_dev(
    uint8_t reg_addr, 
    uint8_t *reg_data)
{
    I2C_STATUS i2c_status = lsm303agr_write_dev(lsm303agr_driver_data.m_addr, 
                                                reg_addr, reg_data, BYTE_1); 
    if (i2c_status)
    {
        return LSM303AGR_WRITE_FAULT; 
    }

    return LSM303AGR_OK; 
}


// Magnetometer register read 
LSM303AGR_STATUS lsm303agr_m_reg_read_dev(
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    byte_num_t data_size)
{
    I2C_STATUS i2c_status = lsm303agr_read_dev(lsm303agr_driver_data.m_addr, 
                                               reg_addr, reg_buff, data_size); 
    if (i2c_status)
    {
        return LSM303AGR_READ_FAULT; 
    }

    return LSM303AGR_OK; 
}

//==================================================

//=======================================================================================


//=======================================================================================
// Status 

// LSM303AGR clear device driver fault flag 
void lsm303agr_clear_status(void)
{
    lsm303agr_driver_data.status = CLEAR; 
}


// LSM303AGR get device driver fault code 
uint8_t lsm303agr_get_status(void)
{
    return lsm303agr_driver_data.status; 
}

//=======================================================================================
