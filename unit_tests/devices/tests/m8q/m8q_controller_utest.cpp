//=======================================================================================
// Notes 

// Test plan 
// - Init state test 
// - Read state test 
// - Idle state test 
// - Low power state test 
// - Low power enter state test 
// - Low power exit state test 
// - Fault state test 
// - Reset state test 

// Trigger different statuses and check faults and states 

//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

extern "C"
{
	// Add your C-only include files here 
    #include "m8q_driver.h"
    #include "m8q_controller.h"
    #include "m8q_config_test.h" 
    #include "i2c_comm.h"
    #include "i2c_comm_mock.h"
    #include "gpio_driver.h"
    #include "gpio_driver_mock.h"
    #include "timers.h"
    #include "timers_mock.h"
}

//=======================================================================================


//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(m8q_controller_test)
{
    // Global test group variables 
    // TIM_TypeDef M8Q_TIMER; 
    GPIO_TypeDef GPIO_LP; 
    GPIO_TypeDef GPIO_TX; 

    // Constructor 
    void setup()
    {
        // Set the clock frequency to be read by the M8Q controller init 

        // Controller init 
        // m8q_controller_init(&M8Q_TIMER); 

        // Driver init 
        m8q_pwr_pin_init_dev(&GPIO_LP, PIN_0); 
        m8q_txr_pin_init_dev(&GPIO_TX, PIN_1); 
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

// M8Q controller - initialization and init state 
TEST(m8q_controller_test, m8q_controller_init_and_init_state)
{
    TIM_TypeDef *TIMER_FAKE_LOCAL_0 = nullptr; 
    TIM_TypeDef TIMER_FAKE_LOCAL_1; 

    // Invalid timer pointer in init function --> triggers fault state 
    m8q_controller_init(TIMER_FAKE_LOCAL_0); 
    m8q_controller(); 
    LONGS_EQUAL(M8Q_FAULT_STATE, m8q_get_state()); 

    // Valid timer pointer, init function runs --> default to init state then read state 
    m8q_controller_init(&TIMER_FAKE_LOCAL_1); 
    m8q_controller(); 
    LONGS_EQUAL(M8Q_INIT_STATE, m8q_get_state()); 
    m8q_controller(); 
    LONGS_EQUAL(M8Q_READ_STATE, m8q_get_state()); 

    // Valid timer pointer, init function runs, set idle state bit --> default to init 
    // state then go to idle state 
    m8q_controller_init(&TIMER_FAKE_LOCAL_1); 
    m8q_set_idle_flag(); 
    m8q_controller(); 
    m8q_controller(); 
    LONGS_EQUAL(M8Q_IDLE_STATE, m8q_get_state()); 
}


// M8Q controller - read state 
TEST(m8q_controller_test, m8q_controller_read_state)
{
    TIM_TypeDef TIMER_FAKE_LOCAL; 
    I2C_TypeDef I2C_FAKE_LOCAL; 

    //==================================================
    // Read data while in the read state - check for data update 

    uint8_t msg0_len = 111; 
    uint8_t stream_len[] = { 0x00, 0x6F }; 

    const char device_msg0[] = 
        "$PUBX,00,081350.00,4717.113210,N,11433.915187,W,546.589,G3,2.1,2.0,0.007,77.52," 
        "0.007,,0.92,1.19,0.77,9,0,0*5F\r\n"; 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_msg0, msg0_len, I2C_MOCK_INDEX_1); 

    m8q_init_dev(&I2C_FAKE_LOCAL, &m8q_config_pkt[0][0], CLEAR, CLEAR, CLEAR); 
    m8q_controller_init(&TIMER_FAKE_LOCAL); 

    DOUBLES_EQUAL(CLEAR, m8q_get_position_lat_dev(), 0.000001); 

    m8q_controller(); 
    m8q_controller(); 
    gpio_mock_set_read_state(GPIO_HIGH); 
    m8q_controller(); 

    DOUBLES_EQUAL(47.285220, m8q_get_position_lat_dev(), 0.000001); 
    
    //==================================================

    //==================================================
    // Go to the fault state from the read state 

    i2c_mock_init(I2C_MOCK_TIMEOUT_ENABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    m8q_controller_init(&TIMER_FAKE_LOCAL); 
    
    m8q_controller(); 
    gpio_mock_set_read_state(GPIO_HIGH); 
    m8q_controller(); 
    m8q_controller(); 
    
    LONGS_EQUAL(M8Q_FAULT_STATE, m8q_get_state()); 
    
    //==================================================

    //==================================================
    // Go to the low power enter state from the read state 

    gpio_mock_set_read_state(GPIO_LOW); 
    m8q_controller_init(&TIMER_FAKE_LOCAL); 

    m8q_controller(); 
    m8q_controller(); 
    m8q_set_low_pwr_flag(); 
    m8q_controller(); 

    LONGS_EQUAL(M8Q_LOW_PWR_ENTER_STATE, m8q_get_state()); 
    
    //==================================================

    //==================================================
    // Go to the idle state from the read state 

    gpio_mock_set_read_state(GPIO_LOW); 
    m8q_controller_init(&TIMER_FAKE_LOCAL); 

    m8q_controller(); 
    m8q_controller(); 
    m8q_set_idle_flag(); 
    m8q_controller(); 
    
    LONGS_EQUAL(M8Q_IDLE_STATE, m8q_get_state()); 
    
    //==================================================
}


// M8Q controller - idle (and low power) state 
TEST(m8q_controller_test, m8q_controller_idle_state)
{
    // 
}


// M8Q controller - low power enter state 
TEST(m8q_controller_test, m8q_controller_lp_eneter_state)
{
    // 
}


// M8Q controller - low power exit state 
TEST(m8q_controller_test, m8q_controller_lp_exit_state)
{
    // 
}


// M8Q controller - reset state 
TEST(m8q_controller_test, m8q_controller_reset_state)
{
    // 
}


// M8Q controller - fault state 
TEST(m8q_controller_test, m8q_controller_fault_state)
{
    // 

    // Read the fault code 
}

//=======================================================================================
