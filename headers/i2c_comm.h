/**
 * @file i2c_comm.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
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
// Macros 
//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief 
 * 
 */
typedef enum {
    I2C1_SCL_PB6 = 0,
    I2C1_SCL_PB8 = 2
} i2c1_scl_pin_t;

/**
 * @brief 
 * 
 */
typedef enum {
    I2C1_SDA_PB7 = 0,
    I2C1_SDA_PB9 = 2
} i2c1_sda_pin_t;

/**
 * @brief 
 * 
 */
typedef enum {
    I2C2_SCL_PB10
} i2c2_scl_pin_t;

/**
 * @brief 
 * 
 */
typedef enum {
    I2C2_SDA_PB3,
    I2C2_SDA_PB9
} i2c2_sda_pin_t;

/**
 * @brief 
 * 
 */
typedef enum {
    I2C3_SCL_PA8
} i2c3_scl_pin_t;

/**
 * @brief 
 * 
 */
typedef enum {
    I2C3_SDA_PB4,
    I2C3_SDA_PB8,
    I2C3_SDA_PC9
} i2c3_sda_pin_t;


typedef enum {
    I2C_SM_MODE,
    I2C_FM_MODE
} i2c_run_mode_t;

/**
 * @brief 
 * 
 */
typedef enum {
    I2C_FM_DUTY_2,   // t_low/t_high = 2
    I2C_FM_DUTY_169  // t_low/t_high = 16/9
} i2c_fm_duty_cycle_t;

/**
 * @brief I2C AP1 frequency 
 * 
 */
typedef enum {
    I2C_APB1_42MHZ = 42,
    I2C_APB1_84MHZ = 84
} i2c_apb1_freq_t;

/**
 * @brief I2C Fm CCR setpoint
 * 
 * @details enum code: I2C_CCR_FM_(X_1)_(X_2)_(X_3)
 *              X_1: Duty cycle - ex. 169 -> 16/9 in Fm mode
 *              X_2: PCLK1 frquency (MHz)
 *              X_3: SCL frquency (kHz)
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual. 
 * 
 */
typedef enum {
    I2C_CCR_FM_169_42_400 = 5
} i2c_fm_ccr_setpoint_t;

/**
 * @brief I2C Sm CCR setpoint
 * 
 * @details enum code: I2C_CCR_SM_(X_1)_(X_2)
 *              X_1: PCLK1 frquency (MHz)
 *              X_2: SCL frquency (kHz)
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual. 
 * 
 */
typedef enum {
    I2C_CCR_SM_42_100 = 210
} i2c_sm_ccr_setpoint_t;

/**
 * @brief I2C TRISE setpoint
 * 
 * @details enum code: I2C_TRISE_(X_1)_(X_2)
 *              X_1: Max rise time (ns)
 *              X_2: PCLK1 frequency (MHz)
 *          
 *          Note: A calculation must be done to determine the numbers that work together.
 *                See the Reference Manual. 
 * 
 */
typedef enum {
    I2C_TRISE_0300_42 = 13,
    I2C_TRISE_1000_42 = 43
} i2c_trise_setpoint_t;

/**
 * @brief I2C data size 
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
// Structures 

/**
 * @brief 
 * 
 */
typedef struct i2c_init_settings_s
{
    // 
    uint32_t i2c_rcc_apb1enr;

    // 
    uint32_t i2c_rcc_ahb1_enr;

    // 
    uint32_t i2c_gpiob_moder;

    // 
    uint32_t i2c_gpiob_otyper;

    // 
    uint32_t i2c_gpiob_ospeedr;

    // 
    uint32_t i2c_gpiob_pupdr;

    // 
    uint32_t i2c_gpiob_afr_h;

    // 
    uint16_t i2c_cr1;

    // 
    uint16_t i2c_cr2;

    // 
    uint16_t i2c_ccr;

    // 
    uint16_t i2c_trise;

} i2c_init_settings_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief Initiate I2C 1 in Mater Mode
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


void i2c2_init(void);


void i2c3_init(void);


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
void i2c1_addr_wait(void);


/**
 * @brief 
 * 
 */
void i2c1_clear_ack(void);


/**
 * @brief 
 * 
 */
void i2c1_set_ack(void);


/**
 * @brief 
 * 
 */
void i2c1_rxne_wait(void);


/**
 * @brief 
 * 
 */
void i2c1_txe_wait(void);


/**
 * @brief 
 * 
 */
void i2c1_btf_wait(void);


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
