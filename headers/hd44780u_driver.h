/**
 * @file wayintop_lcd_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U + PCF8574 20x4 LCD screen driver header 
 * 
 * @version 0.1
 * @date 2022-02-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HD44780U_DRIVER_H_
#define _HD44780U_DRIVER_H_

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

// Device information 

#define HD44780U_NUM_LINES 4           // Number of lines on the screen 
#define HD44780U_MSG_PER_CMD 4         // Number of I2C bytes sent per one screen command
#define HD44780U_NUM_CHAR 80           // Number of character spaces on the screen 
#define HD44780U_LINE_LEN 20           // Number of characters per line on the screen 


// Message information 

// General 
#define HD44780U_4BIT_MASK 0xF0        // Message data mask used for formatting in 4-bit mode 

// Message lengths 
#define HD44780U_MSG_LINE_LEN_1 1      // 1 line screen message 
#define HD44780U_MSG_LINE_LEN_2 2      // 2 line screen message 
#define HD44780U_MSG_LINE_LEN_3 3      // 3 line screen message 
#define HD44780U_MSG_LINE_LEN_4 4      // 4 line screen message 

// Clear display 
#define HD44780U_CLEAR_DISPLAY 0x01    // Standard display clear bit 

// Entry mode set 
#define HD44780U_ENTRY_SET 0x04        // Standard entry mode bit 
#define HD44780U_CURSOR_DIR 0x02       // I/D - Cursor direction (increment/decrement) 
#define HD44780U_DISPLAY_SHIFT 0x01    // S - display shift increment 

// Display control 
#define HD44780U_DISPLAY_CONTROL 0x08  // Standard display control bit 
#define HD44780U_DISPLAY_ON 0x04       // D - Display on/off 
#define HD44780U_CURSOR_ON 0x02        // C - Cursor on/off 
#define HD44780U_BLINK_ON 0x01         // B - Cursor blink on/off 

// Function set 
#define HD44780U_FUNCTION_SET 0x20     // Standard function set bit 
#define HD44780U_8BIT_MODE 0x10        // DL - data length, 8-bit mode 
#define HD44780U_4BIT_MODE 0x00        // DL - data length, 4-bit mode 
#define HD44780U_2_LINE 0x08           // N - 2 line display 
#define HD44780U_1_LINE 0x00           // N - 1 line display 
#define HD44780U_5x10 0x04             // F - 5x10 dot display 
#define HD44780U_5x8 0x00              // F - 5x8 dot display 

// Additional bits 
#define HD44780U_EN 0x04               // Enable bit - 1: enable, 0: disable 
#define HD44780U_RW 0x02               // Read/write bit - 1: read, 0: write 
#define HD44780U_RS 0x01               // Register select bit - 1: data reg, 0: instruction reg 

// Backlight control 
#define HD44780U_BACKLIGHT 0x08        // Backlight on 
#define HD44780U_NO_BACKLIGHT 0x00     // Backlight off 

// Other 
#define HD44780U_CURSOR_HOME 0         // Start of a line 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief PCF8574 I2C addresses
 * 
 * @details The PCF8574 is the i2c module that relays i2c messages from the controller 
 *          to the screen. The module has contacts on its surface that can be grounded to 
 *          manually set the i2c address. By default none of the contacts are grounded.
 *          The following are all the possible write addresses the module can have. The 
 *          addresses are defined in the device user manual. All of the possible read 
 *          addresses are simply each of the below write addresses +1. This means that 
 *          only one of the below addresses needs to be associated with a device and 
 *          the write and read address can be selected by the driver as needed. 
 *          
 *          LLL : write address = 0x40, read address = 0x41 
 *          LLH : write address = 0x42, read address = 0x43 
 *          LHL : write address = 0x44, read address = 0x45 
 *          LHH : write address = 0x46, read address = 0x47 
 *          HLL : write address = 0x48, read address = 0x49 
 *          HLH : write address = 0x4A, read address = 0x4B 
 *          HHL : write address = 0x4C, read address = 0x4D 
 *          HHH : write address = 0x4E, read address = 0x4F 
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
 * @brief HD44780U start of line address
 * 
 * @details Each character space on the display corresponds to a DDRAM address. The 
 *          addresses count up from 0x00 to 0x67 which corresponds to 80 characters in 
 *          a 20x4 dsiplay. The addresses start at the beginning of line 1 (0x00) and 
 *          carry over to line 3, then line 2, and lastly line 4 which is the reason 
 *          for the order in the enum below. These addresses can be used to format 
 *          the information that gets sent to the screen particularily in application 
 *          code where screen messages are more specific. 
 */
typedef enum {
    HD44780U_START_L1 = 0x80,   // 128 
    HD44780U_START_L3 = 0x94,   // 148 
    HD44780U_START_L2 = 0xC0,   // 192
    HD44780U_START_L4 = 0xD4    // 212 
} hd44780u_line_start_position_t;


