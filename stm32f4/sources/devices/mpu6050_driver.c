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

// Device info 
#define MPU6050_WHOAMI_DEFAULT 0x68      // Default value of the WHO_AM_I register 
#define MPU6050_FT_MAX_ERROR 14          // Max % change from factory trim acceptable 

// Register control 
#define MPU6050_STBY_STATUS_MASK 0x3F    // Pwr mgmt 2 standby status mask 
#define MPU6050_FSR_MASK 0x18            // Mask for reading gyro and accel full scale range 
#define MPU6050_EXT_SYNC_DISABLE 0       // Disables the FSYNC feature 

// Register addresses 
#define MPU6050_SELF_TEST    0x0D        // Register 13  - Self-test 
#define MPU6050_SMPRT_DIV    0x19        // Register 25  - Sample Rate Divider 
#define MPU6050_CONFIG       0x1A        // Register 26  - Configuration 
#define MPU6050_GYRO_CONFIG  0x1B        // Register 27  - Gyroscope configuration 
#define MPU6050_ACCEL_CONFIG 0x1C        // Register 28  - Accelerometer configuration 
#define MPU6050_INT_CONFIG   0x37        // Register 55  - Interrupt configuration 
#define MPU6050_INT_ENABLE   0x38        // Register 56  - Interrupt enable 
#define MPU6050_ACCEL_XOUT_H 0x3B        // Register 59  - Accelerometer x-axis high byte 
#define MPU6050_PWR_MGMT_1   0x6B        // Register 107 - Power management 1 
#define MPU6050_PWR_MGMT_2   0x6C        // Register 108 - Power management 2 
#define MPU6050_WHO_AM_I     0x75        // Register 117 - Who Am I 

// Temperature sensor 
#define MPU6050_TEMP_SENSIT 340.0f       // Sensitivity (LSB/degC) - MPU-6050 defined scalar
#define MPU6050_TEMP_OFFSET 36.53f       // Temperature offset 

// Accelerometer 
#define MPU6050_AFS_SEL_MAX 16384        // Max accelerometer calculation scalar 
#define MPU6050_ACCEL_ST_FT_C1 0.0142f   // Accelerometer factory trim calc constant 1 
#define MPU6050_ACCEL_ST_FT_C2 0.6056f   // Accelerometer factory trim calc constant 2 
#define MPU6050_ACCEL_ST_FT_C3 45.752f   // Accelerometer factory trim calc constant 3 
#define MPU6050_ACCEL_ST_FT_C4 1345.2f   // Accelerometer factory trim calc constant 4 

// Gyroscope 
#define MPU6050_FS_SEL_MAX 1310          // Max gyroscopic calculation scalar 
#define MPU6050_FS_CORRECTION 0x02       // Gyroscope calculation correction mask 
#define MPU6050_GYRO_SCALAR 10.0f        // Unscales scaled mpu6050_gyro_scalars_t values 
#define MPU6050_GYRO_ST_FT_C1 0.1001f    // Gyroscope factory trim calc constant 1 
#define MPU6050_GYRO_ST_FT_C2 1.9244f    // Gyroscope factory trim calc constant 3 
#define MPU6050_GYRO_ST_FT_C3 150.56f    // Gyroscope factory trim calc constant 3 
#define MPU6050_GYRO_ST_FT_C4 3112.5f    // Gyroscope factory trim calc constant 4 

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
// Enums 

/**
 * @brief Read and write offset 
 * 
 * @details These provide the necessary offset to the devices I2C address to either read 
 *          or write to from it. Simply add this value to the end of the address when 
 *          reading or writing.  
 * 
 * @see mpu6050_i2c_addr_t
 */
typedef enum {
    MPU6050_W_OFFSET,
    MPU6050_R_OFFSET
} mpu6050_rw_offset_t;


/**
 * @brief PWR_MGMT_1 : CLKSEL
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for the
 *          selection of the devices clock source. The clock source is selected as 
 *          follows:  
 *              - CLKSEL = 0 : Internal 8MHz oscillator                                
 *              - CLKSEL = 1 : PPL with X-axis gyro reference                          
 *              - CLKSEL = 2 : PPL with Y-axis gyro reference                          
 *              - CLKSEL = 3 : PPL with Z-axis gyro reference                          
 *              - CLKSEL = 4 : PPL with external 32.768kHz reference                   
 *              - CLKSEL = 5 : PPL with external 19.2MHz reference                     
 *              - CLKSEL = 6 : Reserved                                                
 *              - CLKSEL = 7 : Stops the clock and keeps the timing generator on reset 
 *            
 *           It is recommended the internal oscillator is not used as the other options 
 *           are much more stable.  
 */
typedef enum {
    MPU6050_CLKSEL_0,  // Internal 8MHz oscillator
    MPU6050_CLKSEL_1,  // PPL with X-axis gyro reference 
    MPU6050_CLKSEL_2,  // PPL with Y-axis gyro reference 
    MPU6050_CLKSEL_3,  // PPL with Z-axis gyro reference 
    MPU6050_CLKSEL_4,  // PPL with external 32.768kHz reference
    MPU6050_CLKSEL_5,  // PPL with external 19.2MHz reference
    MPU6050_CLKSEL_6,  // Reserved 
    MPU6050_CLKSEL_7   // Stops the clock and keeps the timing generator on reset
} mpu6050_clksel_t;


