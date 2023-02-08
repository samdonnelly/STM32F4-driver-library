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
    byte_num_t mpu6050_reg_size,
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
    byte_num_t mpu6050_reg_size,
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
 * @brief Interrupt configuration register write 
 * 
 * @details 
 * 
 * @param i2c 
 * @param addr 
 * @param latch_int_en 
 * @param int_rd_clear 
 */
void mpu6050_int_pin_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_mode_t latch_int_en, 
    mpu6050_mode_t int_rd_clear); 


/**
 * @brief Interrupt enable register write 
 * 
 * @details 
 * 
 * @param i2c 
 * @param addr 
 * @param data_rdy_en 
 */
void mpu6050_int_enable_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_mode_t data_rdy_en); 


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
 *          
 *          The 6 least significant bits of standby_status represent the standby status of each 
 *          device reading. If the bit is set to 1 it means that reading will be put on standby 
 *          and therefore not be reported when reading from the register. If you want all the 
 *          data to be available you can set all the bits to zero. The order of the bits are 
 *          as follows: 
 *          - STBY_XA : bit 5 --> Accelerometer x-axis 
 *          - STBY_YA : bit 4 --> Accelerometer y-axis 
 *          - STBY_ZA : bit 3 --> Accelerometer z-axis 
 *          - STBY_XA : bit 2 --> Gyroscope about x-axis 
 *          - STBY_YG : bit 1 --> Gyroscope about y-axis 
 *          - STBY_ZG : bit 0 --> Gyroscope about z-axis 
 *          
 *          The two most significant bits of the standby_status byte do nothing. 
 * 
 * @see mpu6050_lp_wake_ctrl_t
 * 
 * @param i2c : 
 * @param mpu6050_address : I2C address of MPU6050 
 * @param lp_wake_ctrl : specify wake-up frequency in accelerometer only low power mode
 * @param standby_status : 
 */
void mpu6050_pwr_mgmt_2_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    uint8_t standby_status);


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
 * @brief MPU6050 self-test result calculation 
 * 
 * @details Determines the results (pass/fail) of the device self-test and stores the 
 *          result in "results". This function is called for both the accelerometer and 
 *          gyroscope (separately) and stores the result in bits 0-2 of "result" for the 
 *          accelerometer and bits 3-5 for the gyroscope. mpu6050_self_test
 *          calls this function to calculate the result. See mpu6050_self_test
 *          documentation for a breakdown on reading these results. The result is 
 *          determined for each axis by taking the output of the following equation
 *          and comparing it against the maximum allowable change from factory trim.
 *          
 *          Change from factory trim = ((self-test response) - (factory trim)) / 
 *                                     (factory trim)
 * 
 * @param self_test_results 
 * @param factory_trim 
 * @param results 
 * @param shift 
 */
void mpu6050_self_test_result(
    int16_t *self_test_results,
    float *factory_trim,
    uint8_t *results, 
    self_test_result_shift_t shift); 


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
    int16_t gyro_x_offset;          // Angular velocity offset about the x-axis 
    int16_t gyro_y_offset;          // Angular velocity offset about the y-axis 
    int16_t gyro_z_offset;          // Angular velocity offset about the z-axis 
}
mpu6050_gyro_t; 


// Other device data 
typedef struct mpu6050_other_s
{
    int16_t temp;                   // Temperature 
}
mpu6050_other_t; 


// MPU6050 data record structure 
typedef struct mpu6050_com_data_s 
{
    // Pointer for linked list 
    struct mpu6050_com_data_s *next_device; 

    // Peripherals 
    I2C_TypeDef *i2c;                // I2C port connected to the device 
    GPIO_TypeDef *gpio;              // GPIO port for the INT pin 
    pin_selector_t int_pin;          // INT pin number 

    // Device information 
    mpu6050_i2c_addr_t addr;         // Device I2C address 
    float accel_data_scalar;         // Scales accelerometer raw data into readable values 
    float gyro_data_scalar;          // Scales gyroscope raw data into readable values 

    // Data 
    mpu6050_accel_t accel_data;      // Accelerometer data 
    mpu6050_gyro_t  gyro_data;       // Gyroscope data 
    mpu6050_other_t other_data;      // Other device data 
    MPU6050_FAULT_FLAG fault_flag;   // Driver fault flag 
}
mpu6050_com_data_t; 


