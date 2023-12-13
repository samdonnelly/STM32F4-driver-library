//=======================================================================================
// Notes 
//=======================================================================================


//=======================================================================================
// Includes 

#include "CppUTest/TestHarness.h"

extern "C"
{
	// Add your C-only include files here 
    #include "m8q_driver.h" 
    #include "m8q_config_test.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 

#define CONFIG_TEST_MSG_NUM 3 

//=======================================================================================


//=======================================================================================
// Test group 

TEST_GROUP(m8q_driver)
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

//=======================================================================================


//=======================================================================================
// Helper functions 

// Config message comparison 
void m8q_test_config_compare(
    const char *config_msgs, 
    const char *config_msgs_test, 
    uint8_t msg_num, 
    uint8_t *msg_num_test, 
    uint8_t max_msg_size, 
    uint8_t *msg_status)
{
    if ((config_msgs == nullptr) || 
        (config_msgs_test == nullptr) || 
        (msg_num_test == nullptr) || 
        (msg_status == nullptr))
    {
        return; 
    }

    char config_msg, config_msg_test; 

    for (uint8_t i = CLEAR; i < msg_num; i++)
    {
        for (uint8_t j = CLEAR; j < max_msg_size; j++)
        {
            config_msg = *(config_msgs_test + i*max_msg_size + j); 
            config_msg_test = *(config_msgs + msg_num_test[i]*max_msg_size + j); 
            
            if (config_msg != config_msg_test)
            {
                msg_status[i] = SET_BIT; 
                break; 
            }
            if (config_msg == AST_CHAR)
            {
                break; 
            }
        }
    }
}

//=======================================================================================


//=======================================================================================
// Tests 

//==================================================
// Device init 

// M8Q device initialization - invalid pointer 
TEST(m8q_driver, m8q_init_invalid_ptr)
{
    I2C_TypeDef *I2C_LOCAL_FAKE = nullptr; 

    M8Q_STATUS ptr_status = m8q_init_dev(
        I2C_LOCAL_FAKE, 
        &m8q_config_pkt[0][0], 
        M8Q_CONFIG_NUM_MSG, 
        M8Q_CONFIG_MAX_MSG_LEN); 

    UNSIGNED_LONGS_EQUAL(M8Q_INVALID_PTR, ptr_status); 
}


// M8Q device initialization - config message ok 
TEST(m8q_driver, m8q_init_config_msg_ok)
{
    uint8_t msg_num[CONFIG_TEST_MSG_NUM] = {0, 9, 11}; 
    uint8_t msg_status[CONFIG_TEST_MSG_NUM] = {0, 0, 0}; 

    // Config messages from 'm8q_config_pkt' 
    const char config_msgs[CONFIG_TEST_MSG_NUM][M8Q_CONFIG_MAX_MSG_LEN] = 
    {
        // Message 0 
        "$PUBX,40,GGA,0,0,0,0,0,0*", 
        // Message 9 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        // Message 11 
        "B562,06,09,0C00,00000000,FFFFFFFF,00000000*" 
    };

    m8q_test_config_compare(
        &m8q_config_pkt[0][0], 
        &config_msgs[0][0], 
        CONFIG_TEST_MSG_NUM, 
        msg_num, 
        M8Q_CONFIG_MAX_MSG_LEN, 
        msg_status); 

    LONGS_EQUAL(CLEAR, msg_status[0]); 
    LONGS_EQUAL(CLEAR, msg_status[1]); 
    LONGS_EQUAL(CLEAR, msg_status[2]); 
}


// M8Q device initialization - invalid and valid PUBX NMEA config message check 
TEST(m8q_driver, m8q_init_pubx_nmea_config_msg_check)
{
    I2C_TypeDef I2C_LOCAL_FAKE; 

    // All sample messages except the last one are invalid. Messages are sent one at a 
    // time to check that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message A: Incorrect ID 
    // - Message B: Invalid formatter 
    // - Message C: Invalid message character 
    // - Message D: Incorrect number of fields for the specified message 
    // - Message E: No message termination character ('*') 
    // - Message F: The message termination character ('*') is not the last character 
    // - Message G: None 

    // Sample PUBX NMEA message 
    char config_msg_a[] = "$PUBC,40,GGA,0,0,0,0,0,0*"; 
    char config_msg_b[] = "$PUBX,01,GGA,0,0,0,0,0,0*"; 
    char config_msg_c[] = "$PUBX,40,GGA,0,0,0,0,&,0*"; 
    char config_msg_d[] = "$PUBX,40,GGA,0,0,0,0,0*"; 
    char config_msg_e[] = "$PUBX,40,GGA,0,0,0,0,0,0"; 
    char config_msg_f[] = "$PUBX,40,GGA,0,0,0,0,0,0*0"; 
    char config_msg_g[] = "$PUBX,40,GGA,0,0,0,0,0,0*"; 

    M8Q_STATUS init_check_a = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_a, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_b = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_b, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_c = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_c, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_d = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_d, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_e = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_e, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_f = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_f, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_g = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_g, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_a); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_b); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_c); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_d); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_e); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_f); 
    LONGS_EQUAL(M8Q_OK, init_check_g); 
}


