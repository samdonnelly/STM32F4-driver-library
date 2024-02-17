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
// - Procedures start on page 38 of the datasheet. 
// - The magnetometer works best with its XY plane parallel to the ground. The magnetometer 
//   features of this driver work under the assumption that the device is oriented this 
//   way with the x-axis pointing forward. There are X and Y marking on the device. 
//=======================================================================================


//=======================================================================================
// Macros 

// I2C addresses (datasheet page 39) 
#define LSM303AGR_A_ADDR 0x32         // Accelerometer (7-bit addr (0x19<<1) + Write bit) 
#define LSM303AGR_M_ADDR 0x3C         // Magnetometer (7-bit addr (0x1E<<1) + Write bit) 

// Magnetometer register addresses 
#define LSM303AGR_ADDR_INC 0x80       // Register address increment mask for r/w 
#define LSM303AGR_M_WHO_AM_I 0x4F     // WHO AM I 
#define LSM303AGR_M_CFG_A 0x60        // Configuration register A 
#define LSM303AGR_M_CFG_B 0x61        // Configuration register B 
#define LSM303AGR_M_CFG_C 0x62        // Configuration register C 
#define LSM303AGR_M_STATUS 0x67       // Status register 
#define LSM303AGR_M_X_L 0x68          // X component of magnetic field (first data reg) 

// Magnetometer data 
#define LSM303AGR_M_ID 0x40           // Value returned from the WHO_AM_I register 
#define LSM303AGR_M_SENS_NUM 3        // Magnetometer sensitivity numerator (3/2) 
#define LSM303AGR_M_SENS_DEN 2        // Magnetometer sensitivity denominator (3/2) 
#define LSM303AGR_M_HEAD_MAX 3600     // Max heading value - scaled (360deg * 10)
#define LSM303AGR_M_HEAD_DIFF 1800    // Heading different threshold for filtering 
#define LSM303AGR_M_DIR_OFFSET 450    // 45 degrees*10 - heading sections (ex. N-->NE) 

//=======================================================================================


//=======================================================================================
// Register data 

// Magnetometer data 
typedef union lsm303agr_m_data_s 
{
    // Axis data is a 16-bit value but broken down into low and high byte registers. 
    uint8_t m_axis_bytes[BYTE_2]; 
    int16_t m_axis; 
}
lsm303agr_m_data_t; 


// Magnetometer configuration register A 
typedef union lsm303agr_m_cfga_s 
{
    // Configuration register A bits 
    struct {
        uint8_t md           : 2;   // Mode select 
        uint8_t odr          : 2;   // Output data rate 
        uint8_t lp           : 1;   // Low-power mode enable 
        uint8_t soft_rst     : 1;   // Config and user register reset 
        uint8_t reboot       : 1;   // Reboot memory contents 
        uint8_t comp_temp_en : 1;   // Temperature compensation 
    }; 

    // CFG-A register byte 
    uint8_t cfga_reg; 
}
lsm303agr_m_cfga_t; 


// Magnetometer configuration register B 
typedef union lsm303agr_m_cfgb_s 
{
    // Configuration register C bits 
    struct {
        uint8_t lpf               : 1;   // Low pass filter enable 
        uint8_t off_canc          : 1;   // Enable offset cancellation 
        uint8_t set_freq          : 1;   // Frequency of set pulse 
        uint8_t int_on_dataoff    : 1;   // Interrupt check after hard-ron correction 
        uint8_t off_canc_one_shot : 1;   // Offset cancellation - single measurement mode 
        uint8_t unused_1          : 3;   // Not used 
    }; 

    // CFG-B register byte 
    uint8_t cfgb_reg; 
}
lsm303agr_m_cfgb_t; 


// Magnetometer configuration register C 
typedef union lsm303agr_m_cfgc_s 
{
    // Configuration register C bits 
    struct {
        uint8_t int_mag     : 1;   // DRDY pin configured as digital output 
        uint8_t self_test   : 1;   // Self-test enable 
        uint8_t unused_2    : 1;   // Bit not used 
        uint8_t ble         : 1;   // High and low data inversion 
        uint8_t bdu         : 1;   // Asynchronous read data protection 
        uint8_t i2c_dis     : 1;   // I2C disable 
        uint8_t int_mag_pin : 1;   // Interrupt on INT_MAG_PIN enable 
        uint8_t unused_1    : 1;   // Bit not used 
    }; 

    // CFG-C register byte 
    uint8_t cfgc_reg; 
}
lsm303agr_m_cfgc_t; 