// MPU6050 device com data record instance 
static mpu6050_com_data_t mpu6050_com_data; 
static mpu6050_com_data_t *mpu6050_com_data_ptr = NULL; 

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
MPU6050_INIT_STATUS mpu6050_init(
    device_number_t device_num, 
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t mpu6050_addr,
    uint8_t standby_status, 
    mpu6050_dlpf_cfg_t dlpf_cfg,
    mpu6050_smplrt_div_t smplrt_div,
    mpu6050_afs_sel_set_t afs_sel,
    mpu6050_fs_sel_set_t fs_sel)
{
    // Create a data record if it does not already exist 
    mpu6050_com_data_t *mpu6050_driver_data = 
        (mpu6050_com_data_t *)create_linked_list_entry(
                                        device_num, 
                                        (void *)mpu6050_com_data_ptr, 
                                        sizeof(mpu6050_com_data_t)); 

    mpu6050_driver_data->i2c = i2c; 
    mpu6050_driver_data->addr = mpu6050_addr; 
    mpu6050_driver_data->fault_flag = CLEAR; 

    // Assign device information 
    mpu6050_com_data.i2c = i2c; 
    mpu6050_com_data.addr = mpu6050_addr; 
    mpu6050_com_data.fault_flag = CLEAR; 

    // Read the WHO_AM_I register to establish that there is communication 
    if (mpu6050_who_am_i_read(mpu6050_com_data.i2c, 
                              mpu6050_com_data.addr) != MPU6050_7BIT_ADDR)
    {
        mpu6050_com_data.fault_flag |= SET_BIT; 
    }
    else 
    {
        // Choose which sensors to use and frquency of CYCLE mode
        mpu6050_pwr_mgmt_2_write(
            mpu6050_com_data.i2c, 
            mpu6050_com_data.addr,
            LP_WAKE_CTRL_0,
            standby_status);

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

        // Store the raw data scalars for calculating the actual value 
        mpu6050_com_data.accel_data_scalar = mpu6050_accel_scalar(mpu6050_com_data.i2c, 
                                                                  mpu6050_com_data.addr); 

        mpu6050_com_data.gyro_data_scalar = mpu6050_gyro_scalar(mpu6050_com_data.i2c, 
                                                                mpu6050_com_data.addr); 

        // Record the reference (error) in the gyroscope to correct the readings 
        mpu6050_com_data.gyro_data.gyro_x_offset = CLEAR; 
        mpu6050_com_data.gyro_data.gyro_y_offset = CLEAR; 
        mpu6050_com_data.gyro_data.gyro_z_offset = CLEAR; 
    }

    return mpu6050_com_data.fault_flag; 
}


// MPU6050 INT pin initialization 
void mpu6050_int_pin_init(
    GPIO_TypeDef *gpio, 
    pin_selector_t pin)
{
    // Update the data record 
    mpu6050_com_data.gpio = gpio; 
    mpu6050_com_data.int_pin = pin; 

    // Configure the GPIO input pin 
    gpio_pin_init(mpu6050_com_data.gpio, 
                  mpu6050_com_data.int_pin, 
                  MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD); 

    // Interrupt configuration register 
    mpu6050_int_pin_config_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_MODE_ENABLE, 
        MPU6050_MODE_ENABLE); 

    // Interrupt enable register 
    mpu6050_int_enable_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_MODE_ENABLE); 
}

//=======================================================================================


//=======================================================================================
// Read and Write Functions 

// MPU6050 write to register 
void mpu6050_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_register_addresses_t mpu6050_register,
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c_start(i2c); 

    // Send the MPU6050 address with a write offset
    i2c_write_address(i2c, addr + MPU6050_W_OFFSET);
    i2c_clear_addr(i2c);

    // Send the register address that is going to be written to 
    i2c_write_master_mode(i2c, &mpu6050_register, BYTE_1);

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
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    // Create start condition to initiate master mode 
    i2c_start(i2c); 

    // Send the MPU6050 address with a write offset 
    i2c_write_address(i2c, addr + MPU6050_W_OFFSET);
    i2c_clear_addr(i2c);

    // Send the register address that is going to be read 
    i2c_write_master_mode(i2c, &mpu6050_register, BYTE_1);

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
// Configuration functions 

// MPU6050 reference point set 
void mpu6050_calibrate(void)
{
    mpu6050_gyro_read(); 
    mpu6050_com_data.gyro_data.gyro_x_offset = mpu6050_com_data.gyro_data.gyro_x; 
    mpu6050_com_data.gyro_data.gyro_y_offset = mpu6050_com_data.gyro_data.gyro_y; 
    mpu6050_com_data.gyro_data.gyro_z_offset = mpu6050_com_data.gyro_data.gyro_z; 
}


