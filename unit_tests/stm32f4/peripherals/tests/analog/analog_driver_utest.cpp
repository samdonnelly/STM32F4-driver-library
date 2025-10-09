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

//==================================================
// Register bits 

// HTR & LTR 
#define ADC_WD_THRESH_MASK    0x00000FFF 

// APB2ENR 
#define APB2ENR_ADC1_CLK_BIT  0x0000100

// CCR 
#define CCR_PRE_LOW_BIT       0x00010000 
#define CCR_PRE_HIGH_BIT      0x00020000 

// SR 
#define SR_EOC_BIT            0x00000002 
#define SR_STRT_BIT           0x00000010 

// CR1 
#define CR1_AWDCH_LOW_BITS    0x0000000F
#define CR1_AWDCH_HIGH_BIT    0x00000010
#define CR1_EOC_INT_BIT       0x00000020 
#define CR1_AWDIE_BIT         0x00000040 
#define CR1_SCAN_BIT          0x00000100 
#define CR1_AWDSGL_BIT        0x00000200 
#define CR1_AWDEN_BIT         0x00800000
#define CR1_RES_LOW_BIT       0x01000000 
#define CR1_RES_HIGH_BIT      0x02000000 
#define CR1_OVERRUN_BIT       0x04000000 

// CR2 
#define CR2_ADON_BIT          0x00000001
#define CR2_CONT_BIT          0x00000002 
#define CR2_DMA_BIT           0x00000100 
#define CR2_DMA_DIS_BIT       0x00000200 
#define CR2_EOC_BIT           0x00000400 
#define CR2_SWSTART_BIT       0x40000000

// SMPR1 
#define SMPR1_17_LOW_BIT      0x00200000 
#define SMPR1_17_MED_BIT      0x00400000 
#define SMPR1_17_HIGH_BIT     0x00800000 

// SMPR2 
#define SMPR2_0_LOW_BIT       0x00000001 
#define SMPR2_0_MED_BIT       0x00000002 
#define SMPR2_0_HIGH_BIT      0x00000004 
#define SMPR2_1_LOW_BIT       0x00000008 
#define SMPR2_1_MED_BIT       0x00000010 
#define SMPR2_1_HIGH_BIT      0x00000020 

// SQR1 
#define SQR1_SEQ13_LOW_BITS   0x0000000F 
#define SQR1_SEQ13_HIGH_BIT   0x00000010 
#define SQR1_SEQ14_LOW_BITS   0x000001E0 
#define SQR1_SEQ14_HIGH_BIT   0x00000200 
#define SQR1_L_BITS           0x00F00000

// SQR2 
#define SQR2_SEQ7_LOW_BITS    0x0000000F 
#define SQR2_SEQ7_HIGH_BIT    0x00000010 
#define SQR2_SEQ8_LOW_BITS    0x000001E0 
#define SQR2_SEQ8_HIGH_BIT    0x00000200 

// SQR3 
#define SQR3_SEQ1_LOW_BITS    0x0000000F 
#define SQR3_SEQ1_HIGH_BIT    0x00000010 
#define SQR3_SEQ2_LOW_BITS    0x000001E0 
#define SQR3_SEQ2_HIGH_BIT    0x00000200 

//==================================================

//=======================================================================================


//=======================================================================================
// Test Group 

TEST_GROUP(analog_driver)
{
    // Test group global variables 
    ADC_TypeDef ADC_FAKE; 
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
    ADC_TypeDef *ADC_LOCAL_FAKE = NULL; 
    ADC_Common_TypeDef *ADC_COMMON_LOCAL_FAKE = NULL; 

    // Run initialization functions 
    ADC_STATUS port_status = adc_port_local_init(
        ADC_LOCAL_FAKE, 
        ADC_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_2, 
        ADC_RES_12, 
        adc_port_config); 

    // Check that initialization was skipped due to invalid register addresses 
    LONGS_EQUAL(ADC_INVALID_PTR, port_status); 
}


