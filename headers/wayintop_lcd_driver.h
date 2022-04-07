/**
 * @file wayintop_lcd_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _WAYINTOP_LCD_DRIVER_H_
#define _WAYINTOP_LCD_DRIVER_H_


//=======================================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "timers.h"
#include "i2c_comm.h"
#include "uart_comm.h"

//=======================================================================================


//=======================================================================================
// Macros

#define HD44780U_MSG_PER_CMD 4  // Number of I2C bytes sent per one LCD screen command

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief PCF8574 write addresses
 * 
 */
typedef enum {
    PCF8574_LLL_WRITE_ADDRESS = 0x40,
    PCF8574_LLH_WRITE_ADDRESS = 0x42,
    PCF8574_LHL_WRITE_ADDRESS = 0x44,
    PCF8574_LHH_WRITE_ADDRESS = 0x46,
    PCF8574_HLL_WRITE_ADDRESS = 0x48,
    PCF8574_HLH_WRITE_ADDRESS = 0x4A,
    PCF8574_HHL_WRITE_ADDRESS = 0x4C,
    PCF8574_HHH_WRITE_ADDRESS = 0x4E
} pcf8574_wrte_addresses_t;

/**
 * @brief PCF8574 read addresses
 * 
 */
typedef enum {
    PCF8574_LLL_READ_ADDRESS = 0x41,
    PCF8574_LLH_READ_ADDRESS = 0x43,
    PCF8574_LHL_READ_ADDRESS = 0x45,
    PCF8574_LHH_READ_ADDRESS = 0x47,
    PCF8574_HLL_READ_ADDRESS = 0x49,
    PCF8574_HLH_READ_ADDRESS = 0x4B,
    PCF8574_HHL_READ_ADDRESS = 0x4D,
    PCF8574_HHH_READ_ADDRESS = 0x4F
} pcf8574_read_addresses_t;

/**
 * @brief HD44780U delays 
 * 
 */
typedef enum {
    HD44780U_DELAY_001MS = 1,
    HD44780U_DELAY_005MS = 5,
    HD44780U_DELAY_010MS = 10,
    HD44780U_DELAY_050MS = 50,
    HD44780U_DELAY_200US = 200
} hd44780u_delays_t;

/**
 * @brief HD44780U setup commands
 * 
 */
typedef enum {
    HD44780U_SETUP_CMD_0X01 = 0x01,
    HD44780U_SETUP_CMD_0X06 = 0x06,
    HD44780U_SETUP_CMD_0x08 = 0x08,
    HD44780U_SETUP_CMD_0X0C = 0x0C,
    HD44780U_SETUP_CMD_0X20 = 0x20,
    HD44780U_SETUP_CMD_0X28 = 0x28,
    HD44780U_SETUP_CMD_0X30 = 0x30
} hd44780u_setup_cmds_t;

/**
 * @brief HD44780U configuration commands
 * 
 * @details 
 * 
 */
typedef enum {
    HD44780U_CONFIG_CMD_0X08 = 0x08,
    HD44780U_CONFIG_CMD_0X09 = 0x09,
    HD44780U_CONFIG_CMD_0X0C = 0x0C,
    HD44780U_CONFIG_CMD_0X0D = 0x0D
} hd44780u_config_cmds_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief LCD screen init 
 * 
 */
void hd44780u_init(void);

/**
 * @brief LCD send command
 * 
 * @details The command for the LCD screen gets sent here from hd44780u_init, formatted, 
 *          and send to the LCD using I2C. 
 * 
 * @param hd44780u_cmd 
 */
void hd44780u_init_send(uint8_t hd44780u_cmd);

/**
 * @brief LCD send data
 * 
 * @param hd44780u_data 
 */
void hd44780u_send_data(uint8_t hd44780u_data);

/**
 * @brief LCD send string
 * 
 * @param print_string 
 */
void hd44780u_send_string(char *print_string);

//=======================================================================================


#endif  // _WAYINTOP_LCD_DRIVER_H_