// MPU6050 low power mode config 
void mpu6050_low_pwr_config(
    mpu6050_sleep_mode_t sleep)
{
    mpu6050_pwr_mgmt_1_write(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr,
        DEVICE_RESET_DISABLE,
        sleep,
        CYCLE_SLEEP_DISABLED,
        TEMP_SENSOR_ENABLE,
        CLKSEL_5);
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
}

//=======================================================================================


//=======================================================================================
// Register Functions

// MPU6050 Accelerometer Measurements registers read
void mpu6050_accel_read(void)
{
    // Temporary data storage 
    uint8_t accel_data_reg[BYTE_6];

    // Read the accelerometer data 
    mpu6050_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_ACCEL_XOUT_H,
        BYTE_6,
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
void mpu6050_gyro_read(void)
{
    // Temporary data storage 
    uint8_t gyro_data_reg[BYTE_6];

    // Read the gyroscope data 
    mpu6050_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_GYRO_XOUT_H,
        BYTE_6,
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
void mpu6050_temp_read(void)
{
    // Store the temperature data 
    uint8_t mpu6050_temp_sensor_reg[BYTE_2]; 

    // Read the temperature data 
    mpu6050_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_TEMP_OUT_H,
        BYTE_2,
        mpu6050_temp_sensor_reg);

    // Generate an unformatted signed integer from the combine the return values 
    mpu6050_com_data.other_data.temp = (int16_t)((mpu6050_temp_sensor_reg[0] << SHIFT_8) |
                                                 (mpu6050_temp_sensor_reg[1] << SHIFT_0)); 
}


// MPU6050 read all 
void mpu6050_read_all(void)
{
    // Temporary data storage 
    uint8_t data_reg[BYTE_14];

    // Read the accelerometer data 
    mpu6050_read(
        mpu6050_com_data.i2c, 
        mpu6050_com_data.addr, 
        MPU6050_ACCEL_XOUT_H,
        BYTE_14,
        data_reg);

    // Combine the return values into signed integers - values are unformatted
    mpu6050_com_data.accel_data.accel_x = (int16_t)((data_reg[0] << SHIFT_8) |
                                                    (data_reg[1] << SHIFT_0));
    mpu6050_com_data.accel_data.accel_y = (int16_t)((data_reg[2] << SHIFT_8) |
                                                    (data_reg[3] << SHIFT_0));
    mpu6050_com_data.accel_data.accel_z = (int16_t)((data_reg[4] << SHIFT_8) |
                                                    (data_reg[5] << SHIFT_0));

    // Generate an unformatted signed integer from the combine the return values 
    mpu6050_com_data.other_data.temp = (int16_t)((data_reg[6] << SHIFT_8) |
                                                 (data_reg[7] << SHIFT_0)); 

    // Combine the return values into signed integers - values are unformatted
    mpu6050_com_data.gyro_data.gyro_x = (int16_t)((data_reg[8] << SHIFT_8) |
                                                  (data_reg[9] << SHIFT_0));
    mpu6050_com_data.gyro_data.gyro_y = (int16_t)((data_reg[10] << SHIFT_8) |
                                                  (data_reg[11] << SHIFT_0));
    mpu6050_com_data.gyro_data.gyro_z = (int16_t)((data_reg[12] << SHIFT_8) |
                                                  (data_reg[13] << SHIFT_0));
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
        BYTE_1,
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
        BYTE_1,
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
        BYTE_1,
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
        BYTE_1,
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
        BYTE_1,
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
        BYTE_1,
        &mpu6050_accel_config);

    // Return the value of ACCEL_CONFIG 
    return mpu6050_accel_config;
}


// Interrupt configuration register write 
void mpu6050_int_pin_config_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_mode_t latch_int_en, 
    mpu6050_mode_t int_rd_clear)
{
    // Configure the data
    uint8_t mpu6050_int_config = (latch_int_en << SHIFT_5) | 
                                 (int_rd_clear << SHIFT_4); 
    
    // Write to the INT pin configuration register (register 55) 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_INT_CONFIG,
        BYTE_1,
        &mpu6050_int_config);
}


// Interrupt enable register write 
void mpu6050_int_enable_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_mode_t data_rdy_en)
{
    // Configure the data
    uint8_t mpu6050_int_enable = (data_rdy_en << SHIFT_0); 
    
    // Write to the Interrupt Enable register (register 56) 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_INT_ENABLE,
        BYTE_1,
        &mpu6050_int_enable);
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
        BYTE_1,
        &mpu6050_pwr_mgmt_1);
}