// ADC port init - call the init function with a valid pointer 
TEST(analog_driver, adc_port_valid_ptr)
{
    ADC_TypeDef ADC_LOCAL_FAKE; 
    ADC_Common_TypeDef ADC_COMMON_LOCAL_FAKE; 

    // Run initialization functions 
    ADC_STATUS port_status = adc_port_local_init(
        &ADC_LOCAL_FAKE, 
        &ADC_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_2, 
        ADC_RES_12, 
        adc_port_config); 

    // Check that initialization completed 
    LONGS_EQUAL(ADC_OK, port_status); 
}


// ADC port init - init ok, register bits successfully cleared 
TEST(analog_driver, adc_port_init_ok_bits_cleared)
{
    ADC_Common_TypeDef ADC_COMMON_LOCAL_FAKE; 
    uint32_t ccr_check = CCR_PRE_HIGH_BIT | CCR_PRE_LOW_BIT; 
    uint32_t cr1_check = CR1_RES_HIGH_BIT | CR1_RES_LOW_BIT | CR1_SCAN_BIT | 
                         CR1_EOC_INT_BIT | CR1_OVERRUN_BIT; 
    uint32_t cr2_check = CR2_EOC_BIT | CR2_CONT_BIT | CR2_DMA_BIT | CR2_DMA_DIS_BIT; 

    // Set the register bits high to make sure they get cleared by the init function 
    ADC_COMMON_LOCAL_FAKE.CCR = HIGH_32BIT; 
    ADC_FAKE.CR1 = HIGH_32BIT; 
    ADC_FAKE.CR2 = HIGH_32BIT; 

    // Run initialization functions 
    adc_port_local_init(
        &ADC_FAKE, 
        &ADC_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_2, 
        ADC_RES_12, 
        adc_port_config); 

    // Common Control Register - prescalar 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_COMMON_LOCAL_FAKE.CCR & ccr_check); 
    // Control Register 1 - resolution, scan, EOC interrupt, overrun interrupt 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.CR1 & cr1_check); 
    // Control Register 2 - EOC select, continuous mode, DMA, DMA disable selection 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.CR2 & cr2_check); 
}


// ADC port init - init ok, register bits successfully set 
TEST(analog_driver, adc_port_init_ok_bits_set)
{
    ADC_Common_TypeDef ADC_COMMON_LOCAL_FAKE; 
    uint32_t ccr_check = CCR_PRE_LOW_BIT; 
    uint32_t cr1_check = CR1_RES_HIGH_BIT | CR1_SCAN_BIT | CR1_EOC_INT_BIT | CR1_OVERRUN_BIT; 
    uint32_t cr2_check = CR2_EOC_BIT | CR2_CONT_BIT | CR2_DMA_BIT | CR2_DMA_DIS_BIT; 

    // Clear the register bits to make sure they get set by the init function 
    ADC_COMMON_LOCAL_FAKE.CCR = CLEAR; 
    ADC_FAKE.CR1 = CLEAR; 
    ADC_FAKE.CR2 = CLEAR; 

    // Enable all the initialization parameters 
    for (uint8_t i = CLEAR; i < NUM_PORT_PARAM_CONFIGS; i++)
        adc_port_config[i] = ADC_PARAM_ENABLE; 

    // Initialization 
    adc_port_local_init(
        &ADC_FAKE, 
        &ADC_COMMON_LOCAL_FAKE, 
        ADC_PCLK2_4, 
        ADC_RES_8, 
        adc_port_config); 

    // Common Control Register - prescalar 
    UNSIGNED_LONGS_EQUAL(ccr_check, ADC_COMMON_LOCAL_FAKE.CCR); 
    // Control Register 1 - resolution, scan, EOC interrupt, overrun interrupt 
    UNSIGNED_LONGS_EQUAL(cr1_check, ADC_FAKE.CR1); 
    // Control Register 2 - EOC select, continuous mode, DMA, DMA disable selection 
    UNSIGNED_LONGS_EQUAL(cr2_check, ADC_FAKE.CR2); 
}

//==================================================

//==================================================
// ADC pin init 

// Note: the 'adc_pin_init' function calls the GPIO driver, however these tests do not 
//       test the GPIO driver functionality. GPIO driver functionality is tested in the 
//       GPIO driver utest so we know it works here. 

