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

#define NUM_CONFIG_TEST_MSGS 3 
#define NUM_NMEA_TEST_MSGS 8 
#define NUM_UBX_TEST_MSGS 18 
#define NO_DATA_BUFF_LIMIT 0 

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
        M8Q_CONFIG_MAX_MSG_LEN, 
        NO_DATA_BUFF_LIMIT); 

    UNSIGNED_LONGS_EQUAL(M8Q_INVALID_PTR, ptr_status); 
}


// M8Q device initialization - config message ok 
TEST(m8q_driver, m8q_init_config_msg_ok)
{
    uint8_t msg_num[NUM_CONFIG_TEST_MSGS] = {0, 9, 11}; 
    uint8_t msg_status[NUM_CONFIG_TEST_MSGS] = {0, 0, 0}; 

    // Config messages from 'm8q_config_pkt' 
    const char config_msgs[NUM_CONFIG_TEST_MSGS][M8Q_CONFIG_MAX_MSG_LEN] = 
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
        NUM_CONFIG_TEST_MSGS, 
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
    M8Q_STATUS init_checks[NUM_NMEA_TEST_MSGS]; 

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
    const char config_msgs[NUM_NMEA_TEST_MSGS][M8Q_CONFIG_MAX_MSG_LEN] = 
    {
        "$PUBC,40,GLL,1,0,0,0,0,0*", 
        "$PUBX,01,GLL,1,0,0,0,0,0*", 
        "$PUBX,40GLL,1,0,0,0,0,0*", 
        "$PUBX,40,GLL,1,0,0,0,&,0*", 
        "$PUBX,40,GLL,1,0,0,0,0*", 
        "$PUBX,40,GLL,1,0,0,0,0,0", 
        "$PUBX,40,GLL,1,0,0,0,0,0*0", 
        "$PUBX,40,GLL,1,0,0,0,0,0*" 
    }; 

    // Initialize the mock I2C driver to time out 
    i2c_mock_init(1); 

    for (uint8_t i = CLEAR; i < NUM_NMEA_TEST_MSGS; i++)
    {
        init_checks[i] = m8q_init_dev(&I2C_FAKE, config_msgs[i], 1, 
                                      M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    }

    for (uint8_t i = CLEAR; i < (NUM_NMEA_TEST_MSGS-1); i++)
    {
        LONGS_EQUAL(M8Q_INVALID_CONFIG, init_checks[i]); 
    }

    LONGS_EQUAL(M8Q_WRITE_FAULT, init_checks[NUM_NMEA_TEST_MSGS-1]); 
}


// M8Q device initialization - valid PUBX NMEA config message check 
TEST(m8q_driver, m8q_init_pubx_nmea_config_valid_msg_check)
{
    M8Q_STATUS init_check; 

    // Variables to store the drivers formatted config message 
    char config_msg_check[30]; 
    memset((void *)config_msg_check, CLEAR, sizeof(config_msg_check)); 
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
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, 
                              M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    i2c_mock_get_write_data((void *)config_msg_check, &config_msg_check_len); 

    LONGS_EQUAL(M8Q_OK, init_check); 
    LONGS_EQUAL(config_msg_compare_len, config_msg_check_len); 
    STRCMP_EQUAL(config_msg_compare, config_msg_check); 
}


// M8Q device initialization - invalid standard NMEA config message check 
TEST(m8q_driver, m8q_init_std_nmea_config_invalid_msg_check)
{
    M8Q_STATUS init_checks[NUM_NMEA_TEST_MSGS]; 

    // All of the below sample config messages are invalid except for the last one. 
    // Messages are sent one at a time to verify that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message 0: Incorrect ID  
    // - Message 1: Invalid formatter 
    // - Message 2: Missing a comma separator between the address and data fields 
    // - Message 3: Invalid message character 
    // - Message 4: Incorrect number of fields for the specified message 
    // - Message 5: No message termination character ('*') 
    // - Message 6: The message termination character ('*') is not the last character 
    // - Message 7: None - fails due to a forced I2C timeout 

    // Sample standard NMEA message 
    const char config_msgs[NUM_NMEA_TEST_MSGS][M8Q_CONFIG_MAX_MSG_LEN] = 
    {
        "$GCGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*", 
        "$GNGRZ,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*", 
        "$GNGRS104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*", 
        "$GNGRS,104148.00,1,2.6,+2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*", 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,1,,,1,1*", 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1,", 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*N", 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*" 
    }; 

    // Initialize the mock I2C driver to time out 
    i2c_mock_init(1); 

    for (uint8_t i = CLEAR; i < NUM_NMEA_TEST_MSGS; i++)
    {
        init_checks[i] = m8q_init_dev(&I2C_FAKE, config_msgs[i], 1, 
                                      M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    }

    for (uint8_t i = CLEAR; i < (NUM_NMEA_TEST_MSGS-1); i++)
    {
        LONGS_EQUAL(M8Q_INVALID_CONFIG, init_checks[i]); 
    }

    LONGS_EQUAL(M8Q_WRITE_FAULT, init_checks[NUM_NMEA_TEST_MSGS-1]); 
}


// M8Q device initialization - valid standard NMEA config message check 
TEST(m8q_driver, m8q_init_std_nmea_config_valid_msg_check)
{
    M8Q_STATUS init_check; 

    // Variables to store the drivers formatted config message 
    char config_msg_check[70]; 
    memset((void *)config_msg_check, CLEAR, sizeof(config_msg_check)); 
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
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, 
                              M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    i2c_mock_get_write_data((void *)config_msg_check, &config_msg_check_len); 

    LONGS_EQUAL(M8Q_OK, init_check); 
    LONGS_EQUAL(config_msg_compare_len, config_msg_check_len); 
    STRCMP_EQUAL(config_msg_compare, config_msg_check); 
}


// M8Q device initialization - invalid and valid UBX config message check 
TEST(m8q_driver, m8q_init_ubx_config_invalid_msg_check)
{
    M8Q_STATUS init_checks[NUM_UBX_TEST_MSGS]; 

    // All of the below sample config messages are invalid except for the last one. 
    // Messages are sent one at a time to verify that message checks are done correctly. 
    // The messages have the following errors: 
    // - Message 0: Incorrect ID 
    // - Message 1: Invalid formatter 
    // - Message 2: Missing a comma separator between the address and data fields 
    // - Message 3: Missing message fields - cut off at the ID 
    // - Message 4: ID field too short 
    // - Message 5: Invalid UBX message character in the ID 
    // - Message 6: ID field too long 
    // - Message 7: Missing message fields - Cut off at the payload length 
    // - Message 8: Payload length field too short 
    // - Message 9: Invalid UBX message character in the payload length 
    // - Message 10: Payload length field too long 
    // - Message 11: No termination character seen before the end of the message 
    // - Message 12: Not enough payload bytes 
    // - Message 13: Uneven number of payload characters 
    // - Message 14: Too many payload bytes 
    // - Message 15: Invalid payload character 
    // - Message 16: No termination character at the end of the message - comma instead 
    // - Message 17: None - fails due to a forced I2C timeout 

    // Sample UBX message 
    const char config_msgs[NUM_UBX_TEST_MSGS][M8Q_CONFIG_MAX_MSG_LEN] = 
    {
        "B563,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,22,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,0600,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,0", 
        "B562,06,0,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,-0,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,001,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,00,14", 
        "B562,06,00,14,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,00,14.0,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,00,14009,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000", 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000*", 
        "B562,06,00,1400,01,00,00001,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,000022*", 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,00%0,0000,0000,0000*", 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000,", 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*" 
    }; 

    // Initialize the mock I2C driver to time out 
    i2c_mock_init(1); 

    for (uint8_t i = CLEAR; i < NUM_UBX_TEST_MSGS; i++)
    {
        init_checks[i] = m8q_init_dev(&I2C_FAKE, config_msgs[i], 1, 
                                      M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    }

    for (uint8_t i = CLEAR; i < (NUM_UBX_TEST_MSGS-1); i++)
    {
        LONGS_EQUAL(M8Q_INVALID_CONFIG, init_checks[i]); 
    }

    LONGS_EQUAL(M8Q_WRITE_FAULT, init_checks[NUM_UBX_TEST_MSGS-1]); 
}


// M8Q device initialization - valid UBX config message check 
TEST(m8q_driver, m8q_init_ubx_config_valid_msg_check)
{
    M8Q_STATUS init_check; 

    // Variables to store the drivers formatted config message 
    uint8_t config_msg_check[30]; 
    memset((void *)config_msg_check, CLEAR, sizeof(config_msg_check)); 
    uint8_t config_msg_check_len = CLEAR; 

    // Complete UBX message and its length to compare against 
    const uint8_t config_msg_compare[] = 
        {181,98,6,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 
    uint8_t config_msg_compare_len = sizeof(config_msg_compare)/sizeof(config_msg_compare[0]); 
    uint8_t config_msg_compare_status = SET_BIT; 

    // Correctly formatted sample UBX config message 
    const char config_msg[] = 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 

    // Initialize the mock I2C driver to not time out, run the init and retrieve the 
    // driver formatted message that gets sent to the device. Check that the driver 
    // message and its length are correct. 
    i2c_mock_init(0); 
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, 
                              M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    i2c_mock_get_write_data((void *)config_msg_check, &config_msg_check_len); 

    LONGS_EQUAL(M8Q_OK, init_check); 
    LONGS_EQUAL(config_msg_compare_len, config_msg_check_len); 

    for (uint8_t i = CLEAR; i < config_msg_compare_len; i++)
    {
        if (config_msg_compare[i] != config_msg_check[i])
        {
            config_msg_compare_status = CLEAR; 
        }
    }
    LONGS_EQUAL(SET_BIT, config_msg_compare_status); 
}


// M8Q device initialization - init ok, all config messages valid 
TEST(m8q_driver, m8q_init_valid_config)
{
    M8Q_STATUS init_check = m8q_init_dev(
        &I2C_FAKE, 
        &m8q_config_pkt[0][0], 
        M8Q_CONFIG_NUM_MSG, 
        M8Q_CONFIG_MAX_MSG_LEN, 
        NO_DATA_BUFF_LIMIT); 

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
// Read and write test 

// M8Q read 
TEST(m8q_driver, m8q_read_test)
{
    // 
}

//==================================================

//==================================================
// User functions test 
//==================================================

//=======================================================================================
