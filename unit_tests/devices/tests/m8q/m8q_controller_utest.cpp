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

//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

extern "C"
{
	// Add your C-only include files here 
    #include "m8q_driver.h"
    #include "m8q_controller.h"
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

    // Constructor 
    void setup()
    {
        // Set the clock frequency to be read by the M8Q controller init 

        // Controller init 
        // m8q_controller_init(&M8Q_TIMER); 
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

// 
TEST(m8q_controller_test, test0)
{
    TIM_TypeDef *M8Q_TIMER_LOCAL = nullptr; 
    M8Q_STATE state; 

    m8q_controller_init(M8Q_TIMER_LOCAL); 
    m8q_controller(); 
    state = m8q_get_state(); 
    
    LONGS_EQUAL(M8Q_FAULT_STATE, state); 
}


// // 
// TEST(m8q_controller_test, test1)
// {
//     // 
//     M8Q_STATE state = m8q_get_state(); 

//     LONGS_EQUAL(M8Q_INIT_STATE, state); 

//     m8q_controller(); 

//     state = m8q_get_state(); 
//     LONGS_EQUAL(M8Q_INIT_STATE, state); 
// }

//=======================================================================================
