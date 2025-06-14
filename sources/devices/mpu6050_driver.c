/**
 * @file mpu6050_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 IMU driver 
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
// Macros 

// Registers 
#define MPU6050_SELF_TEST    0x0D        // Register 13  - Self-test 
#define MPU6050_SMPRT_DIV    0x19        // Register 25  - Sample Rate Divider 
#define MPU6050_CONFIG       0x1A        // Register 26  - Configuration 
#define MPU6050_GYRO_CONFIG  0x1B        // Register 27  - Gyroscope configuration 
#define MPU6050_ACCEL_CONFIG 0x1C        // Register 28  - Accelerometer configuration 
#define MPU6050_INT_CONFIG   0x37        // Register 55  - Interrupt configuration 
#define MPU6050_INT_ENABLE   0x38        // Register 56  - Interrupt enable 
#define MPU6050_ACCEL_XOUT_H 0x3B        // Register 59  - Accelerometer x-axis high byte 
#define MPU6050_TEMP_OUT_H   0x41        // Register 65  - Temperature high byte 
#define MPU6050_GYRO_XOUT_H  0x43        // Register 67  - Gyroscope x-axis high byte 
#define MPU6050_PWR_MGMT_1   0x6B        // Register 107 - Power management 1 
#define MPU6050_PWR_MGMT_2   0x6C        // Register 108 - Power management 2 
#define MPU6050_WHO_AM_I     0x75        // Register 117 - Who Am I 

// Temperature sensor 
#define MPU6050_TEMP_SENSIT 340.0f       // Sensitivity (LSB/degC) - MPU6050 defined scalar
#define MPU6050_TEMP_OFFSET 36.53f       // Temperature offset scaled by MPU6050_TEMP_SCALAR

// Accelerometer 
#define MPU6050_AFS_SEL_MAX 16384        // Max accelerometer calculation scalar 
#define MPU6050_ACCEL_ST_FT_C1 142       // Accelerometer factory trim calc constant 1 
#define MPU6050_ACCEL_ST_FT_C2 6056      // Accelerometer factory trim calc constant 2 
#define MPU6050_ACCEL_ST_FT_C4 13452     // Accelerometer factory trim calc constant 3 
#define MPU6050_ACCEL_ST_FT_C3 45752     // Accelerometer factory trim calc constant 4 

// Gyroscope 
#define MPU6050_FS_SEL_MAX 1310          // Max gyroscopic calculation scalar 
#define MPU6050_FS_CORRECTION 0x02       // Gyroscope calculation correction mask 
#define MPU6050_GYRO_SCALAR 10           // Unscales scaled mpu6050_gyro_scalars_t values 
#define MPU6050_GYRO_ST_FT_C1 1001       // Gyroscope factory trim calc constant 1 
#define MPU6050_GYRO_ST_FT_C3 15056      // Gyroscope factory trim calc constant 2 
#define MPU6050_GYRO_ST_FT_C2 19244      // Gyroscope factory trim calc constant 3 
#define MPU6050_GYRO_ST_FT_C4 31125      // Gyroscope factory trim calc constant 4 

// Self-Test 
#define MPU6050_ST_MASK_ZA_TEST_LO 0x03  // Mask to parse self-test y-axis accelerometer data 
#define MPU6050_ST_MASK_YA_TEST_LO 0x0C  // Mask to parse self-test z-axis accelerometer data 
#define MPU6050_ST_MASK_X_TEST     0x1F  // Mask to parse self-test gyroscope data 
#define MPU6050_ST_MASK_XA_TEST_LO 0x30  // Mask to parse self-test x-axis accelerometer data 
#define MPU6050_ST_MASK_A_TEST_HI  0xE0  // Mask to parse self-test x, y and z axis accel data 
#define MPU6050_STR_SHIFT_ACCEL    0x01  // Bit shift for accel self-test results 
#define MPU6050_STR_SHIFT_GYRO     0x08  // Bit shift for gyro self-test results 

//=======================================================================================


//=======================================================================================
// Messages 

// MPU6050 data record structure 
typedef struct mpu6050_driver_data_s 
{
    // Linked list tracking 
    struct mpu6050_driver_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripherals 
    I2C_TypeDef *i2c;                // I2C port connected to the device 
    GPIO_TypeDef *gpio;              // GPIO port for the INT pin 

    // Device information 
    mpu6050_i2c_addr_t addr;         // Device I2C address 
    pin_selector_t int_pin;          // INT pin number 
    float accel_data_scalar;         // Scales accelerometer raw data into readable values 
    float gyro_data_scalar;          // Scales gyroscope raw data into readable values 

    // Data 
    int16_t accel[NUM_AXES];           // Accelerometer data 
    int16_t gyro[NUM_AXES];            // Gyroscope data 
    int16_t gyro_offsets[NUM_AXES];    // Gyroscope offsets 
    int16_t temp;                      // Temperature 

    // Status info 
    // 'status' --> bit 0: i2c status (see i2c_status_t) 
    //          --> bit 1: init status (WHO_AM_I) 
    //          --> bits 2-7: self test results 
    uint8_t status; 
}
mpu6050_driver_data_t; 


// Driver data record first pointer 
static mpu6050_driver_data_t *mpu6050_driver_data = NULL; 

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief MPU6050 write to register 
 * 
 * @details Writes data to the device over the I2C bus. This function is used by the 
 *          register functions. The register address specifies where to begin writing in 
 *          the devices memory and the register size argument specifies the number of 
 *          bytes in memory to write. Bytes in registers are written successively. The 
 *          register addresses are defined in the header file. 
 * 
 * @param device_ptr : pointer to device data record 
 * @param mpu6050_register : register address within the device 
 * @param mpu6050_reg_size : register size (bytes) 
 * @param mpu6050_reg_value : pointer to data to write to the register 
 */