// M8Q device initialization - invalid and valid standard NMEA config message check 
TEST(m8q_driver, m8q_init_std_nmea_config_msg_check)
{
    I2C_TypeDef I2C_LOCAL_FAKE; 

    // All sample messages except the last one are invalid. Messages are sent one at a 
    // time to check that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message A: Incorrect ID  
    // - Message B: Invalid formatter 
    // - Message C: Invalid message character 
    // - Message D: Incorrect number of fields for the specified message 
    // - Message E: No message termination character ('*') 
    // - Message F: The message termination character ('*') is not the last character 
    // - Message G: None 

    // Sample standard NMEA message 
    char config_msg_a[] = "$GCGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 
    char config_msg_b[] = "$GNGRZ,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 
    char config_msg_c[] = "$GNGRS,104148.00,1,2.6,+2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 
    char config_msg_d[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,1,,,1,1*"; 
    char config_msg_e[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1,"; 
    char config_msg_f[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*N"; 
    char config_msg_g[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 

    M8Q_STATUS init_check_a = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_a, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_b = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_b, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_c = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_c, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_d = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_d, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_e = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_e, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_f = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_f, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_g = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_g, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_a); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_b); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_c); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_d); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_e); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_f); 
    LONGS_EQUAL(M8Q_OK, init_check_g); 
}


// M8Q device initialization - invalid and valid UBX config message check 
TEST(m8q_driver, m8q_init_ubx_config_msg_check)
{
    I2C_TypeDef I2C_LOCAL_FAKE; 

    // All sample messages except the last one are invalid. Messages are sent one at a 
    // time to check that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message A: Incorrect ID 
    // - Message B: Invalid formatter 
    // - Message C: Invalid message character 
    // - Message D: Incorrect number of fields for the specified message 

    // Sample UBX message 
    char config_msg_a[] = "B563,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    char config_msg_b[] = "B562,22,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    // char config_msg_c[] = ""; 
    // char config_msg_d[] = ""; 
    char config_msg_e[] = "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 

    M8Q_STATUS init_check_a = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_a, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_b = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_b, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    // M8Q_STATUS init_check_c = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_c, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    // M8Q_STATUS init_check_d = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_d, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_e = m8q_init_dev(&I2C_LOCAL_FAKE, config_msg_e, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_a); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_b); 
    // LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_c); 
    // LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_d); 
    LONGS_EQUAL(M8Q_OK, init_check_e); 
}


// M8Q device initialization - init ok, all config messages valid 
TEST(m8q_driver, m8q_init_valid_config)
{
    I2C_TypeDef I2C_LOCAL_FAKE; 

    M8Q_STATUS init_check = m8q_init_dev(
        &I2C_LOCAL_FAKE, 
        &m8q_config_pkt[0][0], 
        M8Q_CONFIG_NUM_MSG, 
        M8Q_CONFIG_MAX_MSG_LEN); 

    LONGS_EQUAL(M8Q_OK, init_check); 
}

//==================================================

//==================================================
// Pin init 

// These init functions contain calls to the GPIO driver. That driver is not in the 
// scope of this test group so it is not tested in detail. 

// M8Q pin initialization - invalid pointer 
TEST(m8q_driver, m8q_pin_init_invalid_ptr)
{
    GPIO_TypeDef *GPIO_LOCAL_FAKE = nullptr; 

    M8Q_STATUS low_pwr_init_check = m8q_pwr_pin_init_dev(GPIO_LOCAL_FAKE, PIN_0); 
    M8Q_STATUS txr_init_check = m8q_txr_pin_init_dev(GPIO_LOCAL_FAKE, PIN_1); 

    LONGS_EQUAL(M8Q_INVALID_PTR, low_pwr_init_check); 
    LONGS_EQUAL(M8Q_INVALID_PTR, txr_init_check); 
}


// M8Q pin initialization - init ok 
TEST(m8q_driver, m8q_pin_init_init_ok)
{
    GPIO_TypeDef GPIO_LOCAL_FAKE; 

    M8Q_STATUS low_pwr_init_check = m8q_pwr_pin_init_dev(&GPIO_LOCAL_FAKE, PIN_0); 
    M8Q_STATUS txr_init_check = m8q_txr_pin_init_dev(&GPIO_LOCAL_FAKE, PIN_1); 

    LONGS_EQUAL(M8Q_OK, low_pwr_init_check); 
    LONGS_EQUAL(M8Q_OK, txr_init_check); 
}

//==================================================

//==================================================
// User functions test 
//==================================================

//=======================================================================================
