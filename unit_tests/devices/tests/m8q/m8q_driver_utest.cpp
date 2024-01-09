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
    #include "i2c_comm_mock.h"
}

//=======================================================================================


//=======================================================================================
// Macros 

#define NUM_CONFIG_TEST_MSGS 3 
#define NUM_NMEA_TEST_MSGS 8 
#define NUM_UBX_TEST_MSGS 19 
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
        // Initialize the mock I2C driver to default settings - no timeout and no data 
        // buffer increment. 
        i2c_mock_init(
            I2C_MOCK_TIMEOUT_DISABLE, 
            I2C_MOCK_INC_MODE_DISABLE, 
            I2C_MOCK_INC_MODE_DISABLE); 

        // Initialize driver but don't send/check any messages 
        m8q_init_dev(&I2C_FAKE, &m8q_config_pkt[0][0], CLEAR, CLEAR, CLEAR); 
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
// Device init / write test 

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
    i2c_mock_init(I2C_MOCK_TIMEOUT_ENABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_DISABLE); 

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

    // Run the init and retrieve the driver formatted message that gets sent to the device. 
    // Check that the driver message and its length are correct. 
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, 
                              M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    i2c_mock_get_write_data((void *)config_msg_check, &config_msg_check_len, I2C_MOCK_INDEX_0); 

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
    i2c_mock_init(I2C_MOCK_TIMEOUT_ENABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_DISABLE); 

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

    // Run the init and retrieve the driver formatted message that gets sent to the device. 
    // Check that the driver message and its length are correct. 
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, 
                              M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    i2c_mock_get_write_data((void *)config_msg_check, &config_msg_check_len, I2C_MOCK_INDEX_0); 

    LONGS_EQUAL(M8Q_OK, init_check); 
    LONGS_EQUAL(config_msg_compare_len, config_msg_check_len); 
    STRCMP_EQUAL(config_msg_compare, config_msg_check); 
}