void mpu6050_write(
    mpu6050_driver_data_t *device_ptr, 
    MPU6050_REG_ADDR mpu6050_register,
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);


/**
 * @brief MPU6050 read from register 
 * 
 * @details Reads data to the device over the I2C bus. This function is used by the 
 *          register functions. The register address specifies where to begin reading in 
 *          the devices memory and the register size argument specifies the number of 
 *          bytes in memory to read. Bytes in registers are read successively. The 
 *          register addresses are defined in the header file. 
 * 
 * @param device_ptr : pointer to device data record 
 * @param mpu6050_register : register address within the device 
 * @param mpu6050_reg_size : register size (bytes) 
 * @param mpu6050_reg_value : pointer to buffer to store data 
 */
void mpu6050_read(
    mpu6050_driver_data_t *device_ptr, 
    MPU6050_REG_ADDR mpu6050_register, 
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);


/**
 * @brief MPU6050 Sample Rate Divider (SMPRT_DIV) register write 
 * 
 * @details Register number: 25 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - SMPLRT_DIV: sample rate divider - 8-bit unsigned value 
 *          
 *          This register specifies the value used to divide the gyroscope output rate 
 *          to get the device sample rate. The sample rate is calculated as follows: 
 *          
 *          Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV) 
 *          
 *          The Gyroscope Output Rate is determined by the digital low pass filter. The 
 *          accelerometer output rate is always 1kHz so a sample rate greater than this 
 *          will produce repeated accelerometer readings. If the digital low pass filter 
 *          is 0 or 7 then the gyroscope output rate is (typically) 1kHz. When it is 1-6 
 *          then the gyroscope output rate is (typically) 8kHz. Note that the output rate 
 *          is dependent on the clock chosen. 
 * 
 * @see mpu6050_config_write
 * 
 * @param device_ptr : pointer to device data record 
 * @param smprt_div : sample rate divider address
 */
void mpu6050_smprt_div_write(
    mpu6050_driver_data_t *device_ptr, 
    MPU6050_SMPLRT_DIV smprt_div);


/**
 * @brief MPU6050 Configuration (CONFIG) register write
 * 
 * @details Register number: 26 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - EXT_SYNC_SET: external frame synchronization - 3-bit unsigned value 
 *          - DLPF_CFG: digital low pass filter - 3-bit unsigned value 
 *          
 *          The external frame synchronization (FSYNC) feature is not used. The digital low 
 *          pass filter (DLPF) is used to set the bandwidth of the data let through for the 
 *          accelerometer and gyroscope. A lower frequency bandwidth takes longer to get. 
 * 
 * @see mpu6050_dlpf_cfg_t
 * 
 * @param device_ptr : pointer to device data record 
 * @param ext_sync_set : FSYNC pin sampling setpoint 
 * @param dlpf_cfg : digital low pass filter setpoint for all axes
 */
void mpu6050_config_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t ext_sync_set,
    mpu6050_dlpf_cfg_t dlpf_cfg);


/**
 * @brief MPU6050 Gyroscope Configuration (GYRO_CONFIG) register write
 * 
 * @details Register number: 27 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - XG_ST: gyroscope x-axis self-test - bit 7 
 *          - YG_ST: gyroscope y-axis self-test - bit 6 
 *          - ZG_ST: gyroscope z-axis self-test - bit 5 
 *          - FS_SEL: gyroscope full scale range - 2-bit (bits 3-4) unsigned value 
 *          
 *          This register is used to trigger gyroscope self-test for each axis and 
 *          configure the gyroscope full scale range. Self test is used to check 
 *          the validity of the device readings and this mode is only set when using 
 *          mpu6050_self_test. The full scale range defines the range of gyroscope 
 *          readings used (in deg/s). Setting a range that covers a wider range of 
 *          angular velocities will decrease the accuracy of each reading. 
 * 
 * @see mpu6050_gyro_self_test_set_t
 * @see mpu6050_fs_sel_set_t
 * @see mpu6050_self_test
 * 
 * @param device_ptr : pointer to device data record 
 * @param gyro_self_test : enables or disabled self-test mode for all axes
 * @param fs_sel : gyroscope full scale range setpoint for all axes 
 */
void mpu6050_gyro_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_gyro_self_test_set_t gyro_self_test,
    mpu6050_fs_sel_set_t fs_sel);


/**
 * @brief MPU6050 Gyroscope Configuration (GYRO_CONFIG) register read
 * 
 * @details Register number: 27 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - XG_ST: gyroscope x-axis self-test - bit 7 
 *          - YG_ST: gyroscope y-axis self-test - bit 6 
 *          - ZG_ST: gyroscope z-axis self-test - bit 5 
 *          - FS_SEL: gyroscope full scale range - 2-bit (bits 3-4) unsigned value 
 *          
 *          See the description for mpu6050_gyro_config_write for details. A read option 
 *          is provided for this register so the full scale range can be recorded before 
 *          having to set the full scale range specifically for self-test. 
 * 
 * @see mpu6050_gyro_config_write
 * @see mpu6050_self_test
 * 
 * @param device_ptr : pointer to device data record 
 * @return uint8_t : returns the unparsed self-test and full scale range setpoints 
 */
uint8_t mpu6050_gyro_config_read(mpu6050_driver_data_t *device_ptr);