/**
 * @brief PWR_MGMT_2 : LP_WAKE_CTRL
 * 
 * @details Used to configure power management register 2 in mpu6050_init. This allows 
 *          for configuring of the frequency of wake-ups in low power mode. In this mode 
 *          the device will power off all functions except for the primary i2c interface
 *          waking up only the accelerometer at fixed intervals to take a single 
 *          measurements. Values of LP_WAKE_CTRL correspond to the following wake-up
 *          frequencies: 
 *              - LP_WAKE_CTRL = 0 : 1.25 Hz  
 *              - LP_WAKE_CTRL = 1 : 5  Hz    
 *              - LP_WAKE_CTRL = 2 : 20 Hz    
 *              - LP_WAKE_CTRL = 3 : 40 Hz    
 *          
 *          Low power mode can be configured using the following steps carried out in 
 *          power management register 1:             
 *              - Set CYCLE to 1                     
 *              - Set SLEEP to 0                     
 *              - Set TEMP_DIS to 1                  
 *              - Set STBY_XG, STBY_YG, STBY_ZG to 1 
 */
typedef enum {
    MPU6050_LP_WAKE_CTRL_0,    // 1.25 Hz wakeup frequency
    MPU6050_LP_WAKE_CTRL_1,    // 5 Hz wakeup frequency
    MPU6050_LP_WAKE_CTRL_2,    // 20 Hz wakeup frequency
    MPU6050_LP_WAKE_CTRL_3     // 40 Hz wakeup frequency
} mpu6050_lp_wake_ctrl_t;


/**
 * @brief GYRO_CONFIG : XG_ST, YG_ST and ZG_ST setpoint 
 * 
 * @details This is used to enable and disable self-test on the gyroscope. During 
 *          initialization self-test is disabled, but when mpu6050_self_test is called 
 *          self-test is temporarily enabled. 
 * 
 * @see mpu6050_self_test
 */
typedef enum {
    MPU6050_GYRO_ST_DISABLE,
    MPU6050_GYRO_ST_ENABLE
} mpu6050_gyro_self_test_set_t;


/**
 * @brief ACCEL_CONFIG : XA_ST, YA_SET and ZA_ST setpoint
 * 
 * @details This is used to enable and disable self-test on the accelerometer. During 
 *          initialization self-test is disabled, but when mpu6050_self_test is called 
 *          self-test is temporarily enabled. 
 * 
 * @see mpu6050_self_test 
 */
typedef enum {
    MPU6050_ACCEL_ST_DISABLE,
    MPU6050_ACCEL_ST_ENABLE
} mpu6050_accel_self_test_set_t;


/**
 * @brief PWR_MGMT_1 : DEVICE_RESET
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for 
 *          a reset to the devices default settings. 
 */
typedef enum {
    MPU6050_RESET_DISABLE,
    MPU6050_RESET_ENABLE
} mpu6050_device_reset_t; 


/**
 * @brief PWR_MGMT_1 : CYCLE 
 * 
 * @details Used to configure power management register 1 in mpu6050_init. If cycle is 
 *          enabled and sleep mode is disabled, the device will wake up from sleep mode 
 *          periodically to take a single sample of data at a frquency dictated by 
 *          LP_WAKE_CTRL. 
 * 
 * @see mpu6050_sleep_mode_t
 * @see mpu6050_lp_wake_ctrl_t
 */
typedef enum {
    MPU6050_CYCLE_SLEEP_DISABLED,
    MPU6050_CYCLE_SLEEP_ENABLED
} mpu6050_cycle_t;


/**
 * @brief PWR_MGMT_1 : TEMP_DIS
 * 
 * @details Used to configure power management register 1 in mpu6050_init. Allows for 
 *          enabling or disabling of the temperature sensor. 
 */
typedef enum {
    MPU6050_TEMP_SENSOR_ENABLE,
    MPU6050_TEMP_SENSOR_DISABLE
} mpu6050_temp_sensor_t;


/**
 * @brief INT_PIN_CFG (register 55): LATCH_INT_EN 
 */
typedef enum {
    MPU6050_INT_LATCH_PULSE,   // INT pin emits 50us long pulses 
    MPU6050_INT_LATCH_HIGH     // INT pin held high until interrupt is cleared 
} mpu6050_int_latch_t; 


/**
 * @brief INT_PIN_CFG (register 55): INT_RD_CLEAR 
 */
typedef enum {
    MPU6050_INT_CLEAR_RD_STAT,   // INT status is only cleared by reading INT_STATUS 
    MPU6050_INT_CLEAR_RD_ANY     // INT status is cleared by any read 
} mpu6050_int_clear_t; 


/**
 * @brief INT_ENABLE (register 56): DATA_RDY_EN  
 */
typedef enum {
    MPU6050_INT_DATA_RDY_DISABLE,   // Disable the data ready interrupt 
    MPU6050_INT_DATA_RDY_ENABLE     // Enable the data ready interrupt 
} mpu6050_int_data_rdy_t; 

//=======================================================================================


//=======================================================================================
// Driver data 

