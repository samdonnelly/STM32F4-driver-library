/**
 * @file mpu6050_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver for the MPU6050 accelerometer
 * 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "mpu6050_driver.h"

//=======================================================================================


//=======================================================================================
// TODO 
// - Create a data record to store one-time calculation data and possible read data 
//   - Ex. the full scale range that gets requested during self test and calculation 
// - Combine mpu6050_self_test_accel_result and mpu6050_self_test_gyro_result
// - Add a gpio output to the data record and init setup for using multiple devices 
// - Test if the interrupt signal for data ready is worth adding 
//  - Register 55: 
//    - INT_LEVEL: 0 
//    - INT_OPEN
// - Add a low power mode setter function to toggle the SLEEP bit (pwr mngt 1) 
//   - SLEEP = 1 --> low power mode 
// - Possibly add a register reset function - used by controller in reset? 
//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief MPU6050 write to register 
 * 
 * @details This function takes the device address, register address, data size and a 
 *          pointer to data and writes it to an MPU6050 register using the I2C driver.
 *          All register write functions call this function to send data. Only the first 
 *          register address is needed even if there are multiple successive addresses
 *          because the device automatically increments the register address after 
 *          each byte that is written. 
 * 
 * @see mpu6050_register_addresses_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param mpu6050_register : register address within the device 
 * @param mpu6050_reg_size : register byte size within the device 
 * @param mpu6050_reg_value : pointer to data to write to specified register 
 */
void mpu6050_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_register_addresses_t mpu6050_register,
    mpu6050_reg_byte_size_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);


/**
 * @brief MPU6050 read from register 
 * 
 * @details This function takes the device address, register address, data size and a 
 *          pointer to where data can be stored and reads MPU6050 register data using 
 *          the I2C driver. All register read functions call this function to read data. 
 *          Only the first register address is needed even if there are multiple 
 *          successive addresses because the device automatically increments the 
 *          register address after each byte that is read. 
 * 
 * @see mpu6050_register_addresses_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param mpu6050_register : register address within the device 
 * @param mpu6050_reg_size : register byte size within the device 
 * @param mpu6050_reg_value : pointer to where register data can be stored 
 */
void mpu6050_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_register_addresses_t mpu6050_register, 
    mpu6050_reg_byte_size_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);


/**
 * @brief MPU6050 Sample Rate Divider register write 
 * 
 * @details Writes to the SMPRT_DIV register (Register 25 - 1 byte). This register 
 *          specifies the  divider from the gyroscope output rate used to generate the 
 *          device sensor  sample rate. The sample rate is calculated as follows: 
 *          <br><br>
 *          
 *          Sample Rate = Gyroscope Output Rate / (a + SMPLRT_DIV) <br><br>
 *          
 *          The Gyroscope Output Rate is determined by the digital low pass filter. The 
 *          accelerometer output rate is 1kHz so a sample rate greater than this will 
 *          procude repeated accelerometer readings. <br><br>
 *          
 *          Register write information: <br>
 *          - SMPLRT_DIV: 8-bit unsigned value.
 * 
 * @see mpu6050_register_addresses_t
 * @see mpu6050_smplrt_div_t
 * @see mpu6050_dlpf_cfg_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param smprt_div : sample rate divider address
 */
void mpu6050_smprt_div_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_smplrt_div_t smprt_div);


/**
 * @brief MPU6050 Configuration register write
 * 
 * @details Writes to the CONFIG register (Register 26 - 1 byte). This register allows 
 *          for configuring the external frame synchronization (FSYNC) pin sampling and 
 *          the digital low pass filter (DLPF) for both accelerometers and gyroscopes.
 *          <br><br>
 *          
 *          Register write information: <br>
 *          - EXT_SYNC_SET: 3-bit unsigned value. <br>
 *          - DLPF_CFG: 3-bit unsigned value.  
 * 
 * @see mpu6050_register_addresses_t
 * @see mpu6050_ext_sync_set_t
 * @see mpu6050_dlpf_cfg_t
 * 
 * @param mapu6050_address : I2C address of MPU6050 
 * @param ext_sync_set : FSYNC pin sampling setpoint 
 * @param dlpf_cfg : digital low pass filter setpoint for all axes
 */
void mpu6050_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_ext_sync_set_t ext_sync_set,
    mpu6050_dlpf_cfg_t dlpf_cfg);


/**
 * @brief MPU6050 Accelerometer Configuration register write
 * 
 * @details Writes to the ACCEL_CONFIG register (Register 28 - 1 byte). This register is 
 *          used to trigger accelerometer self-test and configure the accelerometer full 
 *          scale range. <br><br>
 *          
 *          Register write information: <br>
 *          - ACCEL_SELF_TEST: 3-bit unsigned value that represents each axis: <br>
 *            - XA_ST: bit 2 <br>
 *            - YA_ST: bit 1 <br>
 *            - ZA_ST: bit 0 <br>
 *          - AFS_SEL: 2-bit unsigned value 
 * 
 * @see mpu6050_register_addresses_t
 * @see mpu6050_accel_self_test_set_t
 * @see mpu6050_afs_sel_set_t
 * @see mpu6050_self_test
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param accel_self_test : enables of disables self-test mode for all axes 
 * @param afs_sel : accelerometer full scale range setpoint for all axes
 */
void mpu6050_accel_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_accel_self_test_set_t accel_self_test,
    mpu6050_afs_sel_set_t afs_sel);


/**
 * @brief MPU6050 Accelerometer Configuration register read 
 * 
 * @details Reads from the ACCEL_CONFIG register (Register 28 - 1 byte). This function 
 *          is used to read the full scale range setpoint of the accelerometer during 
 *          self-test. <br><br>
 *          
 *          Register read information: <br>
 *          - XA_ST: bit 7 <br>
 *          - YA_ST: bit 6 <br>
 *          - ZA_ST: bit 5 <br>
 *          - AFS_SEL: bits 3-4. 2-bit unsigned value. 
 * 
 * @see mpu6050_register_addresses_t
 * @see mpu6050_self_test
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @return uint8_t : returns the unparsed self-test and full scale range setpoints 
 */
uint8_t mpu6050_accel_config_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr);