/**
 * @brief MPU6050 Accelerometer Configuration (ACCEL_CONFIG) register write
 * 
 * @details Register number: 28 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - XA_ST: accelerometer x-axis self-test - bit 7 
 *          - YA_ST: accelerometer y-axis self-test - bit 6 
 *          - ZA_ST: accelerometer z-axis self-test - bit 5 
 *          - AFS_SEL: accelerometer full scale range - 2-bit (bits 3-4) unsigned value 
 *          
 *          This register is used to trigger accelerometer self-test for each axis and 
 *          configure the accelerometer full scale range. Self test is used to check 
 *          the validity of the device readings and this mode is only set when using 
 *          mpu6050_self_test. The full scale range defines the range of accelerometer 
 *          readings used (in g's). Setting a range that covers a wider range of g's 
 *          will decrease the accuracy of each reading. 
 * 
 * @see mpu6050_accel_self_test_set_t
 * @see mpu6050_afs_sel_set_t
 * @see mpu6050_self_test
 * 
 * @param device_ptr : pointer to device data record 
 * @param accel_self_test : enables of disables self-test mode for all axes 
 * @param afs_sel : accelerometer full scale range setpoint for all axes
 */
void mpu6050_accel_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_accel_self_test_set_t accel_self_test,
    mpu6050_afs_sel_set_t afs_sel);


/**
 * @brief MPU6050 Accelerometer Configuration (ACCEL_CONFIG) register read 
 * 
 * @details Register number: 28 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - XA_ST: accelerometer x-axis self-test - bit 7 
 *          - YA_ST: accelerometer y-axis self-test - bit 6 
 *          - ZA_ST: accelerometer z-axis self-test - bit 5 
 *          - AFS_SEL: accelerometer full scale range - 2-bit (bits 3-4) unsigned value 
 *          
 *          See the description for mpu6050_accel_config_write for details. A read option 
 *          is provided for this register so the full scale range can be recorded before 
 *          having to set the full scale range specifically for self-test. 
 * 
 * @see mpu6050_accel_config_write
 * @see mpu6050_self_test
 * 
 * @param device_ptr : pointer to device data record 
 * @return uint8_t : returns the unparsed self-test and full scale range setpoints 
 */
uint8_t mpu6050_accel_config_read(mpu6050_driver_data_t *device_ptr);


/**
 * @brief Interrupt configuration register write 
 * 
 * @details Register number: 55 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - LATCH_INT_EN: interrupt pin latching setting - bit 5 
 *          - INT_RD_CLEAR: Interrupt status clear method - bit 4 
 *          
 *          This register configures the behavior of the interrupt signals at the INT 
 *          pin. The pin can be configured to latch high until the interrupt is cleared 
 *          or it can emit 50us pulses. The pin can also be configured to clear on any 
 *          read operation or only when reading INT_STATUS in register 58. 
 * 
 * @see mpu6050_int_latch_t
 * @see mpu6050_int_clear_t
 * 
 * @param device_ptr : pointer to device data record 
 * @param latch_int_en : interrupt pin latch setting 
 * @param int_rd_clear : interrupt status clear method 
 */
void mpu6050_int_pin_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_int_latch_t latch_int_en, 
    mpu6050_int_clear_t int_rd_clear); 


/**
 * @brief Interrupt enable register write 
 * 
 * @details Register number: 56 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - DATA_RDY_EN: Data Ready interrupt enable - bit 0 
 *          
 *          This register allows for enabling interrupts from different sources. The 
 *          interrupt that can be enabled from this function is the Data Ready interrupt. 
 *          Data Ready occurs each time a write operation to all sensor registers has 
 *          been completed. 
 * 
 * @param device_ptr : pointer to device data record 
 * @param data_rdy_en : Data Ready interrupt enable setting 
 */
void mpu6050_int_enable_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_int_data_rdy_t data_rdy_en); 


/**
 * @brief MPU6050 Power Manangement 1 (PWR_MGMT_1) register write
 * 
 * @details Register number: 107 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - DEVICE_RESET: internal register reset - bit 7 
 *          - SLEEP : sleep mode - bit 6 
 *          - CYCLE : cycle mode - bit 5 
 *          - TEMP_DIS: temp sensor disable - bit 3
 *          - CLKSEL : clock source - 3-bit (bits 0-2) unsigned value 
 *          
 *          This register allows the user to configure the power mode and clock source. 
 *          It can also reset the device registers to their default value and disable the 
 *          temperature sensor if needed. 
 *          If the SLEEP bit is set then the device will enter into low power mode and data 
 *          will not be updated for reading. 
 *          Enabling the CYCLE bit (with SLEEP disabled) allows for low power mode with periodic 
 *          wakeups to collect data at a frequency determined by LP_WAKE_CTRL in PWR_MGMT_2. 
 *          PWR_MGMT_2 says that LP_WAKE_CTRL is for accelerometer only low power mode. 
 *          It is recomemended to use an external clock source for stability and the 
 *          ability to use low power modes. 
 * 
 * @see mpu6050_pwr_mgmt_2_write
 * @see mpu6050_device_reset_t
 * @see mpu6050_sleep_mode_t
 * @see mpu6050_cycle_t
 * @see mpu6050_temp_sensor_t
 * @see mpu6050_clksel_t
 * 
 * @param device_ptr : pointer to device data record 
 * @param device_reset : reset internal registers to default value when set to 1
 * @param sleep : enter sleep mode when set to 1 
 * @param cycle : periodic sleep mode when set to 1 (and sleep = 0) 
 * @param temp_dis : disables temperature sensor when set to 1 
 * @param clksel : specifies clock source of the device
 */
void mpu6050_pwr_mgmt_1_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_device_reset_t device_reset,
    mpu6050_sleep_mode_t sleep,
    mpu6050_cycle_t cycle,
    mpu6050_temp_sensor_t temp_dis,
    mpu6050_clksel_t clksel);