// MPU-6050 data record structure 
typedef struct mpu6050_driver_data_s 
{
    // Linked list tracking 
    struct mpu6050_driver_data_s *next_ptr; 
    device_number_t device_num; 

    // Peripherals 
    I2C_TypeDef *i2c;                  // I2C port connected to the device 
    GPIO_TypeDef *gpio;                // GPIO port for the INT pin 

    // Device information 
    mpu6050_i2c_addr_t addr;           // Device I2C address 
    pin_selector_t int_pin;            // INT pin number 
    float accel_data_scalar;           // Scales accelerometer raw data into readable values 
    float gyro_data_scalar;            // Scales gyroscope raw data into readable values 
    int16_t accel_offsets[NUM_AXES];   // Accelerometer axis offsets 
    int16_t gyro_offsets[NUM_AXES];    // Gyroscope axis offsets 

    // Data 
    int16_t accel[NUM_AXES];           // Accelerometer data 
    int16_t gyro[NUM_AXES];            // Gyroscope data 
    int16_t temp;                      // Temperature 
}
mpu6050_driver_data_t; 


// Driver data record first pointer 
static mpu6050_driver_data_t *mpu6050_driver_data = NULL; 

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief read from register 
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
 * @return MPU6050_STATUS : status of the read operation 
 */
MPU6050_STATUS mpu6050_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t mpu6050_register, 
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);


/**
 * @brief write to register 
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t mpu6050_register,
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value);


/**
 * @brief Sample Rate Divider (SMPRT_DIV) register write 
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_smprt_div_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t smprt_div);


/**
 * @brief Configuration (CONFIG) register write
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_config_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t ext_sync_set,
    mpu6050_dlpf_cfg_t dlpf_cfg);


/**
 * @brief Gyroscope Configuration (GYRO_CONFIG) register write
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_gyro_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_gyro_self_test_set_t gyro_self_test,
    mpu6050_fs_sel_set_t fs_sel);


/**
 * @brief Gyroscope Configuration (GYRO_CONFIG) register read
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
 * @param gyro_config : buffer to store the GYRO_CONFIG register data 
 * @return uint8_t : status if the read operation 
 */
MPU6050_STATUS mpu6050_gyro_config_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *gyro_config);


/**
 * @brief Accelerometer Configuration (ACCEL_CONFIG) register write
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_accel_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_accel_self_test_set_t accel_self_test,
    mpu6050_afs_sel_set_t afs_sel);


/**
 * @brief Accelerometer Configuration (ACCEL_CONFIG) register read 
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
 * @param accel_config : buffer to store the ACCEL_CONFIG register data 
 * @return uint8_t : status of the read operation 
 */
MPU6050_STATUS mpu6050_accel_config_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *accel_config);


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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_int_pin_config_write(
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_int_enable_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_int_data_rdy_t data_rdy_en); 


/**
 * @brief Power Manangement 1 (PWR_MGMT_1) register write
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_pwr_mgmt_1_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_device_reset_t device_reset,
    mpu6050_sleep_mode_t sleep,
    mpu6050_cycle_t cycle,
    mpu6050_temp_sensor_t temp_dis,
    mpu6050_clksel_t clksel);


/**
 * @brief Power Management 2 (PWR_MGMT_2) register write
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
 * @return MPU6050_STATUS : status of the write operation 
 */
MPU6050_STATUS mpu6050_pwr_mgmt_2_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    uint8_t standby_status);


/**
 * @brief Who Am I (WHO_AM_I) register read 
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
 * @param who_am_i : buffer to store value in the WHO_AM_I register 
 * @return MPU6050_STATUS : status of the read 
 */
MPU6050_STATUS mpu6050_who_am_i_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *who_am_i);


/**
 * @brief self-test read 
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
 * @return MPU6050_STATUS : status of the read operation 
 */
MPU6050_STATUS mpu6050_self_test_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *accel_self_test_data,
    uint8_t *gyro_self_test_data);


/**
 * @brief self-test response calculation
 * 
 * @details This function calculates the self-test response of each sensor axis. 
 *          
 *          Self-test response = (sensor output with self-test enabled) - 
 *                               (sensor output with self-test disabled) 
 * 
 * @param self_test_response : pointer that stores the sef-test response calculation
 * @param no_self_test : pointer to raw senor output (without self-test enabled)
 * @param self_test : pointer to raw self-test sensor output 
 */
void mpu6050_str_calc(
    int16_t *self_test_response,
    int16_t *no_self_test,
    int16_t *self_test);


/**
 * @brief self-test result calculation 
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
 * @brief accelerometer factory trim 
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
 * @brief gyroscope factory trim 
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
 * @brief accelerometer scalar 
 * 
 * @details This function reads the ACCEL_CONFIG register to check the chosen full scale
 *          range of the accelerometer. This is used to determine the scalar that  
 *          converts raw sensor output into g's. This is called and the result is stored 
 *          during initialization. 
 * 
 * @see mpu6050_accel_config_read
 * 
 * @param device_ptr : pointer to device data record 
 * @param accel_scalar : buffer to store the accelerometer scalar 
 * @return float : status of the read 
 */
MPU6050_STATUS mpu6050_accel_scalar(
    mpu6050_driver_data_t *device_ptr, 
    float *accel_scalar);


/**
 * @brief gyroscope scalar
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
 * @param gyro_scalar : buffer to store the gyroscope scalar 
 * @return float : status of the read 
 */
