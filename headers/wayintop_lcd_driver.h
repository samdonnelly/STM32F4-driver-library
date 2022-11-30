/**
 * @file wayintop_lcd_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Wayintop 2004 LCD screen (HD44780U + PCF8574) driver
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

//=======================================================================================


//=======================================================================================
// Macros

#define HD44780U_MSG_PER_CMD 4  // Number of I2C bytes sent per one LCD screen command
#define HD44780U_NUM_CHAR 80    // Number of character spaces on the display 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief PCF8574 write addresses
 * 
 * @details The PCF8574 is the i2c module that relays i2c messages from the controller 
 *          to the screen. The module has contacts on its surface that can be grounded to 
 *          manually set the i2c address. By default none of the contacts are grounded.
 *          The following are all the possible write addresses the module can have. The 
 *          addresses are defined in the deviced user manual. 
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
 * @details The PCF8574 is the i2c module that relays i2c messages from the controller 
 *          to the screen. The module has contacts on its surface that can be grounded to 
 *          manually set the i2c address. By default none of the contacts are grounded.
 *          The following are all the possible read addresses the module can have. The 
 *          addresses are defined in the deviced user manual. 
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
 * @details The screen requires certain delays between each initialization instruction 
 *          which are defined in the screens user manaul. The following values are the
 *          various delays needed throughout the initialization sequence. 
 * 
 * @see hd44780u_init
 * 
 */
typedef enum {
    HD44780U_DELAY_001MS = 1,
    HD44780U_DELAY_005MS = 5,
    HD44780U_DELAY_010MS = 10,
    HD44780U_DELAY_050MS = 50,
    HD44780U_DELAY_100MS = 100,
    HD44780U_DELAY_200US = 200
} hd44780u_delays_t;

/**
 * @brief HD44780U setup commands
 * 
 * @details The screen requires certain initialization instructions which are defined in 
 *          user manual. The following values are the needed initialization instructions. 
 * 
 * @see hd44780u_init
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
 * @details These commands provide values for the screen backlight, enable or start
 *          transmission signal, read/write command and register choice. These 
 *          commands are predefined because they don't need to be changed and are 
 *          sent along with instruction/data information to the screen. Screen data 
 *          transmission works by sending 4-bits of instruction/data along with 4-bits 
 *          of these commands to make a byte. <br><br>
 *          
 *          bit 3: backlight ---> 0 = off,     1 = on     <br>
 *          bit 2: Enable ------> 0 = start,   1 = stop   <br>
 *          bit 1: R/W ---------> 0 = read,    1 = write  <br>
 *          bit 0: RS register -> 0 = instruc, 1 = data   <br>
 * 
 * @see hd44780u_send_instruc
 * @see hd44780u_send_data
 * 
 */
typedef enum {
    HD44780U_CONFIG_CMD_0X08 = 0x08,
    HD44780U_CONFIG_CMD_0X09 = 0x09,
    HD44780U_CONFIG_CMD_0X0C = 0x0C,
    HD44780U_CONFIG_CMD_0X0D = 0x0D
} hd44780u_config_cmds_t;

/**
 * @brief HD44780U start of line address
 * 
 * @details Each character space on the display corresponds to a DDRAM address. The 
 *          addresses count up from 0x00 to 0x67 which corresponds to 80 characters in 
 *          a 20x4 dsiplay. The addresses start at the beginning of line 1 (0x00) and 
 *          carry over to line 3, then line 2, and lastly line 4 which is the reason 
 *          for the order in the enum below. These addresses can be used to format 
 *          the information that gets sent to the screen particularily in application 
 *          code where screen messages are more specific. 
 * 
 */
typedef enum {
    HD44780U_START_L1 = 0x80,
    HD44780U_START_L3 = 0x94,
    HD44780U_START_L2 = 0xC0,
    HD44780U_START_L4 = 0xD4
} hd44780u_line_start_position_t;

//=======================================================================================


//=======================================================================================
// Function Prototypes

/**
 * @brief HD44780U screen initialization
 * 
 * @details This function configures the screen for displaying data. The steps for 
 *          manually configuring the device are outlined in the devices user manual. 
 *          The function hd44780u_send_instruc and the commands defined in 
 *          hd44780u_setup_cmds_t are used to configure the screen. 
 * 
 * @see hd44780u_send_instruc
 * @see hd44780u_setup_cmds_t
 * 
 */
void hd44780u_init(void);

/**
 * @brief HD44780U send command
 * 
 * @details This function is used for configuring settings on the screen. The 
 *          hd44780u_init functions uses this function to send configuration commands. 
 *          The function can also be used to set the cursor position by setting the 
 *          DDRAM address value. The i2c driver is used to send the instructions. 
 *          Before sending instructions, the instruction data is formatted using the 
 *          hd44780u_config_cmds_t commands. 
 * 
 * @see hd44780u_init
 * @see hd44780u_config_cmds_t
 * 
 * @param hd44780u_cmd : instruction to configure the screen 
 */
void hd44780u_send_instruc(uint8_t hd44780u_cmd);

/**
 * @brief HD44780U send data
 * 
 * @details This function is used to print information onto the screen one byte at a time. 
 *          The function can be called directly for printing a single character or 
 *          hd44780u_send_string can be used to repeatedly call the function and print a 
 *          string. hd44780u_clear uses this function to send blank characters to the 
 *          screen. The i2c driver is used to send data. Before sending the data, the 
 *          data is formatted using hd44780u_config_cmds_t commands. 
 * 
 * @see hd44780u_send_string
 * @see hd44780u_config_cmds_t
 * 
 * @param hd44780u_data : command to be printed to the screen
 */
void hd44780u_send_data(uint8_t hd44780u_data);

/**
 * @brief HD44780U send string
 * 
 * @details This function is used to print a string onto the screen which is defined 
 *          by the user and application. The function takes a pointer to a string and 
 *          repeatedly calls hd44780u_send_data to print the entire string. 
 * 
 * @see hd44780u_send_data
 * 
 * @param print_string : string of data that gets printed to the screen
 */
void hd44780u_send_string(char *print_string);

/**
 * @brief HD44780U clear display 
 * 
 * @details Blank characters are send to all character spaces on the screen to erase 
 *          any existing text. The DDRAM address also gets updated so that the cursor sits 
 *          at the beginning of line 1. The function repeatedly calls hd44780u_send_data
 *          to send the blank characters. 
 * 
 * @see hd44780u_send_data
 * 
 */
void hd44780u_clear(void);

//=======================================================================================


#endif  // _WAYINTOP_LCD_DRIVER_H_