/**
 * @brief MPU6050 Power Management 2 (PWR_MGMT_2) register write
 * 
 * @details Register number: 108 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - LP_WAKE_CTRL: low power (sleep) mode wake up - 2-bit (bits 6-7) unsigned value 
 *          - STBY_XA: Accelerometer x-axis standby - bit 5 
 *          - STBY_YA: Accelerometer y-axis standby - bit 4 
 *          - STBY_ZA: Accelerometer z-axis standby - bit 3 
 *          - STBY_XA: Gyroscope x-axis standby - bit 2 
 *          - STBY_YG: Gyroscope y-axis standby - bit 1 
 *          - STBY_ZG: Gyroscope z-axis standby - bit 0 
 *          
 *          This register allows the user to configure the frequency of wake-ups in 
 *          acclerometer only low power mode. It can also put individual axes of the 
 *          accelerometer and gyroscope into low power mode. Setting a standby bit to 
 *          1 disables that piece of data. Bits 0-5 of standby_status is used to set the 
 *          standby bits (respectively) to their desired value. 
 * 
 * @see mpu6050_pwr_mgmt_1_write
 * @see mpu6050_lp_wake_ctrl_t
 * 
 * @param device_ptr : pointer to device data record 
 * @param lp_wake_ctrl : specify wake-up frequency in accelerometer only low power mode
 * @param standby_status : byte whos bits specify the standby status for each axis data 
 */
void mpu6050_pwr_mgmt_2_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    uint8_t standby_status);


/**
 * @brief MPU6050 Who Am I (WHO_AM_I) register read 
 * 
 * @details Register number: 117 
 *          Register size: 1 byte 
 *          
 *          Register data: 
 *          - WHO_AM_I: bits 1-6 
 *          
 *          This register is used to verify the identity of the device. The register contains 
 *          the upper 6-bits of the 7-bit I2C address. The address of the device is changed 
 *          using the AD0 pin but that doesn't change the value of this register. If identified 
 *          correctly, the register will return 0x68. This is used as a check during 
 *          initialization to verify the device can be seen. 
 * 
 * @param device_ptr : pointer to device data record 
 * @return uint8_t : device i@c address (will return 0x68 if correct) 
 */
uint8_t mpu6050_who_am_i_read(mpu6050_driver_data_t *device_ptr);


/**
 * @brief MPU6050 self-test read 
 * 
 * @details Called by mpu6050_self_test to read the self-test register values. After 
 *          reading the registers the accelerometer and gyroscope data is parsed to make 
 *          the data sets distinguishable from one another. The results are stored in the 
 *          buffers passed to the function. 
 * 
 * @see mpu6050_self_test
 * 
 * @param device_ptr : pointer to device data record 
 * @param accel_self_test_data : buffer where self-test accel register data gets stored 
 * @param gyro_self_test_data : buffer where self-test gyro register data gets stored 
 */
void mpu6050_self_test_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *accel_self_test_data,
    uint8_t *gyro_self_test_data);


/**
 * @brief MPU6050 self-test response calculation
 * 
 * @details This function calculates the self-test response of each sensor axis. 
 *          
 *          Self-test response = (sensor output with self-test enabled) - 
 *                               (sensor output with self-test disabled) 
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
 *          "shidt" is defined by the macros MPU6050_STR_SHIFT_ACCEL and 
 *          MPU6050_STR_SHIFT_GYRO. 
 * 
 * @param self_test_results ; buffer to store the test results 
 * @param factory_trim : buffer that contains the factory trim of each axes 
 * @param results : buffer to store the status of the results (pass(0)/fail(1))
 * @param shift : test result shift value - based on either gyroscope or accelerometer 
 */
void mpu6050_self_test_result(
    int16_t *self_test_results,
    float *factory_trim,
    uint8_t *results, 
    uint8_t shift); 


/**
 * @brief MPU6050 accelerometer factory trim 
 * 
 * @details The factory trim is used to determine how much the accelerometer sensors
 *          have drifted from their factory configuration as seen in the 
 *          mpu6050_self_test_accel_result function. Factory trim is a complex 
 *          calculation and so this function approximates the result using a third 
 *          order polynomial: 
 *          
 *          Factory trim = C1*x^3 + C2*x^2 + C3*x + C4 
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
 *          order polynomial: 
 *          
 *          Factory trim = C1*x^3 + C2*x^2 + C3*x + C4 
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
 *          converts raw sensor output into g's. This is called and the result is stored 
 *          during initialization. 
 * 
 * @see mpu6050_accel_config_read
 * 
 * @param device_ptr : pointer to device data record 
 * @return float : returns the scalar used to convert raw acceleromeeter data to g's
 */
float mpu6050_accel_scalar(mpu6050_driver_data_t *device_ptr);


/**
 * @brief MPU6050 gyroscope scalar
 * 
 * @details This function reads the GYRO_CONFIG register to check the chosen full scale
 *          range of the gyroscope. This is used to determine the scalar that  
 *          converts raw sensor output into deg/s. This is called and the result is stored 
 *          during initialization. 
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
 * @param device_ptr : pointer to device data record 
 * @return float : returns the scalar used to convert raw gyroscope data to deg/s
 */
float mpu6050_gyro_scalar(mpu6050_driver_data_t *device_ptr);

//=======================================================================================


//=======================================================================================
// Initialization 

