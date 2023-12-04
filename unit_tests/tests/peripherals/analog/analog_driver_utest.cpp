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
    // ADC_TypeDef ADC1_FAKE; 
    // ADC_Common_TypeDef ADC1_COMMON_FAKE; 

    adc_param_config_t adc_port_config[7]; 

    // Constructor 
    void setup()
    {
        // Run initialization functions common to all tests 
        adc1_clock_enable(&RCC_FAKE); 

        memset((void *)adc_port_config, ADC_PARAM_DISABLE, sizeof(adc_port_config)); 

        // adc_port_init(
        //     &ADC1_FAKE, 
        //     &ADC1_COMMON_FAKE, 
        //     ADC_PCLK2_4, 
        //     ADC_RES_8, 
        //     ADC_PARAM_ENABLE,    // End of Conversion (EOC) 
        //     ADC_PARAM_DISABLE,   // End of Conversion (EOC) interrupt 
        //     ADC_PARAM_ENABLE,    // Scan mode 
        //     ADC_PARAM_DISABLE,   // Continuous mode 
        //     ADC_PARAM_DISABLE,   // DMA mode 
        //     ADC_PARAM_DISABLE,   // DMA disable selection 
        //     ADC_PARAM_DISABLE);  // Overrun interrupt 
    }

    // Destructor 
    void teardown()
    {
        // 
    }
}; 


//=======================================================================================
// Helper functions 

// 
ADC_STATUS adc_local_init(
    ADC_TypeDef *adc, 
    ADC_Common_TypeDef *adc_common, 
    adc_param_config_t *parms)
{
    return adc_port_init(adc, 
                         adc_common, 
                         ADC_PCLK2_4, 
                         ADC_RES_8, 
                         parms[0],   // End of Conversion (EOC) 
                         parms[1],   // End of Conversion (EOC) interrupt 
                         parms[2],   // Scan mode 
                         parms[3],   // Continuous mode 
                         parms[4],   // DMA mode 
                         parms[5],   // DMA disable selection 
                         parms[6]);  // Overrun interrupt 
}

//=======================================================================================


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


// Initialization OK 
TEST(analog_driver, init_success)
{
    // Local variables 
    ADC_TypeDef ADC1_LOCAL_FAKE; 
    ADC_Common_TypeDef ADC1_COMMON_LOCAL_FAKE; 

    // Initialization 
    // ADC_STATUS port_status = adc_port_init(
    //     &ADC1_LOCAL_FAKE, 
    //     &ADC1_COMMON_LOCAL_FAKE, 
    //     ADC_PCLK2_4, 
    //     ADC_RES_8, 
    //     ADC_PARAM_DISABLE,   // End of Conversion (EOC) 
    //     ADC_PARAM_DISABLE,   // End of Conversion (EOC) interrupt 
    //     ADC_PARAM_DISABLE,   // Scan mode 
    //     ADC_PARAM_DISABLE,   // Continuous mode 
    //     ADC_PARAM_DISABLE,   // DMA mode 
    //     ADC_PARAM_DISABLE,   // DMA disable selection 
    //     ADC_PARAM_DISABLE);  // Overrun interrupt 
    ADC_STATUS port_status = adc_local_init(&ADC1_LOCAL_FAKE, 
                                            &ADC1_COMMON_LOCAL_FAKE, 
                                            adc_port_config); 

    // Check that initialization was skipped due to invalid register addresses 
    LONGS_EQUAL(ADC_OK, port_status); 
}
