/**
 * @file m8q_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS driver implementation 
 * 
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//=======================================================================================
// Includes 

#include "m8q_driver.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Message identification 
#define MSG_ID_CHARS 8          // Number of characters used to identify message types 

// NMEA message format 
#define NMEA_START 0x24           // '$' --> NMEA protocol start character 
#define NMEA_PUBX_FORMAT_NUM 5    // Number of NMEA PUBX formatters 
#define NMEA_PUBX_FORMAT_LEN 3    // Maximum length of an NMEA PUBX formatter 
#define NMEA_STD_ID_NUM 5         // Number of standard NMEA message IDs 
#define NMEA_STD_ID_LEN 3         // Maximum length of a standard NMEA message ID 
#define NMEA_STD_FORMAT_NUM 19    // Number of standard NMEA formatters 
#define NMEA_STD_FORMAT_LEN 4     // Maximum length of a standard NMEA formatter 

// UBX message format 
#define UBX_SYNC1 0xB5            // 0xB5 --> UBX protocol SYNC CHAR 1 
#define UBX_SYNC1_HI 0x42         // 'B' --> SYNC CHAR 1 (0xB5) high nibble character 
#define UBX_SYNC1_LO 0x35         // '5' --> SYNC CHAR 1 (0xB5) low nibble character 
#define UBX_SYNC2_HI 0x36         // '6' --> SYNC CHAR 2 (0x62) high nibble character 
#define UBX_SYNC2_LO 0x32         // '2' --> SYNC CHAR 2 (0x62) low nibble character 
#define UBX_CLASS_NUM 14          // Number of UBX classes 
#define UBX_CLASS_LEN 3           // Maximum length ofa UBX class 

//=======================================================================================


//=======================================================================================
// Enums 

//==================================================
// new 

/**
 * @brief M8Q message type 
 * 
 * @details 
 */
typedef enum {
    M8Q_MSG_INVALID, 
    M8Q_MSG_NMEA, 
    M8Q_MSG_UBX, 
    M8Q_MSG_RTCM 
} m8q_msg_type_t; 

//==================================================

/**
 * @brief M8Q valid read indicator 
 * 
 * @details Used to define a valid or invalid message read in the m8q_read function. m8q_read 
 *          returns the result indicating the type of message read, if any. 
 * 
 * @see m8q_read
 */
typedef enum {
    M8Q_READ_INVALID, 
    M8Q_READ_NMEA, 
    M8Q_READ_UBX
} m8q_read_status_t; 


/**
 * @brief M8Q NMEA POSITION message data fields 
 * 
 * @details List of all data fields in the POSITION message. This enum allows for indexing 
 *          of the POSITION fields for retreival data in getters. 
 */
typedef enum {
    M8Q_POS_TIME, 
    M8Q_POS_LAT, 
    M8Q_POS_NS, 
    M8Q_POS_LON, 
    M8Q_POS_EW, 
    M8Q_POS_ALTREF, 
    M8Q_POS_NAVSTAT, 
    M8Q_POS_HACC, 
    M8Q_POS_VACC, 
    M8Q_POS_SOG, 
    M8Q_POS_COG, 
    M8Q_POS_VVEL, 
    M8Q_POS_DIFFAGE, 
    M8Q_POS_HDOP, 
    M8Q_POS_VDOP, 
    M8Q_POS_TDOP, 
    M8Q_POS_NUMSVS, 
    M8Q_POS_RES, 
    M8Q_POS_DR 
} m8q_pos_fields_t; 


/**
 * @brief M8Q NMEA TIME message data fields 
 * 
 * @details List of all data fields in the TIME message. This enum allows for indexing 
 *          of the TIME fields for retreival data in getters. 
 */
typedef enum {
    M8Q_TIME_TIME, 
    M8Q_TIME_DATE, 
    M8Q_TIME_UTCTOW, 
    M8Q_TIME_UTCWK, 
    M8Q_TIME_LEAPSEC, 
    M8Q_TIME_CLKBIAS, 
    M8Q_TIME_CLKDRIFT, 
    M8Q_TIME_TPGRAN 
} m8q_time_fields_t; 


/**
 * @brief M8Q driver status codes 
 * 
 * @details 
 *          
 *          Old comments: 
 *          - Codes used to indicate errors during NMEA message processing. These codes help 
 *            with debugging. 
 *          - Codes used to indicate errors during UBX message processing. These codes help 
 *            with debugging. 
 */
typedef enum {
    M8Q_FAULT_NONE,           // No fault 
    M8Q_FAULT_NO_DATA,        // No data available 
    M8Q_FAULT_NMEA_ID,        // Unsupported PUBX message ID 
    M8Q_FAULT_NMEA_FORM,      // Invalid formatting of PUBX message 
    M8Q_FAULT_NMEA_INVALID,   // Only PUBX messages are supported 
    M8Q_FAULT_UBX_SIZE,       // Payload length doesn't match size 
    M8Q_FAULT_UBX_FORM,       // Invalid payload format 
    M8Q_FAULT_UBX_LEN,        // Invalid payload length format 
    M8Q_FAULT_UBX_CONVERT,    // Message conversion failed. Check format 
    M8Q_FAULT_UBX_ID,         // Invalid ID format 
    M8Q_FAULT_UBX_NA,         // Unknown message type 
    M8Q_FAULT_UBX_NAK,        // Message not acknowledged 
    M8Q_FAULT_UBX_RESP        // Response message sent - only used during user config mode 
} m8q_status_codes_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef uint8_t M8Q_MSG_TYPE; 

//=======================================================================================


//=======================================================================================
// Messages 

//==================================================
// NMEA message address 

// Start character of all NMEA messages 
const char nmea_start = NMEA_START; 

// U-Blox defined NMEA message 
const char nmea_pubx_id[] = "PUBX"; 

// NMEA PUBX messages 
const char nmea_pubx_format[NMEA_PUBX_FORMAT_NUM][NMEA_PUBX_FORMAT_LEN] = 
{
    "00",      // POSITION 
    "03",      // SVSTATUS 
    "04",      // TIME 
    "40",      // RATE 
    "41",      // CONFIG 
}; 

// NMEA talker IDs 
const char nmea_std_id[NMEA_STD_ID_NUM][NMEA_STD_ID_LEN] = 
{
    "GP",      // GPS, SBAS, QZSS 
    "GL",      // GLONASS 
    "GA",      // Galileo 
    "GB",      // BeiDou 
    "GN",      // Any combination of GNSS 
}; 

// NMEA standard messages 
const char nmea_std_format[NMEA_STD_FORMAT_NUM][NMEA_STD_FORMAT_LEN] = 
{
    "DTM",     // Datum reference 
    "GBQ",     // Poll a standard message (Talker ID GB) 
    "GBS",     // GNSS satellite fault detection 
    "GGA",     // Global positioning system fix data 
    "GLL",     // Latitude and longitude, with time of position fix and status 
    "GLQ",     // Poll a standard message (Talker ID GL) 
    "GNQ",     // Poll a standard message (Talker ID GN) 
    "GNS",     // GNSS fix data 
    "GPQ",     // Poll a standard message (Talker ID GP) 
    "GRS",     // GNSS range residuals 
    "GSA",     // GNSS DOP and active satellites 
    "GST",     // GNSS pseudorange error statistics 
    "GSV",     // GNSS satellites in view 
    "RMC",     // Recommended minimum data 
    "THS",     // True heading and status 
    "TXT",     // Text transmission 
    "VLW",     // Dual ground/water distance 
    "VTG",     // Course over ground and ground speed 
    "ZDA",     // Time and data 
}; 