// ADC pin init - call the init function with an invalid pointer 
TEST(analog_driver, adc_pin_init_invalid_ptr)
{
    GPIO_TypeDef *GPIO_LOCAL_FAKE = nullptr; 
    ADC_STATUS pin_status = adc_pin_init(
        &ADC_FAKE, 
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
        &ADC_FAKE, 
        &GPIO_LOCAL_FAKE, 
        PIN_0, 
        ADC_CHANNEL_0, 
        ADC_SMP_3); 
    LONGS_EQUAL(ADC_OK, pin_status); 
}


// ADC pin init - init ok, register bits successfully cleared 
TEST(analog_driver, adc_pin_init_ok_bits_cleared)
{
    GPIO_TypeDef GPIO_LOCAL_FAKE; 
    uint32_t smpr2_check = SMPR2_0_LOW_BIT | SMPR2_0_MED_BIT | SMPR2_0_HIGH_BIT; 

    ADC_FAKE.SMPR2 = HIGH_32BIT; 

    adc_pin_init(
        &ADC_FAKE, 
        &GPIO_LOCAL_FAKE, 
        PIN_0, 
        ADC_CHANNEL_0, 
        ADC_SMP_3); 
    
    UNSIGNED_LONGS_EQUAL(ADC_SMP_3, ADC_FAKE.SMPR2 & smpr2_check); 
}


// ADC pin init - init ok, register bits successfully set 
TEST(analog_driver, adc_pin_init_ok_bits_set)
{
    GPIO_TypeDef GPIO_LOCAL_FAKE; 
    uint32_t smpr1_check = SMPR1_17_MED_BIT | SMPR1_17_HIGH_BIT; 
    uint32_t smpr2_check = SMPR2_1_LOW_BIT | SMPR2_1_MED_BIT; 

    ADC_FAKE.SMPR1 = CLEAR; 
    ADC_FAKE.SMPR2 = CLEAR; 

    adc_pin_init(
        &ADC_FAKE, 
        &GPIO_LOCAL_FAKE, 
        PIN_1, 
        ADC_CHANNEL_17, 
        ADC_SMP_144); 
    adc_pin_init(
        &ADC_FAKE, 
        &GPIO_LOCAL_FAKE, 
        PIN_0, 
        ADC_CHANNEL_1, 
        ADC_SMP_56); 
    
    UNSIGNED_LONGS_EQUAL(smpr1_check, ADC_FAKE.SMPR1); 
    UNSIGNED_LONGS_EQUAL(smpr2_check, ADC_FAKE.SMPR2); 
}

//==================================================

//==================================================
// ADC watchdog init 

// ADC watchdog init - register bits successfully cleared 
TEST(analog_driver, adc_wd_init_ok_bits_cleared)
{
    uint32_t cr1_check = CR1_AWDCH_LOW_BITS | CR1_AWDCH_HIGH_BIT | CR1_AWDEN_BIT | 
                         CR1_AWDSGL_BIT | CR1_AWDIE_BIT; 

    ADC_FAKE.CR1 = HIGH_32BIT; 
    ADC_FAKE.HTR = HIGH_32BIT; 
    ADC_FAKE.LTR = HIGH_32BIT; 

    adc_wd_init(
        &ADC_FAKE, 
        ADC_PARAM_DISABLE, 
        ADC_PARAM_DISABLE, 
        ADC_CHANNEL_0, 
        CLEAR, 
        CLEAR, 
        ADC_PARAM_DISABLE); 
    
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.CR1 & cr1_check); 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.HTR); 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.LTR); 
}


// ADC watchdog init - register bits successfully set 
TEST(analog_driver, adc_wd_init_ok_bits_set)
{
    uint32_t cr1_check = CR1_AWDCH_LOW_BITS | CR1_AWDEN_BIT | CR1_AWDSGL_BIT | CR1_AWDIE_BIT; 

    ADC_FAKE.CR1 = CLEAR; 
    ADC_FAKE.HTR = CLEAR; 
    ADC_FAKE.LTR = CLEAR; 

    adc_wd_init(
        &ADC_FAKE, 
        ADC_PARAM_ENABLE, 
        ADC_PARAM_ENABLE, 
        ADC_CHANNEL_15, 
        HIGH_16BIT, 
        HIGH_16BIT, 
        ADC_PARAM_ENABLE); 
    
    UNSIGNED_LONGS_EQUAL(cr1_check, ADC_FAKE.CR1); 
    UNSIGNED_LONGS_EQUAL(ADC_WD_THRESH_MASK, ADC_FAKE.HTR); 
    UNSIGNED_LONGS_EQUAL(ADC_WD_THRESH_MASK, ADC_FAKE.LTR); 
}

