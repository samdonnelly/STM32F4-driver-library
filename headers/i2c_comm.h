/**
 * @file i2c_comm.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief I2C initialization, read and write 
 * 
 * @version 0.1
 * @date 2022-03-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _I2C_COMM_H_
#define _I2C_COMM_H_

//=======================================================================================
// Includes 

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief I2C1 available SCL pins 
 * 
 * @details These pins are the available I2C1 SCL pins on the chip. The values associated 
 *          with these variables are used in i2c1_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c1_init as 
 *          an argument. 
 * 
 * @see i2c1_init
 * 
 */
typedef enum {
    I2C1_SCL_PB6 = 0,
    I2C1_SCL_PB8 = 2
} i2c1_scl_pin_t;


/**
 * @brief I2C1 available SDA pins 
 * 
 * @details These pins are the available I2C1 SDA pins on the chip. The values associated 
 *          with these variables are used in i2c1_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c1_init as 
 *          an argument.
 * 
 * @see i2c1_init
 * 
 */
typedef enum {
    I2C1_SDA_PB7 = 0,
    I2C1_SDA_PB9 = 2
} i2c1_sda_pin_t;


/**
 * @brief I2C2 available SCL pins 
 * 
 * @details These pins are the available I2C2 SCL pins on the chip. The values associated 
 *          with these variables are used in i2c2_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c2_init as 
 *          an argument. 
 * 
 * @see i2c2_init
 * 
 */
typedef enum {
    I2C2_SCL_PB10
} i2c2_scl_pin_t;


/**
 * @brief I2C2 available SDA pins 
 * 
 * @details These pins are the available I2C3 SDA pins on the chip. The values associated 
 *          with these variables are used in i2c2_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c2_init as 
 *          an argument.
 * 
 * @see i2c2_init
 * 
 */
typedef enum {
    I2C2_SDA_PB3,
    I2C2_SDA_PB9
} i2c2_sda_pin_t;


/**
 * @brief I2C3 available SCL pins 
 * 
 * @details These pins are the available I2C3 SCL pins on the chip. The values associated 
 *          with these variables are used in i2c3_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c3_init as 
 *          an argument. 
 * 
 * @see i2c3_init
 * 
 */
typedef enum {
    I2C3_SCL_PA8
} i2c3_scl_pin_t;


/**
 * @brief I2C3 available SDA pins 
 * 
 * @details These pins are the available I2C3 SDA pins on the chip. The values associated 
 *          with these variables are used in i2c3_init to determine how to configure 
 *          the pins alternate function. These variables get passed to i2c3_init as 
 *          an argument.
 * 
 * @see i2c3_init
 * 
 */
typedef enum {
    I2C3_SDA_PB4,
    I2C3_SDA_PB8,
    I2C3_SDA_PC9
} i2c3_sda_pin_t;


/**
 * @brief I2C run mode selection 
 * 
 * @details I2C can run in standard (SM) or fast (FM) mode. The mode dictates the range 
 *          of SCL clock frequencies that can be run. The i2c init functions uses this 
 *          enum as an input when the function is called so it can set the desired
 *          run mode. 
 * 
 */
typedef enum {
    I2C_SM_MODE,
    I2C_FM_MODE
} i2c_run_mode_t;


/**
 * @brief I2C Fm mode duty cycle 
 * 
 * @details When I2C is initialized in FM mode for faster clock frequencies the duty 
 *          cycle can be chosen. The i2c init functions take this as an argument. If the 
 *          i2c is going to be initialized in Sm mode then this input in the init function 
 *          has no effect. 
 * 
 */
typedef enum {
    I2C_FM_DUTY_2,   // t_low/t_high = 2
    I2C_FM_DUTY_169  // t_low/t_high = 16/9
} i2c_fm_duty_cycle_t;


/**
 * @brief I2C AP1 frequency 
 * 
 * @details The i2c init functions take this as an argument in order to program the 
 *          peripheral input clock based in the frequency of APB1. 
 * 
 */
typedef enum {
    I2C_APB1_42MHZ = 42,
    I2C_APB1_84MHZ = 84
} i2c_apb1_freq_t;


/**
 * @brief I2C Fm CCR setpoint
 * 
 * @details The i2c init functions take this an argument to program the clock control 
 *          register when initializing in Fm mode. <br><br>
 *          
 *          enum code: I2C_CCR_FM_(X_1)_(X_2)_(X_3)           <br>
 *              X_1: Duty cycle - ex. 169 -> 16/9 in Fm mode  <br>
 *              X_2: PCLK1 frquency (MHz)                     <br>
 *              X_3: SCL frquency (kHz)                       <br><br>
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual for more information. 
 * 
 */
typedef enum {
    I2C_CCR_FM_169_42_400 = 5
} i2c_fm_ccr_setpoint_t;


/**
 * @brief I2C Sm CCR setpoint
 * 
 * @details The i2c init functions take this an argument to program the clock control 
 *          register when initializing in Sm mode. <br><br>
 *          
 *          enum code: I2C_CCR_SM_(X_1)_(X_2)  <br>
 *              X_1: PCLK1 frquency (MHz)      <br>
 *              X_2: SCL frquency (kHz)        <br><br>
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual for more information. 
 * 
 */
typedef enum {
    I2C_CCR_SM_42_100 = 210
} i2c_sm_ccr_setpoint_t;