// M8Q device initialization - invalid and valid UBX config message check 
TEST(m8q_driver, m8q_init_ubx_config_invalid_msg_check)
{
    M8Q_STATUS init_checks[NUM_UBX_TEST_MSGS]; 

    // Variables to configure the NAK message sent by the device 
    uint8_t msg_len = 10; 
    uint8_t stream_len[] = { 0x00, 0x0A }; 
    uint8_t device_stream[msg_len]; 

    //==================================================
    // Messages 

    // All of the below sample config messages are invalid except for the last two. 
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
    // - Message 18: None - fails due to a forced NAK response (CFG message) 

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
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*", 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*" 
    }; 

    // NAK message to be read in the init function in the final CFG message check 
    const uint8_t device_msg[] = {181,98,5,0,2,0,6,1,15,56}; 
    
    //==================================================

    //==================================================
    // Run the init function for all but the last CFG message 

    // Initialize the mock I2C driver to time out 
    i2c_mock_init(I2C_MOCK_TIMEOUT_ENABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_DISABLE); 

    for (uint8_t i = CLEAR; i < (NUM_UBX_TEST_MSGS - 1); i++)
    {
        init_checks[i] = m8q_init_dev(&I2C_FAKE, config_msgs[i], 1, 
                                      M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    }
    
    //==================================================

    //==================================================
    // Disable I2C timeout and set the NAK message to read for the final CFG message 

    // Make sure a NAK message is returned for the CFG message 
    memcpy((void *)device_stream, (void *)device_msg, msg_len); 
    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data(stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data(device_stream, msg_len, I2C_MOCK_INDEX_1); 

    init_checks[NUM_UBX_TEST_MSGS - 1] = m8q_init_dev(
                                            &I2C_FAKE, config_msgs[NUM_UBX_TEST_MSGS - 1], 
                                            1, 
                                            M8Q_CONFIG_MAX_MSG_LEN, 
                                            NO_DATA_BUFF_LIMIT); 
    
    //==================================================

    //==================================================
    // Check the results 

    for (uint8_t i = CLEAR; i < (NUM_UBX_TEST_MSGS - 2); i++)
    {
        LONGS_EQUAL(M8Q_INVALID_CONFIG, init_checks[i]); 
    }

    LONGS_EQUAL(M8Q_WRITE_FAULT, init_checks[NUM_UBX_TEST_MSGS - 2]); 
    LONGS_EQUAL(M8Q_INVALID_CONFIG, init_checks[NUM_UBX_TEST_MSGS - 1]); 
    
    //==================================================
}


// M8Q device initialization - valid UBX config message check 
TEST(m8q_driver, m8q_init_ubx_config_valid_msg_check)
{
    //==================================================
    // Local variables 

    M8Q_STATUS init_check; 

    // Variables to store the drivers formatted config message 
    uint8_t config_msg_check[30]; 
    memset((void *)config_msg_check, CLEAR, sizeof(config_msg_check)); 
    uint8_t config_msg_check_len = CLEAR; 

    // Variables to configure the ACK message sent by the device 
    uint8_t msg_len = 10; 
    uint8_t stream_len[] = { 0x00, 0x0A }; 
    uint8_t device_stream[msg_len]; 
    
    //==================================================

    //==================================================
    // Messages 

    // Complete UBX message and its length to compare against 
    const uint8_t config_msg_compare[] = 
        {181,98,6,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 
    uint8_t config_msg_compare_len = sizeof(config_msg_compare)/sizeof(config_msg_compare[0]); 
    uint8_t config_msg_compare_status = SET_BIT; 

    // Correctly formatted sample UBX config message 
    const char config_msg[] = 
        "B562,06,00,1400,01,00,0000,C0080000,80250000,0000,0000,0000,0000*"; 

    // ACK message to be read in the init function 
    const uint8_t device_msg[] = {181,98,5,1,2,0,6,1,15,56}; 
    
    //==================================================

    // Make sure an ACK message is returned for the CFG message 
    memcpy((void *)device_stream, (void *)device_msg, msg_len); 
    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_ENABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data(stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data(device_stream, msg_len, I2C_MOCK_INDEX_1); 

    // Run the init and retrieve the driver formatted message that gets sent to the device. 
    // Check that the driver message and its length are correct. 
    init_check = m8q_init_dev(&I2C_FAKE, config_msg, 1, 
                              M8Q_CONFIG_MAX_MSG_LEN, NO_DATA_BUFF_LIMIT); 
    i2c_mock_get_write_data((void *)config_msg_check, &config_msg_check_len, I2C_MOCK_INDEX_0); 

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
    //==================================================
    // Set up the ACK messages to be read after the CFG messages are sent 

    uint8_t msg_len = 10; 
    uint8_t stream_len[] = { 0x00, 0x0A }; 
    uint8_t device_stream[msg_len]; 

    const uint8_t device_msg[] = {181,98,5,1,2,0,6,1,15,56}; 

    memcpy((void *)device_stream, (void *)device_msg, msg_len); 
    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 

    // There are 6 CFG messages (not total messages) in the config file 
    for (uint8_t i = CLEAR; i < 6; i++)
    {
        i2c_mock_set_read_data(stream_len, BYTE_2, 2*i); 
        i2c_mock_set_read_data(device_stream, msg_len, 2*i + 1); 
    }
    
    //==================================================

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
// Pin init test 

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
// Read / user function test 

// M8Q read - stream length is zero (no data available) 
TEST(m8q_driver, m8q_read_stream_length_zero)
{
    M8Q_STATUS read_status; 
    uint8_t stream_len[] = { 0x00, 0x00 }; 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data(stream_len, BYTE_2, I2C_MOCK_INDEX_0); 

    read_status = m8q_read_data_dev(); 

    LONGS_EQUAL(M8Q_NO_DATA_AVAILABLE, read_status); 
}


// M8Q read - data in the stream is larger than the threshold 
TEST(m8q_driver, m8q_read_stream_too_large)
{
    M8Q_STATUS read_status; 
    uint8_t stream_len[] = { 0x01, 0x04 }; 

    // Set the data buffer threshold 
    m8q_init_dev(&I2C_FAKE, &m8q_config_pkt[0][0], CLEAR, CLEAR, 200); 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data(stream_len, BYTE_2, I2C_MOCK_INDEX_0); 

    read_status = m8q_read_data_dev(); 

    LONGS_EQUAL(M8Q_DATA_BUFF_OVERFLOW, read_status); 
}


// M8Q read - I2C timeout 
TEST(m8q_driver, m8q_read_i2c_timeout)
{
    M8Q_STATUS read_status; 
    uint8_t stream_len[] = { 0x01, 0x04 }; 
    // Data to write 

    i2c_mock_init(I2C_MOCK_TIMEOUT_ENABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data(stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    // Set the stream data to read 

    read_status = m8q_read_data_dev(); 

    LONGS_EQUAL(M8Q_READ_FAULT, read_status); 
}


// M8Q read - Unknown stream message - Single message stream 
TEST(m8q_driver, m8q_read_unknown_single_msg)
{
    M8Q_STATUS read_status_0, read_status_1; 

    uint8_t stream_len_0[] = { 0x00, 0x6F }; 
    uint8_t stream_len_1[] = { 0x00, 0x1C }; 
    uint16_t msg_len_0 = (stream_len_0[0] << SHIFT_8) | stream_len_0[1]; 
    uint16_t msg_len_1 = (stream_len_1[0] << SHIFT_8) | stream_len_1[1]; 

    // One NMEA message and one UBX message is checked 
    const char device_msg_0[] = "$PUBC,00,081350.00,4717.113210,N,00833.915187,E,546.589," 
                                "G3,2.1,2.0,0.007,77.52,0.007,,0.92,1.19,0.77,9,0,0*5F"; 
    const uint8_t device_msg_1[] = 
        {181,98,3,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len_0, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_msg_0, msg_len_0, I2C_MOCK_INDEX_1); 
    read_status_0 = m8q_read_data_dev(); 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len_1, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_msg_1, msg_len_1, I2C_MOCK_INDEX_1); 
    read_status_1 = m8q_read_data_dev(); 

    LONGS_EQUAL(M8Q_UNKNOWN_DATA, read_status_0); 
    LONGS_EQUAL(M8Q_UNKNOWN_DATA, read_status_1); 
}


// M8Q read - Known stream message - Single message stream 
TEST(m8q_driver, m8q_read_known_single_msg)
{
    M8Q_STATUS read_status_0, read_status_1; 

    uint8_t stream_len_0[] = { 0x00, 0x6F }; 
    uint8_t stream_len_1[] = { 0x00, 0x1C }; 
    uint16_t msg_len_0 = (stream_len_0[0] << SHIFT_8) | stream_len_0[1]; 
    uint16_t msg_len_1 = (stream_len_1[0] << SHIFT_8) | stream_len_1[1]; 
    
    // One NMEA message and one UBX message is checked 
    const char device_msg_0[] = "$PUBX,00,081350.00,4717.113210,N,00833.915187,E,546.589," 
                                "G3,2.1,2.0,0.007,77.52,0.007,,0.92,1.19,0.77,9,0,0*5F\r\n"; 
    const uint8_t device_msg_1[] = 
        {181,98,6,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len_0, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_msg_0, msg_len_0, I2C_MOCK_INDEX_1); 
    read_status_0 = m8q_read_data_dev(); 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len_1, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_msg_1, msg_len_1, I2C_MOCK_INDEX_1); 
    read_status_1 = m8q_read_data_dev(); 

    LONGS_EQUAL(M8Q_OK, read_status_0); 
    LONGS_EQUAL(M8Q_OK, read_status_1); 
}


// M8Q read - Unknown stream message - Multiple message stream 
TEST(m8q_driver, m8q_read_unknown_multi_msg)
{
    M8Q_STATUS read_status; 

    uint8_t msg0_len = 67; 
    uint8_t msg1_len = 28; 
    uint8_t msg2_len = 28; 
    uint8_t msg3_len = 71; 
    uint8_t msg4_len = 29; 
    uint8_t stream_len[] = { 0x00, 0xDF }; 
    uint16_t msg_len = (stream_len[0] << SHIFT_8) | stream_len[1]; 

    uint8_t device_stream[msg_len]; 

    // The stream contains multiple NMEA and UBX messages. The last message is not valid. 
    const char device_msg0[] = 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*52\r\n"; 
    const uint8_t device_msg1[] = 
        {181,98,6,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 
    const uint8_t device_msg2[] = 
        {181,98,2,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 
    const char device_msg3[] = 
        "$PUBX,04,073731.00,091202,113851.00,1196,15D,1930035,-2660.664,43,*3C\r\n"; 
    const char device_msg4[] = 
        "$PUBX,45,GLL,1,0,0,0,0,0*5D\r\n"; 

    memcpy((void *)&device_stream[0], (void *)device_msg0, msg0_len); 
    memcpy((void *)&device_stream[msg0_len], (void *)device_msg1, msg1_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len], (void *)device_msg2, msg2_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len + msg2_len], 
           (void *)device_msg3, msg3_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len + msg2_len + msg3_len], 
           (void *)device_msg4, msg4_len); 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_stream, msg_len, I2C_MOCK_INDEX_1); 

    read_status = m8q_read_data_dev(); 

    LONGS_EQUAL(M8Q_UNKNOWN_DATA, read_status); 
}


// M8Q read - Known stream message - Multiple message stream 
TEST(m8q_driver, m8q_read_known_multi_msg)
{
    M8Q_STATUS read_status; 

    uint8_t msg0_len = 67; 
    uint8_t msg1_len = 28; 
    uint8_t msg2_len = 28; 
    uint8_t msg3_len = 71; 
    uint8_t msg4_len = 29; 
    uint8_t stream_len[] = { 0x00, 0xDF }; 
    uint16_t msg_len = (stream_len[0] << SHIFT_8) | stream_len[1]; 

    uint8_t device_stream[msg_len]; 

    // The stream contains multiple NMEA and UBX messages 
    const char device_msg0[] = 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*52\r\n"; 
    const uint8_t device_msg1[] = 
        {181,98,6,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 
    const uint8_t device_msg2[] = 
        {181,98,2,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 
    const char device_msg3[] = 
        "$PUBX,04,073731.00,091202,113851.00,1196,15D,1930035,-2660.664,43,*3C\r\n"; 
    const char device_msg4[] = 
        "$PUBX,40,GLL,1,0,0,0,0,0*5D\r\n"; 

    memcpy((void *)&device_stream[0], (void *)device_msg0, msg0_len); 
    memcpy((void *)&device_stream[msg0_len], (void *)device_msg1, msg1_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len], (void *)device_msg2, msg2_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len + msg2_len], 
           (void *)device_msg3, msg3_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len + msg2_len + msg3_len], 
           (void *)device_msg4, msg4_len); 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_stream, msg_len, I2C_MOCK_INDEX_1); 

    read_status = m8q_read_data_dev(); 

    LONGS_EQUAL(M8Q_OK, read_status); 
}