/**
 * @brief MPU6050 Gyroscope Configuration register write
 * 
 * @details Writes to the GYRO_CONFIG register (Register 27 - 1 byte). This register 
 *          is used to trigger gyroscope self-test and configure the gyroscopes full 
 *          scale range. <br><br>
 *          
 *          Register write information: <br>
 *          - GYRO_SELF_TEST: 3-bit unsigned value that represents each axis: <br>
 *            - XG_ST: bit 2 <br>
 *            - YG_ST: bit 1 <br>
 *            - ZG_ST: bit 0 <br>
 *          - FS_SEL: 2-bit unsigned value. 
 * 
 * @see mpu6050_register_addresses_t
 * @see mpu6050_gyro_self_test_set_t
 * @see mpu6050_fs_sel_set_t
 * @see mpu6050_self_test
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param gyro_self_test : enables or disabled self-test mode for all axes
 * @param fs_sel : gyroscope full scale range setpoint for all axes 
 */
void mpu6050_gyro_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_gyro_self_test_set_t gyro_self_test,
    mpu6050_fs_sel_set_t fs_sel);


/**
 * @brief MPU6050 Gyroscope Configuration register read
 * 
 * @details Reads from the GYRO_CONFIG register (Register 27 - 1 byte). This function 
 *          is used to read the full scale range setpoint of the gyroscope during 
 *          self-test. <br><br>
 *          
 *          Register read information: <br>
 *          - XG_ST: bit 7 <br>
 *          - YG_ST: bit 6 <br>
 *          - ZG_ST: bit 5 <br>
 *          - FS_SEL: bits 3-4. 2-bit unsigned value.
 *  
 * @see mpu6050_register_addresses_t
 * @see mpu6050_self_test
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @return uint8_t : returns the unparsed self-test and full scale range setpoints 
 */
uint8_t mpu6050_gyro_config_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr);


/**
 * @brief MPU6050 Power Manangement 1 register write
 * 
 * @details Writes to the PWR_MGMT_1 register (Register 107 - 1 byte). This register allows
 *          the user to configure the power mode and clock source. It can also reset the 
 *          entire device and disable the temperature sensor if needed. This function 
 *          is typically called in the init function to configure the device. <br><br>
 *          
 *          Register write information: <br>
 *          - DEVICE_RESET : 1-bit      <br>
 *          - SLEEP : 1-bit             <br>
 *          - CYCLE : 1-bit             <br>
 *          - TEMP_DIS : 1-bit          <br>
 *          - CLKSEL : 3-bit unsigned value. 
 * 
 * @see mpu6050_device_reset_t
 * @see mpu6050_sleep_mode_t
 * @see mpu6050_cycle_t
 * @see mpu6050_temp_sensor_t
 * @see mpu6050_clksel_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param device_reset : reset internal registers to default value when set to 1
 * @param sleep : enter sleep mode when set to 1 
 * @param cycle : periodic sleep mode when set to 1 and sleep is set to 0 
 * @param temp_dis : disables temperature sensor when set to 1 
 * @param clksel : specifies clock source of the device
 */
void mpu6050_pwr_mgmt_1_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_device_reset_t device_reset,
    mpu6050_sleep_mode_t sleep,
    mpu6050_cycle_t cycle,
    mpu6050_temp_sensor_t temp_dis,
    mpu6050_clksel_t clksel);


/**
 * @brief MPU6050 Power Management 2 register write
 * 
 * @details Writes to the PWR_MGMT_2 register (Register 108 - 1 byte). This register allows 
 *          the user to configure the frequency of wake-ups in acclerometer only low power
 *          mode. It can also put individual axes of the accelerometer and gyroscope into 
 *          low power mode. <br><br>
 *          
 *          Register write information: <br>
 *          - LP_WAKE_CTRL : 2-bit unsigned value. <br>
 *          - STBY_XA : 1-bit  <br>
 *          - STBY_YA : 1-bit  <br>
 *          - STBY_ZA : 1-bit  <br>
 *          - STBY_XA : 1-bit  <br> 
 *          - STBY_YG : 1-bit  <br>
 *          - STBY_ZG : 1-bit 
 * 
 * @see mpu6050_lp_wake_ctrl_t
 * @see mpu6050_stby_xa_t
 * @see mpu6050_stby_ya_t
 * @see mpu6050_stby_za_t
 * @see mpu6050_stby_xg_t
 * @see mpu6050_stby_yg_t
 * @see mpu6050_stby_zg_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param lp_wake_ctrl : specify wake-up frequency in accelerometer only low power mode
 * @param stby_xa : put accelerometer x-axis in standby when set to 1
 * @param stby_ya : put accelerometer y-axis in standby when set to 1
 * @param stby_za : put accelerometer z-axis in standby when set to 1
 * @param stby_xg : put gyroscope x-axis in standby when set to 1
 * @param stby_yg : put gyroscope y-axis in standby when set to 1
 * @param stby_zg : put gyroscope z-axis in standby when set to 1
 */
void mpu6050_pwr_mgmt_2_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    mpu6050_stby_xa_t stby_xa,
    mpu6050_stby_ya_t stby_ya,
    mpu6050_stby_za_t stby_za,
    mpu6050_stby_xg_t stby_xg,
    mpu6050_stby_yg_t stby_yg,
    mpu6050_stby_zg_t stby_zg);


/**
 * @brief MPU6050 Who Am I register read 
 * 
 * @details Read from WHO_AM_I register (Register 117 - 1 byte). This register is used 
 *          to verify the identity of the device. The register contains the upper 6-bits 
 *          of the 7-bit I2C address (the LSB bit being the value of AD0). <br><br>
 *          
 *          Register read information: <br>
 *          - WHO_AM_I: bits 1-6 <br>
 * 
 * @param mpu6050_sddress : I2C address of MPU6050 
 * @return uint8_t : Returns 0x68 if AD0 == 0 and 0x69 is AD0 == 1
 */
uint8_t mpu6050_who_am_i_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr);


/**
 * @brief MPU6050 self-test read 
 * 
 * @details Called within mpu6050_self_test to read the self-test register values. After 
 *          reading the registers the accelerometer and gyroscope data is parsed to make 
 *          the data sets distinguishable from one another. The results are stored in the 
 *          pointers passed to the function. 
 * 
 * @see mpu6050_self_test
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param accel_self_test_data : pointer where self-test accel register data gets stored 
 * @param gyro_self_test_data : pointer where self-test gyro register data gets stored 
 */
