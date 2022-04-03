/**
 * @file wayintop_lcd_driver.c
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
    // 5. Function set - Specify the number of display lines and character font
    // 6. Display off. 
    // 7. Display clear. 
    // 8. Entry mode set. 
    // 9. Display on. 
    // ============================================================

    // TODO create I2C send function 

    // 1. Wait for more than 40 ms after Vcc rises to 2.7V 
    tim9_delay_ms(HD44780U_DELAY_050MS);

    // 2. Function set. Wait for more than 4.1 ms. 
    i2c1_write_master_mode(
        0x30, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_005MS);

    // 3. Function set. Wait for more than 100 us. 
    i2c1_write_master_mode(
        0x30, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_us(HD44780U_DELAY_200US);

    // 4. Function set. No specified wait time. 
    i2c1_write_master_mode(
        0x30, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_010MS);

    // 5. Function set - Specify the number of display lines and character font
    // DL = 1 -> 4-bit data length 
    // N = 1  -> Sets the number of dsiplay lines to 2 
    // F = 0  -> Sets character font to 5x8 dots 
    i2c1_write_master_mode(
        0x20, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 6. Display off 
    // D = 0 -> Display off 
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    i2c1_write_master_mode(
        HD44780U_8BIT_SETUP_CMD_3, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 7. Display clear 
    i2c1_write_master_mode(
        HD44780U_8BIT_SETUP_CMD_1, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 8. Entry mode set 
    // I/D = 1 -> Increment 
    // S = 0   -> No display shifting 
    i2c1_write_master_mode(
        HD44780U_8BIT_SETUP_CMD_2, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 9. Display on 
    // D = 0 -> Display off 
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    i2c1_write_master_mode(
        HD44780U_8BIT_SETUP_CMD_4, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // Test dsiplay
    i2c1_write_master_mode(
        73, 
        I2C_1_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS); 
}

//=======================================================================================


//=======================================================================================
// Print data 
//=======================================================================================