/**
 * @brief I2C TRISE setpoint
 * 
 * @details The i2c init functions take this as an argument to program the rise timer 
 *          register based on the clock frequency and max rise time which changes based 
 *          on the run mode. <br><br> 
 *          
 *          enum code: I2C_TRISE_(X_1)_(X_2)  <br>
 *              X_1: Max rise time (ns)       <br>
 *              X_2: PCLK1 frequency (MHz)    <br><br>
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual for more information. 
 * 
 */
typedef enum {
    I2C_TRISE_0300_42 = 13,
    I2C_TRISE_1000_42 = 43
} i2c_trise_setpoint_t;


/**
 * @brief I2C data size 
 * 
 * @details This is a general enum used for specifying i2c message sizes when sending or 
 *          receiving data. 
 * 
 */
typedef enum {
    I2C_0_BYTE,
    I2C_1_BYTE,
    I2C_2_BYTE,
    I2C_3_BYTE,
    I2C_4_BYTE
} i2c_data_size_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief I2C1 Initialization 
 * 
 * @details 
 *          Pin information for I2C1: <br>
 *              PB6: SCL              <br>
 *              PB7: SDA              <br> 
 *              PB8: SCL              <br>
 *              PB9: SDA              <br><br>
 * 
 * @param sda_pin 
 * @param scl_pin 
 * @param run_mode 
 * @param apb1_freq 
 * @param fm_duty_cycle 
 * @param ccr_reg 
 * @param trise_reg 
 * 
 * @see i2c1_scl_pin_t
 * @see i2c1_sda_pin_t
 * @see i2c_run_mode_t
 * @see i2c_apb1_freq_t
 * @see i2c_fm_duty_cycle_t
 * @see i2c_fm_ccr_setpoint_t
 * @see i2c_sm_ccr_setpoint_t
 * @see i2c_trise_setpoint_t
 * 
 */
void i2c1_init(
    uint8_t sda_pin,
    uint8_t scl_pin,
    uint8_t run_mode,
    uint8_t apb1_freq,
    uint8_t fm_duty_cycle,
    uint8_t ccr_reg,
    uint8_t trise_reg);


/**
 * @brief I2C2 Initialization 
 * 
 * @details 
 *          Pin information for I2C2:  <br>
 *              PB3:  SDA              <br>
 *              PB9:  SDA              <br>
 *              PB10: SCL              <br><br>
 * 
 * @param sda_pin 
 * @param scl_pin 
 * @param run_mode 
 * @param apb1_freq 
 * @param fm_duty_cycle 
 * @param ccr_reg 
 * @param trise_reg 
 * 
 * @see i2c2_scl_pin_t
 * @see i2c2_sda_pin_t
 * @see i2c_run_mode_t
 * @see i2c_apb1_freq_t
 * @see i2c_fm_duty_cycle_t
 * @see i2c_fm_ccr_setpoint_t
 * @see i2c_sm_ccr_setpoint_t
 * @see i2c_trise_setpoint_t
 * 
 */
void i2c2_init(
    uint8_t sda_pin,
    uint8_t scl_pin,
    uint8_t run_mode,
    uint8_t apb1_freq,
    uint8_t fm_duty_cycle,
    uint8_t ccr_reg,
    uint8_t trise_reg);


/**
 * @brief I2C3 Initialization 
 * 
 * @details 
 *          Pin information for I2C3: <br>
 *              PA8: SCL              <br>
 *              PB4: SDA              <br>
 *              PB8: SDA              <br>
 *              PC9: SDA              <br><br>
 * 
 * @param sda_pin 
 * @param scl_pin 
 * @param run_mode 
 * @param apb1_freq 
 * @param fm_duty_cycle 
 * @param ccr_reg 
 * @param trise_reg 
 * 
 * @see i2c3_scl_pin_t
 * @see i2c3_sda_pin_t
 * @see i2c_run_mode_t
 * @see i2c_apb1_freq_t
 * @see i2c_fm_duty_cycle_t
 * @see i2c_fm_ccr_setpoint_t
 * @see i2c_sm_ccr_setpoint_t
 * @see i2c_trise_setpoint_t
 * 
 */
void i2c3_init(
    uint8_t sda_pin,
    uint8_t scl_pin,
    uint8_t run_mode,
    uint8_t apb1_freq,
    uint8_t fm_duty_cycle,
    uint8_t ccr_reg,
    uint8_t trise_reg);


/**
 * @brief 
 * 
 */
void i2c1_start(void);


/**
 * @brief 
 * 
 */
void i2c1_stop(void);


/**
 * @brief 
 * 
 */
void i2c1_clear_addr(void);


/**
 * @brief 
 * 
 */
void i2c1_write_address(uint8_t i2c1_address);


/**
 * @brief Write data to a device with I2C 1
 * 
 * @details Note that this function is set up for 7-bit I2C address and not 10-bit. 
 * 
 * @param data : pointer to data to be sent over the bus 
 * @param data_size : integer indicating the number of bytes to be sent 
 * @param slave_address : 7-bit I2C address of targeted device 
 */
void i2c1_write_master_mode(uint8_t *data, uint8_t data_size);


/**
 * @brief Read data from a device with I2C 1 
 * 
 * @details 
 * 
 * @param data : pointer that data is placed into 
 * @param data_size : integer indicating the number of bytes to be receieved
 * @param slave_address : 7-bit I2C address of targeted device 
 * 
 */
void i2c1_read_master_mode(uint8_t *data, uint8_t data_size);

//=======================================================================================


#endif  // _I2C_COMM_H_ 