//==================================================

//==================================================
// UBX sync characters and class 

const char ubx_sync[] = "B562"; 

const char ubx_class[UBX_CLASS_NUM][UBX_CLASS_LEN] = 
{
    "01",   // NAV 
    "02",   // RXM 
    "04",   // INF 
    "05",   // ACK 
    "06",   // CFG 
    "09",   // UPD 
    "0A",   // MON 
    "0B",   // AID 
    "0D",   // TIM 
    "10",   // ESF 
    "13",   // MGA 
    "21",   // LOG 
    "27",   // SEC 
    "28"    // HNR 
}; 

//==================================================

//==================================================
// Stored messages 
// NMEA POSITION message fields  
typedef struct m8q_nmea_pos_s 
{
    uint8_t time    [BYTE_9];     // UTC time 
    uint8_t lat     [BYTE_10];    // Latitude 
    uint8_t NS      [BYTE_1];     // North/South indicator 
    uint8_t lon     [BYTE_11];    // Longitude 
    uint8_t EW      [BYTE_1];     // East/West indicator 
    uint8_t altRef  [BYTE_9];     // Altitude above user datum ellipsoid 
    uint8_t navStat [BYTE_2];     // Navigation status 
    uint8_t hAcc    [BYTE_5];     // Horizontal accuracy estimate 
    uint8_t vAcc    [BYTE_5];     // Vertical accuracy estimate 
    uint8_t SOG     [BYTE_6];     // Speed over ground 
    uint8_t COG     [BYTE_6];     // Course over ground 
    uint8_t vVel    [BYTE_6];     // Vertical velocity (+ downwards) 
    uint8_t diffAge [BYTE_1];     // Age of differential corrections 
    uint8_t HDOP    [BYTE_5];     // Horizontal dilution of precision 
    uint8_t VDOP    [BYTE_5];     // Vertical dilution of precision 
    uint8_t TDOP    [BYTE_5];     // Time dilution of precision 
    uint8_t numSvs  [BYTE_2];     // Number of satellites used in the navigation solution 
    uint8_t res     [BYTE_1];     // Reserved --> 0 
    uint8_t DR      [BYTE_1];     // DR used 
    uint8_t eom     [BYTE_1];     // End of memory --> used for parsing function only 
} 
m8q_nmea_pos_t;


// NMEA TIME message fields 
typedef struct m8q_nmea_time_s
{
    uint8_t time     [BYTE_9];     // UTC time 
    uint8_t date     [BYTE_6];     // UTC date 
    uint8_t utcTow   [BYTE_9];     // UTC time of week 
    uint8_t utcWk    [BYTE_4];     // UTC week number 
    uint8_t leapSec  [BYTE_3];     // Leap seconds 
    uint8_t clkBias  [BYTE_8];     // Receiver clock bias 
    uint8_t clkDrift [BYTE_10];    // Receiver clock drift 
    uint8_t tpGran   [BYTE_3];     // Time pulse granularity 
    uint8_t eom      [BYTE_1];     // End of memory --> used for parsing function only 
} 
m8q_nmea_time_t;

//==================================================

//=======================================================================================


//=======================================================================================
// Global variables 

// NMEA message data 
typedef struct m8q_driver_data_s
{
    //==================================================
    // New 

    // Messages 
    m8q_nmea_pos_t pos_data;      // POSITION message 
    m8q_nmea_time_t time_data;    // TIME message 

    // Communication 
    I2C_TypeDef *i2c; 
    GPIO_TypeDef *pwr_save_gpio; 
    GPIO_TypeDef *tx_ready_gpio; 

    // Pins 
    pin_selector_t pwr_save;      // Low power mode 
    pin_selector_t tx_ready;      // TX-Ready 

    //==================================================
    
    // Messages 
    uint8_t ubx_resp[M8Q_MSG_MAX_LEN];        // Buffer to store incoming UBX messages 
    uint8_t nmea_resp[M8Q_NMEA_MSG_MAX_LEN];  // Buffer to store incoming NMEA messages 

    // Communications 
    GPIO_TypeDef *gpio; 

    // Status info 
    // 'status' --> bit 0: i2c status (see i2c_status_t) 
    //          --> bits 1-12: driver faults (see status getter) 
    //          --> bits 13-15: not used 
    uint16_t status; 
} 
m8q_driver_data_t; 


// NMEA message data instance 
static m8q_driver_data_t m8q_driver_data; 


// NMEA POSITION message 
static uint8_t* position[M8Q_NMEA_POS_ARGS+1] = 
{ 
    m8q_driver_data.pos_data.time, 
    m8q_driver_data.pos_data.lat, 
    m8q_driver_data.pos_data.NS, 
    m8q_driver_data.pos_data.lon, 
    m8q_driver_data.pos_data.EW, 
    m8q_driver_data.pos_data.altRef, 
    m8q_driver_data.pos_data.navStat, 
    m8q_driver_data.pos_data.hAcc, 
    m8q_driver_data.pos_data.vAcc,
    m8q_driver_data.pos_data.SOG,
    m8q_driver_data.pos_data.COG,
    m8q_driver_data.pos_data.vVel,
    m8q_driver_data.pos_data.diffAge,
    m8q_driver_data.pos_data.HDOP,
    m8q_driver_data.pos_data.VDOP,
    m8q_driver_data.pos_data.TDOP,
    m8q_driver_data.pos_data.numSvs,
    m8q_driver_data.pos_data.res,
    m8q_driver_data.pos_data.DR, 
    m8q_driver_data.pos_data.eom 
}; 

// NMEA TIME message 
static uint8_t* time[M8Q_NMEA_TIME_ARGS+1] = 
{ 
    m8q_driver_data.time_data.time, 
    m8q_driver_data.time_data.date, 
    m8q_driver_data.time_data.utcTow, 
    m8q_driver_data.time_data.utcWk, 
    m8q_driver_data.time_data.leapSec, 
    m8q_driver_data.time_data.clkBias, 
    m8q_driver_data.time_data.clkDrift, 
    m8q_driver_data.time_data.tpGran, 
    m8q_driver_data.time_data.eom 
}; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

//==================================================
// new 

/**
 * @brief Message identification 
 * 
 * @details 
 * 
 * @param msg 
 * @return M8Q_MSG_TYPE 
 */
M8Q_MSG_TYPE m8q_msg_id_dev(
    const char *msg); 


/**
 * @brief Message identification helper function 
 * 
 * @details 
 * 
 * @param msg 
 * @param ref_msg 
 * @param num_compare 
 * @param max_size 
 * @param offset 
 * @return uint8_t 
 */
