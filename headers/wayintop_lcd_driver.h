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
 * @details The PCF8574 is the I2C module that relays I2C messages from the controller 
 *          to the screen. The board has contacts on the surface that allow the address
 *          to be set manually by grounding certain contacts. The contacts are not 
 *          grounded by default. These are the addresses used for writing to the 
 *          screen and the address depends on the contact grounding configuration. 
 *          These addresses are from the user manual for the I2C module. 
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
 * @details The PCF8574 is the I2C module that relays I2C messages from the controller 
 *          to the screen. The board has contacts on the surface that allow the address
 *          to be set manually by grounding certain contacts. The contacts are not 
 *          grounded by default. These are the addresses used for reading from the 
 *          screen and the address depends on the contact grounding configuration. 
 *          These addresses are from the user manual for the I2C module. 
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
 * @details The screen requires certain delays between each instruction initialization 
 *          command. These come from the user manaul. The timers driver is used to 
 *          provide a delay. 
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
 * @details The screen requires certain instructions to be sent to it in order to 
 *          initialize it correctly. These are directly from the user manual. 
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
 *          commands are automatically added to the instruction and data commands 
 *          in the send functions. See the screen manual for more information. 
 *          
 *          bit 3: backlight -> 0 = off, 1 = on
 *          bit 2: Enable -> 0 = start, 1 = stop
 *          bit 1: R/W -> 0 = read, 1 = write
 *          bit 0: RS register -> 0 = instruction, 1 = data
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
 *          the information that gets sent to the screen. 
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
 * @brief LCD screen initialization sequence. 
 * 
 */
void hd44780u_init(void);

/**
 * @brief LCD send command
 * 
 * @details Instructions for the LCD screen get sent here from hd44780u_init, 
 *          formatted, and forwarded to the LCD using I2C. This function is also used
 *          for setting the DDRAM value to choose a position on the screen. 
 * 
 * @param hd44780u_cmd : instruction to configure the screen 
 */
void hd44780u_send_instruc(uint8_t hd44780u_cmd);

/**
 * @brief LCD send data
 * 
 * @details All the information that gets printed on the screen is sent using this 
 *          function. It sends a single byte of information on each pass. Command data 
 *          gets formatted before sending to the screen. 
 * 
 * @param hd44780u_data : command to be printed to the screen
 */
void hd44780u_send_data(uint8_t hd44780u_data);

/**
 * @brief LCD send string
 * 
 * @details Sends a whole string of data to be printed on the screen. One character 
 *          or byte is sent at a time using the hd44780u_send_data function. 
 * 
 * @see hd44780u_send_data
 * 
 * @param print_string : string of data that gets printed to the screen
 */
void hd44780u_send_string(char *print_string);

/**
 * @brief Clear LCD display 
 * 
 * @details Blank characters are send to all character spaces on the screen to erase 
 *          any existing text. The DDRAM address also gets updates to the cursor sits 
 *          at the beginning of line 1. 
 * 
 */
void hd44780u_clear(void);

//=======================================================================================


#endif  // _WAYINTOP_LCD_DRIVER_H_