MPU6050_STATUS mpu6050_gyro_scalar(
    mpu6050_driver_data_t *device_ptr, 
    float *gyro_scalar);

//=======================================================================================


//=======================================================================================
// Initialization 

// MPU-6050 Initialization 
MPU6050_STATUS mpu6050_init(
    device_number_t device_num, 
    I2C_TypeDef *i2c, 
    mpu6050_i2c_addr_t mpu6050_addr,
    uint8_t standby_status, 
    mpu6050_dlpf_cfg_t dlpf_cfg,
    uint8_t smplrt_div,
    mpu6050_afs_sel_set_t afs_sel,
    mpu6050_fs_sel_set_t fs_sel)
{
    // Create a data record if it does not already exist 
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)create_linked_list_entry(device_num, 
                                                          (void *)&mpu6050_driver_data, 
                                                          sizeof(mpu6050_driver_data_t)); 

    if ((device_data == NULL) || (i2c == NULL))
    {
        return MPU6050_INVALID_PTR; 
    }

    device_data->i2c = i2c; 
    device_data->addr = mpu6050_addr; 
    memset((void *)device_data->accel_offsets, CLEAR, sizeof(device_data->accel_offsets)); 
    memset((void *)device_data->gyro_offsets, CLEAR, sizeof(device_data->gyro_offsets)); 
    memset((void *)device_data->accel, CLEAR, sizeof(device_data->accel)); 
    memset((void *)device_data->gyro, CLEAR, sizeof(device_data->gyro)); 

    device_data->temp = CLEAR; 

    MPU6050_STATUS status = MPU6050_OK; 

    // Read the WHO_AM_I register to establish that there is communication with the 
    // correct device. 
    uint8_t who_am_i = CLEAR; 
    status |= mpu6050_who_am_i_read(device_data, &who_am_i); 
    if (who_am_i != MPU6050_WHOAMI_DEFAULT)
    {
        return MPU6050_WHOAMI; 
    }

    // Choose which sensors to use and frequency of CYCLE mode
    status |= mpu6050_pwr_mgmt_2_write(device_data, MPU6050_LP_WAKE_CTRL_0, standby_status);

    // Wake the sensor up through the power management 1 register 
    status |= mpu6050_pwr_mgmt_1_write(
        device_data, 
        MPU6050_RESET_DISABLE,
        MPU6050_SLEEP_MODE_DISABLE,
        MPU6050_CYCLE_SLEEP_DISABLED,
        MPU6050_TEMP_SENSOR_ENABLE,
        MPU6050_CLKSEL_5);

    // Set the output rate of the gyro and accelerometer 
    status |= mpu6050_config_write(device_data, MPU6050_EXT_SYNC_DISABLE, dlpf_cfg);
    
    // Set the Sample Rate (data rate)
    status |= mpu6050_smprt_div_write(device_data, smplrt_div);
    
    // Configure the accelerometer register 
    status |= mpu6050_accel_config_write(device_data, MPU6050_ACCEL_ST_DISABLE, afs_sel);
    
    // Configure the gyroscope register
    status |= mpu6050_gyro_config_write(device_data, MPU6050_GYRO_ST_DISABLE, fs_sel);

    // Store the raw data scalars for calculating the actual value 
    status |= mpu6050_accel_scalar(device_data, &device_data->accel_data_scalar); 
    status |= mpu6050_gyro_scalar(device_data, &device_data->gyro_data_scalar); 

    return status; 
}


// INT pin initialization 
MPU6050_STATUS mpu6050_int_pin_init(
    device_number_t device_num, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pin)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if ((device_data == NULL) || (gpio == NULL)) 
    {
        return MPU6050_INVALID_PTR; 
    }

    device_data->gpio = gpio; 
    device_data->int_pin = pin; 

    MPU6050_STATUS status = MPU6050_OK; 

    // Configure the GPIO input pin 
    gpio_pin_init(device_data->gpio, 
                  device_data->int_pin, 
                  MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD); 

    // Interrupt configuration and interrupt enable register update 
    status |= mpu6050_int_pin_config_write(device_data, MPU6050_INT_LATCH_HIGH, MPU6050_INT_CLEAR_RD_ANY); 
    status |= mpu6050_int_enable_write(device_data, MPU6050_INT_DATA_RDY_ENABLE); 

    return status; 
}


// Low power mode config 
MPU6050_STATUS mpu6050_low_pwr_config(
    device_number_t device_num, 
    mpu6050_sleep_mode_t sleep)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return MPU6050_INVALID_PTR; 
    }

    MPU6050_STATUS status = mpu6050_pwr_mgmt_1_write(
        device_data, 
        MPU6050_RESET_DISABLE,
        sleep,
        MPU6050_CYCLE_SLEEP_DISABLED,
        MPU6050_TEMP_SENSOR_ENABLE,
        MPU6050_CLKSEL_5);

    return status; 
}


// Correct offsets in the accelerometer and gyroscope readings 
MPU6050_STATUS mpu6050_set_offsets(
    device_number_t device_num, 
    const int16_t *accel_offset, 
    const int16_t *gyro_offset)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if ((device_data == NULL) || (accel_offset == NULL) || (gyro_offset == NULL)) 
    {
        return MPU6050_INVALID_PTR; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (accel_offset != NULL); i++)
    {
        device_data->accel_offsets[i] = *accel_offset++; 
    }

    for (uint8_t j = X_AXIS; (j < NUM_AXES) && (gyro_offset != NULL); j++)
    {
        device_data->gyro_offsets[j] = *gyro_offset++; 
    }

    return MPU6050_OK; 
}


