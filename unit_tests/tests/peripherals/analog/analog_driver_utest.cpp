#include "CppUTest/TestHarness.h"

extern "C"
{
	// Add your C-only include files here 
    #include "analog_driver.h" 
}

//==================================================
// Notes 
//==================================================

TEST_GROUP(analog_driver)
{
    // Test group global variables 
    RCC_TypeDef RCC_FAKE; 
    ADC_TypeDef ADC1_FAKE; 
    ADC_Common_TypeDef ADC1_COMMON_FAKE; 

    // Constructor 
    void setup()
    {
        // Run initialization functions common to all tests 
        adc1_clock_enable(&RCC_FAKE); 

        adc_port_init(
            &ADC1_FAKE, 
            &ADC1_COMMON_FAKE, 
            ADC_PCLK2_4, 
            ADC_RES_8, 
            ADC_PARAM_ENABLE,    // End of Conversion (EOC) 
            ADC_PARAM_DISABLE,   // End of Conversion (EOC) interrupt 
            ADC_PARAM_ENABLE,    // Scan mode 
            ADC_PARAM_DISABLE,   // Continuous mode 
            ADC_PARAM_DISABLE,   // DMA mode 
            ADC_PARAM_DISABLE,   // DMA disable selection 
            ADC_PARAM_DISABLE);  // Overrun interrupt 
    }

    // Destructor 
    void teardown()
    {
        // 
    }
}; 


// Invalid ADC register pointers - no initialization performed 
TEST(analog_driver, null_ptr_no_init)
{
    // Local variables 
    RCC_TypeDef *RCC_LOCAL_FAKE = NULL; 
    ADC_TypeDef *ADC1_LOCAL_FAKE = NULL; 
    ADC_Common_TypeDef *ADC1_COMMON_LOCAL_FAKE = NULL; 

    // Run initialization functions 
    ADC_STATUS clock_status = adc1_clock_enable(RCC_LOCAL_FAKE); 

    ADC_STATUS port_status = adc_port_init(
        ADC1_LOCAL_FAKE, 
        ADC1_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        ADC_PARAM_DISABLE,   // End of Conversion (EOC) 
        ADC_PARAM_DISABLE,   // End of Conversion (EOC) interrupt 
        ADC_PARAM_DISABLE,   // Scan mode 
        ADC_PARAM_DISABLE,   // Continuous mode 
        ADC_PARAM_DISABLE,   // DMA mode 
        ADC_PARAM_DISABLE,   // DMA disable selection 
        ADC_PARAM_DISABLE);  // Overrun interrupt 

    // Check that initialization was skipped due to invalid register addresses 
    LONGS_EQUAL(ADC_NO_INIT, clock_status); 
    LONGS_EQUAL(ADC_NO_INIT, port_status); 
}