// Magnetometer status register 
typedef union lsm303agr_m_status_s 
{
    // Status register bits 
    struct {
        uint8_t x_da   : 1;   // X-axis new data available 
        uint8_t y_da   : 1;   // Y-axis new data available 
        uint8_t z_da   : 1;   // Z-axis new data available 
        uint8_t zyx_da : 1;   // XYZ-axis new data available 
        uint8_t x_or   : 1;   // X-axis data overrun 
        uint8_t y_or   : 1;   // Y-axis data overrun 
        uint8_t z_or   : 1;   // Z-axis data overrun 
        uint8_t zyx_or : 1;   // XYZ-axis data overrun 
    }; 

    // Status register byte 
    uint8_t status_reg; 
}
lsm303agr_m_status_t; 

//=======================================================================================


//=======================================================================================
// Data record 

// Magnetometer heading correction equation components 
typedef struct lsm303agr_m_heading_offset_s
{
    double slope;            // Slope of linear offset equation 
    double intercept;        // Intercept of linear offset equation 
}
lsm303agr_m_heading_offset_t; 


// Driver data record 
typedef struct lsm303agr_driver_data_s 
{
    // Peripherals 
    I2C_TypeDef *i2c; 

    // Device info 
    uint8_t m_addr; 

    // Magnetometer register data 
    lsm303agr_m_data_t m_data[NUM_AXES]; 
    lsm303agr_m_cfga_t m_cfga; 
    lsm303agr_m_cfgb_t m_cfgb; 
    lsm303agr_m_cfgc_t m_cfgc; 
    lsm303agr_m_status_t m_status; 

    // Magnetometer calculation info 
    lsm303agr_m_heading_offset_t heading_offset_eqns[LSM303AGR_M_NUM_DIR]; 
    double heading_gain; 
    int16_t heading_offsets[LSM303AGR_M_NUM_DIR]; 
}
lsm303agr_driver_data_t; 


// Driver data record instance 
static lsm303agr_driver_data_t lsm303agr_driver_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Read from device register(s) 
 * 
 * @details Reads data of a given size from the device using the I2C driver functions 
 *          and stores the data in a buffer. The magnetometer and accelerometer have 
 *          different I2C addresses which must be specified. 
 * 
 * @param i2c_addr : I2C address to use (magnetometer or accelerometer) 
 * @param reg_addr : address of the first register to read from 
 * @param reg_buff : buffer to store the register contents 
 * @param data_size : size of the data to be read 
 * @return I2C_STATUS : I2C status of the read operation 
 */
I2C_STATUS lsm303agr_read(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    byte_num_t data_size); 


/**
 * @brief Write to device register(s) 
 * 
 * @details Writes data of a given size to the device using the I2C driver functions. 
 *          The magnetometer and accelerometer have different I2C addresses which must 
 *          be specified. 
 * 
 * @param i2c_addr : I2C address to use (magnetometer or accelerometer) 
 * @param reg_addr : address of the first register to write to 
 * @param reg_value : buffer that contains the data to write 
 * @param data_size : size of the data to write 
 * @return I2C_STATUS : I2C status of the write operation 
 */
I2C_STATUS lsm303agr_write(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *reg_value, 
    byte_num_t data_size); 


/**
 * @brief Magnetometer register read 
 * 
 * @details Reads register data of a specific size from the magnetometer and stores the 
 *          data in a buffer. Returns the status of the operation. 
 *          
 *          Note that LSM303AGR_ADDR_INC must be included with the register address in 
 *          order to read multiple different register bytes. 
 * 
 * @param reg_addr : address of the first register to read from 
 * @param reg_buff : buffer to store the register contents 
 * @param data_size : size of the data to be read 
 * @return LSM303AGR_STATUS : driver status of the read operation 
 */
LSM303AGR_STATUS lsm303agr_m_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    byte_num_t data_size); 


/**
 * @brief Magnetometer register write 
 * 
 * @details Writes 1-byte register data to the magnetometer. Returns the status of the 
 *          operation. 
 *          
 *          Note that LSM303AGR_ADDR_INC must be included with the register address in 
 *          order to write multiple different register bytes. 
 * 
 * @param reg_addr : address of the register to write to 
 * @param reg_data : buffer that contains the data to write 
 * @return LSM303AGR_STATUS : driver status of the write operation 
 */
LSM303AGR_STATUS lsm303agr_m_reg_write(
    uint8_t reg_addr, 
    uint8_t *reg_data); 

//=======================================================================================


//=======================================================================================
// Initialization 

