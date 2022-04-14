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
 * @details enum code: I2C_CCR_(X_1)M_(X_2)_(X_3)_(X_4)
 *              X_1: Specifies mode, either Fm or Sm
 *              X_2: Duty cycle - ex. 169 -> 16/9 in Fm mode
 *              X_3: PCLK1 frquency (MHz)
 *              X_4: SCL frquency (kHz)
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
// Function Prototypes

/**
 * @brief Initiate I2C 1 in Mater Mode
 * 
 */
void i2c1_init_master_mode(void);


/**
 * @brief Write data to a device with I2C 1
 * 
 * @details Note that this function is set up for 7-bit I2C address and not 10-bit. 
 * 
 * @param data : pointer to data to be sent over the bus 
 * @param data_size : integer indicating the number of bytes to be sent 
 * @param slave_address : 7-bit I2C address of targeted device 
 */
void i2c1_write_master_mode(uint8_t *data, uint8_t data_size, uint8_t slave_address);


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
void i2c1_read_master_mode(uint8_t *data, uint8_t data_size, uint8_t slave_address);

//=======================================================================================


#endif  // _I2C_COMM_H_ 