void mpu6050_self_test_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    uint8_t *accel_self_test_data,
    uint8_t *gyro_self_test_data);


/**
 * @brief MPU6050 self-test response calculation
 * 
 * @details This function calculates the self-test response of each sensor axis. The 
 *          self-test response is defined as: <br><br>
 *          
 *          Self-test response = (sensor output with self-test enabled) - 
 *                               (sensor output with self-test disabled) <br><br>
 * 
 * @param self_test_response : pointer that stores the sef-test response calculation
 * @param no_self_test : pointer to raw senor output (without self-test enabled)
 * @param self_test : pointer to raw self-test sensor output 
 * @param num_axes : number of accelerometer or gyroscope axis to calculate
 */
void mpu6050_str_calc(
    int16_t *self_test_response,
    int16_t *no_self_test,
    int16_t *self_test,
    uint8_t num_axes);


/**
 * @brief MPU6050 accelerometer self-test result 
 * 
 * @details Determines the results (pass/fail) of the self-test for the accelerometer 
 *          and stores the result in bits 0-2 of self_test_results. mpu6050_self_test
 *          calls this function to calculate the result. See mpu6050_self_test
 *          documentation for a breakdown on reading these results. The result is 
 *          determined for each axis by taking the output of the following equation
 *          and comparing it against the maximum allowable change from factory trim.
 *          <br><br>
 *          
 *          Change from factory trim = ((self-test response) - (factory trim)) / 
 *                                     (factory trim)
 * 
 * @see mpu6050_accel_ft
 * @see mpu6050_self_test
 * 
 * @param accel_self_test_results : pointer to accelerometer self-test responses
 * @param accel_factory_trim : pointer to accelerometer axes factory trim 
 * @param self_test_results : stores the pass/fail results of the self-test
 * @return uint8_t : returns the updated self_test_results
 */
uint8_t mpu6050_self_test_accel_result(
    int16_t *accel_self_test_results,
    float   *accel_factory_trim,
    uint8_t self_test_results);


/**
 * @brief MPU6050 gyroscope self-test result
 * 
 * @details Determines the results (pass/fail) of the self-test for the gyroscope 
 *          and stores the result in bits 3-5 of self_test_results. mpu6050_self_test
 *          calls this function to calculate the result. See mpu6050_self_test
 *          documentation for a breakdown on reading these results. The result is 
 *          determined for each axis by taking the output of the following equation
 *          and comparing it against the maximum allowable change from factory trim.
 *          <br><br>
 *          
 *          Change from factory trim = ((self-test response) - (factory trim)) / 
 *                                     (factory trim)
 * 
 * @see mpu6050_gyro_ft
 * @see mpu6050_self_test
 * 
 * @param gyro_self_test_results : pointer to gyroscope self-test responses
 * @param gyro_factory_trim : pointer to gyroscope axes factory trim 
 * @param self_test_results : stores the pass/fail results of the self-test
 * @return uint8_t : returns the updated self_test_results
 */
uint8_t mpu6050_self_test_gyro_result(
    int16_t *gyro_self_test_results,
    float   *gyro_factory_trim,
    uint8_t self_test_results);


/**
 * @brief MPU6050 accelerometer factory trim 
 * 
 * @details The factory trim is used to determine how much the accelerometer sensors
 *          have drifted from their factory configuration as seen in the 
 *          mpu6050_self_test_accel_result function. Factory trim is a complex 
 *          calculation and so this function approximates the result using a third 
 *          order polynomial: <br><br>
 *          
 *          Factory trim = C1*x^3 + C2*x^2 + C3*x + C4 <br><br>
 *          
 *          More information on this calculation can be found in 
 *          self_test_accel_constants_t and the register map documentation. 
 * 
 * @see self_test_accel_constants_t
 * @see mpu6050_self_test_accel_result
 * 
 * @param a_test : pointer to self-test accelerometer results 
 * @param accel_ft : pointer to store calculated factory trim 
 */
void mpu6050_accel_ft(
    uint8_t *a_test, 
    float *accel_ft);


/**
 * @brief MPU6050 gyroscope factory trim 
 * 
 * @details The factory trim is used to determine how much the gyroscope sensors
 *          have drifted from their factory configuration as seen in the 
 *          mpu6050_self_test_gyro_result function. Factory trim is a complex 
 *          calculation and so this function approximates the result using a third 
 *          order polynomial: <br><br>
 *          
 *          Factory trim = C1*x^3 + C2*x^2 + C3*x + C4 <br><br>
 *          
 *          More information on this calculation can be found in 
 *          self_test_gyro_constants_t and the register map documentation. 
 * 
 * @see self_test_gyro_constants_t
 * @see mpu6050_self_test_gyro_result
 * 
 * @param g_test : pointer to self-test gyroscope results 
 * @param gyro_ft : pointer to store calcualted factory trim
 */
void mpu6050_gyro_ft(
    uint8_t *g_test, 
    float *gyro_ft);


/**
 * @brief MPU6050 accelerometer scalar 
 * 
 * @details This function reads the ACCEL_CONFIG register to check the chosen full scale
 *          range of the accelerometer. This is used to determine the scalar that  
 *          converts raw sensor output into g's. The mpu6050_accel_x/y/z_calc functions 
 *          call this function. 
 * 
 * @see mpu6050_accel_config_read
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @return float : returns the scalar used to convert raw acceleromeeter data to g's
 */
float mpu6050_accel_scalar(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr);


/**
 * @brief MPU6050 gyroscope scalar
 * 
 * @details This function reads the GYRO_CONFIG register to check the chosen full scale
 *          range of the gyroscope. This is used to determine the scalar that  
 *          converts raw sensor output into deg/s. The mpu6050_gyro_x/y/z_calc functions 
 *          call this function. 
 *          
 *          To calculate the scalar, the full scale range (250-2000 deg/s) of the gyro 
 *          is read and used to determine which scalar to use. This full scale range is 
 *          stored in a device register in the form of a number from 0-3 (see 
 *          mpu6050_fs_sel_set_t). Each scalar is double (or half) of the next range 
 *          scalar so we take the maximum range value scalar and bit shift by the 
 *          range index (mpu6050_fs_sel_set_t) to get the scaling value. For the gyro 
 *          there is an additional correction to the calculation as the lowest two ranges 
 *          are 1 less than their actual value after shifting due to integer decimal 
 *          place truncation. 
 * 
 * @see mpu6050_gyro_config_read
 * @see mpu6050_fs_sel_set_t
 * 
 * @param mpu6050_address : I2C address of MPU6050 
 * @return float : returns the scalar used to convert raw gyroscope data to deg/s
 */