/**
 * @brief HD44780U line numbers 
 */
typedef enum {
    HD44780U_L1,
    HD44780U_L2,
    HD44780U_L3,
    HD44780U_L4
} hd44780u_lines_t;

//=======================================================================================


//=======================================================================================
// Structures 

// Screen information 
typedef struct hd44780u_msgs_s 
{
    hd44780u_lines_t line; 
    char msg[HD44780U_LINE_LEN]; 
    uint8_t offset; 
}
hd44780u_msgs_t; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief HD44780U screen initialization
 * 
 * @details This function configures the screen for displaying data. The steps for 
 *          manually configuring the device are outlined in the devices user manual. 
 *          The function hd44780u_send_instruc and the commands defined in 
 *          hd44780u_setup_cmds_t are used to configure the screen. <br> 
 *          
 *          Note that the timer passed as an argument to this function should be 
 *          initialized as a blocking timer. This init function relies of delays 
 *          between commands sent to the device in order to succeed. 
 * 
 * @see hd44780u_send_instruc
 * @see hd44780u_setup_cmds_t
 * @see pcf8574_addr_t
 * 
 * @param i2c : pointer to i2c port used 
 * @param timer : pointer to timer port used 
 * @param addr : i2c address of screen being initialized 
 */
void hd44780u_init(
    I2C_TypeDef *i2c, 
    TIM_TypeDef *timer, 
    pcf8574_addr_t addr);


/**
 * @brief HD44780U screen re-initialization 
 * 
 * @details Used in the HD44780U controller for resetting the device. This function will 
 *          call hd44780u_init again to go through the device setup steps. 
 */
void hd44780u_re_init(void); 

//=======================================================================================


//=======================================================================================
// Setters and getters 

/**
 * @brief HD44780U set the contents of a line 
 * 
 * @details Updates the contents of a specific line to specified text in the device data 
 *          record. 
 *          
 *          A pointer to a character string of what to write to the line is passed as an 
 *          argument along with the position offset. The position offset determines the 
 *          the character position, starting from the left side of the screen, that the 
 *          string starts writing to. A string longer than the screen line length minus 
 *          the offset will be truncated at the end of the line and not continued onto the 
 *          next line. A line of the screen is 20 characters long. This setter updates only 
 *          the data record of the device, meaning the contents will not be seen on the 
 *          screen until the write state is triggered. Note that this function does not 
 *          erase the old contents of the line, it simply overwrites them starting at the 
 *          offset.
 * 
 * @param line : line content to update 
 * @param line_data : pointer to character string used to update the line 
 * @param offset : offset of where to start writing the character string 
 */
void hd44780u_line_set(
    hd44780u_lines_t line, 
    char *line_data, 
    uint8_t offset); 


/**
 * @brief HD44780U clear a line 
 * 
 * @details Clears all the contents of a specific line in the data record. 
 *          
 *          This function will overwrite the existing contents and replace it with blanks. 
 *          This function will update the devices data record and won't be seen on the screen 
 *          until the write state is triggered. 
 * 
 * @param line : line content to update 
 */
void hd44780u_line_clear(
    hd44780u_lines_t line); 


/**
 * @brief Get status flag 
 * 
 * @details Returns the drivers status flag. The status flag indicates faults that have 
 *          occured. The status flag breakdown is as follows: 
 *          - 0: No faults, everything OK 
 *          - 1: I2C timeout 
 *          
 *          When the status flag is set it can only be cleared using the clear status 
 *          function. 
 * 
 * @see hd44780u_clear_status
 * 
 * @return uint8_t : driver status return 
 */
uint8_t hd44780u_get_status(void); 


/**
 * @brief Clear status flag 
 * 
 * @details Allows for clearing of the driver status flag. See the description for the status 
 *          getter function for details on the status flag. 
 * 
 * @see hd44780u_get_status
 */
void hd44780u_clear_status(void); 

//=======================================================================================


//=======================================================================================
// Data functions 

/**
 * @brief HD44780U send line 
 * 
 * @details Sends the contents of a line in the data record to the screen for viewing. 
 * 
 * @param line : line of the data record to write to the screen 
 */
void hd44780u_send_line(
    hd44780u_lines_t line); 


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
void hd44780u_send_string(
    char *print_string); 

//=======================================================================================


//=======================================================================================
// User commands 

/**
 * @brief HD44780U clear display 
 * 
 * @details Uses the screens built in clear display command to wipe the contents of the screen. 
 *          The cursor position also gets updated to the beginning of line 1. 
 */
void hd44780u_clear(void);


/**
 * @brief HD44780U set cursor position 
 * 
 * @details Sets the cursor position on the screen. The cursor position will dictate where 
 *          text begins to appear on the screen when the screen is written to. This function 
 *          is used by the controller to set specific line content. It can also be used 
 *          without the controller to configure the screen as needed. 
 * 
 * @param line_start : line on the screen of where the cursor should go 
 * @param offset : character offset on the specified line to determine final location of cursor 
 */
