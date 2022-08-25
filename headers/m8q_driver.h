/**
 * @file m8q_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS module driver 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _M8Q_DRIVER_H_
#define _M8Q_DRIVER_H_

//=======================================================================================
// Includes

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Communication drivers 
#include "i2c_comm.h"

// Other drivers 

//=======================================================================================


//=======================================================================================
// Macros 

#define M8Q_DATA_LEN 30    // Max length of data string recieved from the module 
#define M8Q_I2C_ADDR 0x42  // M8Q I2C address (default) 

//=======================================================================================


//=======================================================================================
// Function Prototypes 




//=======================================================================================

#endif  // _M8Q_DRIVER_H_ 
