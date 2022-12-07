/**
 * @file wayintop_lcd_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U + PCF8574 20x4 LCD screen driver 
 * 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HD44780U_DRIVER_H_
#define _HD44780U_DRIVER_H_

//===============================================================================
// Includes 

// Tools 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "timers.h"
#include "i2c_comm.h"

//===============================================================================


//===============================================================================
// Macros

#define HD44780U_NUM_LINES 4    // Number of lines on the screen 
#define HD44780U_MSG_PER_CMD 4  // Number of I2C bytes sent per one screen command
#define HD44780U_NUM_CHAR 80    // Number of character spaces on the screen 
#define HD44780U_LINE_LEN 20    // Number of characters per line on the screem 
#define HD44780U_ADDR_INC 1     // I2C address increment 

//===============================================================================


//===============================================================================
// Enums 

/**
 * @brief PCF8574 I2C addresses
 * 
 * @details The PCF8574 is the i2c module that relays i2c messages from the controller 
 *          to the screen. The module has contacts on its surface that can be grounded to 
 *          manually set the i2c address. By default none of the contacts are grounded.
 *          The following are all the possible write addresses the module can have. The 
 *          addresses are defined in the deviced user manual. All of the possible read 
 *          addresses are simply each of the below write addresses +1. This means that 
 *          only one of the below addresses needs to be associated with a device and 
 *          the write and read address can be selected by the driver as needed. <br> 
 *          
 *          LLL : write address = 0x40, read address = 0x41 <br> 
 *          LLH : write address = 0x42, read address = 0x43 <br> 
 *          LHL : write address = 0x44, read address = 0x45 <br> 
 *          LHH : write address = 0x46, read address = 0x47 <br> 
 *          HLL : write address = 0x48, read address = 0x49 <br> 
 *          HLH : write address = 0x4A, read address = 0x4B <br> 
 *          HHL : write address = 0x4C, read address = 0x4D <br> 
 *          HHH : write address = 0x4E, read address = 0x4F <br> 
 */
typedef enum {
    PCF8574_ADDR_LLL = 0x40,
    PCF8574_ADDR_LLH = 0x42,
    PCF8574_ADDR_LHL = 0x44,
    PCF8574_ADDR_LHH = 0x46,
    PCF8574_ADDR_HLL = 0x48,
    PCF8574_ADDR_HLH = 0x4A,
    PCF8574_ADDR_HHL = 0x4C,
    PCF8574_ADDR_HHH = 0x4E
} pcf8574_addr_t;


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
    HD44780U_DELAY_500MS = 500
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


/**
 * @brief HD44780U lines 
 */
typedef enum {
    HD44780U_L1,
    HD44780U_L2,
    HD44780U_L3,
    HD44780U_L4
} hd44780u_lines_t;


/**
 * @brief HD44780U cursor offset 
 */
typedef enum {
    HD44780U_CURSOR_OFFSET_0,
    HD44780U_CURSOR_OFFSET_1,
    HD44780U_CURSOR_OFFSET_2,
    HD44780U_CURSOR_OFFSET_3, 
    HD44780U_CURSOR_OFFSET_4, 
    HD44780U_CURSOR_OFFSET_5, 
    HD44780U_CURSOR_OFFSET_6, 
    HD44780U_CURSOR_OFFSET_7, 
    HD44780U_CURSOR_OFFSET_8, 
    HD44780U_CURSOR_OFFSET_9, 
    HD44780U_CURSOR_OFFSET_10, 
    HD44780U_CURSOR_OFFSET_11, 
    HD44780U_CURSOR_OFFSET_12, 
    HD44780U_CURSOR_OFFSET_13, 
    HD44780U_CURSOR_OFFSET_14, 
    HD44780U_CURSOR_OFFSET_15, 
    HD44780U_CURSOR_OFFSET_16, 
    HD44780U_CURSOR_OFFSET_17, 
    HD44780U_CURSOR_OFFSET_18, 
    HD44780U_CURSOR_OFFSET_19 
} hd44780u_cursor_offset_t;

//===============================================================================


//===============================================================================
// Initialization 

/**
 * @brief HD44780U screen initialization
 * 
 * @details This function configures the screen for displaying data. The steps for 
 *          manually configuring the device are outlined in the devices user manual. 
 *          The function hd44780u_send_instruc and the commands defined in 
 *          hd44780u_setup_cmds_t are used to configure the screen. 
 * 
 * // TODO add a device instance argument when multiple devices becomes supported 
 * 
 * @see hd44780u_send_instruc
 * @see hd44780u_setup_cmds_t
 * 
 * @param i2c 
 * @param timer 
 * @param addr 
 */
void hd44780u_init(
    I2C_TypeDef *i2c, 
    TIM_TypeDef *timer, 
    pcf8574_addr_t addr);


/**
 * @brief HD44780U screen re-initialization 
 * 
 * @details Used in the HD44780U controller for resetting the device 
 */
void hd44780u_re_init(void); 

//===============================================================================


//===============================================================================
// Send functions 

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


/**
 * @brief Set cursor position 
 * 
 * @details 
 * 
 * @param line_start 
 * @param offset 
 */
void hd44780u_cursor_pos(
    hd44780u_line_start_position_t line_start, 
    uint8_t offset); 


/**
 * @brief HD44780U send line 
 * 
 * @details 
 * 
 * @param line : 
 */
void hd44780u_send_line(
    hd44780u_lines_t line); 

//===============================================================================


//===============================================================================
// Setters 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param line 
 * @param line_data 
 * @param offset 
 */
void hd44780u_line_set(
    hd44780u_lines_t line, 
    char *line_data, 
    uint8_t offset); 


/**
 * @brief Clear a line 
 * 
 * @details 
 * 
 * @param line : 
 */
void hd44780u_line_clear(
    hd44780u_lines_t line); 

//===============================================================================

#endif  // _HD44780U_DRIVER_H_