void hd44780u_cursor_pos(
    hd44780u_line_start_position_t line_start, 
    uint8_t offset); 


/**
 * @brief Set cursor move direction to the right 
 * 
 * @details Part of the entry mode instruction represented by the I/D bit. 
 *          I/D = Increment (I/D = 1) and Decrement (I/D = 0). 
 *          This changes the DDRAM address by 1 when a character code is written or read from 
 *          DDRAM. This can be visualized by enabling the cursor and/or blink. 
 *          This function sets I/D = 1 to increment DDRAM. 
 */
void hd44780u_cursor_right(void); 


/**
 * @brief Set cursor move direction to the left 
 * 
 * @details Part of the entry mode instruction represented by the I/D bit. 
 *          I/D = Increment (I/D = 1) and Decrement (I/D = 0). 
 *          This changes the DDRAM address by 1 when a character code is written or read from 
 *          DDRAM. This can be visualized by enabling the cursor and/or blink. 
 *          This function sets I/D = 0 to decrement DDRAM. 
 */
void hd44780u_cursor_left(void); 


/**
 * @brief Enable display shifting --> will shift in the same direction as cursor move direction 
 * 
 * @details Part of the entry mode instruction represented by the S bit. 
 *          When S = 1 then the entire display will be shifted either to the right (I/D = 0) or 
 *          to the left (I/D = 1). If S = 0 then no shift will occur. 
 *          The display will no shift when reading DDRAM or when writing or reading CGRAM. 
 *          This function sets S = 1. 
 */
void hd44780u_shift_on(void); 


/**
 * @brief Disable display shifting 
 * 
 * @details Part of the entry mode instruction represented by the S bit. 
 *          When S = 1 then the entire display will be shifted either to the right (I/D = 0) or 
 *          to the left (I/D = 1). If S = 0 then no shift will occur. 
 *          The display will no shift when reading DDRAM or when writing or reading CGRAM. 
 *          This function sets S = 0. 
 */
void hd44780u_shift_off(void); 


/**
 * @brief Turn the display on 
 * 
 * @details Part of the display control instruction represented by the D bit. 
 *          The contents DDRAM (display data) will be shown when D = 1 and not shown when 
 *          D = 0. When D = 0 the DDRAM contents are not affected. 
 *          This function sets D = 1. 
 */
void hd44780u_display_on(void); 


/**
 * @brief Turn the display off 
 * 
 * @details Part of the display control instruction represented by the D bit. 
 *          The contents DDRAM (display data) will be shown when D = 1 and not shown when 
 *          D = 0. When D = 0 the DDRAM contents are not affected. 
 *          This function sets D = 0. 
 */
void hd44780u_display_off(void); 


/**
 * @brief Turn the cursor on 
 * 
 * @details Part of the display control instruction represented by the C bit. 
 *          C changes the visibility of the cursor but has no affect on the cursors 
 *          position or movement. When C = 1 the cursor is visible and when C = 0 it is 
 *          not visible. 
 *          This function sets C = 1. 
 */
void hd44780u_cursor_on(void); 


/**
 * @brief Turn the cursor off 
 * 
 * @details Part of the display control instruction represented by the C bit. 
 *          C changes the visibility of the cursor but has no affect on the cursors 
 *          position or movement. When C = 1 the cursor is visible and when C = 0 it is 
 *          not visible. 
 *          This function sets C = 0. 
 */
void hd44780u_cursor_off(void); 


/**
 * @brief Turn the cursor blink on 
 * 
 * @details Part of the display control instruction represented by the B bit. 
 *          B changes the visibility of the cursor blink. When B = 1 the cursor blinks 
 *          and when B = 0 there is no blink. 
 *          This function sets B = 1. 
*/
void hd44780u_blink_on(void); 


/**
 * @brief Turn the cursor blink off 
 * 
 * @details Part of the display control instruction represented by the B bit. 
 *          B changes the visibility of the cursor blink. When B = 1 the cursor blinks 
 *          and when B = 0 there is no blink. 
 *          This function sets B = 0. 
 */
void hd44780u_blink_off(void); 


/**
 * @brief Turn backlight on 
 * 
 * @details The backlight bit is sent along with the nibble that contains the register 
 *          selection, read/write bit and enable bit. When this bit is set the backlight 
 *          will turn on and when the bit is cleared the backlight will turn off. 
 *          This function turns the backlight in. 
 */
void hd44780u_backlight_on(void); 


/**
 * @brief Turn backlight off 
 * 
 * @details The backlight bit is sent along with the nibble that contains the register 
 *          selection, read/write bit and enable bit. When this bit is set the backlight 
 *          will turn on and when the bit is cleared the backlight will turn off. 
 *          This function turns the backlight off. 
 */
void hd44780u_backlight_off(void); 

//=======================================================================================

#endif  // _HD44780U_DRIVER_H_