//==================================================

//==================================================
// ADC sequence 

// ADC sequence init - register bits successfully cleared 
TEST(analog_driver, adc_seq_init_ok_bits_cleared)
{
    uint32_t sqr1_check = SQR1_SEQ13_LOW_BITS | SQR1_SEQ13_HIGH_BIT; 
    uint32_t sqr2_check = SQR2_SEQ7_LOW_BITS | SQR2_SEQ7_HIGH_BIT; 
    uint32_t sqr3_check = SQR3_SEQ1_LOW_BITS | SQR3_SEQ1_HIGH_BIT; 

    ADC_FAKE.SQR1 = HIGH_32BIT; 
    ADC_FAKE.SQR2 = HIGH_32BIT; 
    ADC_FAKE.SQR3 = HIGH_32BIT; 

    adc_seq(
        &ADC_FAKE, 
        ADC_CHANNEL_0, 
        ADC_SEQ_13); 
    adc_seq(
        &ADC_FAKE, 
        ADC_CHANNEL_0, 
        ADC_SEQ_7); 
    adc_seq(
        &ADC_FAKE, 
        ADC_CHANNEL_0, 
        ADC_SEQ_1); 
    
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.SQR1 & sqr1_check); 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.SQR2 & sqr2_check); 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.SQR3 & sqr3_check); 
}


// ADC sequence init - register bits successfully set 
TEST(analog_driver, adc_seq_init_ok_bits_set)
{
    uint32_t sqr1_check = SQR1_SEQ14_LOW_BITS; 
    uint32_t sqr2_check = SQR2_SEQ8_LOW_BITS; 
    uint32_t sqr3_check = SQR3_SEQ2_LOW_BITS; 

    ADC_FAKE.SQR1 = CLEAR; 
    ADC_FAKE.SQR2 = CLEAR; 
    ADC_FAKE.SQR3 = CLEAR; 

    adc_seq(
        &ADC_FAKE, 
        ADC_CHANNEL_15, 
        ADC_SEQ_14); 
    adc_seq(
        &ADC_FAKE, 
        ADC_CHANNEL_15, 
        ADC_SEQ_8); 
    adc_seq(
        &ADC_FAKE, 
        ADC_CHANNEL_15, 
        ADC_SEQ_2); 
    
    UNSIGNED_LONGS_EQUAL(sqr1_check, ADC_FAKE.SQR1); 
    UNSIGNED_LONGS_EQUAL(sqr2_check, ADC_FAKE.SQR2); 
    UNSIGNED_LONGS_EQUAL(sqr3_check, ADC_FAKE.SQR3); 
}

//==================================================

//==================================================
// ADC sequence length 

// ADC sequence length init - register bits successfully cleared 
TEST(analog_driver, adc_seq_len_init_ok_bits_cleared)
{
    uint32_t sqr1_check = SQR1_L_BITS; 

    ADC_FAKE.SQR1 = HIGH_32BIT; 

    adc_seq_len_set(
        &ADC_FAKE, 
        ADC_SEQ_1); 
    
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.SQR1 & sqr1_check); 
}


// ADC sequence length init - register bits successfully set 
TEST(analog_driver, adc_seq_len_init_ok_bits_set)
{
    uint32_t sqr1_check = SQR1_L_BITS; 

    ADC_FAKE.SQR1 = CLEAR; 

    adc_seq_len_set(
        &ADC_FAKE, 
        ADC_SEQ_16); 
    
    UNSIGNED_LONGS_EQUAL(sqr1_check, ADC_FAKE.SQR1); 
}

//==================================================

//==================================================
// ADC on 