// Accelerometer scalar
MPU6050_STATUS mpu6050_accel_scalar(
    mpu6050_driver_data_t *device_ptr, 
    float *accel_scalar)
{
    uint8_t accel_config = CLEAR; 
    MPU6050_STATUS status = mpu6050_accel_config_read(device_ptr, &accel_config);   // Get AFS_SEL 
    uint8_t afs_sel = (accel_config & MPU6050_FSR_MASK) >> SHIFT_3; 
    *accel_scalar = (float)(MPU6050_AFS_SEL_MAX >> afs_sel); 
    return status; 
}


// Gyroscope scalar 
MPU6050_STATUS mpu6050_gyro_scalar(
    mpu6050_driver_data_t *device_ptr, 
    float *gyro_scalar)
{
    uint8_t gyro_config = CLEAR; 
    MPU6050_STATUS status = mpu6050_gyro_config_read(device_ptr, &gyro_config);   // Get FS_SEL 
    uint8_t fs_sel = (gyro_config & MPU6050_FSR_MASK) >> SHIFT_3; 
    *gyro_scalar = (float)((MPU6050_FS_SEL_MAX >> fs_sel) + ((fs_sel & MPU6050_FS_CORRECTION) >> SHIFT_1)) / MPU6050_GYRO_SCALAR; 
    return status; 
}

//=======================================================================================


//=======================================================================================
// Update and get data 

// Read the most recent IMU data 
MPU6050_STATUS mpu6050_update(device_number_t device_num)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return MPU6050_INVALID_PTR; 
    }

    uint8_t data_reg[BYTE_14]; 
    MPU6050_STATUS status = mpu6050_read(device_data, MPU6050_ACCEL_XOUT_H, BYTE_14, data_reg); 

    if (status == MPU6050_OK)
    {
        // Accelerometer 
        device_data->accel[X_AXIS] = (((int16_t)data_reg[0] << SHIFT_8) | (int16_t)data_reg[1]) - device_data->accel_offsets[X_AXIS];
        device_data->accel[Y_AXIS] = (((int16_t)data_reg[2] << SHIFT_8) | (int16_t)data_reg[3]) - device_data->accel_offsets[Y_AXIS];
        device_data->accel[Z_AXIS] = (((int16_t)data_reg[4] << SHIFT_8) | (int16_t)data_reg[5]) - device_data->accel_offsets[Z_AXIS];
    
        // Temperature 
        device_data->temp = ((int16_t)data_reg[6] << SHIFT_8) | (int16_t)data_reg[7]; 
    
        // Gyroscope 
        device_data->gyro[X_AXIS] = (((int16_t)data_reg[8]  << SHIFT_8) | (int16_t)data_reg[9]) - device_data->gyro_offsets[X_AXIS];
        device_data->gyro[Y_AXIS] = (((int16_t)data_reg[10] << SHIFT_8) | (int16_t)data_reg[11]) - device_data->gyro_offsets[Y_AXIS];
        device_data->gyro[Z_AXIS] = (((int16_t)data_reg[12] << SHIFT_8) | (int16_t)data_reg[13]) - device_data->gyro_offsets[Z_AXIS];
    }

    return status; 
}


// Get accelerometer axis data 
void mpu6050_get_accel_axis(
    device_number_t device_num, 
    int16_t *accel_axis)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (accel_axis != NULL); i++)
    {
        *accel_axis++ = device_data->accel[i]; 
    }
}


// Get accelerometer axis g's 
void mpu6050_get_accel_axis_gs(
    device_number_t device_num, 
    float *accel_axis_gs)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (accel_axis_gs != NULL); i++)
    {
        *accel_axis_gs++ = (float)device_data->accel[i] / device_data->accel_data_scalar; 
    }
}


// Get the gyroscope axis data 
void mpu6050_get_gyro_axis(
    device_number_t device_num, 
    int16_t *gyro_axis)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (gyro_axis != NULL); i++)
    {
        *gyro_axis++ = device_data->gyro[i]; 
    }
}


// Get gyroscope axis angular velocity 
void mpu6050_get_gyro_axis_rate(
    device_number_t device_num, 
    float *gyro_axis_rate)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return; 
    }

    for (uint8_t i = X_AXIS; (i < NUM_AXES) && (gyro_axis_rate != NULL); i++)
    {
        *gyro_axis_rate++ = (float)device_data->gyro[i] / device_data->gyro_data_scalar; 
    }
}


// Temperature sensor raw value 
int16_t mpu6050_get_temp_raw(device_number_t device_num)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return 0; 
    }

    return device_data->temp; 
}


// Temperature sensor calculation 
float mpu6050_get_temp(device_number_t device_num)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return 0; 
    }

    // Get the true temperature in degC 
    return ((float)device_data->temp / MPU6050_TEMP_SENSIT) + MPU6050_TEMP_OFFSET; 
}

//=======================================================================================


//=======================================================================================
// Status 