float mpu6050_gyro_scalar(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr);

//=======================================================================================


//=======================================================================================
// Messages 

// Accelerometer data 
typedef struct mpu6050_accel_s
{
    int16_t accel_x;                // Acceleration in the x-axis 
    int16_t accel_y;                // Acceleration in the y-axis 
    int16_t accel_z;                // Acceleration in the z-axis 
}
mpu6050_accel_t;


// Gyroscope data 
typedef struct mpu6050_gyro_s
{
    int16_t gyro_x;                 // Angular velocity about the x-axis 
    int16_t gyro_y;                 // Angular velocity about the y-axis 
    int16_t gyro_z;                 // Angular velocity about the z-axis 
}
mpu6050_gyro_t; 


// Other device data 
typedef struct mpu6050_other_s
{
    int16_t temp;                   // Temperature 
}
mpu6050_other_t; 


// 
typedef struct mpu6050_com_data_s 
{
    // Peripherals 
    I2C_TypeDef *i2c;                // I2C port connected to the device 
    GPIO_TypeDef *gpio;              // 

    // Pins 

    // Device information 
    mpu6050_i2c_addr_t addr;         // Device I2C address 

    // Messages 
    mpu6050_accel_t accel_data;      // Accelerometer data 
    mpu6050_gyro_t gyro_data;        // Gyroscope data 
    mpu6050_other_t other_data;      // Other device data 

    // Data 
    float accel_data_scalar;         // Scales accelerometer raw data into readable values 
    float gyro_data_scalar;          // Scales gyroscope raw data into readable values 
}
mpu6050_com_data_t; 


// MPU6050 device com data record instance 
static mpu6050_com_data_t mpu6050_com_data; 

//=======================================================================================


//=======================================================================================
// Initialization 

//==============================================================
// MPU6050 Init
//  1. Read the WHO_AM_I register to establish that there is communication 
//  2. Choose which sensors to use and frquency of CYCLE mode (see PWR_MGMT_1)
//  3. Wake the sensor up through the power management 1 register 
//  4. Set the output rate of the gyro and accelerometer 
//  5. Set the Sample Rate (data rate) 
//  6. Configure the accelerometer register 
//  7. Configure the gyroscope register
//==============================================================

// MPU6050 Initialization 
uint8_t mpu6050_init(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t mpu6050_address,
    mpu6050_dlpf_cfg_t dlpf_cfg,
    mpu6050_smplrt_div_t smplrt_div,
    mpu6050_afs_sel_set_t afs_sel,
    mpu6050_fs_sel_set_t fs_sel)
{
    // Assign device information 
    mpu6050_com_data.i2c = i2c; 
    mpu6050_com_data.addr = mpu6050_address; 

    // Read the WHO_AM_I register to establish that there is communication 
    if (mpu6050_who_am_i_read(mpu6050_com_data.i2c, 
                              mpu6050_com_data.addr) != MPU6050_7_BIT_ADDRESS)
    {
        return FALSE;
    }

    // Choose which sensors to use and frquency of CYCLE mode
    mpu6050_pwr_mgmt_2_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr,
        LP_WAKE_CTRL_0,
        STBY_XA_DISABLE,
        STBY_YA_DISABLE,
        STBY_ZA_DISABLE,
        STBY_XG_DISABLE,
        STBY_YG_DISABLE,
        STBY_ZG_DISABLE);

    // Wake the sensor up through the power management 1 register 
    mpu6050_pwr_mgmt_1_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr,
        DEVICE_RESET_DISABLE,
        SLEEP_MODE_DISABLE,
        CYCLE_SLEEP_DISABLED,
        TEMP_SENSOR_ENABLE,
        CLKSEL_5);

    // Set the output rate of the gyro and accelerometer 
    mpu6050_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr,
        EXT_SYNC_SET_0,
        dlpf_cfg);
    
    // Set the Sample Rate (data rate)
    mpu6050_smprt_div_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        smplrt_div);
    
    // Configure the accelerometer register 
    mpu6050_accel_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr,
        ACCEL_SELF_TEST_DISABLE,
        afs_sel);
    
    // Configure the gyroscope register
    mpu6050_gyro_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr,
        GYRO_SELF_TEST_DISABLE,
        fs_sel);

    // Store the raw data scalars 
    mpu6050_com_data.accel_data_scalar = mpu6050_accel_scalar(mpu6050_com_data.i2c, 
                                                              mpu6050_com_data.addr); 

    mpu6050_com_data.gyro_data_scalar = mpu6050_gyro_scalar(mpu6050_com_data.i2c, 
                                                            mpu6050_com_data.addr); 

    // Initialization completed successfully 
    return TRUE;
}


// MPU6050 reference point set 
void mpu6050_calibrate(
    int16_t *mpu6050_accel_offset,
    int16_t *mpu6050_gyro_offset)
{
    // Read initial accelerometer values 
    mpu6050_accel_read(mpu6050_accel_offset);

    // Read an instance of gyroscope values 
    mpu6050_gyro_read(mpu6050_gyro_offset); 
    *mpu6050_gyro_offset++ = mpu6050_get_gyro_x_raw(); 
    *mpu6050_gyro_offset++ = mpu6050_get_gyro_y_raw(); 
    *mpu6050_gyro_offset   = mpu6050_get_gyro_z_raw(); 
}

//=======================================================================================


//=======================================================================================
// Read and Write Functions 

// MPU6050 write to register 
void mpu6050_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_register_addresses_t mpu6050_register,
    mpu6050_reg_byte_size_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c_start(i2c); 

    // Send the MPU6050 address with a write offset
    i2c_write_address(i2c, addr + MPU6050_W_OFFSET);
    i2c_clear_addr(i2c);

    // Send the register address that is going to be written to 
    i2c_write_master_mode(i2c, &mpu6050_register, MPU6050_REG_1_BYTE);

    // Write the data to the MPU6050 
    i2c_write_master_mode(i2c, mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c_stop(i2c); 
}