// MPU6050 Initialization 
void mpu6050_init(
    device_number_t device_num, 
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t mpu6050_addr,
    uint8_t standby_status, 
    mpu6050_dlpf_cfg_t dlpf_cfg,
    MPU6050_SMPLRT_DIV smplrt_div,
    mpu6050_afs_sel_set_t afs_sel,
    mpu6050_fs_sel_set_t fs_sel)
{
    // Create a data record if it does not already exist 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)create_linked_list_entry(device_num, 
                                                          (void *)&mpu6050_driver_data, 
                                                          sizeof(mpu6050_driver_data_t)); 

    if (device_data == NULL)
    {
        return; 
    }

    // Initialize data record 
    device_data->status = CLEAR; 
    device_data->i2c = i2c; 
    device_data->addr = mpu6050_addr; 
    memset((void *)device_data->accel, CLEAR, sizeof(device_data->accel)); 
    memset((void *)device_data->gyro, CLEAR, sizeof(device_data->gyro)); 
    memset((void *)device_data->gyro_offsets, CLEAR, sizeof(device_data->gyro_offsets)); 
    device_data->temp = CLEAR; 

    // Read the WHO_AM_I register to establish that there is communication 
    if (mpu6050_who_am_i_read(device_data) != MPU6050_7BIT_ADDR)
    {
        device_data->status |= (SET_BIT << SHIFT_1); 
        return; 
    }

    // Choose which sensors to use and frquency of CYCLE mode
    mpu6050_pwr_mgmt_2_write(device_data, MPU6050_LP_WAKE_CTRL_0, standby_status);

    // Wake the sensor up through the power management 1 register 
    mpu6050_pwr_mgmt_1_write(
        device_data, 
        MPU6050_RESET_DISABLE,
        MPU6050_SLEEP_MODE_DISABLE,
        MPU6050_CYCLE_SLEEP_DISABLED,
        MPU6050_TEMP_SENSOR_ENABLE,
        MPU6050_CLKSEL_5);

    // Set the output rate of the gyro and accelerometer 
    mpu6050_config_write(device_data, MPU6050_EXT_SYNC_DISABLE, dlpf_cfg);
    
    // Set the Sample Rate (data rate)
    mpu6050_smprt_div_write(device_data, smplrt_div);
    
    // Configure the accelerometer register 
    mpu6050_accel_config_write(device_data, MPU6050_ACCEL_ST_DISABLE, afs_sel);
    
    // Configure the gyroscope register
    mpu6050_gyro_config_write(device_data, MPU6050_GYRO_ST_DISABLE, fs_sel);

    // Store the raw data scalars for calculating the actual value 
    device_data->accel_data_scalar = mpu6050_accel_scalar(device_data); 
    device_data->gyro_data_scalar = mpu6050_gyro_scalar(device_data); 
}


// MPU6050 INT pin initialization 
void mpu6050_int_pin_init(
    device_number_t device_num, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    }

    // Update the data record 
    device_data->gpio = gpio; 
    device_data->int_pin = pin; 

    // Configure the GPIO input pin 
    gpio_pin_init(device_data->gpio, 
                  device_data->int_pin, 
                  MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD); 

    // Interrupt configuration register 
    mpu6050_int_pin_config_write(device_data, MPU6050_INT_LATCH_HIGH, MPU6050_INT_CLEAR_RD_ANY); 

    // Interrupt enable register 
    mpu6050_int_enable_write(device_data, MPU6050_INT_DATA_RDY_ENABLE); 
}


// MPU6050 reference point set 
void mpu6050_calibrate(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    }

    // Read the current gyroscope data and save it as the offset/error 
    mpu6050_update(device_num); 
    device_data->gyro_offsets[X_AXIS] = device_data->gyro[X_AXIS]; 
    device_data->gyro_offsets[Y_AXIS] = device_data->gyro[Y_AXIS]; 
    device_data->gyro_offsets[Z_AXIS] = device_data->gyro[Z_AXIS]; 
}


// MPU6050 low power mode config 
void mpu6050_low_pwr_config(
    device_number_t device_num, 
    mpu6050_sleep_mode_t sleep)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    }

    mpu6050_pwr_mgmt_1_write(
        device_data, 
        MPU6050_RESET_DISABLE,
        sleep,
        MPU6050_CYCLE_SLEEP_DISABLED,
        MPU6050_TEMP_SENSOR_ENABLE,
        MPU6050_CLKSEL_5);
}


// MPU6050 accelerometer scalar
float mpu6050_accel_scalar(mpu6050_driver_data_t *device_ptr)
{
    // Get AFS_SEL 
    mpu6050_afs_sel_set_t afs_sel = ((mpu6050_accel_config_read(device_ptr) & 
                                      MPU6050_FSR_MASK) >> SHIFT_3); 
    
    return (float)(MPU6050_AFS_SEL_MAX >> afs_sel); 
}


// MPU6050 gyroscope scalar 
float mpu6050_gyro_scalar(mpu6050_driver_data_t *device_ptr)
{
    // Get FS_SEL 
    mpu6050_fs_sel_set_t fs_sel = ((mpu6050_gyro_config_read(device_ptr) & 
                                    MPU6050_FSR_MASK) >> SHIFT_3); 
    
    // Calculate the gyroscope calculation scalar 
    return ((MPU6050_FS_SEL_MAX >> fs_sel) + ((fs_sel & MPU6050_FS_CORRECTION) >> SHIFT_1)) / 
            (float)(MPU6050_GYRO_SCALAR); 
}

//=======================================================================================


//=======================================================================================
// Read and get data 

// Read the most recent IMU data 
void mpu6050_update(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    } 

    // Temporary data storage 
    uint8_t data_reg[BYTE_14];

    // Read and format the IMU data 
    mpu6050_read(device_data, MPU6050_ACCEL_XOUT_H, BYTE_14, data_reg); 

    // Acceleration 
    device_data->accel[X_AXIS] = (int16_t)((data_reg[0] << SHIFT_8) | (data_reg[1]));
    device_data->accel[Y_AXIS] = (int16_t)((data_reg[2] << SHIFT_8) | (data_reg[3]));
    device_data->accel[Z_AXIS] = (int16_t)((data_reg[4] << SHIFT_8) | (data_reg[5]));

    // Temperature 
    device_data->temp = (int16_t)((data_reg[6] << SHIFT_8) | (data_reg[7])); 

    // Gyroscope 
    device_data->gyro[X_AXIS] = (int16_t)((data_reg[8]  << SHIFT_8) | (data_reg[9]));
    device_data->gyro[Y_AXIS] = (int16_t)((data_reg[10] << SHIFT_8) | (data_reg[11]));
    device_data->gyro[Z_AXIS] = (int16_t)((data_reg[12] << SHIFT_8) | (data_reg[13]));
}