uint8_t m8q_msg_id_check_dev(
    const char *msg, 
    const char *ref_msg, 
    uint8_t num_compare, 
    uint8_t max_size, 
    uint8_t offset); 

//==================================================

/**
 * @brief M8Q message size 
 * 
 * @details Calculates the size of a message by counting up until a termination character 
 *          is seen. The termination character is not counted as part of the length. 
 * 
 * @param msg : pointer to the message of unknown length 
 * @param term_char : character or byte that signifies the end of the message 
 * @return uint8_t : length of the message 
 */
uint8_t m8q_message_size(
    uint8_t *msg, 
    uint8_t term_char); 


/**
 * @brief M8Q NMEA message sort 
 * 
 * @details Identifies which NMEA message has been received so an appropriate function 
 *          call to m8q_nmea_parse can be made. If a message received doesn't match any of the 
 *          created data records then the message is ignored. This function gets called 
 *          by the m8q_read function after is reads an incoming NMEA message. 
 * 
 * @see m8q_nmea_parse
 * @see m8q_read 
 * 
 * @param msg : pointer to the message to analyze 
 */
void m8q_nmea_sort(
    uint8_t *msg); 


/**
 * @brief M8Q NMEA message parse 
 * 
 * @details Parses a full NMEA message payload into it's fields and stores the information 
 *          in the message data record. This function gets called by m8q_nmea_sort when 
 *          an NMEA message with a data record is seen. 
 * 
 * @see m8q_nmea_sort
 * 
 * @param msg : pointer to the message being parsed 
 * @param start_byte : byte to start parsing data - where the payload starts 
 * @param arg_num : number of fields the payload of the message carries 
 * @param data : double pointer to the message data record where the information is stored 
 */
void m8q_nmea_parse(
    uint8_t *msg, 
    uint8_t start_byte, 
    uint8_t arg_num, 
    uint8_t **data); 


/**
 * @brief M8Q NMEA config function 
 * 
 * @details Checks the format of an outgoing NMEA message and prepares it for sending. 
 *          If the message is not formatted correctly it will not send. The function 
 *          calls m8q_nmea_checksum in order to append that information to the end 
 *          of the message. 
 * 
 * @see m8q_nmea_checksum 
 * 
 * @param msg : pointer to the input message buffer 
 */
void m8q_nmea_config(
    uint8_t *msg); 


/**
 * @brief M8Q NMEA checkusm calculation 
 * 
 * @details Calculates the NMEA config message checksum to be sent along with the message 
 *          to the receiver using an exlusive OR (XOR) operation on all bytes of the 
 *          message string. 
 * 
 * @param msg : pointer to message buffer  
 * @return uint16_t : checksum of an NMEA message to be sent to the receiver 
 */
uint16_t m8q_nmea_checksum(
    uint8_t *msg); 


/**
 * @brief M8Q UBX config function 
 * 
 * @details Checks the format of an outgoing UBX message and prepares it for sending. 
 *          If the message is not formatted correctly it will not send. The function 
 *          calls m8q_ubx_checksum in order to append that information to the end 
 *          of the message. UBX messages are input by users as strings of ASCII characters 
 *          but UBX messages are sent as numbers. This function converts messages 
 *          using m8q_ubx_msg_convert before sending. 
 * 
 * @see m8q_ubx_checksum 
 * @see m8q_ubx_msg_convert
 * 
 * @param input_msg : pointer to the input message buffer 
 */
void m8q_ubx_config(
    uint8_t *input_msg); 


/**
 * @brief M8Q UBX message conversion 
 * 
 * @details Converts a UBX message ASCII character string into it's equivalent decimal 
 *          value so that it can be properly interpreted by the receiver when the message 
 *          is sent. This function gets called by m8q_ubx_config. 
 * 
 * @see m8q_ubx_config
 * 
 * @param input_msg_len : length of the message string being converted 
 * @param input_msg_start : starting byte o message string being converted 
 * @param input_msg : pointer to message string buffer 
 * @param new_msg_byte_count : stores the byte count of the converted message 
 * @param new_msg : pointer to the new (converted) message string buffer 
 * @return M8Q_STATUS : status of the conversion 
 */
M8Q_STATUS m8q_ubx_msg_convert(
    uint8_t input_msg_len,
    uint8_t input_msg_start, 
    uint8_t *input_msg, 
    uint16_t *new_msg_byte_count, 
    uint8_t *new_msg); 


/**
 * @brief M8Q UBX checksum calculation 
 * 
 * @details Calculates the checksum of the UBX message. This function is called after 
 *          the message has been verified to be valid and it has been fully converted to 
 *          a readable format for the receiver. 
 * 
 * @param msg : pointer to message buffer 
 * @param len : Length of checksum calculation range 
 * @return uint16_t : checksum of a UBX message to be sent to the receiver 
 */
uint16_t m8q_ubx_checksum(
    uint8_t *msg, 
    uint16_t len); 


/**
 * @brief M8Q NMEA config user interface 
 * 
 * @details Prints a prompt to the serial terminal to guide the user during configuration 
 *          of the receiver. 
 */
void m8q_user_config_prompt(void); 

//=======================================================================================


//=======================================================================================
// Initialization (public) - dev 

// Device initialization 
M8Q_STATUS m8q_init_dev(
    I2C_TypeDef *i2c, 
    const char *config_msgs, 
    uint8_t msg_num, 
    uint8_t max_msg_size)
{
    if (i2c == NULL)
    {
        return M8Q_INVALID_PTR; 
    }

    // Local varaibles 
    M8Q_MSG_TYPE msg_type = M8Q_MSG_INVALID; 

    // Assign data record information 
    m8q_driver_data.i2c = i2c; 

    // Send configuration messages 
    for (uint8_t i = CLEAR; i < msg_num; i++)
    {
        msg_type = m8q_msg_id_dev(config_msgs); 
        config_msgs += max_msg_size; 

        if (msg_type == M8Q_MSG_NMEA)
        {
            // m8q_nmea_config(config_msgs); 
        }
        else if (msg_type == M8Q_MSG_UBX)
        {
            // m8q_ubx_config(config_msgs); 
        }
        else 
        {
            return M8Q_INVALID_CONFIG; 
        }
    }

    return M8Q_OK; 
}