// MPU6050 read from register 
void mpu6050_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_register_addresses_t mpu6050_register, 
    mpu6050_reg_byte_size_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c_start(i2c); 

    // Send the MPU6050 address with a write offset 
    i2c_write_address(i2c, addr + MPU6050_W_OFFSET);
    i2c_clear_addr(i2c);

    // Send the register address that is going to be read 
    i2c_write_master_mode(i2c, &mpu6050_register, MPU6050_REG_1_BYTE);

    // Create another start signal 
    i2c_start(i2c); 

    // Send the MPU6050 address with a read offset 
    i2c_write_address(i2c, addr + MPU6050_R_OFFSET);

    // Read the data sent by the MPU6050 
    i2c_read_master_mode(i2c, mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c_stop(i2c); 
}

//=======================================================================================


//=======================================================================================
// Register Functions

// MPU6050 Accelerometer Measurements registers read
void mpu6050_accel_read(
    int16_t *accel_data)
{
    // Temporary data storage 
    uint8_t accel_data_reg[MPU6050_REG_6_BYTE];

    // Read the accelerometer data 
    mpu6050_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_ACCEL_XOUT_H,
        MPU6050_REG_6_BYTE,
        accel_data_reg);

    // Combine the return values into signed integers - values are unformatted
    mpu6050_com_data.accel_data.accel_x = (int16_t)((accel_data_reg[0] << SHIFT_8) |
                                                    (accel_data_reg[1] << SHIFT_0));
    mpu6050_com_data.accel_data.accel_y = (int16_t)((accel_data_reg[2] << SHIFT_8) |
                                                    (accel_data_reg[3] << SHIFT_0));
    mpu6050_com_data.accel_data.accel_z = (int16_t)((accel_data_reg[4] << SHIFT_8) |
                                                    (accel_data_reg[5] << SHIFT_0));
}


// MPU6050 Gyroscope Measurements registers read
void mpu6050_gyro_read(
    int16_t *gyro_data)
{
    // Temporary data storage 
    uint8_t gyro_data_reg[MPU6050_REG_6_BYTE];

    // Read the gyroscope data 
    mpu6050_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_GYRO_XOUT_H,
        MPU6050_REG_6_BYTE,
        gyro_data_reg);

    // Combine the return values into signed integers - values are unformatted
    mpu6050_com_data.gyro_data.gyro_x = (int16_t)((gyro_data_reg[0] << SHIFT_8) |
                                                  (gyro_data_reg[1] << SHIFT_0));
    mpu6050_com_data.gyro_data.gyro_y = (int16_t)((gyro_data_reg[2] << SHIFT_8) |
                                                  (gyro_data_reg[3] << SHIFT_0));
    mpu6050_com_data.gyro_data.gyro_z = (int16_t)((gyro_data_reg[4] << SHIFT_8) |
                                                  (gyro_data_reg[5] << SHIFT_0));
}


// MPU6050 Temperature Measurements registers read
// int16_t mpu6050_temp_read(void)
void mpu6050_temp_read(void)
{
    // Store the temperature data 
    uint8_t mpu6050_temp_sensor_reg[MPU6050_REG_2_BYTE]; 

    // Read the temperature data 
    mpu6050_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_TEMP_OUT_H,
        MPU6050_REG_2_BYTE,
        mpu6050_temp_sensor_reg);

    // Generate an unformatted signed integer from the combine the return values 
    mpu6050_com_data.other_data.temp = (int16_t)((mpu6050_temp_sensor_reg[0] << SHIFT_8)  |
                                                 (mpu6050_temp_sensor_reg[1] << SHIFT_0)); 
}


// MPU6050 Sample Rate Divider register write 
void mpu6050_smprt_div_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_smplrt_div_t smprt_div)
{
    // Write to the Sample Rate Divider register
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_SMPRT_DIV,
        MPU6050_REG_1_BYTE,
        &smprt_div);
}


// MPU6050 Configuration register write
void mpu6050_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_ext_sync_set_t ext_sync_set,
    mpu6050_dlpf_cfg_t dlpf_cfg)
{
    // Configure the data
    uint8_t mpu6050_config = (ext_sync_set << SHIFT_3) | (dlpf_cfg << SHIFT_0);

    // Write to the Configuration register 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_CONFIG,
        MPU6050_REG_1_BYTE,
        &mpu6050_config);
}


// MPU6050 Gyroscope Configuration register write
void mpu6050_gyro_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_gyro_self_test_set_t gyro_self_test,
    mpu6050_fs_sel_set_t fs_sel)
{
    // Configure the data 
    uint8_t mpu6050_gyro_config = (gyro_self_test << SHIFT_5) | (fs_sel << SHIFT_3);

    // Write to the Gyroscope Configuration register 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_GYRO_CONFIG,
        MPU6050_REG_1_BYTE,
        &mpu6050_gyro_config);
}


// MPU6050 Gyroscope Configuration register read
uint8_t mpu6050_gyro_config_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr)
{
    // Place to store the value of GYRO_CONFIG
    uint8_t mpu6050_gyro_config;

    // Read the value of GYRO_CONFIG register 
    mpu6050_read(
        i2c, 
        addr, 
        MPU6050_GYRO_CONFIG, 
        MPU6050_REG_1_BYTE,
        &mpu6050_gyro_config);

    // Return the value of GYRO_CONFIG 
    return mpu6050_gyro_config;
}


// MPU6050 Accelerometer Configuration register write
void mpu6050_accel_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_accel_self_test_set_t accel_self_test,
    mpu6050_afs_sel_set_t afs_sel)
{
    // Configure the data 
    uint8_t mpu6050_accel_config = (accel_self_test << SHIFT_5) | (afs_sel << SHIFT_3);

    // Write to the Accelerometer Configuration register 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_ACCEL_CONFIG,
        MPU6050_REG_1_BYTE,
        &mpu6050_accel_config);
}


// MPU6050 Accelerometer Configuration register read 
uint8_t mpu6050_accel_config_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr)
{
    // Place to store the value of ACCEL_CONFIG
    uint8_t mpu6050_accel_config;

    // Read the value of ACCEL_CONFIG register 
    mpu6050_read(
        i2c, 
        addr, 
        MPU6050_ACCEL_CONFIG, 
        MPU6050_REG_1_BYTE,
        &mpu6050_accel_config);

    // Return the value of ACCEL_CONFIG 
    return mpu6050_accel_config;
}