// Magnetometer initialization 
LSM303AGR_STATUS lsm303agr_m_init(
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
    lsm303agr_driver_data.m_cfga.cfga_reg = CLEAR; 
    lsm303agr_driver_data.m_cfga.md = m_mode; 
    lsm303agr_driver_data.m_cfga.odr = m_odr; 
    // Configuration register B 
    lsm303agr_driver_data.m_cfgb.cfgb_reg = CLEAR; 
    lsm303agr_driver_data.m_cfgb.lpf = m_lpf; 
    lsm303agr_driver_data.m_cfgb.off_canc = m_off_canc; 
    // Configuration register C 
    lsm303agr_driver_data.m_cfgc.cfgc_reg = CLEAR; 
    lsm303agr_driver_data.m_cfgc.int_mag = m_int_mag; 
    lsm303agr_driver_data.m_cfgc.int_mag_pin = m_int_mag_pin; 

    // Generate heading offset equations 
    lsm303agr_m_heading_calibration(offsets); 
    
    // Identify the device 
    init_status |= lsm303agr_m_reg_read(LSM303AGR_M_WHO_AM_I, &whoami_status, BYTE_1); 
    
    if (whoami_status != LSM303AGR_M_ID)
    {
        init_status = LSM303AGR_WHOAMI; 
    }
    
    // Configure magnetometer 
    init_status |= lsm303agr_m_reg_write(
                        LSM303AGR_M_CFG_A, &lsm303agr_driver_data.m_cfga.cfga_reg); 
    init_status |= lsm303agr_m_reg_write(
                        LSM303AGR_M_CFG_B, &lsm303agr_driver_data.m_cfgb.cfgb_reg); 
    init_status |= lsm303agr_m_reg_write(
                        LSM303AGR_M_CFG_C, &lsm303agr_driver_data.m_cfgc.cfgc_reg); 

    return init_status; 
}


// Calibrate the magnetometer heading - generate heading offset equations 
void lsm303agr_m_heading_calibration(const int16_t *offsets)
{
    if (offsets == NULL)
    {
        return; 
    }

    lsm303agr_m_heading_offset_t *offset_eqn = lsm303agr_driver_data.heading_offset_eqns; 
    double delta1, delta2, heading = CLEAR; 
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
        lsm303agr_driver_data.heading_offsets[i] = delta1; 
    }

    // The heading loops back to 0/360 degrees so the final offset direction is also the 
    // first (index 0). 
    offset_eqn[last].slope = dir_spacing / (dir_spacing + delta2 - offsets[BYTE_0]); 
    offset_eqn[last].intercept = heading - offset_eqn[last].slope*(heading - delta2); 
    lsm303agr_driver_data.heading_offsets[last] = delta2; 
}

//=======================================================================================


//=======================================================================================
// User functions 

// Read the most recent magnetometer data 
LSM303AGR_STATUS lsm303agr_m_update(void)
{
    // Read the magnetometer axis data (units: milli-gauss). The LSM303AGR_ADDR_INC mask 
    // is used to increment to the next register address after each byte read. 
    return lsm303agr_m_reg_read(LSM303AGR_M_X_L | LSM303AGR_ADDR_INC, 
                                lsm303agr_driver_data.m_data[X_AXIS].m_axis_bytes, 
                                BYTE_6); 
}


// Get magnetometer axis data 
void lsm303agr_m_get_axis_data(int16_t *m_axis_data)
{
    if (m_axis_data == NULL)
    {
        return; 
    }

    memcpy((void *)m_axis_data, (void *)lsm303agr_driver_data.m_data, BYTE_6); 
}


