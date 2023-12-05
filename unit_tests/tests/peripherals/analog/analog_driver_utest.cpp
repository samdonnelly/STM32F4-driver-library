//=======================================================================================
// Notes 
//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

extern "C"
{
	// Add your C-only include files here 
    #include "analog_driver.h" 
}

//=======================================================================================


//=======================================================================================
// Maros 

#define NUM_PORT_PARAM_CONFIGS 7 

// APB2ENR 
#define APB2ENR_ADC1_CLK_BIT  0x0000100

// CCR 
#define CCR_PRE_LOW_BIT       0x00010000 
#define CCR_PRE_HIGH_BIT      0x00020000 

// CR1 
#define CR1_EOC_INT_BIT       0x00000020 
#define CR1_SCAN_BIT          0x00000100 
#define CR1_RES_LOW_BIT       0x01000000 
#define CR1_RES_HIGH_BIT      0x02000000 
#define CR1_OVERRUN_BIT       0x04000000 

// CR2 
#define CR2_CONT_BIT          0x00000002 
#define CR2_DMA_BIT           0x00000100 
#define CR2_DMA_DIS_BIT       0x00000200 
#define CR2_EOC_BIT           0x00000400 

//=======================================================================================


//=======================================================================================
// Test Group 

TEST_GROUP(analog_driver)
{
    // Test group global variables 
    ADC_TypeDef ADC1_FAKE; 
    adc_param_config_t adc_port_config[NUM_PORT_PARAM_CONFIGS]; 

    // Constructor 
    void setup()
    {
        // Initialize data 
        for (uint8_t i = CLEAR; i < NUM_PORT_PARAM_CONFIGS; i++)
            adc_port_config[i] = ADC_PARAM_DISABLE; 
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

// Run ADC port initialization 
ADC_STATUS adc_port_local_init(
    ADC_TypeDef *adc, 
    ADC_Common_TypeDef *adc_common, 
    adc_prescalar_t prescalar, 
    adc_res_t resolution, 
    adc_param_config_t *parms)
{
    ADC_STATUS port_status = adc_port_init(
        adc, 
        adc_common, 
        prescalar, 
        resolution, 
        parms[0],   // End of Conversion (EOC) 
        parms[1],   // End of Conversion (EOC) interrupt 
        parms[2],   // Scan mode 
        parms[3],   // Continuous mode 
        parms[4],   // DMA mode 
        parms[5],   // DMA disable selection 
        parms[6]);  // Overrun interrupt 

    return port_status; 
}

//=======================================================================================


//=======================================================================================
// Tests 

//==================================================
// ADC1 clock enable 

// ADC1 clock enable - call the init function with an invalid pointer 
TEST(analog_driver, adc1_clock_null_ptr)
{
    RCC_TypeDef *RCC_LOCAL_FAKE = nullptr; 
    ADC_STATUS clock_status = adc1_clock_enable(RCC_LOCAL_FAKE); 
    LONGS_EQUAL(ADC_INVALID_PTR, clock_status); 
}


// ADC1 clock enable - call the init function with a valid pointer 
TEST(analog_driver, adc1_clock_valid_ptr)
{
    RCC_TypeDef RCC_LOCAL_FAKE; 
    ADC_STATUS clock_status = adc1_clock_enable(&RCC_LOCAL_FAKE); 
    LONGS_EQUAL(ADC_OK, clock_status); 
}


// ADC1 clock enable - clock successfully enabled 
TEST(analog_driver, adc1_clock_init_ok)
{
    RCC_TypeDef RCC_LOCAL_FAKE; 
    RCC_LOCAL_FAKE.APB2ENR = CLEAR; 
    adc1_clock_enable(&RCC_LOCAL_FAKE); 
    LONGS_EQUAL(APB2ENR_ADC1_CLK_BIT, RCC_LOCAL_FAKE.APB2ENR); 
}

//==================================================

//==================================================
// ADC port init 

// ADC port init - call the init function with an invalid pointer 
TEST(analog_driver, adc_port_null_ptr)
{
    // Local variables 
    ADC_TypeDef *ADC1_LOCAL_FAKE = NULL; 
    ADC_Common_TypeDef *ADC1_COMMON_LOCAL_FAKE = NULL; 

    // Run initialization functions 
    ADC_STATUS port_status = adc_port_local_init(
        ADC1_LOCAL_FAKE, 
        ADC1_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_2, 
        ADC_RES_12, 
        adc_port_config); 

    // Check that initialization was skipped due to invalid register addresses 
    LONGS_EQUAL(ADC_INVALID_PTR, port_status); 
}


// ADC port init - call the init function with a valid pointer 
TEST(analog_driver, adc_port_valid_ptr)
{
    // Local variables 
    ADC_TypeDef ADC1_LOCAL_FAKE; 
    ADC_Common_TypeDef ADC1_COMMON_LOCAL_FAKE; 

    // Run initialization functions 
    ADC_STATUS port_status = adc_port_local_init(
        &ADC1_LOCAL_FAKE, 
        &ADC1_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_2, 
        ADC_RES_12, 
        adc_port_config); 

    // Check that initialization completed 
    LONGS_EQUAL(ADC_OK, port_status); 
}


// ADC port init - init ok, register bits successfully cleared 
TEST(analog_driver, adc_port_init_ok_bits_cleared)
{
    // Local variables 
    ADC_Common_TypeDef ADC1_COMMON_LOCAL_FAKE; 
    uint32_t ccr_check = CCR_PRE_HIGH_BIT | CCR_PRE_LOW_BIT; 
    uint32_t cr1_check = CR1_RES_HIGH_BIT | CR1_RES_LOW_BIT | CR1_SCAN_BIT | 
                         CR1_EOC_INT_BIT | CR1_OVERRUN_BIT; 
    uint32_t cr2_check = CR2_EOC_BIT | CR2_CONT_BIT | CR2_DMA_BIT | CR2_DMA_DIS_BIT; 

    // Set the register bits high to make sure they get cleared by the init function 
    ADC1_COMMON_LOCAL_FAKE.CCR = HIGH_32BIT; 
    ADC1_FAKE.CR1 = HIGH_32BIT; 
    ADC1_FAKE.CR2 = HIGH_32BIT; 

    // Run initialization functions 
    adc_port_local_init(
        &ADC1_FAKE, 
        &ADC1_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_2, 
        ADC_RES_12, 
        adc_port_config); 

    // Common Control Register - prescalar 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC1_COMMON_LOCAL_FAKE.CCR & ccr_check); 
    // Control Register 1 - resolution, scan, EOC interrupt, overrun interrupt 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC1_FAKE.CR1 & cr1_check); 
    // Control Register 2 - EOC select, continuous mode, DMA, DMA disable selection 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC1_FAKE.CR2 & cr2_check); 
}


// ADC port init - init ok, register bits successfully set 
TEST(analog_driver, adc_port_init_ok_bits_set)
{
    // Local variables 
    ADC_Common_TypeDef ADC1_COMMON_LOCAL_FAKE; 
    uint32_t ccr_check = CCR_PRE_LOW_BIT; 
    uint32_t cr1_check = CR1_RES_HIGH_BIT | CR1_SCAN_BIT | CR1_EOC_INT_BIT | CR1_OVERRUN_BIT; 
    uint32_t cr2_check = CR2_EOC_BIT | CR2_CONT_BIT | CR2_DMA_BIT | CR2_DMA_DIS_BIT; 

    // Clear the register bits to make sure they get set by the init function 
    ADC1_COMMON_LOCAL_FAKE.CCR = CLEAR; 
    ADC1_FAKE.CR1 = CLEAR; 
    ADC1_FAKE.CR2 = CLEAR; 

    // Enable all the initialization parameters 
    for (uint8_t i = CLEAR; i < NUM_PORT_PARAM_CONFIGS; i++)
        adc_port_config[i] = ADC_PARAM_ENABLE; 

    // Initialization 
    adc_port_local_init(
        &ADC1_FAKE, 
        &ADC1_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        adc_port_config); 

    // Common Control Register - prescalar 
    UNSIGNED_LONGS_EQUAL(ccr_check, ADC1_COMMON_LOCAL_FAKE.CCR); 
    // Control Register 1 - resolution, scan, EOC interrupt, overrun interrupt 
    UNSIGNED_LONGS_EQUAL(cr1_check, ADC1_FAKE.CR1); 
    // Control Register 2 - EOC select, continuous mode, DMA, DMA disable selection 
    UNSIGNED_LONGS_EQUAL(cr2_check, ADC1_FAKE.CR2); 
}

//==================================================

//==================================================
// ADC pin init 

// ADC pin init - call the init function with an invalid pointer 
TEST(analog_driver, adc_pin_init_invalid_ptr)
{
    GPIO_TypeDef *GPIO_LOCAL_FAKE = nullptr; 
    ADC_STATUS pin_status = adc_pin_init(
        &ADC1_FAKE, 
        GPIO_LOCAL_FAKE, 
        PIN_0, 
        ADC_CHANNEL_0, 
        ADC_SMP_3); 
    LONGS_EQUAL(ADC_INVALID_PTR, pin_status); 
}


// ADC pin init - call the init function with a valid pointer 
TEST(analog_driver, adc_pin_init_valid_ptr)
{
    GPIO_TypeDef GPIO_LOCAL_FAKE; 
    ADC_STATUS pin_status = adc_pin_init(
        &ADC1_FAKE, 
        &GPIO_LOCAL_FAKE, 
        PIN_0, 
        ADC_CHANNEL_0, 
        ADC_SMP_3); 
    LONGS_EQUAL(ADC_OK, pin_status); 
}

//==================================================

//==================================================
// ADC watchdog init 
//==================================================

//==================================================
// ADC sequence 
//==================================================

//==================================================
// ADC sequence length 
//==================================================

//==================================================
// ADC on 
//==================================================

//==================================================
// ADC off 
//==================================================

//==================================================
// ADC start 
//==================================================

//==================================================
// ADC read single (no DMA) 
//==================================================

//==================================================
// ADC scan sequence (no DMA) 
//==================================================

//=======================================================================================
