/**
 * @file lsm303agr.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR accelerometer and magnetometer driver 
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

// File inclusion 
extern "C" 
{
    //===================================================
    // C functions 

    // I2C functions 
    I2C_STATUS i2c_start(I2C_TypeDef); 
    void i2c_stop(I2C_TypeDef); 
    void i2c_clear_addr(I2C_TypeDef); 
    I2C_STATUS i2c_write_addr(I2C_TypeDef, uint8_t); 
    I2C_STATUS i2c_write(I2C_TypeDef, uint8_t, uint8_t); 
    I2C_STATUS i2c_read(I2C_TypeDef, uint8_t, uint16_t); 
    
    //===================================================

    //===================================================
    // C++ functions 

    // void lsm303agr_init(
    //     I2C_TypeDef *, 
    //     lsm303agr_m_odr_cfg_t, 
    //     lsm303agr_m_sys_mode_t,
    //     lsm303agr_cfg_t, 
    //     lsm303agr_cfg_t, 
    //     lsm303agr_cfg_t, 
    //     lsm303agr_cfg_t); 

    //     // Read from register 
    //     void lsm303agr_read(LSM303AGR_REG_ADDR, uint8_t *, byte_num_t); 

    //     // Write to register 
    //     void lsm303agr_write(LSM303AGR_REG_ADDR, uint8_t *, byte_num_t); 

    //     // Magnetometer data read 
    //     void lsm303agr_m_read(void); 

    //     // Get magnetometer data 
    //     void lsm303agr_m_get_data(int16_t *, int16_t *, int16_t *); 

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Register functions 

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

//=======================================================================================


//=======================================================================================
// Notes 

// Procedures start on page 38 of the datasheet 

// Low power mode can achieved by setting the low power bit but also putting the device into 
// idle mode as well 

//=======================================================================================


//=======================================================================================
// Register data 

// Magnetometer data 
typedef struct lsm303agr_m_data_s 
{
    int16_t m_x;                             // 
    int16_t m_y;                             // 
    int16_t m_z;                             // 
}
lsm303agr_m_data_t; 


// Magnetometer configuration register A 
typedef struct lsm303agr_m_cfga_s 
{
    uint8_t comp_temp_en      : 1;           // 
    uint8_t reboot            : 1;           // 
    uint8_t soft_rst          : 1;           // 
    uint8_t lp                : 1;           // 
    uint8_t odr1              : 1;           // 
    uint8_t odr0              : 1;           // 
    uint8_t md1               : 1;           // 
    uint8_t md0               : 1;           // 
}
lsm303agr_m_cfga_t; 


// Magnetometer configuration register B 
typedef struct lsm303agr_m_cfgb_s 
{
    uint8_t unused_1          : 3;           // Not used 
    uint8_t off_canc_one_shot : 1;           // 
    uint8_t int_on_dataoff    : 1;           // 
    uint8_t set_freq          : 1;           // 
    uint8_t off_canc          : 1;           // 
    uint8_t lpf               : 1;           // 
}
lsm303agr_m_cfgb_t; 


// Magnetometer configuration register C 
typedef struct lsm303agr_m_cfgc_s 
{
    uint8_t unused_1    : 1;                 // Bit not used 
    uint8_t int_mag_pin : 1;                 // 
    uint8_t i2c_dis     : 1;                 // 
    uint8_t bdu         : 1;                 // 
    uint8_t ble         : 1;                 // 
    uint8_t unused_2    : 1;                 // Bit not used 
    uint8_t self_test   : 1;                 // 
    uint8_t int_mag     : 1;                 // 
}
lsm303agr_m_cfgc_t; 


// Magnetometer status register 
typedef struct lsm303agr_m_status_s 
{
    uint8_t zyx_or : 1;                      // 
    uint8_t z_or   : 1;                      // 
    uint8_t y_or   : 1;                      // 
    uint8_t x_or   : 1;                      // 
    uint8_t zyx_da : 1;                      // 
    uint8_t z_da   : 1;                      // 
    uint8_t y_da   : 1;                      // 
    uint8_t x_da   : 1;                      // 
}
lsm303agr_m_status_t; 

//=======================================================================================


//=======================================================================================
// Variables 

// Data record structure 
typedef struct lsm303agr_driver_data_s 
{
    // Peripherals 
    I2C_TypeDef *i2c; 

    // Device info 
    uint8_t a_addr; 
    uint8_t m_addr; 

    // Status info 
    // 'status' --> bit 0: i2c status (see i2c_status_t) 
    //          --> bit 1: init status (WHO_AM_I) 
    //          --> bits 2-7: self test results 
    uint8_t status; 

    // Magnetometer parameters 
    lsm303agr_m_data_t m_data; 
    lsm303agr_m_cfga_t m_cfga; 
    lsm303agr_m_cfgb_t m_cfgb; 
    lsm303agr_m_cfgc_t m_cfgc; 
    lsm303agr_m_status_t m_status; 
}
lsm303agr_driver_data_t; 


// Driver data record instance 
static lsm303agr_driver_data_t lsm303agr_driver_data; 

//=======================================================================================


//=======================================================================================
// Initialization 

// Initialization 
void lsm303agr_init(
    I2C_TypeDef *i2c, 
    lsm303agr_m_odr_cfg_t m_odr, 
    lsm303agr_m_sys_mode_t m_mode, 
    lsm303agr_cfg_t m_off_canc, 
    lsm303agr_cfg_t m_lpf, 
    lsm303agr_cfg_t m_int_mag_pin, 
    lsm303agr_cfg_t m_int_mag)
{
    //===================================================
    // Initialize data record 

    // Peripherals 
    lsm303agr_driver_data.i2c = i2c; 

    // Device info 
    lsm303agr_driver_data.a_addr = LSM303AGR_ACCEL_ADDR; 
    lsm303agr_driver_data.m_addr = LSM303AGR_MAG_ADDR; 

    // Status info 
    lsm303agr_driver_data.status = CLEAR; 

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
    
    //===================================================

    //===================================================
    // Check ID and configure device 

    // Check WHO AM I 
    if (lsm303agr_m_whoami_read() != LSM303AGR_ID_M)
    {
        lsm303agr_driver_data.status |= (SET_BIT << SHIFT_1); 
        return; 
    }

    // Config magnetometer 
    lsm303agr_m_cfga_write(); 
    lsm303agr_m_cfgb_write(); 
    lsm303agr_m_cfgc_write(); 
    
    // Config accelerometer 
    
    //===================================================

    //===================================================
    // Run self test 
    //===================================================
}

//=======================================================================================


//=======================================================================================
// Read and write 

// Read from register 
void lsm303agr_read(
    LSM303AGR_I2C_ADDR i2c_addr, 
    LSM303AGR_REG_ADDR reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    // Local variables 
    uint8_t i2c_status = I2C_OK; 

    // Generate a start condition 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the slave address with a write bit 
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          i2c_addr + LSM303AGR_W_OFFSET); 
    i2c_clear_addr(lsm303agr_driver_data.i2c); 

    // Send the register address that is going to be read 
    i2c_status |= (uint8_t)i2c_write(lsm303agr_driver_data.i2c, &reg_addr, BYTE_1);

    // Create another start signal 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the LSM303AGR address with a read offset 
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          i2c_addr + LSM303AGR_R_OFFSET);
    
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
    LSM303AGR_I2C_ADDR i2c_addr, 
    LSM303AGR_REG_ADDR reg_addr, 
    uint8_t *lsm303agr_reg_value, 
    byte_num_t lsm303agr_data_size)
{
    // Local variables 
    uint8_t i2c_status = I2C_OK; 
    
    // Create start condition to initiate master mode 
    i2c_status |= (uint8_t)i2c_start(lsm303agr_driver_data.i2c); 

    // Send the MPU6050 address with a write offset
    i2c_status |= (uint8_t)i2c_write_addr(lsm303agr_driver_data.i2c, 
                                          i2c_addr + LSM303AGR_W_OFFSET);
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
    lsm303agr_driver_data.status |= (uint8_t)i2c_status; 
}

//=======================================================================================


//=======================================================================================
// Self-test 

// Self-test procedure outlined on page 26 of the user manual 

//=======================================================================================


//=======================================================================================
// Register functions 

// Magnetometer WHO AM I read 
uint8_t lsm303agr_m_whoami_read(void)
{
    // Local variables 
    uint8_t who_am_i; 

    // Read and return the value of the WHO AM I register 
    lsm303agr_read(LSM303AGR_MAG_ADDR, LSM303AGR_WHO_AM_I_M, &who_am_i, BYTE_1); 

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
    lsm303agr_write(LSM303AGR_MAG_ADDR, LSM303AGR_CFG_A_M, &cfga, BYTE_1); 
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
    lsm303agr_write(LSM303AGR_MAG_ADDR, LSM303AGR_CFG_B_M, &cfgb, BYTE_1); 
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
    lsm303agr_write(LSM303AGR_MAG_ADDR, LSM303AGR_CFG_C_M, &cfgc, BYTE_1); 
}


// Magnetometer status register read 
void lsm303agr_m_status_read(void)
{
    // Local variables 
    uint8_t status; 

    // Read the magnetometer status 
    lsm303agr_read(LSM303AGR_MAG_ADDR, LSM303AGR_STATUS_M, &status, BYTE_1); 

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
    lsm303agr_read(LSM303AGR_MAG_ADDR, LSM303AGR_X_L_M | LSM303AGR_ADDR_INC, mag_data, BYTE_6); 

    // Combine the return values into signed integers 
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
    *m_x_data = lsm303agr_driver_data.m_data.m_x; 
    *m_y_data = lsm303agr_driver_data.m_data.m_y; 
    *m_z_data = lsm303agr_driver_data.m_data.m_z; 
}


// Reboot / reset 

// Low poer mode set 

//=======================================================================================