// INT pin status 
uint8_t mpu6050_int_status(device_number_t device_num)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if (device_data == NULL) 
    {
        return GPIO_LOW; 
    }

    return gpio_read(device_data->gpio, (SET_BIT << device_data->int_pin)); 
}

//=======================================================================================


//=======================================================================================
// Register configuration functions 

// Sample Rate Divider (SMPRT_DIV) register write 
MPU6050_STATUS mpu6050_smprt_div_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t smprt_div)
{
    return mpu6050_write(device_ptr, MPU6050_SMPRT_DIV, BYTE_1, &smprt_div);
}


// Configuration (CONFIG) register write
MPU6050_STATUS mpu6050_config_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t ext_sync_set,
    mpu6050_dlpf_cfg_t dlpf_cfg)
{
    uint8_t mpu6050_config = (ext_sync_set << SHIFT_3) | (dlpf_cfg << SHIFT_0);
    return mpu6050_write(device_ptr, MPU6050_CONFIG, BYTE_1, &mpu6050_config);
}


// Gyroscope Configuration (GYRO_CONFIG) register write
MPU6050_STATUS mpu6050_gyro_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_gyro_self_test_set_t gyro_self_test,
    mpu6050_fs_sel_set_t fs_sel)
{
    uint8_t mpu6050_gyro_config = (gyro_self_test << SHIFT_5) | (fs_sel << SHIFT_3);
    return mpu6050_write(device_ptr, MPU6050_GYRO_CONFIG, BYTE_1, &mpu6050_gyro_config);
}


// Gyroscope Configuration (GYRO_CONFIG) register read
MPU6050_STATUS mpu6050_gyro_config_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *gyro_config)
{
    return mpu6050_read(device_ptr, MPU6050_GYRO_CONFIG, BYTE_1, gyro_config);
}


// Accelerometer Configuration (ACCEL_CONFIG) register write
MPU6050_STATUS mpu6050_accel_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_accel_self_test_set_t accel_self_test,
    mpu6050_afs_sel_set_t afs_sel)
{
    uint8_t mpu6050_accel_config = (accel_self_test << SHIFT_5) | (afs_sel << SHIFT_3);
    return mpu6050_write(device_ptr, MPU6050_ACCEL_CONFIG, BYTE_1, &mpu6050_accel_config);
}


// Accelerometer Configuration (ACCEL_CONFIG) register read 
MPU6050_STATUS mpu6050_accel_config_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *accel_config)
{
    return mpu6050_read(device_ptr, MPU6050_ACCEL_CONFIG, BYTE_1, accel_config); 
}


// Interrupt Pin Configuration (INT_PIN_CFG) register write 
MPU6050_STATUS mpu6050_int_pin_config_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_int_latch_t latch_int_en, 
    mpu6050_int_clear_t int_rd_clear)
{
    uint8_t mpu6050_int_config = (latch_int_en << SHIFT_5) | (int_rd_clear << SHIFT_4); 
    return mpu6050_write(device_ptr, MPU6050_INT_CONFIG, BYTE_1, &mpu6050_int_config);
}


// Interrupt Enable (INT_ENABLE) register write 
MPU6050_STATUS mpu6050_int_enable_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_int_data_rdy_t data_rdy_en)
{
    uint8_t mpu6050_int_enable = (data_rdy_en << SHIFT_0); 
    return mpu6050_write(device_ptr, MPU6050_INT_ENABLE, BYTE_1, &mpu6050_int_enable);
}


// Power Manangement 1 (PWR_MGMT_1) register write
MPU6050_STATUS mpu6050_pwr_mgmt_1_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_device_reset_t device_reset,
    mpu6050_sleep_mode_t sleep,
    mpu6050_cycle_t cycle,
    mpu6050_temp_sensor_t temp_dis,
    mpu6050_clksel_t clksel)
{
    uint8_t mpu6050_pwr_mgmt_1 = (device_reset << SHIFT_7) |
                                 (sleep        << SHIFT_6) |
                                 (cycle        << SHIFT_5) |
                                 (temp_dis     << SHIFT_3) |
                                 (clksel       << SHIFT_0);
    
    return mpu6050_write(device_ptr, MPU6050_PWR_MGMT_1, BYTE_1, &mpu6050_pwr_mgmt_1);
}


// Power Manangement 2 (PWR_MGMT_2) register write
MPU6050_STATUS mpu6050_pwr_mgmt_2_write(
    mpu6050_driver_data_t *device_ptr, 
    mpu6050_lp_wake_ctrl_t lp_wake_ctrl,
    uint8_t standby_status)
{
    uint8_t mpu6050_pwr_mgmt_2 = (lp_wake_ctrl << SHIFT_6) | 
                                 (standby_status & MPU6050_STBY_STATUS_MASK); 
    
    return mpu6050_write(device_ptr, MPU6050_PWR_MGMT_2, BYTE_1, &mpu6050_pwr_mgmt_2);
}


// Who Am I (WHO_AM_I) register read 
MPU6050_STATUS mpu6050_who_am_i_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *who_am_i)
{
    return mpu6050_read(device_ptr, MPU6050_WHO_AM_I, BYTE_1, who_am_i); 
}

//=======================================================================================


//=======================================================================================
// Self-test 

