/**
 * @file lsm303agr_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR IMU driver 
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
// Macros 

// I2C addresses (datasheet page 39) 
#define LSM303AGR_A_ADDR 0x32         // Accelerometer (7-bit addr (0x19<<1) + Write bit) 
#define LSM303AGR_M_ADDR 0x3C         // Magnetometer (7-bit addr (0x1E<<1) + Write bit) 

// Registers 
#define LSM303AGR_ADDR_INC 0x80       // Register address increment mask for r/w 

// Magnetometer register addresses 
#define LSM303AGR_M_OFFSET_X_L 0x45      // X-axis hard-iron offset low byte 
#define LSM303AGR_M_WHO_AM_I 0x4F        // WHO AM I 
#define LSM303AGR_M_CFG_A 0x60           // Configuration register A 
#define LSM303AGR_M_CFG_B 0x61           // Configuration register B 
#define LSM303AGR_M_CFG_C 0x62           // Configuration register C 
#define LSM303AGR_M_STATUS 0x67          // Status register 
#define LSM303AGR_M_OUT_X_L 0x68         // X-axis data output low byte 

// Magnetometer data 
#define LSM303AGR_M_ID 0x40              // Value returned from the WHO_AM_I register 
// Ratio of magnetic field (mG) to digital output data 
#define LSM303AGR_M_SENS_I1 3            // Integer - numerator 
#define LSM303AGR_M_SENS_I2 2            // Integer - denomenator  
#define LSM303AGR_M_SENS_F 1.5f          // Float 
// Heading 
#define LSM303AGR_M_HEADING_NORTH 0      // Heading reading when facing North (0 deg*10) 
#define LSM303AGR_M_HEADING_RANGE 3600   // Full heading range (360 deg*10) 

//=======================================================================================


//=======================================================================================
// Register data 

// Magnetometer data 
typedef union lsm303agr_m_data_s 
{
    // Axis data is a 16-bit value but broken down into low and high byte registers. 
    // STM32 stores bytes in memory in little endian format. 
    uint8_t m_axis_bytes[BYTE_2]; 
    int16_t m_axis; 
}
lsm303agr_m_data_t; 


// Magnetometer configuration register A 
typedef union lsm303agr_m_cfga_s 
{
    // Configuration register A bits 
    struct 
    {
        uint8_t md           : 2;   // Bits 0-1: Mode select 
        uint8_t odr          : 2;   // Bits 2-3: Output data rate 
        uint8_t lp           : 1;   // Bit 4: Low-power mode enable 
        uint8_t soft_rst     : 1;   // Bit 5: Config and user register reset 
        uint8_t reboot       : 1;   // Bit 6: Reboot memory contents 
        uint8_t comp_temp_en : 1;   // Bit 7: Temperature compensation 
    }; 

    // CFG-A register byte 
    uint8_t cfga_reg; 
}
lsm303agr_m_cfga_t; 


// Magnetometer configuration register B 
typedef union lsm303agr_m_cfgb_s 
{
    // Configuration register C bits 
    struct 
    {
        uint8_t lpf               : 1;   // Bit 0: Low pass filter enable 
        uint8_t off_canc          : 1;   // Bit 1: Enable offset cancellation 
        uint8_t set_freq          : 1;   // Bit 2: Frequency of set pulse 
        uint8_t int_on_dataoff    : 1;   // Bit 3: Interrupt check after hard-ron correction 
        uint8_t off_canc_one_shot : 1;   // Bit 4: Offset cancellation - single measurement mode 
        uint8_t unused_1          : 3;   // Bits 5-7: Not used 
    }; 

    // CFG-B register byte 
    uint8_t cfgb_reg; 
}
lsm303agr_m_cfgb_t; 


// Magnetometer configuration register C 
typedef union lsm303agr_m_cfgc_s 
{
    // Configuration register C bits 
    struct 
    {
        uint8_t int_mag     : 1;   // Bit 0: DRDY pin configured as digital output 
        uint8_t self_test   : 1;   // Bit 1: Self-test enable 
        uint8_t unused_2    : 1;   // Bit 2: Bit not used 
        uint8_t ble         : 1;   // Bit 3: High and low data inversion 
        uint8_t bdu         : 1;   // Bit 4: Asynchronous read data protection 
        uint8_t i2c_dis     : 1;   // Bit 5: I2C disable 
        uint8_t int_mag_pin : 1;   // Bit 6: Interrupt on INT_MAG_PIN enable 
        uint8_t unused_1    : 1;   // Bit 7: Bit not used 
    }; 

    // CFG-C register byte 
    uint8_t cfgc_reg; 
}
lsm303agr_m_cfgc_t; 


// Magnetometer status register 
typedef union lsm303agr_m_status_s 
{
    // Status register bits 
    struct 
    {
        uint8_t x_da   : 1;   // Bit 0: X-axis new data available 
        uint8_t y_da   : 1;   // Bit 1: Y-axis new data available 
        uint8_t z_da   : 1;   // Bit 2: Z-axis new data available 
        uint8_t zyx_da : 1;   // Bit 3: XYZ-axis new data available 
        uint8_t x_or   : 1;   // Bit 4: X-axis data overrun 
        uint8_t y_or   : 1;   // Bit 5: Y-axis data overrun 
        uint8_t z_or   : 1;   // Bit 6: Z-axis data overrun 
        uint8_t zyx_or : 1;   // Bit 7: XYZ-axis data overrun 
    }; 

    // Status register byte 
    uint8_t status_reg; 
}
lsm303agr_m_status_t; 

//=======================================================================================


//=======================================================================================
// Data record 

// Driver data record 
typedef struct lsm303agr_driver_data_s 
{
    // Peripherals 
    I2C_TypeDef *i2c; 

    //==================================================
    // Magnetometer 
    
    // Device info 
    uint8_t m_addr; 
    
    // Magnetometer register data 
    lsm303agr_m_data_t m_data[NUM_AXES];   // Axis data registers 
    lsm303agr_m_cfga_t m_cfga;             // Configuration register A 
    lsm303agr_m_cfgb_t m_cfgb;             // Configuration register B 
    lsm303agr_m_cfgc_t m_cfgc;             // Configuration register C 
    lsm303agr_m_status_t m_status;         // Status register 
    
    // Magnetometer calculation info 
    float hi_offsets[NUM_AXES];            // Hard-iron offsets 
    float sid_values[NUM_AXES];            // Soft-iron diagonal values 
    float sio_values[NUM_AXES];            // Soft-iron off-diagonal values 
    
    //==================================================
}
lsm303agr_driver_data_t; 


// Driver data record instance 
static lsm303agr_driver_data_t lsm303agr_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Apply calibrated correction values to magnetometer axis data 
 * 
 * @param m_axis_data : buffer that contains axis data to be corrected 
 */