// MPU6050 Power Manangement 1 register write
void mpu6050_pwr_mgmt_1_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_device_reset_t device_reset,
    mpu6050_sleep_mode_t sleep,
    mpu6050_cycle_t cycle,
    mpu6050_temp_sensor_t temp_dis,
    mpu6050_clksel_t clksel)
{
    // Configure the data
    uint8_t mpu6050_pwr_mgmt_1 = (device_reset << SHIFT_7) |
                                 (sleep        << SHIFT_6) |
                                 (cycle        << SHIFT_5) |
                                 (temp_dis     << SHIFT_3) |
                                 (clksel       << SHIFT_0);
    
    // Write to the Power Management 1 register 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_PWR_MGMT_1,
        MPU6050_REG_1_BYTE,
        &mpu6050_pwr_mgmt_1);
}


// MPU6050 Power Manangement 2 register write
void mpu6050_pwr_mgmt_2_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    mpu6050_stby_xa_t stby_xa,
    mpu6050_stby_ya_t stby_ya,
    mpu6050_stby_za_t stby_za,
    mpu6050_stby_xg_t stby_xg,
    mpu6050_stby_yg_t stby_yg,
    mpu6050_stby_zg_t stby_zg)
{
    // Configure the data 
    uint8_t mpu6050_pwr_mgmt_2 = (lp_wake_ctrl << SHIFT_6) |
                                 (stby_xa      << SHIFT_5) |
                                 (stby_ya      << SHIFT_4) |
                                 (stby_za      << SHIFT_3) |
                                 (stby_xg      << SHIFT_2) |
                                 (stby_yg      << SHIFT_1) |
                                 (stby_zg      << SHIFT_0);
    
    // Write to the Power Management 2 register 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_PWR_MGMT_2,
        MPU6050_REG_1_BYTE,
        &mpu6050_pwr_mgmt_2);
}


// MPU6050 Who Am I register read
uint8_t mpu6050_who_am_i_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr)
{
    // Place to store the value WHO_AM_I
    uint8_t mpu6050_who_am_i;

    // Read the value of WHO_AM_I register 
    mpu6050_read(
        i2c, 
        addr, 
        MPU6050_WHO_AM_I, 
        MPU6050_REG_1_BYTE,
        &mpu6050_who_am_i);

    // Return the value of who_am_i 
    return mpu6050_who_am_i;
}

//=======================================================================================


//=======================================================================================
// Self-test functions 

//==============================================================
// Steps for self-test 
//  1. Record the full scale range set in the init function 
//  2. Set the full scale range of the accel to +/- 8g and gyro to +/- 250 deg/s
//  3. Read and store the sensor values during non-self-test
//  4. Enable self-test 
//  5. Read and store the sensor values during self-test
//  6. Read the self-test registers
//  7. Calculate the factory trim
//  8. Calculate self-test response
//  9. Calculate the change from factory trim and check against the acceptable range
//  10. Disable self-test and set the full scale ranges back to their original values
//============================================================== 

// MPU6050 self-test
uint8_t mpu6050_self_test(void)
{
    //===================================================
    // Local variables 
    
    // Used to record the existing full scale range 
    uint8_t accel_fsr;
    uint8_t gyro_fsr;

    // Sensor readings with no self-test 
    int16_t accel_no_st[MPU6050_NUM_ACCEL_AXIS];
    int16_t gyro_no_st[MPU6050_NUM_GYRO_AXIS];

    // Sensor readings with self-test 
    int16_t accel_st[MPU6050_NUM_ACCEL_AXIS];
    int16_t gyro_st[MPU6050_NUM_GYRO_AXIS];

    // Self-test result 
    int16_t accel_str[MPU6050_NUM_ACCEL_AXIS];
    int16_t gyro_str[MPU6050_NUM_GYRO_AXIS];

    // Self-test register readings 
    uint8_t accel_test[MPU6050_NUM_ACCEL_AXIS];
    uint8_t gyro_test[MPU6050_NUM_GYRO_AXIS];

    // Factory trim calculation 
    float accel_ft[MPU6050_NUM_ACCEL_AXIS];
    float gyro_ft[MPU6050_NUM_GYRO_AXIS];

    // Status of the self-test 
    uint8_t self_test_result = 0;

    //===================================================


    //===================================================
    // Self-test procedure 

    // Record the full scale range set in the init function 
    accel_fsr = ((mpu6050_accel_config_read(mpu6050_com_data.i2c, mpu6050_com_data.addr) & 
                  MPU6050_AFS_SEL_MASK) >> SHIFT_3);
    gyro_fsr  = ((mpu6050_gyro_config_read(mpu6050_com_data.i2c, mpu6050_com_data.addr) & 
                  MPU6050_FS_SEL_MASK) >> SHIFT_3);

    // Set the full scale range of the accel to +/- 8g and gyro to +/- 250 deg/s
    mpu6050_accel_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        ACCEL_SELF_TEST_DISABLE,
        AFS_SEL_8);
    mpu6050_gyro_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        GYRO_SELF_TEST_DISABLE,
        FS_SEL_250);

    // Read and store the sensor values during non-self-test
    mpu6050_accel_read(accel_no_st);
    mpu6050_gyro_read(gyro_no_st);

    // Enable self-test 
    mpu6050_accel_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        ACCEL_SELF_TEST_ENABLE,
        AFS_SEL_8);
    mpu6050_gyro_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        GYRO_SELF_TEST_ENABLE,
        FS_SEL_250);
    
    // Read and store the sensor values during self-test
    mpu6050_accel_read(accel_st);
    mpu6050_gyro_read(gyro_st);
    
    // Read the self-test registers
    mpu6050_self_test_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        accel_test,
        gyro_test);
    
    // Calculate the factory trim
    mpu6050_accel_ft(accel_test, accel_ft);
    mpu6050_gyro_ft(gyro_test, gyro_ft);
    
    // Calculate self-test response
    mpu6050_str_calc(
        accel_str,
        accel_no_st,
        accel_st,
        MPU6050_NUM_ACCEL_AXIS);
    mpu6050_str_calc(
        gyro_str,
        gyro_no_st,
        gyro_st,
        MPU6050_NUM_GYRO_AXIS);
    
    // Calculate the change from factory trim and check against the acceptable range
    self_test_result = mpu6050_self_test_accel_result(
                                            accel_str,
                                            accel_ft,
                                            self_test_result);
    self_test_result = mpu6050_self_test_gyro_result(
                                            gyro_str,
                                            gyro_ft,
                                            self_test_result);
    
    // Disable self-test and set the full scale ranges back to their original values
    mpu6050_accel_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        ACCEL_SELF_TEST_DISABLE,
        accel_fsr);
    mpu6050_gyro_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        GYRO_SELF_TEST_DISABLE,
        gyro_fsr);
    
    //===================================================
    
    return self_test_result;
}