// ADC on - register bit successfully set 
TEST(analog_driver, adc_on_bit_set)
{
    uint32_t cr2_check = CR2_ADON_BIT; 

    ADC_FAKE.CR2 = CLEAR; 

    adc_on(&ADC_FAKE); 
    
    UNSIGNED_LONGS_EQUAL(cr2_check, ADC_FAKE.CR2); 
}

//==================================================

//==================================================
// ADC off 

// ADC off - register bit successfully set 
TEST(analog_driver, adc_on_bit_cleared)
{
    uint32_t cr2_check = CR2_ADON_BIT; 

    ADC_FAKE.CR2 = HIGH_32BIT; 

    adc_off(&ADC_FAKE); 
    
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.CR2 & cr2_check); 
}

//==================================================

//==================================================
// ADC start 

// Note: This function sets the start bit in the CR2 register and then waits for the 
//       hardware to set the start bit in the status register (SR) before returning. 
//       However, before setting the start bit in CR2, the function must clear the status 
//       register so the code doesn't prematurely read the status register start bit as 
//       true. Since this function depends on a hardware action and we can't preemptively 
//       set the status register to the value we want, there is no easy way to test that 
//       the status register start bit gets read properly. As a result, this functionality 
//       is copied here and tested locally instead of in the driver. 

// ADC start - register bits successfully changed 
TEST(analog_driver, adc_start_bits_changed)
{
    // Check that 'adc_start' clears the status register, the start bit in CR2 gets set, 
    // the function returns a timeout status, and the status register start bit logic 
    // from the driver works as expected (tested locally - see note above). 

    uint32_t sr_check = SR_STRT_BIT; 
    uint32_t cr2_check = CR2_SWSTART_BIT; 

    ADC_FAKE.SR = HIGH_32BIT; 
    ADC_FAKE.CR2 = CLEAR; 

    ADC_STATUS start_check = adc_start(&ADC_FAKE); 
    
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.SR); 
    UNSIGNED_LONGS_EQUAL(cr2_check, ADC_FAKE.CR2); 
    UNSIGNED_LONGS_EQUAL(sr_check, (uint32_t)(SET_BIT << SHIFT_4)); 
    LONGS_EQUAL(ADC_TIMEOUT, start_check); 
}

//==================================================

//==================================================
// ADC read single (no DMA) 

// This function depends on 'adc_start' so refer to the note on that above. This 
// function also depends on 'adc_eoc_wait' which comes after 'adc_start' in this function 
// which means it cannot be reached due to the dependency in 'adc_start'. This prevents 
// the data register function from being tested. However, the data register function just 
// returns the register value so we can be confident in it's functionality without testing 
// it. 

// ADC read single - register bits successfully changed 
TEST(analog_driver, adc_read_single_bits_changed)
{
    // Check that 'adc_read_single' clears the SQR registers and sets only a sequence 
    // of length 1 and the channel number in sequence slot 1. Also check that the function 
    // returns zero and that the logic of 'adc_eoc_wait' works (tested locally - see note 
    // above). 

    uint32_t sqr3_check = SQR3_SEQ1_LOW_BITS; 
    uint32_t sr_check = SR_EOC_BIT; 

    ADC_FAKE.SQR1 = HIGH_32BIT; 
    ADC_FAKE.SQR2 = HIGH_32BIT; 
    ADC_FAKE.SQR3 = HIGH_32BIT; 

    ADC_STATUS read_single_check = adc_read_single(
        &ADC_FAKE, 
        ADC_CHANNEL_15); 

    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.SQR1); 
    UNSIGNED_LONGS_EQUAL(CLEAR, ADC_FAKE.SQR2); 
    UNSIGNED_LONGS_EQUAL(sqr3_check, ADC_FAKE.SQR3); 
    UNSIGNED_LONGS_EQUAL(sr_check, (uint32_t)(SET_BIT << SHIFT_1)); 
    LONGS_EQUAL(NONE, read_single_check); 
}

//==================================================

//==================================================
// ADC scan sequence (no DMA) 

// This function depends on 'adc_start' which is called before anything else happens. Due 
// to the dependency in 'adc_start', this function cannot be easily tested without hardware. 

//==================================================

//=======================================================================================
