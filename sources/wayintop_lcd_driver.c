/**
 * @file wayintop_lcd_driver.c
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

//=======================================================================================
// Includes 

#include "wayintop_lcd_driver.h"

//=======================================================================================


//=======================================================================================
// Initialization 

// LCD screen init 
void hd44780u_init(void)
{
    // ============================================================
    // Steps to initialize HD44780U (LCD screen) 
    // 1. Wait for more than 40 ms after Vcc rises to 2.7V. 
    // 2. Function set. Wait for more than 4.1 ms. 
    // 3. Function set. Wait for more than 100 us. 
    // 4. Function set. No specified wait time. 
    // 5. Function set - Choose 4-bit mode
    // 6. Function set - Specify the number of display lines and character font
    // 7. Display off. 
    // 8. Display clear. 
    // 9. Entry mode set. 
    // 10. Display on. 
    // ============================================================

    // 1. Wait for more than 40 ms after Vcc rises to 2.7V 
    tim9_delay_ms(HD44780U_DELAY_050MS);

    // 2. Function set. Wait for more than 4.1 ms. 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X30);
    tim9_delay_ms(HD44780U_DELAY_005MS);

    // 3. Function set. Wait for more than 100 us. 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X30);
    tim9_delay_us(HD44780U_DELAY_200US);

    // 4. Function set. No specified wait time. 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X30);
    tim9_delay_ms(HD44780U_DELAY_010MS);

    // 5. Function set - Choose 4-bit mode
    // DL = 0 -> 4-bit data length 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X20);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 6. Function set - Specify the number of display lines and character font
    // N = 1  -> Sets the number of dsiplay lines to 2 
    // F = 0  -> Sets character font to 5x8 dots
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X28);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 7. Display off 
    // D = 0 -> Display off 
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0x08);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 8. Display clear 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X01);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 9. Entry mode set 
    // I/D = 1 -> Increment 
    // S = 0   -> No display shifting 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X06);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 10. Display on 
    // D = 1 -> Display on
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    hd44780u_send_instruc(HD44780U_SETUP_CMD_0X0C);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // Clear the display 
    hd44780u_clear();
}

// Send single byte of instruction information 
void hd44780u_send_instruc(uint8_t hd44780u_cmd)
{
    // Organize send data into a sendable format
    uint8_t lcd_setup_data[HD44780U_MSG_PER_CMD];
    lcd_setup_data[0] = (hd44780u_cmd & 0xF0) | HD44780U_CONFIG_CMD_0X0C;
    lcd_setup_data[1] = (hd44780u_cmd & 0xF0) | HD44780U_CONFIG_CMD_0X08;
    lcd_setup_data[2] = ((hd44780u_cmd << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X0C;
    lcd_setup_data[3] = ((hd44780u_cmd << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X08;

    // Create start condition to initiate master mode 
    i2c1_start(); 

    // Send the MPU6050 address with a write offset
    i2c1_write_address(PCF8574_HHH_WRITE_ADDRESS);
    i2c1_clear_addr();

    // Send data over I2C 1
    i2c1_write_master_mode(lcd_setup_data, HD44780U_MSG_PER_CMD);

    // Create a stop condition
    i2c1_stop();
}

//=======================================================================================


//=======================================================================================
// Print data 

// Send single byte of printable data 
void hd44780u_send_data(uint8_t hd44780u_data)
{
    // Organize send data into a sendable format
    uint8_t lcd_print_data[HD44780U_MSG_PER_CMD];
    lcd_print_data[0] = (hd44780u_data & 0xF0) | HD44780U_CONFIG_CMD_0X0D;
    lcd_print_data[1] = (hd44780u_data & 0xF0) | HD44780U_CONFIG_CMD_0X09;
    lcd_print_data[2] = ((hd44780u_data << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X0D;
    lcd_print_data[3] = ((hd44780u_data << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X09;

    // Create start condition to initiate master mode 
    i2c1_start(); 

    // Send the MPU6050 address with a write offset
    i2c1_write_address(PCF8574_HHH_WRITE_ADDRESS);
    i2c1_clear_addr();

    // Send data over I2C 1
    i2c1_write_master_mode(lcd_print_data, HD44780U_MSG_PER_CMD);

    // Create a stop condition
    i2c1_stop();
}

// Send a string of data string
void hd44780u_send_string(char *print_string)
{
    // Send one string character at a time
    while(*print_string)
    {
        // Cast from char to integer for send_data function 
        hd44780u_send_data((uint8_t)(*print_string));
        print_string++;
    }
}

// Clear the HD44780U screen 
void hd44780u_clear(void)
{
    // Set the write address to the first place on the screen 
    hd44780u_send_instruc(HD44780U_START_L1);

    // Send a blank string to each display character to clear the display
    for(uint8_t i = 0; i < HD44780U_NUM_CHAR; i++)
    {
        hd44780u_send_data(' ');
    }
}

//=======================================================================================
