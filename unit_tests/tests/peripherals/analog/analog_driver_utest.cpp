#include "CppUTest/TestHarness.h"

extern "C"
{
	// Add your C-only include files here 
    #include "analog_driver.h" 
}

//==================================================
// Notes 

// Test plan 
// - 

//==================================================

TEST_GROUP(analog_driver)
{
    // Constructor 
    void setup()
    {
        // 
    }

    // Destructor 
    void teardown()
    {
        // 
    }
}; 


// Test 0 
TEST(analog_driver, test0)
{
    // ADC1 
    // 0x40000000UL + 0x00010000UL + 0x2000UL; 

    // uint32_t adc1_fake = CLEAR; 
    // ADC_TypeDef *ADC1_FAKE = (ADC_TypeDef *)((uint32_t)&adc1_fake); 

    // ADC1_FAKE->DR = CLEAR; 

    // ADC COMMON 

    // // 
    // adc_port_init(
    //     ADC1, 
    //     ADC1_COMMON, 
    //     ADC_PCLK2_4, 
    //     ADC_RES_8, 
    //     ADC_EOC_EACH, 
    //     ADC_SCAN_ENABLE, 
    //     ADC_CONT_DISABLE, 
    //     ADC_DMA_DISABLE, 
    //     ADC_DDS_DISABLE, 
    //     ADC_EOC_INT_DISABLE, 
    //     ADC_OVR_INT_DISABLE); 
}