void lsm303agr_m_correct_axes(float *m_axis_data); 


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
    uint8_t data_size); 


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
    uint8_t data_size); 


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
 * @return LSM303AGR_STATUS : status of the read operation 
 */
LSM303AGR_STATUS lsm303agr_m_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    uint8_t data_size); 


/**
 * @brief Magnetometer register write 
 * 
 * @details Writes register data of a specified size to the magnetometer. 
 *          
 *          Note that LSM303AGR_ADDR_INC must be included with the register address in 
 *          order to write multiple different register bytes. 
 * 
 * @param reg_addr : address of the register to write to 
 * @param reg_data : buffer that contains the data to write 
 * @param data_size : size of data to write 
 * @return LSM303AGR_STATUS : status of the write operation 
 */
LSM303AGR_STATUS lsm303agr_m_reg_write(
    uint8_t reg_addr, 
    uint8_t *reg_data, 
    uint8_t data_size); 

//=======================================================================================


//=======================================================================================
// Initialization 

// Magnetometer initialization 
LSM303AGR_STATUS lsm303agr_m_init(
    I2C_TypeDef *i2c, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag)
{
    if (i2c == NULL)
    {
        return LSM303AGR_INVALID_PTR; 
    }

    LSM303AGR_STATUS init_status = LSM303AGR_OK; 
    uint8_t whoami_status = CLEAR; 
    
    //==================================================
    // Initialize data 
    
    lsm303agr_data.i2c = i2c; 
    lsm303agr_data.m_addr = LSM303AGR_M_ADDR; 
    
    lsm303agr_data.m_cfga.cfga_reg = CLEAR; 
    lsm303agr_data.m_cfga.md = m_mode; 
    lsm303agr_data.m_cfga.odr = m_odr; 
    
    lsm303agr_data.m_cfgb.cfgb_reg = CLEAR; 
    lsm303agr_data.m_cfgb.lpf = m_lpf; 
    lsm303agr_data.m_cfgb.off_canc = m_off_canc; 
    
    lsm303agr_data.m_cfgc.cfgc_reg = CLEAR; 
    lsm303agr_data.m_cfgc.int_mag = m_int_mag; 
    lsm303agr_data.m_cfgc.int_mag_pin = m_int_mag_pin; 
    
    memset((void *)lsm303agr_data.hi_offsets, CLEAR, sizeof(lsm303agr_data.hi_offsets)); 
    lsm303agr_data.sid_values[X_AXIS] = 1.0f; 
    lsm303agr_data.sid_values[Y_AXIS] = 1.0f; 
    lsm303agr_data.sid_values[Z_AXIS] = 1.0f; 
    memset((void *)lsm303agr_data.sio_values, CLEAR, sizeof(lsm303agr_data.sio_values)); 

    //==================================================
    
    // Identify the device 
    init_status |= lsm303agr_m_reg_read(LSM303AGR_M_WHO_AM_I, &whoami_status, BYTE_1); 
    
    if (whoami_status != LSM303AGR_M_ID)
    {
        init_status = LSM303AGR_WHOAMI; 
    }
    
    // Configure magnetometer 
    init_status |= lsm303agr_m_reg_write(LSM303AGR_M_CFG_A, &lsm303agr_data.m_cfga.cfga_reg, BYTE_1); 
    init_status |= lsm303agr_m_reg_write(LSM303AGR_M_CFG_B, &lsm303agr_data.m_cfgb.cfgb_reg, BYTE_1); 
    init_status |= lsm303agr_m_reg_write(LSM303AGR_M_CFG_C, &lsm303agr_data.m_cfgc.cfgc_reg, BYTE_1); 

    return init_status; 
}