// M8Q read - Message data record update 
TEST(m8q_driver, m8q_read_msg_record_update)
{
    M8Q_STATUS read_status; 
    uint8_t lat_str[BYTE_11]; 
    uint8_t lon_str[BYTE_12]; 
    uint8_t utc_time[BYTE_10]; 
    uint8_t utc_date[BYTE_7]; 

    memset((void *)lat_str, CLEAR, sizeof(lat_str)); 
    memset((void *)lon_str, CLEAR, sizeof(lon_str)); 
    memset((void *)utc_time, CLEAR, sizeof(utc_time)); 
    memset((void *)utc_date, CLEAR, sizeof(utc_date)); 

    uint8_t msg0_len = 111, msg1_len = 67, msg2_len = 28, msg3_len = 10, msg4_len = 71; 
    uint8_t stream_len[] = { 0x01, 0x1F }; 
    uint16_t msg_len = (stream_len[0] << SHIFT_8) | stream_len[1]; 
    uint8_t device_stream[msg_len]; 

    // The stream contains multiple NMEA and UBX messages. Some messages are stored in the 
    // driver data record and others are discarded. 
    const char device_msg0[] = 
        "$PUBX,00,081350.00,4717.113210,N,11433.915187,W,546.589,G3,2.1,2.0,0.007,77.52," 
        "0.007,,0.92,1.19,0.77,9,0,0*5F\r\n"; 
    const char device_msg1[] = 
        "$GNGRS,104148.00,1,2.6,2.2,-1.6,-1.1,-1.7,-1.5,5.8,1.7,,,,,1,1*52\r\n"; 
    const uint8_t device_msg2[] = 
        {181,98,6,0,20,0,1,0,0,0,192,8,0,0,128,37,0,0,0,0,0,0,0,0,0,0,136,107}; 
    const uint8_t device_msg3[] = 
        {181,98,5,1,2,0,6,1,15,56}; 
    const char device_msg4[] = 
        "$PUBX,04,073731.00,091202,113851.00,1196,15D,1930035,-2660.664,43,*3C\r\n"; 

    memcpy((void *)&device_stream[0], (void *)device_msg0, msg0_len); 
    memcpy((void *)&device_stream[msg0_len], (void *)device_msg1, msg1_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len], (void *)device_msg2, msg2_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len + msg2_len], 
           (void *)device_msg3, msg3_len); 
    memcpy((void *)&device_stream[msg0_len + msg1_len + msg2_len + msg3_len], 
           (void *)device_msg4, msg4_len); 

    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_stream, msg_len, I2C_MOCK_INDEX_1); 

    //==================================================
    // Read data record - check for no data 

    DOUBLES_EQUAL(CLEAR, m8q_get_position_lat_dev(), 0.000001); 
    DOUBLES_EQUAL(CLEAR, m8q_get_position_lon_dev(), 0.000001); 

    LONGS_EQUAL(M8Q_DATA_BUFF_OVERFLOW, m8q_get_position_lat_str_dev(lat_str, BYTE_5)); 
    LONGS_EQUAL(M8Q_DATA_BUFF_OVERFLOW, m8q_get_position_lon_str_dev(lon_str, BYTE_5)); 
    LONGS_EQUAL(M8Q_OK, m8q_get_position_lat_str_dev(lat_str, BYTE_11)); 
    LONGS_EQUAL(M8Q_OK, m8q_get_position_lon_str_dev(lon_str, BYTE_12)); 
    STRCMP_EQUAL("0000000000", (char *)lat_str); 
    STRCMP_EQUAL("00000000000", (char *)lon_str); 

    LONGS_EQUAL(CLEAR, m8q_get_position_NS_dev()); 
    LONGS_EQUAL(CLEAR, m8q_get_position_EW_dev()); 
    
    LONGS_EQUAL(CLEAR, m8q_get_position_navstat_dev()); 
    LONGS_EQUAL(FALSE, m8q_get_position_navstat_lock_dev()); 

    LONGS_EQUAL(M8Q_DATA_BUFF_OVERFLOW, m8q_get_time_utc_time_dev(utc_time, BYTE_5)); 
    LONGS_EQUAL(M8Q_DATA_BUFF_OVERFLOW, m8q_get_time_utc_date_dev(utc_date, BYTE_5)); 
    LONGS_EQUAL(M8Q_OK, m8q_get_time_utc_time_dev(utc_time, BYTE_10)); 
    LONGS_EQUAL(M8Q_OK, m8q_get_time_utc_date_dev(utc_date, BYTE_7)); 
    STRCMP_EQUAL("", (char *)utc_time); 
    STRCMP_EQUAL("", (char *)utc_date); 
    
    //==================================================

    // Read data from the device so the data record gets updated 
    read_status = m8q_read_data_dev(); 

    //==================================================
    // Read data record - check for populated data 

    LONGS_EQUAL(M8Q_OK, read_status); 

    DOUBLES_EQUAL(47.285220, m8q_get_position_lat_dev(), 0.000001); 
    DOUBLES_EQUAL(-114.565253, m8q_get_position_lon_dev(), 0.000001); 

    m8q_get_position_lat_str_dev(lat_str, BYTE_11); 
    m8q_get_position_lon_str_dev(lon_str, BYTE_12); 
    STRCMP_EQUAL("4717.11321", (char *)lat_str); 
    STRCMP_EQUAL("11433.91518", (char *)lon_str); 

    LONGS_EQUAL(N_UP_CHAR, m8q_get_position_NS_dev()); 
    LONGS_EQUAL(W_UP_CHAR, m8q_get_position_EW_dev()); 
    
    LONGS_EQUAL(M8Q_NAVSTAT_G3, m8q_get_position_navstat_dev()); 
    LONGS_EQUAL(TRUE, m8q_get_position_navstat_lock_dev()); 

    m8q_get_time_utc_time_dev(utc_time, BYTE_10); 
    m8q_get_time_utc_date_dev(utc_date, BYTE_7); 
    STRCMP_EQUAL("073731.00", (char *)utc_time); 
    STRCMP_EQUAL("091202", (char *)utc_date); 
    
    //==================================================
}


