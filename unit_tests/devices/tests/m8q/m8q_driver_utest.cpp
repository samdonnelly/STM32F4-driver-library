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
    #include "i2c_comm.h" 
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
    // Global test group variables 
    I2C_TypeDef I2C_FAKE; 

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


// M8Q device initialization - invalid PUBX NMEA config message check 
TEST(m8q_driver, m8q_init_pubx_nmea_config_invalid_msg_check)
{
    // All of the below sample config messages are invalid except for the last one. 
    // Messages are sent one at a time to verify that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message A: Incorrect ID 
    // - Message B: Invalid formatter 
    // - Message C: Missing a comma separator between the address and data fields 
    // - Message D: Invalid message character 
    // - Message E: Incorrect number of fields for the specified message 
    // - Message F: No message termination character ('*') 
    // - Message G: The message termination character ('*') is not the last character 
    // - Message H: None - fails due to a forced I2C timeout 

    // Sample PUBX NMEA message 
    const char config_msg_a[] = "$PUBC,40,GLL,1,0,0,0,0,0*"; 
    const char config_msg_b[] = "$PUBX,01,GLL,1,0,0,0,0,0*"; 
    const char config_msg_c[] = "$PUBX,40GLL,1,0,0,0,0,0*"; 
    const char config_msg_d[] = "$PUBX,40,GLL,1,0,0,0,&,0*"; 
    const char config_msg_e[] = "$PUBX,40,GLL,1,0,0,0,0*"; 
    const char config_msg_f[] = "$PUBX,40,GLL,1,0,0,0,0,0"; 
    const char config_msg_g[] = "$PUBX,40,GLL,1,0,0,0,0,0*0"; 
    const char config_msg_h[] = "$PUBX,40,GLL,1,0,0,0,0,0*"; 

    // Initialize the mock I2C driver to time out 
    i2c_mock_init(1); 

    M8Q_STATUS init_check_a = m8q_init_dev(&I2C_FAKE, config_msg_a, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_b = m8q_init_dev(&I2C_FAKE, config_msg_b, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_c = m8q_init_dev(&I2C_FAKE, config_msg_c, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_d = m8q_init_dev(&I2C_FAKE, config_msg_d, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_e = m8q_init_dev(&I2C_FAKE, config_msg_e, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_f = m8q_init_dev(&I2C_FAKE, config_msg_f, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_g = m8q_init_dev(&I2C_FAKE, config_msg_g, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_h = m8q_init_dev(&I2C_FAKE, config_msg_h, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_a); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_b); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_c); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_d); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_e); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_f); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_g); 
    LONGS_EQUAL(M8Q_WRITE_FAULT, init_check_h); 
}


// M8Q device initialization - valid PUBX NMEA config message check 
TEST(m8q_driver, m8q_init_pubx_nmea_config_valid_msg_check)
{
    M8Q_STATUS init_check; 

    // Variables to store the drivers formatted config message 
    char config_msg_check[30]; 
    uint8_t config_msg_check_len = CLEAR; 

    // Complete PUBX NMEA message and its length to compare against 
    const char config_msg_compare[] = "$PUBX,40,GLL,1,0,0,0,0,0*5D\r\n"; 
    uint8_t config_msg_compare_len = strlen(config_msg_compare); 

    // Correctly formatted sample PUBX NMEA config message 
    const char config_msg[] = "$PUBX,40,GLL,1,0,0,0,0,0*"; 

    // Initialize the mock I2C driver to not time out, run the init and retrieve the 
    // driver formatted message that gets sent to the device. Check that the driver 
    // message and its length are correct. 
    i2c_mock_init(0); 
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    i2c_mock_get_write_data(config_msg_check, &config_msg_check_len); 

    LONGS_EQUAL(M8Q_OK, init_check); 
    LONGS_EQUAL(config_msg_compare_len, config_msg_check_len); 
    STRCMP_EQUAL(config_msg_compare, config_msg_check); 
}


// M8Q device initialization - invalid standard NMEA config message check 
TEST(m8q_driver, m8q_init_std_nmea_config_invalid_msg_check)
{
    // All of the below sample config messages are invalid except for the last one. 
    // Messages are sent one at a time to verify that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message A: Incorrect ID  
    // - Message B: Invalid formatter 
    // - Message C: Missing a comma separator between the address and data fields 
    // - Message D: Invalid message character 
    // - Message E: Incorrect number of fields for the specified message 
    // - Message F: No message termination character ('*') 
    // - Message G: The message termination character ('*') is not the last character 
    // - Message H: None - fails due to a forced I2C timeout 

    // Sample standard NMEA message 
    const char config_msg_a[] = "$GCGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 
    const char config_msg_b[] = "$GNGRZ,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 
    const char config_msg_c[] = "$GNGRS104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 
    const char config_msg_d[] = "$GNGRS,104148.00,1,2.6,+2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 
    const char config_msg_e[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,1,,,1,1*"; 
    const char config_msg_f[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1,"; 
    const char config_msg_g[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*N"; 
    const char config_msg_h[] = "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 

    // Initialize the mock I2C driver to time out 
    i2c_mock_init(1); 

    M8Q_STATUS init_check_a = m8q_init_dev(&I2C_FAKE, config_msg_a, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_b = m8q_init_dev(&I2C_FAKE, config_msg_b, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_c = m8q_init_dev(&I2C_FAKE, config_msg_c, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_d = m8q_init_dev(&I2C_FAKE, config_msg_d, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_e = m8q_init_dev(&I2C_FAKE, config_msg_e, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_f = m8q_init_dev(&I2C_FAKE, config_msg_f, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_g = m8q_init_dev(&I2C_FAKE, config_msg_g, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_h = m8q_init_dev(&I2C_FAKE, config_msg_h, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_a); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_b); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_c); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_d); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_e); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_f); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_g); 
    LONGS_EQUAL(M8Q_WRITE_FAULT, init_check_h); 
}


// M8Q device initialization - valid standard NMEA config message check 
TEST(m8q_driver, m8q_init_std_nmea_config_valid_msg_check)
{
    M8Q_STATUS init_check; 

    // Variables to store the drivers formatted config message 
    char config_msg_check[70]; 
    uint8_t config_msg_check_len = CLEAR; 

    // Complete PUBX NMEA message and its length to compare against 
    const char config_msg_compare[] = 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*52\r\n"; 
    uint8_t config_msg_compare_len = strlen(config_msg_compare); 

    // Correctly formatted sample PUBX NMEA config message 
    const char config_msg[] = 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*"; 

    // Initialize the mock I2C driver to not time out, run the init and retrieve the 
    // driver formatted message that gets sent to the device. Check that the driver 
    // message and its length are correct. 
    i2c_mock_init(0); 
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    i2c_mock_get_write_data(config_msg_check, &config_msg_check_len); 

    LONGS_EQUAL(M8Q_OK, init_check); 
    LONGS_EQUAL(config_msg_compare_len, config_msg_check_len); 
    STRCMP_EQUAL(config_msg_compare, config_msg_check); 
}


// M8Q device initialization - invalid and valid UBX config message check 
TEST(m8q_driver, m8q_init_ubx_config_msg_check)
{
    // All of the below sample config messages are invalid except for the last one. 
    // Messages are sent one at a time to verify that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message A: Incorrect ID 
    // - Message B: Invalid formatter 
    // - Message C: Missing a comma separator between the address and data fields 
    // - Message D: Missing message fields - cut off at the ID 
    // - Message E: ID field too short 
    // - Message F: Invalid UBX message character in the ID 
    // - Message G: ID field too long 
    // - Message H: Missing message fields - Cut off at the payload length 
    // - Message I: Payload length field too short 
    // - Message J: Invalid UBX message character in the payload length 
    // - Message K: Payload length field too long 
    // - Message 
    // - Message Z: None 

    // Sample UBX message 
    const char config_msg_a[] = "B563,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_b[] = "B562,22,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_c[] = "B562,0600,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_d[] = "B562,06,0"; 
    const char config_msg_e[] = "B562,06,0,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_f[] = "B562,06,-0,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_g[] = "B562,06,001,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_h[] = "B562,06,00,14"; 
    const char config_msg_i[] = "B562,06,00,14,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_j[] = "B562,06,00,14.0,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 
    const char config_msg_k[] = "B562,06,00,14009,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 

    const char config_msg_z[] = "B562,06,00,1441,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 

    M8Q_STATUS init_check_a = m8q_init_dev(&I2C_FAKE, config_msg_a, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_b = m8q_init_dev(&I2C_FAKE, config_msg_b, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_c = m8q_init_dev(&I2C_FAKE, config_msg_c, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_d = m8q_init_dev(&I2C_FAKE, config_msg_d, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_e = m8q_init_dev(&I2C_FAKE, config_msg_e, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_f = m8q_init_dev(&I2C_FAKE, config_msg_f, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_g = m8q_init_dev(&I2C_FAKE, config_msg_g, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_h = m8q_init_dev(&I2C_FAKE, config_msg_h, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_i = m8q_init_dev(&I2C_FAKE, config_msg_i, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_j = m8q_init_dev(&I2C_FAKE, config_msg_j, 1, M8Q_CONFIG_MAX_MSG_LEN); 
    M8Q_STATUS init_check_k = m8q_init_dev(&I2C_FAKE, config_msg_k, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    M8Q_STATUS init_check_z = m8q_init_dev(&I2C_FAKE, config_msg_z, 1, M8Q_CONFIG_MAX_MSG_LEN); 

    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_a); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_b); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_c); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_d); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_e); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_f); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_g); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_h); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_i); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_j); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_check_k); 

    LONGS_EQUAL(M8Q_OK, init_check_z); 
}


// // M8Q device initialization - init ok, all config messages valid 
// TEST(m8q_driver, m8q_init_valid_config)
// {
//     M8Q_STATUS init_check = m8q_init_dev(
//         &I2C_FAKE, 
//         &m8q_config_pkt[0][0], 
//         M8Q_CONFIG_NUM_MSG, 
//         M8Q_CONFIG_MAX_MSG_LEN); 

//     LONGS_EQUAL(M8Q_OK, init_check); 
// }

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