// Self-test 
MPU6050_STATUS mpu6050_self_test(
    device_number_t device_num, 
    uint8_t *st_result)
{
    mpu6050_driver_data_t *device_data = 
        (mpu6050_driver_data_t *)get_linked_list_entry(device_num, mpu6050_driver_data); 
    
    if ((device_data == NULL) || (st_result == NULL)) 
    {
        return MPU6050_INVALID_PTR; 
    }

    MPU6050_STATUS status = MPU6050_OK; 
    uint8_t accel_config = CLEAR, gyro_config = CLEAR; 
    uint8_t accel_fsr, gyro_fsr;                           // Used to record the existing full scale range 
    int16_t accel_no_st[NUM_AXES], gyro_no_st[NUM_AXES];   // Sensor readings with no self-test 
    int16_t accel_st[NUM_AXES], gyro_st[NUM_AXES];         // Sensor readings with self-test 
    int16_t accel_str[NUM_AXES], gyro_str[NUM_AXES];       // Self-test result 
    uint8_t accel_test[NUM_AXES], gyro_test[NUM_AXES];     // Self-test register readings 
    float accel_ft[NUM_AXES], gyro_ft[NUM_AXES];           // Factory trim calculation 

    // Record the full scale range set in the init function 
    status |= mpu6050_accel_config_read(device_data, &accel_config); 
    status |= mpu6050_gyro_config_read(device_data, &gyro_config); 
    accel_fsr = (accel_config & MPU6050_FSR_MASK) >> SHIFT_3;
    gyro_fsr = (gyro_config & MPU6050_FSR_MASK) >> SHIFT_3;

    // Set the full scale range of the accel to +/- 8g and gyro to +/- 250 deg/s
    status |= mpu6050_accel_config_write(device_data, MPU6050_ACCEL_ST_DISABLE, MPU6050_AFS_SEL_8);
    status |= mpu6050_gyro_config_write(device_data, MPU6050_GYRO_ST_DISABLE, MPU6050_FS_SEL_250);

    // Read and store the sensor values during non-self-test 
    status |= mpu6050_update(device_num); 
    accel_no_st[X_AXIS] = device_data->accel[X_AXIS]; 
    accel_no_st[Y_AXIS] = device_data->accel[Y_AXIS]; 
    accel_no_st[Z_AXIS] = device_data->accel[Z_AXIS]; 
    gyro_no_st[X_AXIS] = device_data->gyro[X_AXIS]; 
    gyro_no_st[Y_AXIS] = device_data->gyro[Y_AXIS]; 
    gyro_no_st[Z_AXIS] = device_data->gyro[Z_AXIS]; 

    // Enable self-test 
    status |= mpu6050_accel_config_write(device_data, MPU6050_ACCEL_ST_ENABLE, MPU6050_AFS_SEL_8);
    status |= mpu6050_gyro_config_write(device_data, MPU6050_GYRO_ST_ENABLE, MPU6050_FS_SEL_250);
    
    // Read and store the sensor values during self-test 
    status |= mpu6050_update(device_num); 
    accel_st[X_AXIS] = device_data->accel[X_AXIS]; 
    accel_st[Y_AXIS] = device_data->accel[Y_AXIS]; 
    accel_st[Z_AXIS] = device_data->accel[Z_AXIS]; 
    gyro_st[X_AXIS] = device_data->gyro[X_AXIS]; 
    gyro_st[Y_AXIS] = device_data->gyro[Y_AXIS]; 
    gyro_st[Z_AXIS] = device_data->gyro[Z_AXIS]; 
    
    // Read the self-test registers
    status |= mpu6050_self_test_read(device_data, accel_test, gyro_test);
    
    // Calculate the factory trim
    mpu6050_accel_ft(accel_test, accel_ft);
    mpu6050_gyro_ft(gyro_test, gyro_ft);
    
    // Calculate self-test response
    mpu6050_str_calc(accel_str, accel_no_st, accel_st);
    mpu6050_str_calc(gyro_str, gyro_no_st, gyro_st);
    
    // Calculate the change from factory trim and check against the acceptable range 
    mpu6050_self_test_result(accel_str, accel_ft, st_result, MPU6050_STR_SHIFT_ACCEL); 
    mpu6050_self_test_result(gyro_str, gyro_ft, st_result, MPU6050_STR_SHIFT_GYRO); 
    
    // Disable self-test and set the full scale ranges back to their original values 
    status |= mpu6050_accel_config_write(device_data, MPU6050_ACCEL_ST_DISABLE, accel_fsr);
    status |= mpu6050_gyro_config_write(device_data, MPU6050_GYRO_ST_DISABLE, gyro_fsr);

    if (*st_result)
    {
        return MPU6050_ST_FAULT; 
    }
    
    return status;
}


