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
    // ============================================================

    // TODO create I2C send function 

    // 1. Wait for more than 40 ms after Vcc rises to 2.7V 
    tim9_delay_ms(HD44780U_DELAY_050MS);

    // 2. Function set. Wait for more than 4.1 ms. 
    tim9_delay_ms(HD44780U_DELAY_005MS);

    // 3. Function set. Wait for more than 100 us. 
    tim9_delay_us(HD44780U_DELAY_200US);

    // 4. Function set. No specified wait time. 
    tim9_delay_ms(HD44780U_DELAY_010MS);
}

//=======================================================================================


//=======================================================================================
// Print data 
//=======================================================================================
