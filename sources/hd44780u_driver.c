/**
 * @file wayintop_lcd_driver.c
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

//===============================================================================
// Includes 

#include "hd44780u_driver.h"

//===============================================================================


//===============================================================================
// Notes 
// - Sometimes the screen will display garbage data on startup/restart and it is suspected 
//   that this is from poor timing during initialization. It is unclear why the timing may 
//   be poor and in what exact steps of the initialization that this occurs. Most of the 
//   time it seems to be ok and a manual reset tends to "fix" the problem. The exact 
//   problem and fix is unknown. 
// - The screen is really sensitive to timing. Adding delays between write operations 
//   seems to be more reliable. 
//===============================================================================


//===============================================================================
// Function Prototypes 

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
 * @brief Send data to screen 
 * 
 * @details Takes instruction and data information from the user send functions and 
 *          sends it to the screen via I2C. 
 * 
 * @param i2c : 
 * @param data : pointer to data to send 
 */
void hd44780u_send(
    I2C_TypeDef *i2c, 
    uint8_t *data);

//===============================================================================


//===============================================================================
// Variables 

// HD44780U data record 
typedef struct hd44780u_data_record_s 
{
    // I2C port used by the device 
    I2C_TypeDef *i2c; 

    // Device I2C addresses 
    uint8_t write_addr;              // Write address 
    uint8_t read_addr;               // Read address 

    // Display data 
    char line1[HD44780U_LINE_LEN];   // LCD line 1 data output 
    char line2[HD44780U_LINE_LEN];   // LCD line 2 data output 
    char line3[HD44780U_LINE_LEN];   // LCD line 3 data output 
    char line4[HD44780U_LINE_LEN];   // LCD line 4 data output 
}
hd44780u_data_record_t; 


// HD44780U data record instance 
static hd44780u_data_record_t hd44780u_data_record; 

//===============================================================================


//===============================================================================
// Initialization 

// HD44780U screen init 
void hd44780u_init(
    I2C_TypeDef *i2c, 
    TIM_TypeDef *timer, 
    pcf8574_addr_t addr)
{
    // Initialize the device record 
    hd44780u_data_record.i2c = i2c;                               // I2C port used 
    hd44780u_data_record.write_addr = addr;                       // I2C write address 
    hd44780u_data_record.read_addr = addr + HD44780U_ADDR_INC;    // I2C read address 
    hd44780u_line_clear(HD44780U_L1);                             // Clear line 1 data 
    hd44780u_line_clear(HD44780U_L2);                             // Clear line 2 data 
    hd44780u_line_clear(HD44780U_L3);                             // Clear line 3 data 
    hd44780u_line_clear(HD44780U_L4);                             // Clear line 4 data 

    // Initialize the screen 

    // Wait for more than 40 ms after Vcc rises to 2.7V 
    tim_delay_ms(timer, HD44780U_DELAY_100MS);

    // Function set. Wait for more than 4.1 ms. 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X30);
    tim_delay_ms(timer, HD44780U_DELAY_005MS); 

    // Function set. Wait for more than 100 us. 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X30);
    tim_delay_ms(timer, HD44780U_DELAY_001MS); 

    // Function set. No specified wait time. 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X30);
    tim_delay_ms(timer, HD44780U_DELAY_010MS); 

    // Function set - Choose 4-bit mode
    // DL = 0 -> 4-bit data length 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X20);
    tim_delay_ms(timer, HD44780U_DELAY_010MS); 

    // Function set - Specify the number of display lines and character font
    // N = 1  -> Sets the number of dsiplay lines to 2 
    // F = 0  -> Sets character font to 5x8 dots
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X28);
    tim_delay_ms(timer, HD44780U_DELAY_001MS); 

    // Display off 
    // D = 0 -> Display off 
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0x08);
    tim_delay_ms(timer, HD44780U_DELAY_001MS); 

    // Display clear 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X01);
    tim_delay_ms(timer, HD44780U_DELAY_001MS); 

    // Entry mode set 
    // I/D = 1 -> Increment 
    // S   = 0 -> No display shifting 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X06);
    tim_delay_ms(timer, HD44780U_DELAY_001MS); 

    // Display on 
    // D = 1 -> Display on
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X0C);
    tim_delay_ms(timer, HD44780U_DELAY_001MS); 

    // Clear the display and pause briefly 
    hd44780u_clear();
    tim_delay_ms(timer, HD44780U_DELAY_500MS);   // Helps screen to stabilize before use 
}