// M8Q read - Get whole data stream 
TEST(m8q_driver, m8q_read_get_data_stream)
{
    // Read and get the whole data stream from the device. No driver data record is 
    // updated. The buffer to store the stream must be at least 1 byte larger than the 
    // stream size so the buffer can be terminated with a null character. 

    M8Q_STATUS read_status; 

    uint8_t msg0_len = 111, msg1_len = 71; 
    uint8_t stream_len[] = { 0x00, 0xB6 }; 
    uint16_t msg_len = (stream_len[0] << SHIFT_8) | stream_len[1]; 
    uint8_t device_stream[msg_len]; 
    uint8_t stream_buffer[msg_len + BYTE_1]; 

    const char device_msg0[] = 
        "$PUBX,00,081350.00,4717.113210,N,00833.915187,E,546.589,G3,2.1,2.0,0.007,77.52," 
        "0.007,,0.92,1.19,0.77,9,0,0*5F\r\n"; 
    const char device_msg1[] = 
        "$PUBX,04,073731.00,091202,113851.00,1196,15D,1930035,-2660.664,43,*3C\r\n"; 

    memcpy((void *)&device_stream[0], (void *)device_msg0, msg0_len); 
    memcpy((void *)&device_stream[msg0_len], (void *)device_msg1, msg1_len); 

    // Check that a buffer that is too small won't be used 
    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    read_status = m8q_read_ds_dev(stream_buffer, msg_len); 
    LONGS_EQUAL(M8Q_DATA_BUFF_OVERFLOW, read_status); 

    // Check that the stream was read 
    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_stream, msg_len, I2C_MOCK_INDEX_1); 
    read_status = m8q_read_ds_dev(stream_buffer, msg_len + BYTE_1); 
    LONGS_EQUAL(M8Q_OK, read_status); 
    STRCMP_EQUAL((char *)device_stream, (char *)stream_buffer); 
}