// Set the hard-iron offset registers 
LSM303AGR_STATUS lsm303agr_m_offset_reg_set(const int16_t *offset_reg)
{
    lsm303agr_m_data_t hi_offsets_reg[NUM_AXES];

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (offset_reg != NULL); i++)
    {
        hi_offsets_reg[i].m_axis = (offset_reg[i] * LSM303AGR_M_SENS_I2) / LSM303AGR_M_SENS_I1;
    }

    return lsm303agr_m_reg_write(LSM303AGR_M_OFFSET_X_L | LSM303AGR_ADDR_INC, 
                                 hi_offsets_reg->m_axis_bytes, 
                                 BYTE_6); 
}


// Set the hard and soft-iron calibration values 
void lsm303agr_m_calibration_set(
    const float *hi_offsets, 
    const float *sid_values, 
    const float *sio_values)
{
    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (hi_offsets != NULL); i++)
    {
        lsm303agr_data.hi_offsets[i] = hi_offsets[i]; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (sid_values != NULL) && (sio_values != NULL); i++)
    {
        lsm303agr_data.sid_values[i] = sid_values[i]; 
        lsm303agr_data.sio_values[i] = sio_values[i]; 
    }
}

//=======================================================================================


//=======================================================================================
// Update and get data 

// Read the most recent magnetometer data 
LSM303AGR_STATUS lsm303agr_m_update(void)
{
    // Read the magnetometer axis data (units: milli-gauss). The LSM303AGR_ADDR_INC mask 
    // is used to increment to the next register address after each byte read. 
    return lsm303agr_m_reg_read(LSM303AGR_M_OUT_X_L | LSM303AGR_ADDR_INC, 
                                lsm303agr_data.m_data[X_AXIS].m_axis_bytes, 
                                BYTE_6);
}