// Get accelerometer axis data 
void mpu6050_get_accel_axis(
    device_number_t device_num, 
    int16_t *accel_axis)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (accel_axis != NULL); i++)
    {
        accel_axis[i] = device_data->accel[i]; 
    }
}


// Get accelerometer axis g's 
void mpu6050_get_accel_axis_gs(
    device_number_t device_num, 
    float *accel_axis_gs)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (accel_axis_gs != NULL); i++)
    {
        accel_axis_gs[i] = (float)device_data->accel[i] / device_data->accel_data_scalar; 
    }
}


// Get the gyroscope axis data 
void mpu6050_get_gyro_axis(
    device_number_t device_num, 
    int16_t *gyro_axis)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (gyro_axis != NULL); i++)
    {
        gyro_axis[i] = device_data->gyro[i] - device_data->gyro_offsets[i]; 
    }
}


// Get gyroscope axis angular velocity 
void mpu6050_get_gyro_axis_rate(
    device_number_t device_num, 
    float *gyro_axis_rate)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (gyro_axis_rate != NULL); i++)
    {
        gyro_axis_rate[i] = (float)(device_data->gyro[i] - device_data->gyro_offsets[i]) / 
                             device_data->gyro_data_scalar; 
    }
}


// Temperature sensor raw value 
int16_t mpu6050_get_temp_raw(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return NULL_PTR_RETURN; 
    }

    return device_data->temp; 
}


// Temperature sensor calculation 
float mpu6050_get_temp(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return NULL_PTR_RETURN; 
    }

    // Get the true temperature in degC 
    return ((float)device_data->temp / MPU6050_TEMP_SENSIT) + MPU6050_TEMP_OFFSET; 
}

//=======================================================================================


//=======================================================================================
// Status 

// Clear status flag 
void mpu6050_clear_status(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return; 
    } 

    device_data->status = CLEAR; 
}


// Get status flag 
uint8_t mpu6050_get_status(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return NULL_PTR_RETURN; 
    }

    return device_data->status; 
}


// INT pin status 
MPU6050_INT_STATUS mpu6050_int_status(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return NULL_PTR_RETURN; 
    }

    return gpio_read(device_data->gpio, (SET_BIT << device_data->int_pin)); 
}

//=======================================================================================


//=======================================================================================
// Read and Write Functions 

// MPU6050 write to register 
void mpu6050_write(
    mpu6050_driver_data_t *device_ptr, 
    MPU6050_REG_ADDR mpu6050_register,
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    I2C_STATUS i2c_status = I2C_OK; 
    
    // Create start condition to initiate master mode 
    i2c_status |= i2c_start(device_ptr->i2c); 

    // Send the MPU6050 address with a write offset
    i2c_status |= i2c_write_addr(device_ptr->i2c, device_ptr->addr + MPU6050_W_OFFSET);
    i2c_clear_addr(device_ptr->i2c);

    // Send the register address that is going to be written to 
    i2c_status |= i2c_write(device_ptr->i2c, &mpu6050_register, BYTE_1);

    // Write the data to the MPU6050 
    i2c_status |= i2c_write(device_ptr->i2c, mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c_stop(device_ptr->i2c); 

    // Update the driver status 
    device_ptr->status |= (uint8_t)i2c_status; 
}


// MPU6050 read from register 
void mpu6050_read(
    mpu6050_driver_data_t *device_ptr, 
    MPU6050_REG_ADDR mpu6050_register, 
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Create start condition to initiate master mode 
    i2c_status |= i2c_start(device_ptr->i2c); 

    // Send the MPU6050 address with a write offset 
    i2c_status |= i2c_write_addr(device_ptr->i2c, device_ptr->addr + MPU6050_W_OFFSET);
    i2c_clear_addr(device_ptr->i2c);

    // Send the register address that is going to be read 
    i2c_status |= i2c_write(device_ptr->i2c, &mpu6050_register, BYTE_1);

    // Create another start signal 
    i2c_status |= i2c_start(device_ptr->i2c); 

    // Send the MPU6050 address with a read offset 
    i2c_status |= i2c_write_addr(device_ptr->i2c, device_ptr->addr + MPU6050_R_OFFSET);

    // Read the data sent by the MPU6050 
    i2c_status |= i2c_read(device_ptr->i2c, mpu6050_reg_value, mpu6050_reg_size);

    // Create a stop condition
    i2c_stop(device_ptr->i2c); 

    // Update the driver status 
    device_ptr->status |= (uint8_t)i2c_status; 
}

//=======================================================================================


//=======================================================================================
// Register functions 

// MPU6050 Sample Rate Divider register write 
void mpu6050_smprt_div_write(
    mpu6050_driver_data_t *device_ptr, 
    MPU6050_SMPLRT_DIV smprt_div)
{
    // Write to the Sample Rate Divider register
    mpu6050_write(device_ptr, MPU6050_SMPRT_DIV, BYTE_1, &smprt_div);
}


// MPU6050 Configuration register write
void mpu6050_config_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t ext_sync_set,
    mpu6050_dlpf_cfg_t dlpf_cfg)
{
    // Configure the data 
    uint8_t mpu6050_config = (ext_sync_set << SHIFT_3) | (dlpf_cfg << SHIFT_0);

    // Write to the Configuration register 
    mpu6050_write(device_ptr, MPU6050_CONFIG, BYTE_1, &mpu6050_config);
}


// MPU6050 Gyroscope Configuration register write
void mpu6050_gyro_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_gyro_self_test_set_t gyro_self_test,
    mpu6050_fs_sel_set_t fs_sel)
{
    // Configure the data 
    uint8_t mpu6050_gyro_config = (gyro_self_test << SHIFT_5) | (fs_sel << SHIFT_3);

    // Write to the Gyroscope Configuration register 
    mpu6050_write(device_ptr, MPU6050_GYRO_CONFIG, BYTE_1, &mpu6050_gyro_config);
}


