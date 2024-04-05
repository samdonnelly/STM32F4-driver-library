/**
 * @file ws2812_controller_utest.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 controller unit tests 
 * 
 * @version 0.1
 * @date 2024-04-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Notes 
//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

#include "ws2812_controller.h" 

extern "C"
{
	// Add your C-only include files here 
    #include "ws2812_driver_mock.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 

// Strobe info 
#define STROBE_LED_MASK 0x3C   // LEDs 2-5 are strobe LEDs (00111100b) 
#define STROBE_PERIOD 10 

// LED colours 
#define LED_COLOUR_0 0x0000002F 
#define LED_COLOUR_1 0x00001E00 
#define LED_COLOUR_2 0x003A0000 
#define LED_COLOUR_3 0x004B5C6D 

//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(ws2812_controller_test)
{
    // Global test group variables 
    WS2812_Controller leds_test = 
        WS2812_Controller(DEVICE_ONE, STROBE_LED_MASK, STROBE_PERIOD); 
    uint32_t led_colour_data[WS2812_LED_NUM]; 
    uint32_t led_colour_check[WS2812_LED_NUM]; 

    // Constructor 
    void setup()
    {
        ws2812_mock_init(); 
        memset((void *)led_colour_data, CLEAR, sizeof(led_colour_data)); 
        memset((void *)led_colour_check, CLEAR, sizeof(led_colour_check)); 
    }

    // Destructor 
    void teardown()
    {
        // 
    }
}; 

//=======================================================================================


//=======================================================================================
// Helper functions 
//=======================================================================================


//=======================================================================================
// Tests 

// Initialization 
TEST(ws2812_controller_test, ws2812_controller_init)
{
    // All LED colours will be initialized to zero. These are written and checked to 
    // check that this is true. 

    leds_test.LEDWrite(); 
    ws2812_mock_get_colour_data(led_colour_data); 
    MEMCMP_EQUAL(led_colour_check, led_colour_data, sizeof(led_colour_check)); 
}


// Set LED colours 
TEST(ws2812_controller_test, ws2812_controller_set_leds)
{
    // LEDs get set using single and multiple methods. LEDs dedicated to strobes are 
    // set as well but the controller should reject thia attempt. The colour is then 
    // written and checked. 

    leds_test.SetLEDColour(WS2812_LED_1, LED_COLOUR_0); 
    leds_test.SetLEDsColour(
        (SET_BIT << WS2812_LED_6) | (SET_BIT << WS2812_LED_7), LED_COLOUR_1); 
    leds_test.SetLEDColour(WS2812_LED_3, LED_COLOUR_2); 

    led_colour_check[WS2812_LED_1] = LED_COLOUR_0; 
    led_colour_check[WS2812_LED_6] = LED_COLOUR_1; 
    led_colour_check[WS2812_LED_7] = LED_COLOUR_1; 

    leds_test.LEDWrite(); 
    ws2812_mock_get_colour_data(led_colour_data); 
    MEMCMP_EQUAL(led_colour_check, led_colour_data, sizeof(led_colour_check)); 
}


// Set Strobe colour 
TEST(ws2812_controller_test, ws2812_controller_set_strobe)
{
    // The strobe colour gets set then the strobe is run until it lights up at which 
    // point the colour is checked. The strobe is then run until it turns off and it's 
    // checked again. 

    // Set the strobe colour 
    leds_test.SetStrobeColour(LED_COLOUR_3); 

    // Set the expected strobe colour 
    led_colour_check[WS2812_LED_2] = LED_COLOUR_3; 
    led_colour_check[WS2812_LED_3] = LED_COLOUR_3; 
    led_colour_check[WS2812_LED_4] = LED_COLOUR_3; 
    led_colour_check[WS2812_LED_5] = LED_COLOUR_3; 

    // Check that strobe colour was updated 
    for (uint8_t i = CLEAR; i < STROBE_PERIOD; i++)
    {
        leds_test.Strobe(); 
    }
    ws2812_mock_get_colour_data(led_colour_data); 
    MEMCMP_EQUAL(led_colour_check, led_colour_data, sizeof(led_colour_check)); 

    // Set the new expected strobe colour 
    // memset((void *)&led_colour_check[WS2812_LED_2], CLEAR, 4*sizeof(uint32_t)); 
    led_colour_check[WS2812_LED_2] = CLEAR; 
    led_colour_check[WS2812_LED_3] = CLEAR; 
    led_colour_check[WS2812_LED_4] = CLEAR; 
    led_colour_check[WS2812_LED_5] = CLEAR; 

    // Check that the strobe turned off 
    leds_test.Strobe(); 
    ws2812_mock_get_colour_data(led_colour_data); 
    MEMCMP_EQUAL(led_colour_check, led_colour_data, sizeof(led_colour_check)); 
}


// Turn the strobe lights off 
TEST(ws2812_controller_test, ws2812_controller_strobe_off)
{
    // This is the same as the test above except the strobe is manually turned off and 
    // a non-strobe LED is set to show that changing the strobe doesn't change the the 
    // other LEDs. 

    // Set the strobe colour 
    leds_test.SetStrobeColour(LED_COLOUR_2); 
    leds_test.SetLEDColour(WS2812_LED_0, LED_COLOUR_2); 

    // Set the expected strobe colour 
    led_colour_check[WS2812_LED_0] = LED_COLOUR_2; 
    led_colour_check[WS2812_LED_2] = LED_COLOUR_2; 
    led_colour_check[WS2812_LED_3] = LED_COLOUR_2; 
    led_colour_check[WS2812_LED_4] = LED_COLOUR_2; 
    led_colour_check[WS2812_LED_5] = LED_COLOUR_2; 

    // Check that strobe colour was updated 
    for (uint8_t i = CLEAR; i < STROBE_PERIOD; i++)
    {
        leds_test.Strobe(); 
    }
    ws2812_mock_get_colour_data(led_colour_data); 
    MEMCMP_EQUAL(led_colour_check, led_colour_data, sizeof(led_colour_check)); 

    // Set the new expected strobe colour 
    // memset((void *)&led_colour_check[WS2812_LED_2], CLEAR, 4*sizeof(uint32_t)); 
    led_colour_check[WS2812_LED_2] = CLEAR; 
    led_colour_check[WS2812_LED_3] = CLEAR; 
    led_colour_check[WS2812_LED_4] = CLEAR; 
    led_colour_check[WS2812_LED_5] = CLEAR; 

    // Check that the strobe turned off 
    leds_test.StrobeOff(); 
    ws2812_mock_get_colour_data(led_colour_data); 
    MEMCMP_EQUAL(led_colour_check, led_colour_data, sizeof(led_colour_check)); 
}

//=======================================================================================
