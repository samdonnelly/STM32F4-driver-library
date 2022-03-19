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
 * @brief I2C CCR setpoint
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
} i2c_ccr_setpoint_t;

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
    I2C_TRISE_300_42 = 13
} i2c_trise_setpoint_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief Initiate I2C 1 in Mater Mode
 * 
 */
void i2c1_init_master_mode(void);


/**
 * @brief Initiate I2C 1 in Slave Mode 
 * 
 */
void i2c1_init_slave_mode(void);


/**
 * @brief Write data to a device with I2C 1
 * 
 */
void i2c1_write(void);


/**
 * @brief Read data from a device with I2C 1 
 * 
 */
void i2c1_write(void);

//=======================================================================================


#endif  // _I2C_COMM_H_ 
