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

    // Data list 
    uint8_t data_list[4];
    data_list[0] = (uint8_t)(0x00);
    data_list[1] = (uint8_t)(0x00);
    data_list[2] = (uint8_t)(0x00);
    data_list[3] = (uint8_t)(0x00);

    // 1. Wait for more than 40 ms after Vcc rises to 2.7V 
    tim9_delay_ms(HD44780U_DELAY_050MS);

    // 2. Function set. Wait for more than 4.1 ms. 
    data_list[0] = (uint8_t)(0x3C);
    data_list[1] = (uint8_t)(0x38);
    data_list[2] = (uint8_t)(0x0C);
    data_list[3] = (uint8_t)(0x08);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_005MS);

    // 3. Function set. Wait for more than 100 us. 
    data_list[0] = (uint8_t)(0x3C);
    data_list[1] = (uint8_t)(0x38);
    data_list[2] = (uint8_t)(0x0C);
    data_list[3] = (uint8_t)(0x08);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_us(HD44780U_DELAY_200US);

    // 4. Function set. No specified wait time. 
    data_list[0] = (uint8_t)(0x3C);
    data_list[1] = (uint8_t)(0x38);
    data_list[2] = (uint8_t)(0x0C);
    data_list[3] = (uint8_t)(0x08);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_010MS);

    // 5. Function set - Specify the number of display lines and character font
    // DL = 0 -> 4-bit data length 
    data_list[0] = (uint8_t)(0x2C);
    data_list[1] = (uint8_t)(0x28);
    data_list[2] = (uint8_t)(0x0C);
    data_list[3] = (uint8_t)(0x08);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 
    // N = 1  -> Sets the number of dsiplay lines to 2 
    // F = 0  -> Sets character font to 5x8 dots
    data_list[0] = (uint8_t)(0x2C);
    data_list[1] = (uint8_t)(0x28);
    data_list[2] = (uint8_t)(0x8C);
    data_list[3] = (uint8_t)(0x88);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 6. Display off 
    // D = 0 -> Display off 
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    data_list[0] = (uint8_t)(0x0C);
    data_list[1] = (uint8_t)(0x08);
    data_list[2] = (uint8_t)(0x8C);
    data_list[3] = (uint8_t)(0x88);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 7. Display clear 
    data_list[0] = (uint8_t)(0x0C);
    data_list[1] = (uint8_t)(0x08);
    data_list[2] = (uint8_t)(0x1C);
    data_list[3] = (uint8_t)(0x18);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 8. Entry mode set 
    // I/D = 1 -> Increment 
    // S = 0   -> No display shifting 
    data_list[0] = (uint8_t)(0x0C);
    data_list[1] = (uint8_t)(0x08);
    data_list[2] = (uint8_t)(0x6C);
    data_list[3] = (uint8_t)(0x68);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 9. Display on 
    // D = 1 -> Display on
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    data_list[0] = (uint8_t)(0x0C);
    data_list[1] = (uint8_t)(0x08);
    data_list[2] = (uint8_t)(0xCC);
    data_list[3] = (uint8_t)(0xC8);
    i2c1_write_master_mode(
        data_list, 
        I2C_4_BYTE, 
        PCF8574_HHH_WRITE_ADDRESS);
    tim9_delay_ms(HD44780U_DELAY_001MS);
}

//=======================================================================================


//=======================================================================================
// Print data 
//=======================================================================================