// MPU6050 Power Manangement 2 register write
void mpu6050_pwr_mgmt_2_write(
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t addr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    uint8_t standby_status)
{
    // Configure the data 
    uint8_t mpu6050_pwr_mgmt_2 = (lp_wake_ctrl << SHIFT_6) | 
                                 (standby_status & MPU6050_STBY_STATUS_MASK); 
    
    // Write to the Power Management 2 register 
    mpu6050_write(
        i2c, 
        addr, 
        MPU6050_PWR_MGMT_2,
        BYTE_1,
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
        BYTE_1,
        &mpu6050_who_am_i);

    // Return the value of who_am_i 
    return mpu6050_who_am_i;
}

//=======================================================================================


//=======================================================================================
// Self-test 

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
    // Used to record the existing full scale range 
    uint8_t accel_fsr;
    uint8_t gyro_fsr;

    // Sensor readings with no self-test 
    int16_t accel_no_st[MPU6050_NUM_AXIS];
    int16_t gyro_no_st[MPU6050_NUM_AXIS];

    // Sensor readings with self-test 
    int16_t accel_st[MPU6050_NUM_AXIS];
    int16_t gyro_st[MPU6050_NUM_AXIS];

    // Self-test result 
    int16_t accel_str[MPU6050_NUM_AXIS];
    int16_t gyro_str[MPU6050_NUM_AXIS];

    // Self-test register readings 
    uint8_t accel_test[MPU6050_NUM_AXIS];
    uint8_t gyro_test[MPU6050_NUM_AXIS];

    // Factory trim calculation 
    float accel_ft[MPU6050_NUM_AXIS];
    float gyro_ft[MPU6050_NUM_AXIS];

    // Status of the self-test 
    uint8_t self_test_result = 0;

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
    mpu6050_accel_read(); 
    mpu6050_gyro_read(); 
    accel_no_st[0] = mpu6050_com_data.accel_data.accel_x; 
    accel_no_st[1] = mpu6050_com_data.accel_data.accel_y; 
    accel_no_st[2] = mpu6050_com_data.accel_data.accel_z; 
    gyro_no_st[0] = mpu6050_com_data.gyro_data.gyro_x; 
    gyro_no_st[1] = mpu6050_com_data.gyro_data.gyro_y; 
    gyro_no_st[2] = mpu6050_com_data.gyro_data.gyro_z; 

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
    mpu6050_accel_read(); 
    mpu6050_gyro_read(); 
    accel_st[0] = mpu6050_com_data.accel_data.accel_x; 
    accel_st[1] = mpu6050_com_data.accel_data.accel_y; 
    accel_st[2] = mpu6050_com_data.accel_data.accel_z; 
    gyro_st[0] = mpu6050_com_data.gyro_data.gyro_x; 
    gyro_st[1] = mpu6050_com_data.gyro_data.gyro_y; 
    gyro_st[2] = mpu6050_com_data.gyro_data.gyro_z; 
    
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
        MPU6050_NUM_AXIS);
    mpu6050_str_calc(
        gyro_str,
        gyro_no_st,
        gyro_st,
        MPU6050_NUM_AXIS);
    
    // Calculate the change from factory trim and check against the acceptable range 
    mpu6050_self_test_result(
        accel_str,
        accel_ft,
        &self_test_result, 
        SELF_TEST_RESULT_SHIFT_ACCEL); 
    mpu6050_self_test_result(
        gyro_str,
        gyro_ft,
        &self_test_result, 
        SELF_TEST_RESULT_SHIFT_GYRO); 
    
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

    // Update the fault flags 
    mpu6050_com_data.fault_flag |= (self_test_result << SHIFT_2); 
    
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
    uint8_t st_data[BYTE_4];

    // Read the value of SELF_TEST registers 
    mpu6050_read(
        i2c, 
        addr, 
        MPU6050_SELF_TEST, 
        BYTE_4,
        st_data);
    
    // Parse the acceleration data in X, Y and Z respectively 
    *accel_st_data++ = ((st_data[BYTE_0] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                       ((st_data[BYTE_3] & SELF_TEST_MASK_XA_TEST_LO) >> SHIFT_4);
    
    *accel_st_data++ = ((st_data[BYTE_1] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                       ((st_data[BYTE_3] & SELF_TEST_MASK_YA_TEST_LO) >> SHIFT_2);
    
    *accel_st_data = ((st_data[BYTE_2] & SELF_TEST_MASK_A_TEST_HI)  >> SHIFT_3) |
                     ((st_data[BYTE_3] & SELF_TEST_MASK_ZA_TEST_LO) >> SHIFT_0);

    // Parse the gyro data in X, Y and Z respectively 
    *gyro_st_data++ = st_data[BYTE_0] & SELF_TEST_MASK_X_TEST;
    
    *gyro_st_data++ = st_data[BYTE_1] & SELF_TEST_MASK_X_TEST;
    
    *gyro_st_data = st_data[BYTE_2] & SELF_TEST_MASK_X_TEST;
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
    for (uint8_t i = 0; i < MPU6050_NUM_AXIS; i++)
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
    for (uint8_t i = 0; i < MPU6050_NUM_AXIS; i++)
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


// MPU6050 self-test result calculation 
void mpu6050_self_test_result(
    int16_t *self_test_results,
    float *factory_trim,
    uint8_t *results, 
    self_test_result_shift_t shift)
{
    // Place to store the % change 
    float ft_change;

    // Determine the result of the self-test 
    for (uint8_t i = 0; i < MPU6050_NUM_AXIS; i++)
    {
        // Check % change from factory trim 
        ft_change = (*self_test_results - *factory_trim) / *factory_trim;
        
        // Check change against maximum allowed value 
        if ((ft_change > MPU6050_FT_MAX_ERROR) || (ft_change < -(MPU6050_FT_MAX_ERROR)))
        {
            *results |= (shift << SHIFT_1*i); 
        }

        self_test_results++;
        factory_trim++;
    }
}

//=======================================================================================


//=======================================================================================
// Setters 

// Clear driver fault flag 
void mpu6050_clear_fault_flag(void)
{
    mpu6050_com_data.fault_flag = CLEAR; 
}

//=======================================================================================


//=======================================================================================
// Getters 

// Get driver fault flag 
MPU6050_FAULT_FLAG mpu6050_get_fault_flag(void)
{
    return mpu6050_com_data.fault_flag; 
}


// MPU6050 INT pin status 
MPU6050_INT_STATUS mpu6050_int_status(void)
{
    return gpio_read(mpu6050_com_data.gpio, (SET_BIT << mpu6050_com_data.int_pin)); 
}


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
float mpu6050_get_accel_x(void)
{
    // Get the raw value scalar and calculate the true x-axis acceleration 
    return mpu6050_get_accel_x_raw() / mpu6050_com_data.accel_data_scalar; 
}


// MPU6050 accelerometer y-axis calculation 
float mpu6050_get_accel_y(void)
{
    // Get the raw value scalar and calculate the true y-axis acceleration 
    return mpu6050_get_accel_y_raw() / mpu6050_com_data.accel_data_scalar; 
}


// MPU6050 accelerometer z-axis calculation 
float mpu6050_get_accel_z(void)
{
    // Get the raw value scalar and calculate the true z-axis acceleration 
    return mpu6050_get_accel_z_raw() / mpu6050_com_data.accel_data_scalar; 
}


// MPU6050 gyroscope x-axis raw value 
int16_t mpu6050_get_gyro_x_raw(void)
{
    return (mpu6050_com_data.gyro_data.gyro_x - mpu6050_com_data.gyro_data.gyro_x_offset); 
}


// MPU6050 gyroscope y-axis raw value 
int16_t mpu6050_get_gyro_y_raw(void)
{
    return (mpu6050_com_data.gyro_data.gyro_y - mpu6050_com_data.gyro_data.gyro_y_offset); 
}


// MPU6050 gyroscope z-axis raw value 
int16_t mpu6050_get_gyro_z_raw(void)
{
    return (mpu6050_com_data.gyro_data.gyro_z - mpu6050_com_data.gyro_data.gyro_z_offset); 
}


// MPU6050 gyroscopic (angular acceleration) calculation around x-axis 
float mpu6050_get_gyro_x(void)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration 
    return (mpu6050_get_gyro_x_raw() / mpu6050_com_data.gyro_data_scalar); 
}


// MPU6050 gyroscopic value calculation around y-axis 
float mpu6050_get_gyro_y(void)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration 
    return (mpu6050_get_gyro_y_raw() / mpu6050_com_data.gyro_data_scalar); 
}


// MPU6050 gyroscopic value calculation around z-axis 
float mpu6050_get_gyro_z(void)
{
    // Get the raw value scalar and calculate the true x-axis angular acceleration 
    return (mpu6050_get_gyro_z_raw() / mpu6050_com_data.gyro_data_scalar); 
}


// MPU6050 temperature sensor raw value 
int16_t mpu6050_get_temp_raw(void)
{
    return mpu6050_com_data.other_data.temp; 
}


// MPU6050 temperature sensor calculation 
float mpu6050_get_temp(void)
{
    // Get the true temperature in degC
    return mpu6050_get_temp_raw() / ((float)(MPU6050_TEMP_SENSIT)) + 
           MPU6050_TEMP_OFFSET / ((float)(MPU6050_TEMP_SCALAR)); 
}

//=======================================================================================