// Get digital output magnetometer axis data 
void lsm303agr_m_get_axis_raw(int16_t *m_axis_buff)
{
    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (m_axis_buff != NULL); i++)
    {
        *m_axis_buff++ = lsm303agr_data.m_data[i].m_axis;
    }
}


// Get magnetometer axis data as integers in milligauss (mG) 
void lsm303agr_m_get_axis(int16_t *m_axis_buff)
{
    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (m_axis_buff != NULL); i++)
    {
        *m_axis_buff++ = (lsm303agr_data.m_data[i].m_axis * LSM303AGR_M_SENS_I1) / LSM303AGR_M_SENS_I2;
    }
}


// Get magnetometer axis data as floats in milligauss (mG) 
void lsm303agr_m_get_axis_f(float *m_axis_buff)
{
    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (m_axis_buff != NULL); i++)
    {
        *m_axis_buff++ = (float)lsm303agr_data.m_data[i].m_axis * LSM303AGR_M_SENS_F;
    }
}


// Get calibrated magnetometer axis data as integers in milligauss (mG) 
void lsm303agr_m_get_axis_cal(int16_t *m_cal_axis_buff)
{
    float mag_cal[NUM_AXES];
    lsm303agr_m_correct_axes(mag_cal);

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (m_cal_axis_buff != NULL); i++)
    {
        *m_cal_axis_buff++ = (int16_t)mag_cal[i];
    }
}


// Get calibrated magnetometer axis data as floats in milligauss (mG) 
void lsm303agr_m_get_axis_cal_f(float *m_cal_axis_buff)
{
    float mag_cal[NUM_AXES];
    lsm303agr_m_correct_axes(mag_cal);

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (m_cal_axis_buff != NULL); i++)
    {
        *m_cal_axis_buff++ = mag_cal[i];
    }
}


// Get magnetometer (compass) heading 
int16_t lsm303agr_m_get_heading(void)
{
    float mag_cal[NUM_AXES]; 
    int16_t heading = CLEAR; 

    lsm303agr_m_correct_axes(mag_cal);

    // Find the magnetic heading based on the magnetometer X and Y axis data. atan2f 
    // looks at the value and sign of X and Y to determine the correct output so axis 
    // values don't have to be checked for potential errors (ex. divide by zero). The 
    // sign on the calculated angle is inverted so that heading increases from 0 in the 
    // clockwise direction which aligns with the NED frame orientation. Following the 
    // NED orientation, X is assumed to be positive forward and Y positive right (check 
    // physical board orientation). 
    heading = (int16_t)(-atan2f(mag_cal[Y_AXIS], mag_cal[X_AXIS])*RAD_TO_DEG*SCALE_10);

    // Adjust the heading range. The magnetic heading is calculated within the range 
    // -180 to 180 degrees, however the returned heading needs to be in the range 0 to 
    // 360 degrees (or 0-3600 deg*10). 
    if (heading < LSM303AGR_M_HEADING_NORTH)
    {
        heading += LSM303AGR_M_HEADING_RANGE;
    }

    return heading; 
}