//===============================================================================


//===============================================================================
// Send functions 

// Add: position selector, send integer, send float 

// HD44780U send a string of data 
void hd44780u_send_string(
    char *print_string)
{
    // Send one string character at a time
    while(*print_string)
    {
        // Cast from char to integer for send_data function 
        hd44780u_send_data((uint8_t)(*print_string));
        print_string++;
    }
}


// HD44780U clear screen 
void hd44780u_clear(void)
{
    // Set the write address to the first place on the screen 
    hd44780u_send_instruc(HD44780U_START_L1);

    // Send a blank string to each display character to clear the display
    for(uint8_t i = 0; i < HD44780U_NUM_CHAR; i++)
        hd44780u_send_data(' ');
}


// Set cursor position 
void hd44780u_cursor_pos(
    hd44780u_line_start_position_t line_start, 
    uint8_t offset)
{
    // Check that the currsor remains on the specified line 
    if (offset >= HD44780U_LINE_LEN) return; 

    // Update the cursor position 
    hd44780u_send_instruc(line_start + offset); 
}


// HD44780U send line 
void hd44780u_send_line(
    hd44780u_lines_t line)
{
    // Move to the screen position address 
    char *line_data = hd44780u_data_record.line1; 
    line_data += (line*HD44780U_LINE_LEN); 

    // Send a line of data 
    for(uint8_t i = 0; i < HD44780U_LINE_LEN; i++)
        hd44780u_send_data((uint8_t)(*line_data++));
}


// HD44780U send a single byte of instruction information 
void hd44780u_send_instruc(
    uint8_t hd44780u_cmd)
{
    // Organize send data into a sendable format
    uint8_t lcd_instruction[HD44780U_MSG_PER_CMD];
    lcd_instruction[0] = (hd44780u_cmd & 0xF0) | HD44780U_CONFIG_CMD_0X0C;
    lcd_instruction[1] = (hd44780u_cmd & 0xF0) | HD44780U_CONFIG_CMD_0X08;
    lcd_instruction[2] = ((hd44780u_cmd << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X0C;
    lcd_instruction[3] = ((hd44780u_cmd << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X08;

    // Send the data to the screen 
    hd44780u_send(hd44780u_data_record.i2c, lcd_instruction); 
}


// HD44780U send a single byte of printable data 
void hd44780u_send_data(
    uint8_t hd44780u_data)
{
    // Organize send data into a sendable format
    uint8_t lcd_display_data[HD44780U_MSG_PER_CMD];
    lcd_display_data[0] = (hd44780u_data & 0xF0) | HD44780U_CONFIG_CMD_0X0D;
    lcd_display_data[1] = (hd44780u_data & 0xF0) | HD44780U_CONFIG_CMD_0X09;
    lcd_display_data[2] = ((hd44780u_data << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X0D;
    lcd_display_data[3] = ((hd44780u_data << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X09;

    // Send the data to the screen 
    hd44780u_send(hd44780u_data_record.i2c, lcd_display_data); 
}


// Send information to screen 
void hd44780u_send(
    I2C_TypeDef *i2c, 
    uint8_t *data)
{
    // Create start condition to initiate master mode 
    i2c_start(i2c); 

    // Send the MPU6050 address with a write offset
    i2c_write_address(i2c, hd44780u_data_record.write_addr);
    i2c_clear_addr(i2c);

    // Send data over I2C1
    i2c_write_master_mode(i2c, data, HD44780U_MSG_PER_CMD); 

    // Create a stop condition
    i2c_stop(i2c); 
}

//===============================================================================


//===============================================================================
// Setters 

// Set the content of a line on the screen 
void hd44780u_line_set(
    hd44780u_lines_t line, 
    char *data, 
    uint8_t offset)
{
    // Move to the screen position address 
    char *line_data = hd44780u_data_record.line1; 
    line_data += (line*HD44780U_LINE_LEN + offset); 

    // Copy the new line data to the data record 
    while ((offset++ < HD44780U_LINE_LEN) && (*data != NULL_CHAR))
        *line_data++ = *data++; 
}


// Clear the contents of a line 
void hd44780u_line_clear(
    hd44780u_lines_t line)
{
    // Move to the screen position address 
    char *line_data = hd44780u_data_record.line1; 
    line_data += (line*HD44780U_LINE_LEN); 

    for(uint8_t i = 0; i < HD44780U_LINE_LEN; i++) 
        *line_data++ = ' '; 
}

//===============================================================================