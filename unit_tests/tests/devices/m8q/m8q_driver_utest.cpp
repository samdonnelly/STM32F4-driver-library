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
    I2C_TypeDef *I2C_LOCAL_FAKE = NULL; 

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


// M8Q device initialization - no init, invalid config messages 
TEST(m8q_driver, m8q_init_invalid_config)
{
    I2C_TypeDef I2C_LOCAL_FAKE; 

    // In the following message samples, the first two are invalid and the third is valid. 
    // Messages are sent one at a time to check that message checks are done correctly. 

    // Sample PUBX NMEA message 
    const char config_msg0[] = "$PUBC,40,GGA,0,0,0,0,0,0*"; 
    const char config_msg1[] = "$PUBX,01,GGA,0,0,0,0,0,0*"; 
    const char config_msg2[] = "$PUBX,40,GGA,0,0,0,0,0,0*"; 
    // Sample standard NMEA message 
    const char config_msg3[] = "$GCGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*52"; 
    const char config_msg4[] = "$GNGRZ,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*52"; 
    const char config_msg5[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*52"; 
    // // Sample UBX message 
    const char config_msg6[] = "B563,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg7[] = "B562,22,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg8[] = "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 

    M8Q_STATUS init_check0 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg0, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check1 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg1, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check2 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg2, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check3 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg3, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check4 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg4, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check5 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg5, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check6 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg6, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check7 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg7, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check8 = m8q_init_dev(
                                &I2C_LOCAL_FAKE, config_msg8, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    // LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check0); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check1); 
    LONGS_EQUAL(M8Q_OK, init_check2); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check3); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check4); 
    LONGS_EQUAL(M8Q_OK, init_check5); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check6); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check7); 
    LONGS_EQUAL(M8Q_OK, init_check8); 
}


// // M8Q device initialization - no init, invalid config messages 
// TEST(m8q_driver, m8q_init_invalid_config)
// {
//     I2C_TypeDef I2C_LOCAL_FAKE; 

//     // Config messages from 'm8q_config_pkt' but the ID characters (first character of 
//     // the NMEA message and first four characters of the UBX messages) are purposely 
//     // written incorrectly. 
//     const char config_msgs[CONFIG_TEST_MSG_NUM][M8Q_CONFIG_MAX_MSG_LEN] = 
//     {
//         // Message 0 
//         "$PUBX,40,GGA,0,0,0,0,0,0*", 
//         // Message 9 
//         "B5,62,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
//         // Message 11 - SYNC CHAR 2 set incorrectly to trip the init function 
//         "B5,63,06,09,0C00,00000000,FFFFFFFF,00000000*" 
//     };

//     M8Q_STATUS init_check = m8q_init_dev(
//         &I2C_LOCAL_FAKE, 
//         &config_msgs[0][0], 
//         CONFIG_TEST_MSG_NUM, 
//         M8Q_CONFIG_MAX_MSG_LEN); 

//     LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check); 
// }


// // M8Q device initialization - init ok, valid pointer and config messages 
// TEST(m8q_driver, m8q_init_valid_config)
// {
//     I2C_TypeDef I2C_LOCAL_FAKE; 

//     M8Q_STATUS init_check = m8q_init_dev(
//         &I2C_LOCAL_FAKE, 
//         &m8q_config_pkt[0][0], 
//         CONFIG_TEST_MSG_NUM, 
//         M8Q_CONFIG_MAX_MSG_LEN); 

//     LONGS_EQUAL(M8Q_OK, init_check); 
// }

//==================================================

//=======================================================================================