// M8Q read - Flush data stream when it's larger than the buffer size 
TEST(m8q_driver, m8q_read_flush_stream)
{
    M8Q_STATUS read_status_0, read_status_1; 

    uint8_t msg0_len = 111; 
    uint8_t msg1_len = 71; 
    uint8_t max_buff_size = 80; 
    uint8_t stream_len_0[] = { 0x00, 0xB6 }; 
    uint8_t stream_len_1[] = { 0x00, 0x00 }; 
    uint16_t msg_len = (stream_len_0[0] << SHIFT_8) | stream_len_0[1]; 
    uint8_t device_stream[msg_len]; 

    // These messages don't actually get used by the mock i2c driver. They are here for 
    // consistency with the other tests. When the stream is larger than the buffer size the 
    // M8Q driver flushes the data stream by reading the whole stream but not storing any 
    // data to therefore not exceed any buffer limits. 
    const char device_msg0[] = 
        "$PUBX,00,081350.00,4717.113210,N,00833.915187,E,546.589,G3,2.1,2.0,0.007,77.52," 
        "0.007,,0.92,1.19,0.77,9,0,0*5F\r\n"; 
    const char device_msg1[] = 
        "$PUBX,04,073731.00,091202,113851.00,1196,15D,1930035,-2660.664,43,*3C\r\n"; 

    memcpy((void *)&device_stream[0], (void *)device_msg0, msg0_len); 
    memcpy((void *)&device_stream[msg0_len], (void *)device_msg1, msg1_len); 

    // Set the data buffer threshold 
    m8q_init_dev(&I2C_FAKE, &m8q_config_pkt[0][0], CLEAR, CLEAR, max_buff_size); 

    // The stream will only be read to 'max_buff_size' on each read when being flushed so 
    // the whole stream has to be spread out over multiple calls to the i2c (mock) driver. 
    i2c_mock_init(I2C_MOCK_TIMEOUT_DISABLE, I2C_MOCK_INC_MODE_DISABLE, I2C_MOCK_INC_MODE_ENABLE); 
    i2c_mock_set_read_data((void *)stream_len_0, BYTE_2, I2C_MOCK_INDEX_0); 
    i2c_mock_set_read_data((void *)device_stream, msg_len, I2C_MOCK_INDEX_1); 
    i2c_mock_set_read_data((void *)stream_len_1, BYTE_2, I2C_MOCK_INDEX_2); 

    read_status_0 = m8q_read_data_dev(); 
    read_status_1 = m8q_read_data_dev(); 

    // Getting M8Q_NO_DATA_AVAILABLE as the status of the second read attempt confirms that 
    // the data stream flush/clear function works. An M8Q_READ_FAULT status would indicate 
    // the mock i2c driver was read too many times, and not getting either this or an 
    // M8Q_NO_DATA_AVAILABLE status would mean the mock i2c driver was not read enough time. 
    LONGS_EQUAL(M8Q_DATA_BUFF_OVERFLOW, read_status_0); 
    LONGS_EQUAL(M8Q_NO_DATA_AVAILABLE, read_status_1); 
}

//==================================================

//=======================================================================================