// MPU6050 self-test read 
void mpu6050_self_test_read(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    uint8_t *accel_st_data,
    uint8_t *gyro_st_data)
{
    // Store unformatted data 
    uint8_t st_data[MPU6050_REG_4_BYTE];

    // Read the value of SELF_TEST registers 
    mpu6050_read(
        i2c, 
        addr, 
        MPU6050_SELF_TEST, 
        MPU6050_REG_4_BYTE,
        st_data);
    
    // Parse the acceleration data in X, Y and Z respectively 
    *accel_st_data++ = ((st_data[MPU6050_REG_IDX_0] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                       ((st_data[MPU6050_REG_IDX_3] & SELF_TEST_MASK_XA_TEST_LO) >> SHIFT_4);
    
    *accel_st_data++ = ((st_data[MPU6050_REG_IDX_1] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                       ((st_data[MPU6050_REG_IDX_3] & SELF_TEST_MASK_YA_TEST_LO) >> SHIFT_2);
    
    *accel_st_data = ((st_data[MPU6050_REG_IDX_2] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                     ((st_data[MPU6050_REG_IDX_3] & SELF_TEST_MASK_ZA_TEST_LO) >> SHIFT_0);

    // Parse the gyro data in X, Y and Z respectively 
    *gyro_st_data++ = st_data[MPU6050_REG_IDX_0] & SELF_TEST_MASK_X_TEST;
    
    *gyro_st_data++ = st_data[MPU6050_REG_IDX_1] & SELF_TEST_MASK_X_TEST;
    
    *gyro_st_data = st_data[MPU6050_REG_IDX_2] & SELF_TEST_MASK_X_TEST;
}


// MPU6050 accelerometer factory trim 
void mpu6050_accel_ft(
    uint8_t *a_test, 
    float *accel_ft)
{
    // Constants 
    float c1 = SELF_TEST_ACCEL_FT_C1 / ((float)(DIVIDE_10000));
    float c2 = SELF_TEST_ACCEL_FT_C2 / ((float)(DIVIDE_10000));
    float c3 = SELF_TEST_ACCEL_FT_C3 / ((float)(DIVIDE_1000));
    float c4 = SELF_TEST_ACCEL_FT_C4 / ((float)(DIVIDE_10));

    // Determine the factory trim 
    for (uint8_t i = 0; i < MPU6050_NUM_ACCEL_AXIS; i++)
    {
        if (*a_test == 0)
        {
            *accel_ft = 0;
        }
        else
        {
            *accel_ft = (*a_test)*((*a_test)*(c1*(*a_test) + c2) + c3) + c4;
        }

        accel_ft++;
        a_test++;
    }
}


// MPU6050 gyroscope factory trim 
void mpu6050_gyro_ft(
    uint8_t *g_test, 
    float *gyro_ft)
{
    // Constants 
    float c1 = SELF_TEST_GYRO_FT_C1 / ((float)(DIVIDE_10000));
    float c2 = SELF_TEST_GYRO_FT_C2 / ((float)(DIVIDE_10000));
    float c3 = SELF_TEST_GYRO_FT_C3 / ((float)(DIVIDE_100));
    float c4 = SELF_TEST_GYRO_FT_C4 / ((float)(DIVIDE_10));

    // Determine the factory trim 
    for (uint8_t i = 0; i < MPU6050_NUM_GYRO_AXIS; i++)
    {
        if (*g_test == 0)
        {
            *gyro_ft = 0;
        }
        else 
        {
            *gyro_ft = (*g_test)*((*g_test)*(c1*(*g_test) + c2) + c3) + c4;
        }

        if (i == 1)
        {
            *gyro_ft = -(*gyro_ft);
        }

        gyro_ft++;
        g_test++;
    }
}


// MPU6050 self-test response calculation
void mpu6050_str_calc(
    int16_t *self_test_response,
    int16_t *no_self_test,
    int16_t *self_test,
    uint8_t num_axes)
{
    for (uint8_t i = 0; i < num_axes; i++)
    {
        *self_test_response++ = *self_test++ - *no_self_test++;
    }
}


// MPU6050 accelerometer self-test result 
uint8_t mpu6050_self_test_accel_result(
    int16_t *accel_self_test_results,
    float  *accel_factory_trim,
    uint8_t self_test_results)
{
    // Place to store the % change 
    float ft_change;

    // Determine the result of the self-test
    for (uint8_t i = 0; i < MPU6050_NUM_ACCEL_AXIS; i++)
    {
        // Check % change from factory trim 
        ft_change = (*accel_self_test_results - *accel_factory_trim) / *accel_factory_trim;
        
        // Check change against maximum allowed value
        if ((ft_change > MPU6050_ACCEL_FT_MAX_ERROR) ||
            (ft_change < -(MPU6050_ACCEL_FT_MAX_ERROR)))
        {
            self_test_results |= (SELF_TEST_RESULT_SHIFT_ACCEL << SHIFT_1*i);
        }

        accel_self_test_results++;
        accel_factory_trim++;
    }

    return self_test_results;
}


// MPU6050 gyroscope self-test result 
uint8_t mpu6050_self_test_gyro_result(
    int16_t *gyro_self_test_results,
    float *gyro_factory_trim,
    uint8_t self_test_results)
{
    // Place to store the % change 
    float ft_change;

    // Determine the result of the self-test
    for (uint8_t i = 0; i < MPU6050_NUM_GYRO_AXIS; i++)
    {
        // Check % change from factory trim 
        ft_change = (*gyro_self_test_results - *gyro_factory_trim) / *gyro_factory_trim;
        
        // Check change against maximum allowed value
        if ((ft_change > MPU6050_GYRO_FT_MAX_ERROR) ||
            (ft_change < -(MPU6050_GYRO_FT_MAX_ERROR)))
        {
            self_test_results |= (SELF_TEST_RESULT_SHIFT_GYRO << SHIFT_1*i);
        }

        gyro_self_test_results++;
        gyro_factory_trim++;
    }

    return self_test_results;
}

//=======================================================================================


//=======================================================================================
// Getters 

// MPU6050 accelerometer x-axis raw value 
int16_t mpu6050_get_accel_x_raw(void)
{
    return mpu6050_com_data.accel_data.accel_x; 
}


// MPU6050 accelerometer y-axis raw value 
int16_t mpu6050_get_accel_y_raw(void)
{
    return mpu6050_com_data.accel_data.accel_y; 
}


// MPU6050 accelerometer z-axis raw value 
int16_t mpu6050_get_accel_z_raw(void)
{
    return mpu6050_com_data.accel_data.accel_z; 
}


// MPU6050 accelerometer x-axis calculation 
float mpu6050_get_accel_x(
    int16_t accel_x_axis_raw)
{
    // Get the raw value scalar and calculate the true x-axis acceleration 
    return mpu6050_get_accel_x_raw() / mpu6050_com_data.accel_data_scalar; 
}


// MPU6050 accelerometer y-axis calculation 
float mpu6050_get_accel_y(
    int16_t accel_y_axis_raw)
{
    // Get the raw value scalar and calculate the true y-axis acceleration 
    return mpu6050_get_accel_y_raw() / mpu6050_com_data.accel_data_scalar; 
}


// MPU6050 accelerometer z-axis calculation 
float mpu6050_get_accel_z(
    int16_t accel_z_axis_raw)
{
    // Get the raw value scalar and calculate the true z-axis acceleration 
    return mpu6050_get_accel_z_raw() / mpu6050_com_data.accel_data_scalar; 
}


// MPU6050 accelerometer scalar
float mpu6050_accel_scalar(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr)
{
    // Get AFS_SEL 
    mpu6050_afs_sel_set_t afs_sel = ((mpu6050_accel_config_read(i2c, addr) & 
                                      MPU6050_AFS_SEL_MASK) >> SHIFT_3); 
    
    return (float)(MPU6050_AFS_SEL_MAX >> afs_sel); 
}


// MPU6050 gyroscope x-axis raw value 
int16_t mpu6050_get_gyro_x_raw(void)
{
    return mpu6050_com_data.gyro_data.gyro_x; 
}


// MPU6050 gyroscope y-axis raw value 
int16_t mpu6050_get_gyro_y_raw(void)
{
    return mpu6050_com_data.gyro_data.gyro_y; 
}


// MPU6050 gyroscope z-axis raw value 
int16_t mpu6050_get_gyro_z_raw(void)
{
    return mpu6050_com_data.gyro_data.gyro_z; 
}


// MPU6050 gyroscopic (angular acceleration) calculation around x-axis 
float mpu6050_get_gyro_x(
    int16_t gyro_x_axis_raw,
    int16_t gyro_x_axis_offset)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration 
    return (mpu6050_get_gyro_x_raw() - gyro_x_axis_offset) / mpu6050_com_data.gyro_data_scalar; 
}


// MPU6050 gyroscopic value calculation around y-axis 
float mpu6050_get_gyro_y(
    int16_t gyro_y_axis_raw,
    int16_t gyro_y_axis_offset)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration 
    return (mpu6050_get_gyro_y_raw() - gyro_y_axis_offset) / mpu6050_com_data.gyro_data_scalar; 
}


// MPU6050 gyroscopic value calculation around z-axis 
float mpu6050_get_gyro_z(
    int16_t gyro_z_axis_raw,
    int16_t gyro_z_axis_offset)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration 
    return (mpu6050_get_gyro_z_raw() - gyro_z_axis_offset) / mpu6050_com_data.gyro_data_scalar; 
}


// MPU6050 gyroscope scalar
float mpu6050_gyro_scalar(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr)
{
    // Get FS_SEL 
    mpu6050_fs_sel_set_t fs_sel = ((mpu6050_gyro_config_read(i2c, addr) & 
                                    MPU6050_FS_SEL_MASK) >> SHIFT_3); 
    
    // Calculate the gyroscope calculation scalar 
    return ((MPU6050_FS_SEL_MAX >> fs_sel) + ((fs_sel & MPU6050_FS_CORRECTION) >> SHIFT_1)) / 
            (float)(MPU6050_GYRO_SCALAR); 

    // float gyro_scalar;

    // // Get FS_SEL 
    // uint8_t fs_sel = ((mpu6050_gyro_config_read(mpu6050_address) & 
    //                    MPU6050_FS_SEL_MASK) >> SHIFT_3);
    
    // // Return the scalar 
    // switch (fs_sel)
    // {
    //     case FS_SEL_250:
    //         gyro_scalar = GYRO_SCALE_FS_SEL_250 / ((float)(MPU6050_GYRO_SCALAR));
    //         break;
        
    //     case FS_SEL_500:
    //         gyro_scalar = GYRO_SCALE_FS_SEL_500 / ((float)(MPU6050_GYRO_SCALAR));
    //         break;
        
    //     case FS_SEL_1000:
    //         gyro_scalar = GYRO_SCALE_FS_SEL_1000 / ((float)(MPU6050_GYRO_SCALAR));
    //         break;
        
    //     case FS_SEL_2000:
    //         gyro_scalar = GYRO_SCALE_FS_SEL_2000 / ((float)(MPU6050_GYRO_SCALAR));
    //         break;
        
    //     default:
    //         gyro_scalar = 0;
    //         break;
    // }

    // // Divide the scalar to restore decimal places
    // return gyro_scalar; 
}


// MPU6050 temperature sensor raw value 
int16_t mpu6050_get_temp_raw(void)
{
    return mpu6050_com_data.other_data.temp; 
}


// MPU6050 temperature sensor calculation 
float mpu6050_get_temp(
    int16_t temp_raw)
{
    // Get the true temperature in degC
    return mpu6050_get_temp_raw() / ((float)(MPU6050_TEMP_SENSIT)) + 
           MPU6050_TEMP_OFFSET / ((float)(MPU6050_TEMP_SCALAR)); 
}

//=======================================================================================