// Self-test read 
MPU6050_STATUS mpu6050_self_test_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t *accel_st_data,
    uint8_t *gyro_st_data)
{
    uint8_t st_data[BYTE_4]; 
    MPU6050_STATUS status = mpu6050_read(device_ptr, MPU6050_SELF_TEST, BYTE_4, st_data); 

    if (status == MPU6050_OK)
    {
        // Parse the acceleration data in X, Y and Z respectively 
        *accel_st_data++ = ((st_data[BYTE_0] & MPU6050_ST_MASK_A_TEST_HI)  >> SHIFT_3) |
                           ((st_data[BYTE_3] & MPU6050_ST_MASK_XA_TEST_LO) >> SHIFT_4);
        
        *accel_st_data++ = ((st_data[BYTE_1] & MPU6050_ST_MASK_A_TEST_HI)  >> SHIFT_3) |
                           ((st_data[BYTE_3] & MPU6050_ST_MASK_YA_TEST_LO) >> SHIFT_2);
        
        *accel_st_data = ((st_data[BYTE_2] & MPU6050_ST_MASK_A_TEST_HI)  >> SHIFT_3) |
                         ((st_data[BYTE_3] & MPU6050_ST_MASK_ZA_TEST_LO) >> SHIFT_0);
    
        // Parse the gyro data in X, Y and Z respectively 
        for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
        {
            *gyro_st_data++ = st_data[i] & MPU6050_ST_MASK_X_TEST; 
        }
    }

    return status; 
}


// Accelerometer factory trim 
void mpu6050_accel_ft(
    uint8_t *a_test, 
    float *accel_ft)
{
    float c1 = MPU6050_ACCEL_ST_FT_C1; 
    float c2 = MPU6050_ACCEL_ST_FT_C2; 
    float c3 = MPU6050_ACCEL_ST_FT_C3; 
    float c4 = MPU6050_ACCEL_ST_FT_C4; 

    // Determine the factory trim 
    for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
    {
        *accel_ft++ = (*a_test == 0) ? 0 : (*a_test)*((*a_test)*(c1*(*a_test) + c2) + c3) + c4; 
        a_test++;
    }
}


// Gyroscope factory trim 
void mpu6050_gyro_ft(
    uint8_t *g_test, 
    float *gyro_ft)
{
    float c1 = MPU6050_GYRO_ST_FT_C1; 
    float c2 = MPU6050_GYRO_ST_FT_C2; 
    float c3 = MPU6050_GYRO_ST_FT_C3; 
    float c4 = MPU6050_GYRO_ST_FT_C4; 

    // Determine the factory trim 
    for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
    {
        *gyro_ft = (*g_test == 0) ? 0 : (*g_test)*((*g_test)*(c1*(*g_test) + c2) + c3) + c4; 

        if (i == 1)
        {
            *gyro_ft = -(*gyro_ft);
        }

        gyro_ft++;
        g_test++;
    }
}


// Self-test response calculation
void mpu6050_str_calc(
    int16_t *self_test_response,
    int16_t *no_self_test,
    int16_t *self_test)
{
    for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
    {
        *self_test_response++ = *self_test++ - *no_self_test++;
    }
}


// Self-test result calculation 
void mpu6050_self_test_result(
    int16_t *self_test_results,
    float *factory_trim,
    uint8_t *results, 
    uint8_t shift)
{
    float ft_change;

    // Determine the result of the self-test 
    for (uint8_t i = X_AXIS; i < NUM_AXES; i++)
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


//=======================================================================================
// Read and write 

// MPU-6050 read from register 
MPU6050_STATUS mpu6050_read(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t mpu6050_register, 
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Generate a start condition, send the MPU-6050 address with a write offset, then send 
    // the register address that is going to be read. 
    i2c_status |= i2c_start(device_ptr->i2c); 
    i2c_status |= i2c_write_addr(device_ptr->i2c, device_ptr->addr + MPU6050_W_OFFSET);
    i2c_clear_addr(device_ptr->i2c);
    i2c_status |= i2c_write(device_ptr->i2c, &mpu6050_register, BYTE_1);

    // Generate another start condition, send the MPU-6050 address with a read offset and 
    // read the data sent by the MPU-6050 before stopping the transaction. 
    i2c_status |= i2c_start(device_ptr->i2c); 
    i2c_status |= i2c_write_addr(device_ptr->i2c, device_ptr->addr + MPU6050_R_OFFSET);
    i2c_status |= i2c_read(device_ptr->i2c, mpu6050_reg_value, mpu6050_reg_size);
    i2c_stop(device_ptr->i2c); 

    if (i2c_status)
    {
        return MPU6050_READ_FAULT; 
    }

    return MPU6050_OK; 
}


// MPU-6050 write to register 
MPU6050_STATUS mpu6050_write(
    mpu6050_driver_data_t *device_ptr, 
    uint8_t mpu6050_register,
    byte_num_t mpu6050_reg_size,
    uint8_t *mpu6050_reg_value)
{
    I2C_STATUS i2c_status = I2C_OK; 
    
    // Generate a start condition, send the MPU-6050 address with a write offset, then 
    // send the register address that is going to be written to. 
    i2c_status |= i2c_start(device_ptr->i2c); 
    i2c_status |= i2c_write_addr(device_ptr->i2c, device_ptr->addr + MPU6050_W_OFFSET);
    i2c_clear_addr(device_ptr->i2c);
    i2c_status |= i2c_write(device_ptr->i2c, &mpu6050_register, BYTE_1);

    // Write the data to the MPU-6050 then stop the transaction 
    i2c_status |= i2c_write(device_ptr->i2c, mpu6050_reg_value, mpu6050_reg_size);
    i2c_stop(device_ptr->i2c); 

    if (i2c_status)
    {
        return MPU6050_WRITE_FAULT; 
    }

    return MPU6050_OK; 
}

//=======================================================================================