// Apply calibrated correction values to axis data 
void lsm303agr_m_correct_axes(float *m_axis_data)
{
    // Correct the magnetometer axis readings with calibration values. First the hard-
    // iron offsets are subtracted from the axis reading that come from the magnetometer, 
    // then soft-iron scale values are applied using a matrix multiplication. 

    lsm303agr_m_get_axis_f(m_axis_data);

    // Hard-iron offsets 
    float mag_off[NUM_AXES] = 
    {
        m_axis_data[X_AXIS] - lsm303agr_data.hi_offsets[X_AXIS],
        m_axis_data[Y_AXIS] - lsm303agr_data.hi_offsets[Y_AXIS],
        m_axis_data[Z_AXIS] - lsm303agr_data.hi_offsets[Z_AXIS]
    };

    // Soft-iron offsets 
    m_axis_data[X_AXIS] = (lsm303agr_data.sid_values[X_AXIS]*mag_off[X_AXIS]) + 
                          (lsm303agr_data.sio_values[X_AXIS]*mag_off[Y_AXIS]) + 
                          (lsm303agr_data.sio_values[Y_AXIS]*mag_off[Z_AXIS]);
    
    m_axis_data[Y_AXIS] = (lsm303agr_data.sio_values[X_AXIS]*mag_off[X_AXIS]) + 
                          (lsm303agr_data.sid_values[Y_AXIS]*mag_off[Y_AXIS]) + 
                          (lsm303agr_data.sio_values[Z_AXIS]*mag_off[Z_AXIS]);
    
    m_axis_data[Z_AXIS] = (lsm303agr_data.sio_values[Y_AXIS]*mag_off[X_AXIS]) + 
                          (lsm303agr_data.sio_values[Z_AXIS]*mag_off[Y_AXIS]) + 
                          (lsm303agr_data.sid_values[Z_AXIS]*mag_off[Z_AXIS]);
}

//=======================================================================================


//=======================================================================================
// Register functions 

// Magnetometer register read 
LSM303AGR_STATUS lsm303agr_m_reg_read(
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    uint8_t data_size)
{
    I2C_STATUS i2c_status = lsm303agr_read(lsm303agr_data.m_addr, reg_addr, reg_buff, data_size); 

    if (i2c_status)
    {
        return LSM303AGR_READ_FAULT; 
    }

    return LSM303AGR_OK; 
}


// Magnetometer register write 
LSM303AGR_STATUS lsm303agr_m_reg_write(
    uint8_t reg_addr, 
    uint8_t *reg_data, 
    uint8_t data_size)
{
    I2C_STATUS i2c_status = lsm303agr_write(lsm303agr_data.m_addr, reg_addr, reg_data, data_size); 

    if (i2c_status)
    {
        return LSM303AGR_WRITE_FAULT; 
    }

    return LSM303AGR_OK; 
}

//=======================================================================================


//=======================================================================================
// Read and write 

// Read from register 
I2C_STATUS lsm303agr_read(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *reg_buff, 
    uint8_t data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Generate a start condition, send the slave address with a write offset, then send 
    // the slave's register address to read. 
    i2c_status |= i2c_start(lsm303agr_data.i2c); 
    i2c_status |= i2c_write_addr(lsm303agr_data.i2c, i2c_addr + I2C_W_OFFSET); 
    i2c_clear_addr(lsm303agr_data.i2c); 
    i2c_status |= i2c_write(lsm303agr_data.i2c, &reg_addr, BYTE_1); 

    // Generate another start condition, send the slave address with a read offset and 
    // finally read the device data before stopping the transaction. 
    i2c_status |= i2c_start(lsm303agr_data.i2c); 
    i2c_status |= i2c_write_addr(lsm303agr_data.i2c, i2c_addr + I2C_R_OFFSET);
    i2c_status |= i2c_read(lsm303agr_data.i2c, reg_buff, (uint16_t)data_size); 
    i2c_stop(lsm303agr_data.i2c); 

    return i2c_status; 
}


// Write to register 
I2C_STATUS lsm303agr_write(
    uint8_t i2c_addr, 
    uint8_t reg_addr, 
    uint8_t *reg_value, 
    uint8_t data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 
    
    // Generate a start condition, send the slave address with a write offset, then sned 
    // the slave's register address to write to. 
    i2c_status |= i2c_start(lsm303agr_data.i2c); 
    i2c_status |= i2c_write_addr(lsm303agr_data.i2c, i2c_addr + I2C_W_OFFSET);
    i2c_clear_addr(lsm303agr_data.i2c);
    i2c_status |= i2c_write(lsm303agr_data.i2c, &reg_addr, BYTE_1); 

    // Write the data to the device then stop the transaction. 
    i2c_status |= i2c_write(lsm303agr_data.i2c, reg_value, data_size);
    i2c_stop(lsm303agr_data.i2c); 

    return i2c_status; 
}

//=======================================================================================