// MPU6050 Gyroscope Configuration register read
uint8_t mpu6050_gyro_config_read(mpu6050_driver_data_t *device_ptr)
{
    uint8_t mpu6050_gyro_config;

    // Read the value of GYRO_CONFIG register 
    mpu6050_read(device_ptr, MPU6050_GYRO_CONFIG, BYTE_1, &mpu6050_gyro_config);

    // Return the value of GYRO_CONFIG 
    return mpu6050_gyro_config;
}


// MPU6050 Accelerometer Configuration register write
void mpu6050_accel_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_accel_self_test_set_t accel_self_test,
    mpu6050_afs_sel_set_t afs_sel)
{
    // Configure the data 
    uint8_t mpu6050_accel_config = (accel_self_test << SHIFT_5) | (afs_sel << SHIFT_3);

    // Write to the Accelerometer Configuration register 
    mpu6050_write(device_ptr, MPU6050_ACCEL_CONFIG, BYTE_1, &mpu6050_accel_config);
}


// MPU6050 Accelerometer Configuration register read 
uint8_t mpu6050_accel_config_read(mpu6050_driver_data_t *device_ptr)
{
    // Place to store the value of ACCEL_CONFIG
    uint8_t mpu6050_accel_config;

    // Read the value of ACCEL_CONFIG register 
    mpu6050_read(device_ptr, MPU6050_ACCEL_CONFIG, BYTE_1, &mpu6050_accel_config);

    // Return the value of ACCEL_CONFIG 
    return mpu6050_accel_config;
}


// Interrupt configuration register write 
void mpu6050_int_pin_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_int_latch_t latch_int_en, 
    mpu6050_int_clear_t int_rd_clear)
{
    // Configure the data
    uint8_t mpu6050_int_config = (latch_int_en << SHIFT_5) | (int_rd_clear << SHIFT_4); 
    
    // Write to the INT pin configuration register (register 55) 
    mpu6050_write(device_ptr, MPU6050_INT_CONFIG, BYTE_1, &mpu6050_int_config);
}


// Interrupt enable register write 
void mpu6050_int_enable_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_int_data_rdy_t data_rdy_en)
{
    // Configure the data
    uint8_t mpu6050_int_enable = (data_rdy_en << SHIFT_0); 
    
    // Write to the Interrupt Enable register (register 56) 
    mpu6050_write(device_ptr, MPU6050_INT_ENABLE, BYTE_1, &mpu6050_int_enable);
}


// MPU6050 Power Manangement 1 register write
void mpu6050_pwr_mgmt_1_write(
    mpu6050_driver_data_t *device_ptr, 
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
    mpu6050_write(device_ptr, MPU6050_PWR_MGMT_1, BYTE_1, &mpu6050_pwr_mgmt_1);
}


// MPU6050 Power Manangement 2 register write
void mpu6050_pwr_mgmt_2_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    uint8_t standby_status)
{
    // Configure the data 
    uint8_t mpu6050_pwr_mgmt_2 = (lp_wake_ctrl << SHIFT_6) | 
                                 (standby_status & MPU6050_STBY_STATUS_MASK); 
    
    // Write to the Power Management 2 register 
    mpu6050_write(device_ptr, MPU6050_PWR_MGMT_2, BYTE_1, &mpu6050_pwr_mgmt_2);
}


// MPU6050 Who Am I register read
uint8_t mpu6050_who_am_i_read(mpu6050_driver_data_t *device_ptr)
{
    uint8_t mpu6050_who_am_i;

    // Read the value of WHO_AM_I register 
    mpu6050_read(device_ptr, MPU6050_WHO_AM_I, BYTE_1, &mpu6050_who_am_i);

    return mpu6050_who_am_i;
}

//=======================================================================================


//=======================================================================================
// Self-test 

