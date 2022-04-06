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
    hd44780u_init_send(HD44780U_SETUP_CMD_0X30);
    tim9_delay_ms(HD44780U_DELAY_005MS);

    // 3. Function set. Wait for more than 100 us. 
    hd44780u_init_send(HD44780U_SETUP_CMD_0X30);
    tim9_delay_us(HD44780U_DELAY_200US);

    // 4. Function set. No specified wait time. 
    hd44780u_init_send(HD44780U_SETUP_CMD_0X30);
    tim9_delay_ms(HD44780U_DELAY_010MS);

    // 5. Function set - Choose 4-bit mode
    // DL = 0 -> 4-bit data length 
    hd44780u_init_send(HD44780U_SETUP_CMD_0X20);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 6. Function set - Specify the number of display lines and character font
    // N = 1  -> Sets the number of dsiplay lines to 2 
    // F = 0  -> Sets character font to 5x8 dots
    hd44780u_init_send(HD44780U_SETUP_CMD_0X28);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 7. Display off 
    // D = 0 -> Display off 
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    hd44780u_init_send(HD44780U_SETUP_CMD_0x08);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 8. Display clear 
    hd44780u_init_send(HD44780U_SETUP_CMD_0X01);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 9. Entry mode set 
    // I/D = 1 -> Increment 
    // S = 0   -> No display shifting 
    hd44780u_init_send(HD44780U_SETUP_CMD_0X06);
    tim9_delay_ms(HD44780U_DELAY_001MS);

    // 10. Display on 
    // D = 1 -> Display on
    // C = 0 -> Cursor not displayed 
    // B = 0 -> No blinking 
    hd44780u_init_send(HD44780U_SETUP_CMD_0X0C);
    tim9_delay_ms(HD44780U_DELAY_001MS);
}

// 
void hd44780u_init_send(uint8_t hd44780u_cmd)
{
    // Data list 
    uint8_t data_list[HD44780U_MSG_PER_CMD];
    data_list[0] = (hd44780u_cmd & 0xF0) | HD44780U_CONFIG_CMD_0X0C;
    data_list[1] = (hd44780u_cmd & 0xF0) | HD44780U_CONFIG_CMD_0X08;
    data_list[2] = ((hd44780u_cmd << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X0C;
    data_list[3] = ((hd44780u_cmd << SHIFT_4) & 0xF0) | HD44780U_CONFIG_CMD_0X08;

    // Send data over I2C 1
    i2c1_write_master_mode(
        data_list, 
        HD44780U_MSG_PER_CMD, 
        PCF8574_HHH_WRITE_ADDRESS);
}

//=======================================================================================


//=======================================================================================
// Print data 
//=======================================================================================