// Low power mode pin initialization 
M8Q_STATUS m8q_pwr_pin_init_dev(
    GPIO_TypeDef *gpio, 
    pin_selector_t pwr_save_pin)
{
    if (gpio == NULL)
    {
        return M8Q_INVALID_PTR; 
    }

    m8q_driver_data.pwr_save_gpio = gpio; 
    m8q_driver_data.pwr_save = pwr_save_pin; 

    gpio_pin_init(
        m8q_driver_data.pwr_save_gpio, 
        m8q_driver_data.pwr_save, 
        MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    m8q_clear_low_pwr_dev(); 

    return M8Q_OK; 
}


// TX ready pin initialization 
M8Q_STATUS m8q_txr_pin_init_dev(
    GPIO_TypeDef *gpio, 
    pin_selector_t tx_ready_pin)
{
    if (gpio == NULL)
    {
        return M8Q_INVALID_PTR; 
    }

    m8q_driver_data.tx_ready_gpio = gpio; 
    m8q_driver_data.tx_ready = tx_ready_pin; 

    gpio_pin_init(
        m8q_driver_data.tx_ready_gpio, 
        m8q_driver_data.tx_ready, 
        MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD); 

    return M8Q_OK; 
}

//=======================================================================================


//=======================================================================================
// User functions (public) 

// Read - using TX ready pin 

// Read - using device register status 

// Write configuration (singular) 

// Clear driver fault code 

// Get driver fault code 


// Enter low power mode 
void m8q_set_low_pwr_dev(void)
{
    gpio_write(m8q_driver_data.pwr_save_gpio, (SET_BIT << m8q_driver_data.pwr_save), GPIO_LOW); 
}


// Exit low power mode 
void m8q_clear_low_pwr_dev(void)
{
    gpio_write(m8q_driver_data.pwr_save_gpio, (SET_BIT << m8q_driver_data.pwr_save), GPIO_HIGH); 
}

// Get TX-Ready 

// Get latitude 

// Get North/South 

// Get longitude 

// Get East/West 

// Get navigation status 

// Get time 

// Get date 

//=======================================================================================


//=======================================================================================
// Read and write functions (private) 

// Read 

// Write 

//=======================================================================================


//=======================================================================================
// Message processing (private) 

// Message identification 
M8Q_MSG_TYPE m8q_msg_id_dev(
    const char *msg)
{
    // Check for the start of an NMEA message 
    if (*msg == nmea_start)
    {
        // Check for a PUBX message ID 
        if (str_compare(nmea_pubx_id, msg, BYTE_1))
        {
            // Check for a valid PUBX format 
            if (m8q_msg_id_check_dev(msg, 
                                     &nmea_pubx_format[0][0], 
                                     NMEA_PUBX_FORMAT_NUM, 
                                     NMEA_PUBX_FORMAT_LEN, 
                                     BYTE_6))
            {
                return M8Q_MSG_NMEA; 
            }
        }

        // Check for a standard NMEA message ID 
        if (m8q_msg_id_check_dev(msg, 
                                 &nmea_std_id[0][0], 
                                 NMEA_STD_ID_NUM, 
                                 NMEA_STD_ID_LEN, 
                                 BYTE_1))
        {
            // Check for a valid standard NMEA message format 
            if (m8q_msg_id_check_dev(msg, 
                                     &nmea_std_format[0][0], 
                                     NMEA_STD_FORMAT_NUM, 
                                     NMEA_STD_FORMAT_LEN, 
                                     BYTE_3))
            {
                return M8Q_MSG_NMEA; 
            }
        }
    }
    // Check for the start of a UBX message 
    else if (str_compare(ubx_sync, msg, BYTE_0))
    {
        // Check for a valid UBX class 
        if (m8q_msg_id_check_dev(msg, 
                                 &ubx_class[0][0], 
                                 UBX_CLASS_NUM, 
                                 UBX_CLASS_LEN, 
                                 BYTE_5))
        {
            return M8Q_MSG_UBX; 
        }
    }

    return M8Q_MSG_INVALID; 
}


// Message identification helper function 
uint8_t m8q_msg_id_check_dev(
    const char *msg, 
    const char *ref_msg, 
    uint8_t num_compare, 
    uint8_t max_size, 
    uint8_t offset)
{
    for (uint8_t i = CLEAR; i < num_compare; i++)
    {
        if (str_compare(ref_msg + i*max_size, msg, offset))
        {
            return TRUE; 
        }
    }

    return FALSE; 
}


// Sort NMEA data 

//=======================================================================================


//=======================================================================================
// Device configuration - config message formatting (private) 
//=======================================================================================


//=======================================================================================
// Initialization 

// M8Q initialization 
void m8q_init(
    I2C_TypeDef *i2c, 
    GPIO_TypeDef *gpio, 
    pin_selector_t pwr_save_pin, 
    pin_selector_t tx_ready_pin, 
    uint8_t msg_num, 
    uint8_t msg_max_size, 
    uint8_t *config_msgs)
{
    // Local variables 
    uint16_t m8q_status = M8Q_FAULT_NONE; 

    // Initialize the device communication info 
    m8q_driver_data.i2c = i2c; 
    m8q_driver_data.gpio = gpio; 
    m8q_driver_data.pwr_save = pwr_save_pin; 
    m8q_driver_data.tx_ready = tx_ready_pin; 
    m8q_driver_data.status = CLEAR; 

    // Configure a GPIO output for low power mode 
    gpio_pin_init(m8q_driver_data.gpio, 
                  m8q_driver_data.pwr_save, 
                  MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
    m8q_set_low_power(GPIO_HIGH); 

    // Configure a GPIO input for TX_READY 
    gpio_pin_init(m8q_driver_data.gpio, 
                  m8q_driver_data.tx_ready, 
                  MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD);

    // Send configuration messages 
    for (uint8_t i = 0; i < msg_num; i++)
    {
        // Identify the message type 
        switch (*(config_msgs))
        {
            case NMEA_START:  // NMEA message 
                m8q_nmea_config(config_msgs); 
                break;

            case UBX_SYNC1_HI:  // UBX message 
                m8q_ubx_config((config_msgs)); 
                break;
            
            default:  // Unknown config message 
                m8q_status |= (SET_BIT << M8Q_FAULT_NO_DATA); 
                break;  
        }
        
        config_msgs += msg_max_size; 
    }

    // Update the driver status 
    m8q_driver_data.status |= m8q_status; 
}

//=======================================================================================


//=======================================================================================
// Read and write functions 

// Read a message from the M8Q 
M8Q_STATUS m8q_read(void)
{
    // Local variables 
    M8Q_STATUS read_status = M8Q_READ_INVALID; 
    uint8_t data_check = CLEAR; 
    uint8_t *nmea_data = m8q_driver_data.nmea_resp; 
    uint8_t *ubx_data  = m8q_driver_data.ubx_resp; 
    I2C_STATUS i2c_status = I2C_OK; 

    // Check for a valid data stream 
    m8q_check_data_stream(&data_check); 

    switch (data_check)
    {
        case M8Q_NO_DATA:  // No data stream available 
            break;

        case NMEA_START:  // Start of NMEA message 
            // Capture the byte checked in the message response 
            *nmea_data++ = data_check; 

            // Generate a start condition 
            i2c_status |= i2c_start(m8q_driver_data.i2c); 

            // Send the device address with a read offset 
            i2c_status |= i2c_write_addr(m8q_driver_data.i2c, 
                                         M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET); 
            i2c_clear_addr(m8q_driver_data.i2c);  

            // Read the rest of the data stream until "\r\n" 
            i2c_status |= i2c_read_to_term(m8q_driver_data.i2c, 
                                           nmea_data, 
                                           M8Q_NMEA_END_PAY, 
                                           BYTE_4); 

            // Parse the message data into its data record 
            m8q_nmea_sort(nmea_data); 

            read_status = M8Q_READ_NMEA; 
            break;
        
        case UBX_SYNC1:  // Start of UBX message 
            // Capture the byte checked in the message response 
            *ubx_data++ = data_check; 

            // Generate a start condition 
            i2c_status |= i2c_start(m8q_driver_data.i2c); 

            // Send the device address with a read offset 
            i2c_status |= i2c_write_addr(m8q_driver_data.i2c, 
                                         M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET); 
            i2c_clear_addr(m8q_driver_data.i2c); 

            // Read the rest of the UBX message 
            i2c_status |= i2c_read_to_len(m8q_driver_data.i2c, 
                                          M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET, 
                                          ubx_data, 
                                          M8Q_UBX_LENGTH_OFST-BYTE_1, 
                                          M8Q_UBX_LENGTH_LEN, 
                                          M8Q_UBX_CS_LEN); 

            read_status = M8Q_READ_UBX; 
            break; 

        default:  // Unknown data stream 
            break;
    }

    // Update the driver status 
    m8q_driver_data.status |= (uint8_t)i2c_status; 

    return read_status; 
}


// Read the data stream size 
void m8q_check_data_size(
    uint16_t *data_size)
{
    // Local variables 
    uint8_t num_bytes[BYTE_2];        // Store the high and low byte of the data size 
    uint8_t address = M8Q_REG_0XFD;   // Address of high byte for the data size 
    I2C_STATUS i2c_status = I2C_OK; 

    // Generate a start condition 
    i2c_status |= i2c_start(m8q_driver_data.i2c); 

    // Write the slave address with write access 
    i2c_status |= i2c_write_addr(m8q_driver_data.i2c, M8Q_I2C_8_BIT_ADDR + I2C_W_OFFSET); 
    i2c_clear_addr(m8q_driver_data.i2c); 

    // Send the first data size register address to start reading from there 
    i2c_status |= i2c_write(m8q_driver_data.i2c, &address, BYTE_1); 

    // Generate another start condition 
    i2c_status |= i2c_start(m8q_driver_data.i2c); 

    // Send the device address again with a read offset 
    i2c_status |= i2c_write_addr(m8q_driver_data.i2c, M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET);  
    i2c_clear_addr(m8q_driver_data.i2c); 

    // Read the data size registers 
    i2c_status |= i2c_read(m8q_driver_data.i2c, num_bytes, BYTE_2); 

    // Format the data into the data size 
    *data_size = (uint16_t)((num_bytes[BYTE_0] << SHIFT_8) | num_bytes[BYTE_1]); 

    // Update the driver status 
    m8q_driver_data.status |= (uint8_t)i2c_status; 
}


// Read the current value at the data stream register 
void m8q_check_data_stream(
    uint8_t *data_check)
{
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 

    // Generate a start condition 
    i2c_status |= i2c_start(m8q_driver_data.i2c); 

    // Send the device address with a read offset 
    i2c_status |= i2c_write_addr(m8q_driver_data.i2c, M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET); 
    i2c_clear_addr(m8q_driver_data.i2c); 

    // Read the first byte of the data stream 
    i2c_status |= i2c_read(m8q_driver_data.i2c, data_check, BYTE_1); 

    // Update the driver status 
    m8q_driver_data.status |= (uint8_t)i2c_status; 
}


//=======================================================================================


//=======================================================================================
// Write functions 

// M8Q write 
void m8q_write(
    uint8_t *data, 
    uint8_t data_size)
{
    // Local variables 
    I2C_STATUS i2c_status = I2C_OK; 

    // Generate a start condition 
    i2c_status |= i2c_start(m8q_driver_data.i2c); 

    // Send the device address with a write offset 
    i2c_status |= i2c_write_addr(m8q_driver_data.i2c, M8Q_I2C_8_BIT_ADDR + I2C_W_OFFSET); 
    i2c_clear_addr(m8q_driver_data.i2c); 

    // Send data (at least 2 bytes) 
    i2c_status |= i2c_write(m8q_driver_data.i2c, data, data_size); 

    // Generate a stop condition 
    i2c_stop(m8q_driver_data.i2c); 

    // Update the driver status 
    m8q_driver_data.status |= (uint8_t)i2c_status; 
}

//=======================================================================================


//=======================================================================================
// Message processing 

// Read the length of a message 
uint8_t m8q_message_size(
    uint8_t *msg, 
    uint8_t term_char)
{
    // Local variables 
    uint8_t msg_len = CLEAR; 

    // Calculate message size 
    while (*msg++ != term_char) 
    {
        msg_len++; 
    }

    return msg_len; 
}


// NMEA message sort 
void m8q_nmea_sort(
    uint8_t *msg)
{
    // Local variables 
    uint8_t *msg_ptr = msg; 

    // Go to location of message ID 
    msg_ptr += M8Q_PUBX_ID_OFST; 

    // Identify the message and parse the message data 
    switch (*msg_ptr)
    {
        case M8Q_NMEA_POS_ID:  // 0x00 --> POSITION 
            m8q_nmea_parse(msg, M8Q_NMEA_PUBX_ARG_OFST-BYTE_1, M8Q_NMEA_POS_ARGS, position); 
            break;
        
        case M8Q_NMEA_SV_ID: // 0x03 --> SVSTATUS 
            // Not supported yet 
            break;

        case M8Q_NMEA_TIME_ID: // 0x04 --> TIME  
            m8q_nmea_parse(msg, M8Q_NMEA_PUBX_ARG_OFST-BYTE_1, M8Q_NMEA_TIME_ARGS, time); 
            break;
        
        default:
            break;
    }
}


// NMEA message parse 
void m8q_nmea_parse(
    uint8_t *msg, 
    uint8_t start_byte, 
    uint8_t arg_num, 
    uint8_t **data)
{
    // Local variables 
    uint8_t arg_index = 0; 
    uint8_t arg_len = 0; 
    uint8_t data_index = 0; 

    // Make sure the data array has a valid length 
    if (!arg_num) return; 
    
    // Increment to the first data field 
    msg += start_byte; 

    // Calculate the first data field length 
    arg_len = *(&data[data_index] + 1) - data[data_index];  

    // Read and parse the message 
    // TODO term char exit condition 
    while (TRUE)
    {
        // Check for end of message data 
        if (*msg != AST_CHAR)
        {
            // Check for argument separation 
            if (*msg != COMMA_CHAR)
            {
                // Record the byte if there is space 
                if (arg_index < arg_len)
                {
                    data[data_index][arg_index] = *msg; 
                    arg_index++; 
                }
            }
            else
            {
                // Terminate the argument if needed 
                if (arg_index < arg_len) data[data_index][arg_index] = NULL_CHAR; 

                // Increment jagged array index 
                data_index++; 

                // Exit if the storage array has been exceeded 
                if (data_index >= arg_num) break; 

                // Reset arg index and calculate the new argument length 
                arg_index = CLEAR; 
                arg_len = *(&data[data_index] + 1) - data[data_index]; 
            }

            // Increment msg index 
            msg++; 
        }
        else
        {
            // Terminate the argument if needed 
            if (arg_index < arg_len) data[data_index][arg_index] = NULL_CHAR; 
            break; 
        }
    }
}

//=======================================================================================


//=======================================================================================
// Setters and Getters 

// M8Q clear device driver fault flag 
void m8q_clear_status(void)
{
    m8q_driver_data.status = CLEAR; 
}


// M8Q get device driver fault code 
uint16_t m8q_get_status(void)
{
    return m8q_driver_data.status; 
}


// Low Power Mode setter 
void m8q_set_low_power(
    gpio_pin_state_t pin_state)
{
    gpio_write(m8q_driver_data.gpio, (SET_BIT << m8q_driver_data.pwr_save), pin_state);
}


// TX-Ready getter 
uint8_t m8q_get_tx_ready(void)
{
    return gpio_read(m8q_driver_data.gpio, (SET_BIT << m8q_driver_data.tx_ready)); 
}


// Latitude coordinate getter 
double m8q_get_lat(void)
{    
    // Local variables 
    int32_t deg_int = CLEAR; 
    int32_t deg_frac = CLEAR; 
    uint8_t dec_pos = M8Q_LAT_LEN - (M8Q_MIN_FRAC_LEN + BYTE_1); 

    // Parse the integer and fractional parts of the degree 
    for (uint8_t i = CLEAR, j = M8Q_MIN_DIGIT_INDEX; i < M8Q_LAT_LEN; i++)
    {
        if (i < M8Q_LAT_DEG_INT_LEN)
        {
            // Integer portion of degrees 
            deg_int += (int32_t)char_to_int(position[M8Q_POS_LAT][i], 1-i); 
        }
        else if (i == dec_pos)
        {
            // Decimal point character - in the middle of the fractional parts 
            continue; 
        }
        else 
        {
            // Fractional portion of degrees 
            deg_frac += (int32_t)char_to_int(position[M8Q_POS_LAT][i], j--); 
        }
    }

    // Adjust the sign of the degree depending on Northern or Southern hemisphere 
    if (m8q_get_NS() == M8Q_DIR_SOUTH) 
    {
        deg_int = ~deg_int + 1; 
        deg_frac = ~deg_frac + 1; 
    }

    // Calculate and return the final degree value 
    return ((double)deg_int) + 
           (((double)deg_frac) / (pow(SCALE_10, M8Q_MIN_FRAC_LEN)*M8Q_MIN_TO_DEG)); 
}


// Latitude string getter 
void m8q_get_lat_str(
    uint8_t *deg_min, 
    uint8_t *min_frac)
{
    // Integer part length 
    uint8_t int_len = M8Q_LAT_LEN - M8Q_COO_LEN; 

    // Copy the latitude into integer and fractional parts 
    for (uint8_t i = CLEAR; i <= M8Q_LAT_LEN; i++) 
    {
        if (i < int_len) 
        {
            *deg_min++ = position[M8Q_POS_LAT][i]; 
        }
        else if (i == int_len) 
        {
            *deg_min = NULL_CHAR; 
        }
        else if (i < M8Q_LAT_LEN) 
        {
            *min_frac++ = position[M8Q_POS_LAT][i]; 
        }
        else 
        {
            *min_frac = NULL_CHAR; 
        }
    }
}


// North/South getter 
uint8_t m8q_get_NS(void)
{
    return *(position[M8Q_POS_NS]); 
}


// Longitude coordinate getter 
double m8q_get_long(void)
{
    // Local variables 
    int32_t deg_int = CLEAR; 
    int32_t deg_frac = CLEAR; 
    uint8_t dec_pos = M8Q_LON_LEN - (M8Q_MIN_FRAC_LEN + BYTE_1); 

    // Parse the integer and fractional parts of the degree 
    for (uint8_t i = CLEAR, j = M8Q_MIN_DIGIT_INDEX; i < M8Q_LON_LEN; i++)
    {
        if (i < M8Q_LON_DEG_INT_LEN)
        {
            // Integer portion of degrees 
            deg_int += (int32_t)char_to_int(position[M8Q_POS_LON][i], 2-i); 
        }
        else if (i == dec_pos)
        {
            // Decimal point character - in the middle of the fractional parts 
            continue; 
        }
        else 
        {
            // Fractional portion of degrees 
            deg_frac += (int32_t)char_to_int(position[M8Q_POS_LON][i], j--); 
        }
    }

    // Adjust the sign of the degree depending on Eastern or Western hemisphere 
    if (m8q_get_EW() == M8Q_DIR_WEST) 
    {
        deg_int = ~deg_int + 1; 
        deg_frac = ~deg_frac + 1; 
    }

    // Calculate and return the final degree value 
    return ((double)deg_int) + 
           (((double)deg_frac) / (pow(SCALE_10, M8Q_MIN_FRAC_LEN)*M8Q_MIN_TO_DEG)); 
}


// Longitude string getter 
void m8q_get_long_str(
    uint8_t *deg_min, 
    uint8_t *min_frac)
{
    // Integer part length 
    uint8_t int_len = M8Q_LON_LEN - M8Q_COO_LEN; 

    // Copy the longitude into integer (degees + minutes) and fractional (minutes) parts 
    for (uint8_t i = CLEAR; i <= M8Q_LON_LEN; i++) 
    {
        if (i < int_len) 
        {
            *deg_min++ = position[M8Q_POS_LON][i]; 
        }
        else if (i == int_len) 
        {
            *deg_min = NULL_CHAR; 
        }
        else if (i < M8Q_LON_LEN) 
        {
            *min_frac++ = position[M8Q_POS_LON][i]; 
        }
        else 
        {
            *min_frac = NULL_CHAR; 
        }
    }
}


// East/West getter 
uint8_t m8q_get_EW(void)
{
    return *(position[M8Q_POS_EW]);
}


// Navigation status getter 
uint16_t m8q_get_navstat(void)
{
    // Format the status 
    uint16_t navstat_high = position[M8Q_POS_NAVSTAT][BYTE_0] << SHIFT_8; 
    uint16_t navstat_low = position[M8Q_POS_NAVSTAT][BYTE_1]; 

    return (navstat_high | navstat_low); 
}


// Time getter 
void m8q_get_time(
    uint8_t *utc_time)
{
    for (uint8_t i = 0; i < M8Q_TIME_CHAR_LEN; i++)
    {
        *utc_time++ = time[M8Q_TIME_TIME][i]; 
    }
}


// Date getter 
void m8q_get_date(
    uint8_t *utc_date)
{
    for (uint8_t i = 0; i < M8Q_DATE_CHAR_LEN; i++)
    {
        *utc_date++ = time[M8Q_TIME_DATE][i]; 
    }
}

//=======================================================================================


//=======================================================================================
// User Configuration Mode 

#if M8Q_USER_CONFIG

// 
void m8q_user_config_init(
    I2C_TypeDef *i2c) 
{
    // Initialize I2C pointer 
    m8q_driver_data.i2c = i2c; 

    // Prompt the user for the first message 
    m8q_user_config_prompt();
}


// M8Q user configuration 
void m8q_user_config(void)
{
    // Local variables 
    uint16_t error_code = 0; 
    uint8_t config_msg[M8Q_MSG_MAX_LEN]; 
    uint8_t ubx_pl_len = 0; 
    uint8_t ubx_ack_clear_counter = 0; 

    // Check if there is user input waiting 
    if (uart_data_ready(USART2))
    {
        // Read the input 
        uart_getstr(USART2, (char *)config_msg, M8Q_MSG_MAX_LEN, UART_STR_TERM_CARRIAGE); 

        uart_send_new_line(USART2); 

        // Identify the message type 
        switch (config_msg[0])
        {
            case NMEA_START:  // NMEA message 
                error_code = m8q_nmea_config(config_msg); 
                break;

            case UBX_SYNC1:  // UBX message 
                error_code = m8q_ubx_config(config_msg); 

                // Communicate the results 
                if (!error_code)
                { 
                    ubx_pl_len = (m8q_driver_data.ubx_resp[M8Q_UBX_LENGTH_OFST+1] << SHIFT_8) | 
                                  m8q_driver_data.ubx_resp[M8Q_UBX_LENGTH_OFST]; 
                    
                    for (uint8_t i = 0; i < (M8Q_UBX_HEADER_LEN+ubx_pl_len+M8Q_UBX_CS_LEN); i++)
                    {
                        uart_send_integer(USART2, (int16_t)m8q_driver_data.ubx_resp[i]); 
                        uart_send_new_line(USART2);
                    }

                    // Clear the ACK message? 
                    while (ubx_ack_clear_counter < 10)
                    {
                        if (m8q_read() == M8Q_READ_UBX) break; 
                        ubx_ack_clear_counter++; 
                    }
                    // while(m8q_read() != M8Q_READ_UBX); 
                }
                break;
            
            default:  // Unknown input 
                uart_send_new_line(USART2); 
                uart_sendstring(USART2, "Unknown message type\r\n"); 
                break;
        }

        // Print any error codes 
        if (error_code)
        {
            uart_sendstring(USART2, "Error code: "); 
            uart_send_integer(USART2, (int16_t)error_code); 
            uart_send_new_line(USART2); 
        }

        // Prompt the user for the next message 
        m8q_user_config_prompt();
    }
}


// M8Q NMEA config user interface 
void m8q_user_config_prompt(void)
{
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, ">>> Config message: "); 
}

#endif   // M8Q_USER_CONFIG

//=======================================================================================


//=======================================================================================
// Message configuration functions 

// NMEA config function 
void m8q_nmea_config(
    uint8_t *msg)
{
    // Local variables 
    uint16_t m8q_status = M8Q_FAULT_NONE; 
    uint8_t *msg_ptr = msg; 
    uint8_t msg_args = CLEAR; 
    uint8_t msg_arg_count = CLEAR; 
    uint8_t msg_arg_mask = CLEAR; 
    uint16_t checksum = CLEAR; 
    char term_str[M8Q_NMEA_END_MSG]; 

    // Check message header and ID 
    if (str_compare("$PUBX,", (char *)msg, BYTE_0))
    {
        // RATE (ID=40) 
        if (str_compare("40,", (char *)msg, BYTE_6)) 
        {
            msg_args = M8Q_NMEA_RATE_ARGS; 
        }
        
        // CONFIG (ID=41) 
        else if (str_compare("41,", (char *)msg, BYTE_6)) 
        {
            msg_args = M8Q_NMEA_CONFIG_ARGS; 
        }

        // Unsupported message ID 
        else
        {
            m8q_status |= (SET_BIT << M8Q_FAULT_NMEA_ID); 
        }

        // Check the number of message inputs 
        if (msg_args)
        {
            msg_ptr += BYTE_9; 

            while(*msg_ptr != CR_CHAR) 
            {
                if (*msg_ptr == COMMA_CHAR)
                {
                    msg_arg_mask = 0; 
                }
                else 
                {
                    if (!msg_arg_mask)
                    {
                        msg_arg_count++;   // Count an input 
                        msg_arg_mask++;    // Prevent from double counting an input 
                    }
                }
                msg_ptr++; 
            }

            // Check if the message is valid 
            if (msg_arg_count == msg_args)
            {
                // Calculate a checksum 
                checksum = m8q_nmea_checksum(msg);

                // Append the checksum and termination characters onto the message 
                sprintf(term_str, "*%c%c\r\n", (char)(checksum >> SHIFT_8), (char)(checksum)); 
                for (uint8_t i = 0; i < M8Q_NMEA_END_MSG; i++) *msg_ptr++ = (uint8_t)term_str[i]; 

                // Pass the message along to the M8Q write function 
                m8q_write(msg, m8q_message_size(msg, NULL_CHAR));
            } 
            else
            {
                m8q_status |= (SET_BIT << M8Q_FAULT_NMEA_FORM); 
            }
        }
    }
    else 
    {
        m8q_status |= (SET_BIT << M8Q_FAULT_NMEA_INVALID); 
    }

    // Update the driver status 
    m8q_driver_data.status |= m8q_status; 
}


// NMEA message calculation 
uint16_t m8q_nmea_checksum(
    uint8_t *msg)
{
    // Local variables 
    uint16_t checksum = 0; 
    uint8_t xor_result = 0; 
    uint8_t checksum_char = 0; 

    msg++;  // Ignore the "$" at the beginning of the message 

    // Perform and exlusive OR (XOR) on the NMEA message 
    while (*msg != CR_CHAR) 
    {
        xor_result = xor_result ^ *msg;
        msg++; 
    }

    // Format the checksum 
    for (uint8_t i = 0; i < M8Q_NMEA_CS_LEN; i++)
    {
        checksum_char = (xor_result & (FILTER_4_MSB >> SHIFT_4*i)) >> SHIFT_4*(1-i); 
        // TODO test and replace 
        // if (checksum_char <= MAX_CHAR_DIGIT) checksum_char += NUM_TO_CHAR_OFFSET; 
        // else checksum_char += HEX_TO_LET_CHAR; 
        checksum_char = (checksum_char <= MAX_CHAR_DIGIT) ? (checksum_char + NUM_TO_CHAR_OFFSET) : 
                                                            (checksum_char + HEX_TO_LET_CHAR); 
        checksum |= (uint16_t)(checksum_char << SHIFT_8*(1-i)); 
    }

    return checksum; 
}


// UBX config function 
// TODO Clean up UBX config function 
void m8q_ubx_config(
    uint8_t *input_msg)
{
    // Local variables 
    uint16_t m8q_status = M8Q_FAULT_NONE; 
    uint8_t *msg_ptr = input_msg; 
    uint8_t config_msg[M8Q_MSG_MAX_LEN];   // Formatted UBX message to send to the receiver 
    uint16_t checksum = CLEAR; 
    // TODO replace ghost number 
    uint16_t response_timeout = 0xFFFF; 

    // User inputs 
    uint8_t input_msg_len = m8q_message_size(input_msg, CR_CHAR); 
    uint8_t msg_len_pl_check = CLEAR; 
    uint8_t msg_id_input = CLEAR; 
    uint8_t pl_inputs[M8Q_PYL_MAX_LEN]; 

    // Formatters 
    uint16_t pl_len = CLEAR; 
    uint16_t byte_count = CLEAR; 
    uint8_t  format_ok = CLEAR; 

    // Check the sync characters and class 
    if (str_compare("B5,62,06,", (char *)input_msg, BYTE_0))
    {
        // Validate the ID formatting 
        if (m8q_ubx_msg_convert(BYTE_2, BYTE_9, input_msg, &byte_count, &msg_id_input))
        {
            // Check payload length 
            if (str_compare("poll", (char *)input_msg, BYTE_12))  // Poll request 
            {
                // Replace "poll" with zeros to define the payload length 
                msg_ptr += BYTE_12; 
                for (uint8_t i = 0; i < BYTE_4; i++) *msg_ptr++ = ZERO_CHAR; 

                format_ok++; 
            }
            else  // Not (necessarily) a poll request 
            {
                // Read the specified payload length and check the format 
                if (m8q_ubx_msg_convert(BYTE_4, BYTE_12, 
                                        input_msg, &byte_count, pl_inputs))
                {
                    // Format the payload length 
                    pl_len = (pl_inputs[1] << SHIFT_8) | pl_inputs[0]; 

                    byte_count = 0;  // Reset the byte count to check payload length 

                    // Check the argument format  
                    if (input_msg_len > BYTE_17) msg_len_pl_check = input_msg_len - BYTE_17; 
                    
                    if (m8q_ubx_msg_convert(msg_len_pl_check, BYTE_17,
                                            input_msg, &byte_count, pl_inputs))
                    {                        
                        if (pl_len == byte_count)
                        {
                            format_ok++; 
                        }
                        else 
                        {
                            m8q_status |= (SET_BIT << M8Q_FAULT_UBX_SIZE); 
                        }
                    } 
                    else
                    {
                        m8q_status |= (SET_BIT << M8Q_FAULT_UBX_FORM); 
                    }
                }
                else
                {
                    m8q_status |= (SET_BIT << M8Q_FAULT_UBX_LEN); 
                }
            }

            if (format_ok)
            {
                // Convert the input message to the proper UBX message format 
                if (m8q_ubx_msg_convert(input_msg_len, BYTE_0, 
                                        input_msg, &byte_count, config_msg))
                {
                    // Calculate the checksum 
                    checksum = m8q_ubx_checksum(config_msg, M8Q_UBX_MSG_FMT_LEN+pl_len); 

                    // Add the checksum to the end of the message buffer 
                    config_msg[M8Q_UBX_HEADER_LEN+pl_len] = (uint8_t)(checksum >> SHIFT_8); 
                    config_msg[M8Q_UBX_HEADER_LEN+pl_len+BYTE_1] = (uint8_t)(checksum); 

                    // Send the UBX message 
                    m8q_write(config_msg, M8Q_UBX_HEADER_LEN + pl_len + M8Q_UBX_CS_LEN); 

                    // Read the UBX CFG response 
                    while((m8q_read() != M8Q_READ_UBX) && --response_timeout); 

                    // Check the response type 
                    if (m8q_driver_data.ubx_resp[M8Q_UBX_CLASS_OFST] == M8Q_UBX_ACK_CLASS) 
                    {
                        if (m8q_driver_data.ubx_resp[M8Q_UBX_ID_OFST] != M8Q_UBX_ACK_ID)
                        {
                            m8q_status |= (SET_BIT << M8Q_FAULT_UBX_NAK); 
                        }
                    }
                    else
                    {
                        if (!M8Q_USER_CONFIG) m8q_status |= (SET_BIT << M8Q_FAULT_UBX_RESP); 
                    }
                }
                else
                {
                    m8q_status |= (SET_BIT << M8Q_FAULT_UBX_CONVERT); 
                }
            } 
        } 
        else 
        {

            m8q_status |= (SET_BIT << M8Q_FAULT_UBX_ID); 
        }
    }
    else
    {
        m8q_status |= (SET_BIT << M8Q_FAULT_UBX_NA); 
    }

    // Update the driver status 
    m8q_driver_data.status |= m8q_status; 
}


// UBX message convert 
M8Q_STATUS m8q_ubx_msg_convert(
    uint8_t input_msg_len, 
    uint8_t input_msg_start, 
    uint8_t *input_msg, 
    uint16_t *new_msg_byte_count, 
    uint8_t *new_msg)
{
    // Local variable 
    M8Q_STATUS status = M8Q_UBX_MSG_CONV_FAIL; 
    uint8_t char_count = 0; 
    uint8_t low_nibble = 0; 
    uint8_t high_nibble = 0; 
    uint8_t msg_index = 0; 
    uint8_t comma_count = 0;  // Protect against multiple commas in a row 

    // Go to specified starting address of message 
    input_msg += input_msg_start; 

    // Loop through the user input 
    while (TRUE)
    {
        if (msg_index != input_msg_len)
        {
            low_nibble = *input_msg++; 

            if (low_nibble != COMMA_CHAR)
            {
                // Not a comma 
                comma_count = 0; 

                // Check the character validity 
                if ((low_nibble >= ZERO_CHAR) && (low_nibble <= NINE_CHAR))
                {
                    low_nibble -= NUM_TO_CHAR_OFFSET; 
                }
                
                else if ((low_nibble >= A_UP_CHAR) && (low_nibble <= F_UP_CHAR)) 
                {
                    low_nibble -= HEX_TO_LET_CHAR; 
                }
                
                else 
                {
                    break; 
                }
                
                // Format two characters into one byte 
                if (char_count)  // Format byte
                {
                    *new_msg_byte_count += 1; 
                    *new_msg++ = (high_nibble << SHIFT_4) | low_nibble; 
                }

                else  // Store the byte 
                {
                    high_nibble = low_nibble; 
                }

                char_count = 1 - char_count;
            }

            else 
            {
                comma_count++; 
                if ((!msg_index) || char_count || (comma_count == 2)) break; 
            }

            msg_index++; 
        }
        else
        {
            if (!char_count) status = M8Q_UBX_MSG_CONV_SUCC; 
            break; 
        }
    }
    
    return status; 
}


// UBX checksum calculation 
uint16_t m8q_ubx_checksum(
    uint8_t *msg, 
    uint16_t len)
{
    // Local variables 
    uint8_t checksum_A = 0; 
    uint8_t checksum_B = 0; 
    uint16_t checksum = 0; 

    // Exclude the sync characters from the checksum calculation 
    msg += BYTE_2; 

    // Calculate the checksum 
    for (uint16_t i = 0; i < len; i++)
    {
        checksum_A += *msg++; 
        checksum_B += checksum_A; 
    }

    checksum = (checksum_A << SHIFT_8) | checksum_B; 
    return checksum; 
}

//=======================================================================================