// Get magnetometer applied magnetic field reading for each axis 
void lsm303agr_m_get_field(int32_t *m_field_data)
{
    if (m_field_data == NULL)
    {
        return; 
    }

    lsm303agr_m_data_t *m_data = lsm303agr_driver_data.m_data; 
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
int16_t lsm303agr_m_get_heading(void)
{
    static int16_t heading_stored = CLEAR; 
    int16_t heading, heading_check = CLEAR, heading_diff; 
    double atan2_calc, slope = CLEAR, intercept = CLEAR; 
    uint8_t last_offset_index = LSM303AGR_M_NUM_DIR - 1, offset_index = last_offset_index; 

    // If the x-axis is zero then it's adjusted by 0.1 degrees to prevent a divide by zero 
    // error and the heading calculation is done anyway. This adjustment is considered 
    // negligible and this way the filtered heading is still updated. 
    if (!lsm303agr_driver_data.m_data[X_AXIS].m_axis)
    {
        lsm303agr_driver_data.m_data[X_AXIS].m_axis++; 
    }

    // Using the Y and X axis data read from the magnetometer, the heading relative to 
    // magnetic north is calculated. For this to work properly, the X-axis must be 
    // oriented in the forward direction and the Z-axis must be vertical. The heading is 
    // calculated using atan2 which is the inverse tangent that accounts for the sign of 
    // Y and X. The returned angle is a double in the range +/-pi but 0-359.9 degrees is 
    // required so it gets converted to degrees, scaled by 10 to maintain one decimal 
    // place of accuracy and shifted to be a positive value if less than zero. The 
    // returned angle is also positive in the counter clockwise direction, however the 
    // opposite is needed so the sign is inverted. 
    atan2_calc = atan2((double)lsm303agr_driver_data.m_data[Y_AXIS].m_axis, 
                       (double)lsm303agr_driver_data.m_data[X_AXIS].m_axis); 
    heading = -(int16_t)(atan2_calc * RAD_TO_DEG * SCALE_10); 

    if (heading < 0)
    {
        heading += LSM303AGR_M_HEAD_MAX; 
    }

    // To account for errors in the data read from the device, an offset is added to the 
    // calculated heading based on the direction the device is pointing (see the 
    // lsm303agr_m_heading_calibration function). The correction is a linear interpolation 
    // between each (45*i - offset[i]) degree heading interval. To find the right 
    // equation, the calculated heading is checked against each interval. Once the 
    // interval where the heading resides is found then the index of that interval is 
    // used to apply the needed offset equation that adjusts the heading. Since the 
    // heading goes from 359.9 degrees back to 0, it's possible the final equation 
    // interval crosses this boundary, and therefore it needs it's own check before 
    // checking the other equations. 
    if (heading < (heading_check - lsm303agr_driver_data.heading_offsets[0]))
    {
        heading += LSM303AGR_M_HEAD_MAX; 
    }
    else if (heading < (LSM303AGR_M_HEAD_MAX - LSM303AGR_M_DIR_OFFSET - 
                        lsm303agr_driver_data.heading_offsets[last_offset_index]))
    {
        offset_index--; 

        for (uint8_t i = 1; i < last_offset_index; i++)
        {
            heading_check += LSM303AGR_M_DIR_OFFSET; 

            if (heading < (heading_check - lsm303agr_driver_data.heading_offsets[i]))
            {
                offset_index = i - 1; 
                break; 
            }
        }
    }

    slope = lsm303agr_driver_data.heading_offset_eqns[offset_index].slope; 
    intercept = lsm303agr_driver_data.heading_offset_eqns[offset_index].intercept; 
    heading = (int16_t)(slope*(double)heading + intercept); 

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

//=======================================================================================


//=======================================================================================
// Read and write 

// Read from register 
I2C_STATUS lsm303agr_read(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    byte_num_t data_size)
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
    i2c_status |= i2c_read(lsm303agr_driver_data.i2c, reg_buff, data_size); 
    i2c_stop(lsm303agr_driver_data.i2c); 

    return i2c_status; 
}


// Write to register 
I2C_STATUS lsm303agr_write(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *reg_value, 
    byte_num_t data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 
    
    // Generate a start condition, send the slave address with a write offset, then sned 
    // the slave's register address to write to. 
    i2c_status |= i2c_start(lsm303agr_driver_data.i2c); 
    i2c_status |= i2c_write_addr(lsm303agr_driver_data.i2c, i2c_addr + I2C_W_OFFSET);
    i2c_clear_addr(lsm303agr_driver_data.i2c);
    i2c_status |= i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1); 

    // Write the data to the device then stop the transaction. 
    i2c_status |= i2c_write(lsm303agr_driver_data.i2c, reg_value, data_size);
    i2c_stop(lsm303agr_driver_data.i2c); 

    return i2c_status; 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// Magnetometer register read 
LSM303AGR_STATUS lsm303agr_m_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    byte_num_t data_size)
{
    I2C_STATUS i2c_status = lsm303agr_read(lsm303agr_driver_data.m_addr, 
                                           reg_addr, reg_buff, data_size); 
    if (i2c_status)
    {
        return LSM303AGR_READ_FAULT; 
    }

    return LSM303AGR_OK; 
}


// Magnetometer register write 
LSM303AGR_STATUS lsm303agr_m_reg_write(
    uint8_t reg_addr, 
    uint8_t *reg_data)
{
    I2C_STATUS i2c_status = lsm303agr_write(lsm303agr_driver_data.m_addr, 
                                            reg_addr, reg_data, BYTE_1); 
    if (i2c_status)
    {
        return LSM303AGR_WRITE_FAULT; 
    }

    return LSM303AGR_OK; 
}

//=======================================================================================