// MPU6050 self-test
MPU6050_ST_RESULT mpu6050_self_test(device_number_t device_num)
{
    // Get the device data record 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    // Check that the data record is valid 
    if (device_data == NULL) 
    {
        return NULL_PTR_RETURN; 
    }

    // Used to record the existing full scale range 
    uint8_t accel_fsr;
    uint8_t gyro_fsr;

    // Sensor readings with no self-test 
    int16_t accel_no_st[NUM_AXES];
    int16_t gyro_no_st[NUM_AXES];

    // Sensor readings with self-test 
    int16_t accel_st[NUM_AXES];
    int16_t gyro_st[NUM_AXES];

    // Self-test result 
    int16_t accel_str[NUM_AXES];
    int16_t gyro_str[NUM_AXES];

    // Self-test register readings 
    uint8_t accel_test[NUM_AXES];
    uint8_t gyro_test[NUM_AXES];

    // Factory trim calculation 
    float accel_ft[NUM_AXES];
    float gyro_ft[NUM_AXES];

    // Status of the self-test 
    uint8_t self_test_result = 0; 

    // Record the full scale range set in the init function 
    accel_fsr = ((mpu6050_accel_config_read(device_data) & MPU6050_FSR_MASK) >> SHIFT_3);
    gyro_fsr  = ((mpu6050_gyro_config_read(device_data) & MPU6050_FSR_MASK) >> SHIFT_3);

    // Set the full scale range of the accel to +/- 8g and gyro to +/- 250 deg/s
    mpu6050_accel_config_write(
        device_data, 
        MPU6050_ACCEL_ST_DISABLE,
        MPU6050_AFS_SEL_8);
    mpu6050_gyro_config_write(
        device_data, 
        MPU6050_GYRO_ST_DISABLE,
        MPU6050_FS_SEL_250);

    // Read and store the sensor values during non-self-test 
    mpu6050_update(device_num); 
    accel_no_st[X_AXIS] = device_data->accel[X_AXIS]; 
    accel_no_st[Y_AXIS] = device_data->accel[Y_AXIS]; 
    accel_no_st[Z_AXIS] = device_data->accel[Z_AXIS]; 
    gyro_no_st[X_AXIS] = device_data->gyro[X_AXIS]; 
    gyro_no_st[Y_AXIS] = device_data->gyro[Y_AXIS]; 
    gyro_no_st[Z_AXIS] = device_data->gyro[Z_AXIS]; 

    // Enable self-test 
    mpu6050_accel_config_write(
        device_data, 
        MPU6050_ACCEL_ST_ENABLE,
        MPU6050_AFS_SEL_8);
    mpu6050_gyro_config_write(
        device_data, 
        MPU6050_GYRO_ST_ENABLE,
        MPU6050_FS_SEL_250);
    
    // Read and store the sensor values during self-test 
    mpu6050_update(device_num); 
    accel_st[X_AXIS] = device_data->accel[X_AXIS]; 
    accel_st[Y_AXIS] = device_data->accel[Y_AXIS]; 
    accel_st[Z_AXIS] = device_data->accel[Z_AXIS]; 
    gyro_st[X_AXIS] = device_data->gyro[X_AXIS]; 
    gyro_st[Y_AXIS] = device_data->gyro[Y_AXIS]; 
    gyro_st[Z_AXIS] = device_data->gyro[Z_AXIS]; 
    
    // Read the self-test registers
    mpu6050_self_test_read(
        device_data, 
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
        NUM_AXES);
    mpu6050_str_calc(
        gyro_str,
        gyro_no_st,
        gyro_st,
        NUM_AXES);
    
    // Calculate the change from factory trim and check against the acceptable range 
    mpu6050_self_test_result(
        accel_str,
        accel_ft,
        &self_test_result, 
        MPU6050_STR_SHIFT_ACCEL); 
    mpu6050_self_test_result(
        gyro_str,
        gyro_ft,
        &self_test_result, 
        MPU6050_STR_SHIFT_GYRO); 
    
    // Disable self-test and set the full scale ranges back to their original values 
    mpu6050_accel_config_write(
        device_data, 
        MPU6050_ACCEL_ST_DISABLE,
        accel_fsr);
    mpu6050_gyro_config_write(
        device_data, 
        MPU6050_GYRO_ST_DISABLE,
        gyro_fsr);

    // Update the fault flags 
    device_data->status |= (self_test_result << SHIFT_2); 
    
    return self_test_result;
}


// MPU6050 self-test read 
void mpu6050_self_test_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *accel_st_data,
    uint8_t *gyro_st_data)
{
    // Store unformatted data 
    uint8_t st_data[BYTE_4];

    // Read the value of SELF_TEST registers 
    mpu6050_read(
        device_ptr, 
        MPU6050_SELF_TEST, 
        BYTE_4,
        st_data);
    
    // Parse the acceleration data in X, Y and Z respectively 
    *accel_st_data++ = ((st_data[BYTE_0] & MPU6050_ST_MASK_A_TEST_HI)  >> SHIFT_3) |
                       ((st_data[BYTE_3] & MPU6050_ST_MASK_XA_TEST_LO) >> SHIFT_4);
    
    *accel_st_data++ = ((st_data[BYTE_1] & MPU6050_ST_MASK_A_TEST_HI)  >> SHIFT_3) |
                       ((st_data[BYTE_3] & MPU6050_ST_MASK_YA_TEST_LO) >> SHIFT_2);
    
    *accel_st_data = ((st_data[BYTE_2] & MPU6050_ST_MASK_A_TEST_HI)  >> SHIFT_3) |
                     ((st_data[BYTE_3] & MPU6050_ST_MASK_ZA_TEST_LO) >> SHIFT_0);

    // Parse the gyro data in X, Y and Z respectively 
    *gyro_st_data++ = st_data[BYTE_0] & MPU6050_ST_MASK_X_TEST;
    
    *gyro_st_data++ = st_data[BYTE_1] & MPU6050_ST_MASK_X_TEST;
    
    *gyro_st_data = st_data[BYTE_2] & MPU6050_ST_MASK_X_TEST;
}


// MPU6050 accelerometer factory trim 
void mpu6050_accel_ft(
    uint8_t *a_test, 
    float *accel_ft)
{
    // Constants 
    float c1 = MPU6050_ACCEL_ST_FT_C1 / ((float)(DIVIDE_10000));
    float c2 = MPU6050_ACCEL_ST_FT_C2 / ((float)(DIVIDE_10000));
    float c3 = MPU6050_ACCEL_ST_FT_C3 / ((float)(DIVIDE_1000));
    float c4 = MPU6050_ACCEL_ST_FT_C4 / ((float)(DIVIDE_10));

    // Determine the factory trim 
    for (uint8_t i = 0; i < NUM_AXES; i++)
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
    float c1 = MPU6050_GYRO_ST_FT_C1 / ((float)(DIVIDE_10000));
    float c2 = MPU6050_GYRO_ST_FT_C2 / ((float)(DIVIDE_10000));
    float c3 = MPU6050_GYRO_ST_FT_C3 / ((float)(DIVIDE_100));
    float c4 = MPU6050_GYRO_ST_FT_C4 / ((float)(DIVIDE_10));

    // Determine the factory trim 
    for (uint8_t i = 0; i < NUM_AXES; i++)
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
    uint8_t shift)
{
    // Place to store the % change 
    float ft_change;

    // Determine the result of the self-test 
    for (uint8_t i = 0; i < NUM_AXES; i++)
    {
        // Check % change from factory trim 
        ft_change = ((float)(*self_test_results) - *factory_trim) / *factory_trim;
        
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
