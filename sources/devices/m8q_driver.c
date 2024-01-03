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

// NMEA message format 
#define NMEA_START 0x24           // '$' --> NMEA protocol start character 
#define NMEA_PUBX_NUM_MSGS 5      // Number of NMEA PUBX messages 
#define NMEA_STD_ID_NUM 5         // Number of standard NMEA message IDs 
#define NMEA_STD_ID_LEN 3         // Maximum length of a standard NMEA message ID 
#define NMEA_STD_NUM_MSGS 19      // Number of standard NMEA formatters 
#define NMEA_MSG_FORMAT_LEN 4     // Maximum length of an NMEA message formatter 

// UBX message format 
#define UBX_CLASS_NUM 14          // Number of UBX classes 
#define UBX_CLASS_LEN 3           // Maximum length of a UBX class 

// M8Q messages 
#define M8Q_MSG_MAX_LEN 150              // Message buffer that can hold any message


#define M8Q_UBX_CFG_INDEX 4 

#define M8Q_EOM_BYTE 1 

#define ACK_ACK 0x0501 
#define ACK_NAK 0x0500 

#define ACK_TIMEOUT 10 

//==================================================
// Checking if needed 

// Device parameters 
#define M8Q_I2C_8_BIT_ADDR 0x84          // Receiver I2C address (default : 0x42 << 1) 

// M8Q registers 
#define M8Q_REG_0XFD 0xFD                // Available data bytes (high byte) register 
#define M8Q_REG_0XFE 0xFE                // Available data bytes (low byte) register 
#define M8Q_REG_0XFF 0xFF                // Data stream register 

// NMEA message format 
#define M8Q_NMEA_MSG_MAX_LEN 150         // NMEA message buffer that can hold any received message
#define M8Q_NMEA_END_PAY     0x2A        // 0x2A == '*' --> indicates end of NMEA message payload 
#define M8Q_NMEA_END_MSG     6           // Length of string to append to NMEA message after payload 
#define M8Q_NMEA_CS_LEN      2           // Number of characters in NMEA message checksum 
#define M8Q_PUBX_ID_OFST     6           // Starting position of PUBX message ID in message string 

// NMEA messages 
#define M8Q_NMEA_RATE_ARGS     7         // Number of data fields in RATE 
#define M8Q_NMEA_CONFIG_ARGS   5         // Number of data fields in CONFIG 
#define M8Q_NMEA_POS_ARGS      19        // Number of data fields in POSITION 
#define M8Q_NMEA_POS_ID        48        // "0" == 48 --> Message ID for POSITION 
#define M8Q_NMEA_SV_ARGS       7         // Number of data fields in SVSTATUS 
#define M8Q_NMEA_SV_ID         51        // "3" == 51 --> Message ID for SVSTATUS 
#define M8Q_NMEA_TIME_ARGS     8         // Number of data fields in TIME 
#define M8Q_NMEA_TIME_ID       52        // "4" == 52 --> Message ID for TIME 
#define M8Q_NMEA_PUBX_ARG_OFST 9         // First data field offset for PUBX messages 
#define M8Q_NMEA_STRD_ARG_OFST 7         // First data field offset for standard messages 

// NMEA POSITION coordinate calculation 
#define M8Q_LAT_LEN 10                   // Latitude message length 
#define M8Q_LON_LEN 11                   // Longitude message length 
#define M8Q_LAT_DEG_INT_LEN 2            // Number of digits for integer portion of latitude 
#define M8Q_LON_DEG_INT_LEN 3            // Number of digits for integer portion of longitude 
#define M8Q_MIN_DIGIT_INDEX 6            // Index of scaled minute portion of coordinate 
#define M8Q_MIN_FRAC_LEN 5               // Length of fractional part of minutes 
#define M8Q_DIR_SOUTH 0x53               // Value of South from the North/South indicator 
#define M8Q_DIR_WEST 0x57                // Value of West from the East/West indicator 
#define M8Q_MIN_TO_DEG 60                // Used to convert coordinate minutes to degrees 
#define M8Q_COO_LEN 6                    // Length of integer and fractional parts of coordinates 

// UBX message format 
#define M8Q_UBX_MSG_FMT_LEN 4            // Message format length: CLASS + ID + LENGTH 
#define M8Q_UBX_LENGTH_LEN  2            // LENGTH field length 
#define M8Q_UBX_HEADER_LEN  6            // Number of bytes before the payload 
#define M8Q_UBX_CS_LEN      2            // Number of bytes in a UBC message checksum 

#define M8Q_UBX_SYNC1_OFST  0            // First sync character offset 
#define M8Q_UBX_SYNC2_OFST  1            // Second sync character offset 
#define M8Q_UBX_CLASS_OFST  2            // Class character offset 
#define M8Q_UBX_ID_OFST     3            // Message ID character offset 
#define M8Q_UBX_LENGTH_OFST 4            // LENGTH field offset from start of UBX message frame 

// UBX messages 
#define M8Q_UBX_ACK_CLASS 0x05           // Class of ACK message 
#define M8Q_UBX_ACK_ID    0x01           // ID of ACK message 
#define M8Q_TIME_CHAR_LEN 9              // Number of characters for the time in NMEA messages 
#define M8Q_DATE_CHAR_LEN 6              // Number of characters for the date in NMEA messages 

//==================================================

//==================================================
// No longer used (to be deleted) 

// M8Q messages 
#define M8Q_NO_DATA     0xff             // Return value for an invalid NMEA data stream 
#define M8Q_PYL_MAX_LEN 100              // Message payload buffer to store any apyload length 

// Message identification 
#define MSG_ID_CHARS 8            // Number of characters used to identify message types 

#define UBX_SYNC1 0xB5            // 0xB5 --> UBX protocol SYNC CHAR 1 
#define UBX_SYNC1_HI 0x42         // 'B' --> SYNC CHAR 1 (0xB5) high nibble character 
#define UBX_SYNC1_LO 0x35         // '5' --> SYNC CHAR 1 (0xB5) low nibble character 
#define UBX_SYNC2_HI 0x36         // '6' --> SYNC CHAR 2 (0x62) high nibble character 
#define UBX_SYNC2_LO 0x32         // '2' --> SYNC CHAR 2 (0x62) low nibble character 

//==================================================

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q message type 
 */
typedef enum {
    M8Q_MSG_INVALID, 
    M8Q_MSG_NMEA, 
    M8Q_MSG_UBX, 
    M8Q_MSG_RTCM 
} m8q_msg_type_t; 


/**
 * @brief Number of fields in an NMEA message 
 */
typedef enum {
    NMEA_NUM_FIELDS_POSITION = 19, 
    NMEA_NUM_FIELDS_SVSTATUS = 7, 
    NMEA_NUM_FIELDS_TIME = 8, 
    NMEA_NUM_FIELDS_RATE = 7, 
    NMEA_NUM_FIELDS_CONFIG = 5, 
    NMEA_NUM_FIELDS_DTM = 8, 
    NMEA_NUM_FIELDS_GBQ = 1, 
    NMEA_NUM_FIELDS_GBS = 10, 
    NMEA_NUM_FIELDS_GGA = 14, 
    NMEA_NUM_FIELDS_GLL = 7, 
    NMEA_NUM_FIELDS_GLQ = 1, 
    NMEA_NUM_FIELDS_GNQ = 1, 
    NMEA_NUM_FIELDS_GNS = 13, 
    NMEA_NUM_FIELDS_GPQ = 1, 
    NMEA_NUM_FIELDS_GRS = 16, 
    NMEA_NUM_FIELDS_GSA = 18, 
    NMEA_NUM_FIELDS_GST = 8, 
    NMEA_NUM_FIELDS_GSV = 8, 
    NMEA_NUM_FIELDS_RMC = 13, 
    NMEA_NUM_FIELDS_THS = 2, 
    NMEA_NUM_FIELDS_TXT = 4, 
    NMEA_NUM_FIELDS_VLW = 8, 
    NMEA_NUM_FIELDS_VTG = 9, 
    NMEA_NUM_FIELDS_ZDA = 6 
} nmea_num_fields_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef uint8_t M8Q_MSG_TYPE; 

//=======================================================================================


//=======================================================================================
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

//=======================================================================================


//=======================================================================================
// Data record 

// Driver data record 
typedef struct m8q_driver_data_s
{
    //==================================================
    // New 

    // Messages 
    m8q_nmea_pos_t pos_data;      // POSITION message 
    m8q_nmea_time_t time_data;    // TIME message 
    uint8_t ack_msg_count;        // ACK-ACK message counter 
    uint8_t nak_msg_count;        // ACK-NAK message counter 

    // Communication 
    I2C_TypeDef *i2c; 
    GPIO_TypeDef *pwr_save_gpio; 
    GPIO_TypeDef *tx_ready_gpio; 

    // Pins 
    pin_selector_t pwr_save;      // Low power mode 
    pin_selector_t tx_ready;      // TX-Ready 

    // Other 
    uint16_t data_buff_limit; 

    //==================================================
    
    //==================================================
    // Old (to be deleted) 

    // Messages 
    uint8_t ubx_resp[M8Q_MSG_MAX_LEN];        // Buffer to store incoming UBX messages 
    uint8_t nmea_resp[M8Q_MSG_MAX_LEN];       // Buffer to store incoming NMEA messages 

    // Communications 
    GPIO_TypeDef *gpio; 

    // Status info 
    // 'status' --> bit 0: i2c status (see i2c_status_t) 
    //          --> bits 1-12: driver faults (see status getter) 
    //          --> bits 13-15: not used 
    uint16_t status; 
    
    //==================================================
} 
m8q_driver_data_t; 


// Driver data record instance 
static m8q_driver_data_t m8q_driver_data; 

//=======================================================================================


//=======================================================================================
// Message processing 

//==================================================
// Indexing 

// NMEA POSITION message 
static uint8_t *position[NMEA_NUM_FIELDS_POSITION + M8Q_EOM_BYTE] = 
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
static uint8_t *time[NMEA_NUM_FIELDS_TIME + M8Q_EOM_BYTE] = 
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

//==================================================

//==================================================
// Identification 

// Message info 
typedef struct nmea_msg_data_s 
{
    uint8_t num_param; 
    uint8_t **msg_data; 
}
nmea_msg_data_t; 

static nmea_msg_data_t nmea_msg_target; 


// NMEA message format 
typedef struct nmea_msg_format_s 
{
    char nmea_msg_format[NMEA_MSG_FORMAT_LEN]; 
    nmea_msg_data_t nmea_msg_data; 
}
nmea_msg_format_t; 


// UBX message class data 
typedef struct ubx_msg_class_s 
{
    char ubx_msg_class_str[UBX_CLASS_LEN]; 
    uint8_t ubc_msg_class_data[2]; 
}
ubx_msg_class_t; 

//==================================================

//==================================================
// NMEA message address 

// Start character of all NMEA messages 
static const char nmea_msg_start = NMEA_START; 

// U-Blox defined NMEA message ID 
static const char nmea_pubx_msg_id[] = "PUBX"; 

// NMEA PUBX messages 
static const nmea_msg_format_t nmea_pubx_msgs[NMEA_PUBX_NUM_MSGS] =   
{
    {"00", {NMEA_NUM_FIELDS_POSITION, position}},   // POSITION 
    {"03", {NMEA_NUM_FIELDS_SVSTATUS, NULL}},       // SVSTATUS 
    {"04", {NMEA_NUM_FIELDS_TIME, time}},           // TIME 
    {"40", {NMEA_NUM_FIELDS_RATE, NULL}},           // RATE 
    {"41", {NMEA_NUM_FIELDS_CONFIG, NULL}}          // CONFIG 
}; 

// NMEA talker IDs 
static const char nmea_std_msg_id[NMEA_STD_ID_NUM][NMEA_STD_ID_LEN] = 
{
    "GP",   // GPS, SBAS, QZSS 
    "GL",   // GLONASS 
    "GA",   // Galileo 
    "GB",   // BeiDou 
    "GN"    // Any combination of GNSS 
}; 

// NMEA standard messages 
static const nmea_msg_format_t nmea_std_msgs[NMEA_STD_NUM_MSGS] =   
{
    {"DTM", {NMEA_NUM_FIELDS_DTM, NULL}},   // Datum reference 
    {"GBQ", {NMEA_NUM_FIELDS_GBQ, NULL}},   // Poll a standard message (Talker ID GB) 
    {"GBS", {NMEA_NUM_FIELDS_GBS, NULL}},   // GNSS satellite fault detection 
    {"GGA", {NMEA_NUM_FIELDS_GGA, NULL}},   // Global positioning system fix data 
    {"GLL", {NMEA_NUM_FIELDS_GLL, NULL}},   // Lat and long, with time of position fix and status 
    {"GLQ", {NMEA_NUM_FIELDS_GLQ, NULL}},   // Poll a standard message (Talker ID GL) 
    {"GNQ", {NMEA_NUM_FIELDS_GNQ, NULL}},   // Poll a standard message (Talker ID GN) 
    {"GNS", {NMEA_NUM_FIELDS_GNS, NULL}},   // GNSS fix data 
    {"GPQ", {NMEA_NUM_FIELDS_GPQ, NULL}},   // Poll a standard message (Talker ID GP) 
    {"GRS", {NMEA_NUM_FIELDS_GRS, NULL}},   // GNSS range residuals 
    {"GSA", {NMEA_NUM_FIELDS_GSA, NULL}},   // GNSS DOP and active satellites 
    {"GST", {NMEA_NUM_FIELDS_GST, NULL}},   // GNSS pseudorange error statistics 
    {"GSV", {NMEA_NUM_FIELDS_GSV, NULL}},   // GNSS satellites in view 
    {"RMC", {NMEA_NUM_FIELDS_RMC, NULL}},   // Recommended minimum data 
    {"THS", {NMEA_NUM_FIELDS_THS, NULL}},   // True heading and status 
    {"TXT", {NMEA_NUM_FIELDS_TXT, NULL}},   // Text transmission 
    {"VLW", {NMEA_NUM_FIELDS_VLW, NULL}},   // Dual ground/water distance 
    {"VTG", {NMEA_NUM_FIELDS_VTG, NULL}},   // Course over ground and ground speed 
    {"ZDA", {NMEA_NUM_FIELDS_ZDA, NULL}}    // Time and data 
}; 

//==================================================

//==================================================
// UBX sync characters and class 

static const char ubx_msg_sync_str[] = "B562"; 

static const uint8_t ubx_msg_sync_data[] = { 0xB5, 0x62 , NULL_CHAR}; 

static const ubx_msg_class_t ubx_msg_class[UBX_CLASS_NUM] = 
{
    {"01", {0x01, NULL_CHAR}},   // NAV 
    {"02", {0x02, NULL_CHAR}},   // RXM 
    {"04", {0x04, NULL_CHAR}},   // INF 
    {"05", {0x05, NULL_CHAR}},   // ACK 
    {"06", {0x06, NULL_CHAR}},   // CFG 
    {"09", {0x09, NULL_CHAR}},   // UPD 
    {"0A", {0x0A, NULL_CHAR}},   // MON 
    {"0B", {0x0B, NULL_CHAR}},   // AID 
    {"0D", {0x0D, NULL_CHAR}},   // TIM 
    {"10", {0x10, NULL_CHAR}},   // ESF 
    {"13", {0x13, NULL_CHAR}},   // MGA 
    {"21", {0x21, NULL_CHAR}},   // LOG 
    {"27", {0x27, NULL_CHAR}},   // SEC 
    {"28", {0x28, NULL_CHAR}}    // HNR 
}; 

//==================================================

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Read the data stream size 
 * 
 * @details 
 * 
 * @param data_size 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_read_ds_size_dev(
    uint16_t *data_size); 


/**
 * @brief Flush/clear the data stream - no data stored or returned 
 * 
 * @details 
 * 
 * @param max_buff_len 
 * @param stream_len 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_flush_ds_dev(
    uint16_t max_buff_len, 
    uint16_t stream_len); 


/**
 * @brief Read the whole data stream and store the data 
 * 
 * @details 
 * 
 * @param stream_len 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_read_sort_ds_dev(
    uint16_t stream_len); 


/**
 * @brief Read data from the device 
 * 
 * @details 
 * 
 * @param data_buff 
 * @param data_size 
 * @return I2C_STATUS 
 */
I2C_STATUS m8q_read_dev(
    uint8_t *data_buff, 
    uint16_t data_size); 


/**
 * @brief Send a formatted message to the device 
 * 
 * @details 
 * 
 * @param msg 
 * @param msg_size 
 * @return M8Q_STATUS 
 */
M8Q_STATUS m8q_write_msg_dev(
    const void *msg, 
    uint8_t msg_size); 


/**
 * @brief Send messages to the device 
 * 
 * @details 
 * 
 * @param data 
 * @param data_size 
 * @return I2C_STATUS 
 */
I2C_STATUS m8q_write_dev(
    const uint8_t *data, 
    uint8_t data_size); 


/**
 * @brief Start an I2C transmission 
 * 
 * @details 
 * 
 * @param offset 
 * @return I2C_STATUS 
 */
I2C_STATUS m8q_start_trans_dev(
    i2c_rw_offset_t offset); 


/**
 * @brief Message identification 
 * 
 * @details 
 * 
 * @param msg 
 * @param msg_offset 
 * @return M8Q_MSG_TYPE 
 */
M8Q_MSG_TYPE m8q_msg_id_dev(
    const char *msg, 
    uint8_t *msg_offset); 


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
 * @param msg_index 
 * @return uint8_t 
 */
uint8_t m8q_msg_id_check_dev(
    const char *msg, 
    const void *ref_msg, 
    uint8_t num_compare, 
    uint8_t max_size, 
    uint8_t offset, 
    uint8_t *msg_index); 


/**
 * @brief Incoming NMEA message parse 
 * 
 * @details 
 * 
 * @param nmea_msg 
 * @param msg_index 
 * @param stream_len 
 * @param arg_num 
 * @param data 
 */
void m8q_nmea_msg_parse_dev(
    const uint8_t *nmea_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len, 
    uint8_t arg_num, 
    uint8_t **data); 


/**
 * @brief Incoming NMEA message counter 
 * 
 * @details 
 * 
 * @param nmea_msg 
 * @param stream_index 
 * @param stream_len 
 */
void m8q_nmea_msg_count_dev(
    const uint8_t *nmea_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len); 


/**
 * @brief Incoming UBX message parse 
 * 
 * @details 
 * 
 * @param ubx_msg 
 * @param stream_index 
 * @param stream_len 
 */
void m8q_ubx_msg_parse_dev(
    const uint8_t *ubx_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len); 


/**
 * @brief Send NMEA configuration messages 
 * 
 * @details 
 * 
 * @param msg 
 * @param num_args 
 * @param field_offset 
 * @param msg_len 
 */
M8Q_STATUS m8q_nmea_config_dev(
    const char *config_msg, 
    uint8_t num_args, 
    uint8_t field_offset, 
    uint8_t msg_len); 


/**
 * @brief NMEA message checksum calculation 
 * 
 * @details 
 * 
 * @param msg 
 * @return uint16_t 
 */
uint16_t m8q_nmea_checksum_dev(
    const char *msg); 


/**
 * @brief Send UBX configuration messages 
 * 
 * @details 
 * 
 * @param input_msg 
 * @param max_msg_len 
 */
M8Q_STATUS m8q_ubx_config_dev(
    const char *config_msg, 
    uint8_t max_msg_len); 


/**
 * @brief Check for a correctly formatted message ID input 
 * 
 * @details 
 * 
 * @param msg_id 
 * @return uint8_t 
 */
uint8_t ubx_config_id_check_dev(
    const char *msg_id); 


/**
 * @brief Check for a correctly formatted message length input and get the length value 
 * 
 * @details 
 * 
 * @param msg_pl_len 
 * @param pl_len 
 * @return uint8_t 
 */
uint8_t ubx_config_len_check_dev(
    const char *msg_pl_len, 
    uint16_t *pl_len); 


/**
 * @brief Check that the payload is of correct length and format 
 * 
 * @details 
 * 
 * @param msg_payload 
 * @param pl_len 
 * @return uint8_t 
 */
uint8_t ubx_config_payload_check_dev(
    const char *msg_payload, 
    uint16_t pl_len); 


/**
 * @brief Convert the message string into a sendable message 
 * 
 * @details 
 * 
 * @param msg_str 
 * @param msg_data 
 * @param msg_len 
 */
void ubx_config_msg_convert_dev(
    const char *msg_str, 
    uint8_t *msg_data, 
    uint8_t *msg_len); 


/**
 * @brief Check the size and format of a message field 
 * 
 * @details 
 * 
 * @param msg_field 
 * @param byte_count 
 * @param term_char 
 */
void ubx_config_field_check_dev(
    const char *msg_field, 
    uint8_t *byte_count, 
    uint8_t *term_char); 


/**
 * @brief Check for a valid UBX message character input 
 * 
 * @details 
 * 
 * @param msg_char 
 * @return uint8_t 
 */
uint8_t ubx_config_valid_char_dev(
    char msg_char); 


/**
 * @brief Convert two hex string characters to a value 
 * 
 * @details 
 * 
 * @param msg_bytes 
 * @return uint8_t 
 */
uint8_t ubx_config_byte_convert_dev(
    const char *msg_bytes); 

//=======================================================================================


//=======================================================================================
// Initialization (public) - dev 

// Device initialization 
M8Q_STATUS m8q_init_dev(
    I2C_TypeDef *i2c, 
    const char *config_msgs, 
    uint8_t msg_num, 
    uint8_t max_msg_size, 
    uint16_t data_buff_limit)
{
    if ((i2c == NULL) || (config_msgs == NULL))
    {
        return M8Q_INVALID_PTR; 
    }

    // Local varaibles 
    M8Q_STATUS init_status = M8Q_OK; 
    uint16_t ack_status; 
    uint8_t ack_timeout; 

    // Initialize driver data 
    memset((void *)&m8q_driver_data.pos_data, CLEAR, sizeof(m8q_driver_data.pos_data)); 
    memset((void *)&m8q_driver_data.pos_data.lat, ZERO_CHAR, 
           sizeof(m8q_driver_data.pos_data.lat)); 
    memset((void *)&m8q_driver_data.pos_data.lon, ZERO_CHAR, 
           sizeof(m8q_driver_data.pos_data.lon)); 
    memset((void *)&m8q_driver_data.time_data, CLEAR, sizeof(m8q_driver_data.time_data)); 
    m8q_driver_data.i2c = i2c; 
    m8q_driver_data.data_buff_limit = (!data_buff_limit) ? HIGH_16BIT : data_buff_limit; 
    memset((void *)&nmea_msg_target, CLEAR, sizeof(nmea_msg_data_t)); 

    // Check, format and write each message to the device. If there is a problem with a 
    // message then the operation is aborted and the status is returned. If the message 
    // being sent is a UBX CFG message and it successfully writes to the device then the 
    // code looks for an ACK response. 
    for (uint8_t i = CLEAR; i < msg_num; i++)
    {
        init_status = m8q_send_msg_dev(config_msgs, max_msg_size); 

        if (init_status)
        {
            break; 
        }

        if (*(config_msgs + BYTE_6) == 
            *(ubx_msg_class[M8Q_UBX_CFG_INDEX].ubx_msg_class_str + BYTE_1))
        {
            ack_timeout = ACK_TIMEOUT; 

            do
            {
                if (!m8q_read_data_dev())
                {
                    ack_status = m8q_get_ack_status_dev(); 

                    if (ack_status && (ack_status <= HIGH_8BIT))
                    {
                        break; 
                    }
                }
            }
            while (--ack_timeout); 

            if (!ack_timeout)
            {
                init_status = M8Q_INVALID_CONFIG; 
                break; 
            }
        }

        config_msgs += max_msg_size; 
    }

    return init_status; 
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

// Read the data stream and sort/store relevant message data 
M8Q_STATUS m8q_read_data_dev(void)
{
    M8Q_STATUS read_status; 
    uint16_t stream_len = CLEAR; 

    // Clear the ACK and NAK counters so they can be checked for by the application. 
    m8q_driver_data.ack_msg_count = CLEAR; 
    m8q_driver_data.nak_msg_count = CLEAR; 

    // Read the size of the data stream. If it's not zero and the stream isn't greater 
    // than the max buffer size then read the data stream in its entirety and sort all 
    // the read messages. If the data stream size is greater than the max buffer size 
    // then clear the stream data and return a buffer overflow status. 
    read_status = m8q_read_ds_size_dev(&stream_len); 

    if (!read_status)
    {
        read_status = (stream_len > m8q_driver_data.data_buff_limit) ? 
                      m8q_flush_ds_dev(m8q_driver_data.data_buff_limit, stream_len) : 
                      m8q_read_sort_ds_dev(stream_len); 
    }

    return read_status; 
}


// Read and return the data stream contents 
M8Q_STATUS m8q_read_ds_dev(
    uint8_t *data_buff, 
    uint16_t buff_size)
{
    M8Q_STATUS read_status; 
    I2C_STATUS i2c_status; 
    uint16_t stream_len = CLEAR; 

    // Read the size of the data stream. If it's not zero and the stream isn't greater 
    // than the max buffer size then read the data stream in its entirety and copy it 
    // to the buffer passed to the function. If the data stream size is greater than the 
    // max buffer size then clear the stream data and return a buffer overflow status. 
    read_status = m8q_read_ds_size_dev(&stream_len); 

    if (!read_status)
    {
        if (stream_len > buff_size)
        {
            read_status = m8q_flush_ds_dev(buff_size, stream_len); 
        }
        else 
        {
            i2c_status = m8q_read_dev(data_buff, stream_len); 

            if (i2c_status)
            {
                read_status = M8Q_READ_FAULT; 
            }
        }
    }

    return read_status; 
}


// Return the ACK/NAK message counter status 
uint16_t m8q_get_ack_status_dev(void)
{
    uint16_t ack_status = (m8q_driver_data.nak_msg_count << SHIFT_8) | 
                           m8q_driver_data.ack_msg_count; 
    return ack_status; 
}


// Send a message to the device 
M8Q_STATUS m8q_send_msg_dev(
    const char *write_msg, 
    uint8_t max_msg_size)
{
    if (write_msg == NULL)
    {
        return M8Q_INVALID_PTR; 
    }

    // Local varaibles 
    M8Q_MSG_TYPE msg_type = M8Q_MSG_INVALID; 
    M8Q_STATUS init_status = M8Q_OK; 
    uint8_t msg_offset = CLEAR; 

    // Identify the message type. If the message is identifiable then attempt to configure 
    // it and send it to the device. 
    msg_type = m8q_msg_id_dev(write_msg, &msg_offset); 

    if (msg_type == M8Q_MSG_NMEA)
    {
        init_status = m8q_nmea_config_dev(write_msg, 
                                          nmea_msg_target.num_param, 
                                          msg_offset, 
                                          max_msg_size); 
    }
    else if (msg_type == M8Q_MSG_UBX)
    {
        init_status = m8q_ubx_config_dev(write_msg, max_msg_size); 
    }
    else
    {
        init_status = M8Q_INVALID_CONFIG; 
    }

    return init_status; 
}


// Get TX-Ready status 
GPIO_STATE m8q_get_tx_ready_dev(void)
{
    return gpio_read(m8q_driver_data.tx_ready_gpio, (SET_BIT << m8q_driver_data.tx_ready)); 
}


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


// Get latitude coordinate 
double m8q_get_position_lat_dev(void)
{
    // Latitude is formatted as a string with the following characters: DDMM.MMMMM - where 
    // 'D' is a degrees digit and 'M' is a minutes digit. This function converts the string 
    // to a double in degrees only. The below conversion is based on the number of digits 
    // for each part of the coordinate. Based on the format above, there are two digits each 
    // for the integer portion of degrees and minutes, a decimal point, and five digits for 
    // the fractional part of the minutes. Since the final product is a double expressed 
    // in degrees, 'deg_int' holds the conversion of the first two digits and 'deg_frac' 
    // holds the conversion of the minutes which makes up the fractional/decimal part of 
    // the degrees. 

    int32_t deg_int = CLEAR; 
    int32_t deg_frac = CLEAR; 
    uint8_t lat_index = CLEAR; 

    // Convert the integer portion of the degrees and minutes, both of which are two 
    // digit/characters in length. The minutes get converted to a scaled value to 
    // prepare for adding the fractional/decimal part of the minute. 
    do
    {
        deg_int += (int32_t)char_to_int(m8q_driver_data.pos_data.lat[lat_index], 
                                        BYTE_1 - lat_index); 
        deg_frac += (int32_t)char_to_int(m8q_driver_data.pos_data.lat[lat_index + BYTE_2], 
                                         BYTE_6 - lat_index); 
    }
    while (++lat_index < BYTE_2); 

    // Bypass the decimal point character 
    lat_index += BYTE_3; 

    // Convert the fractional part of the minute and add it to the scaled value of minutes. 
    // The latitude string contains 10 bytes so that is where the conversion stops. The 
    // fractional part of the minute is 5 characters in length. 
    do
    {
        deg_frac += (int32_t)char_to_int(m8q_driver_data.pos_data.lat[lat_index], 
                                         BYTE_9 - lat_index); 
    }
    while (++lat_index < BYTE_10); 

    // If the latitude coordinate is in the southern hemisphere then change the sign on the 
    // value. The returned value is meant to be interpretted without needing the N/S 
    // indicator. 
    if (m8q_get_position_NS_dev() == S_UP_CHAR) 
    {
        deg_int = ~deg_int + 1; 
        deg_frac = ~deg_frac + 1; 
    }

    // Calculate and return the final degree value. This requires changing the scaled 
    // minutes value into a fractional/decimal degrees value. 
    return ((double)deg_int) + (((double)deg_frac) / (pow(SCALE_10, BYTE_5)*MIN_TO_DEG)); 
}


// Get latitude coordinate string 
M8Q_STATUS m8q_get_position_lat_str_dev(
    uint8_t *lat_str, 
    uint8_t lat_str_len)
{
    if (lat_str_len < BYTE_10)
    {
        return M8Q_DATA_BUFF_OVERFLOW; 
    }

    memcpy((void *)lat_str, (void *)m8q_driver_data.pos_data.lat, BYTE_10); 
    return M8Q_OK; 
}


// Get North/South hemisphere 
uint8_t m8q_get_position_NS_dev(void)
{
    return m8q_driver_data.pos_data.NS[BYTE_0]; 
}


// Get longitude coordinate 
double m8q_get_position_lon_dev(void)
{
    // Longitude is formatted as a string with the following characters: DDDMM.MMMMM - where 
    // 'D' is a degrees digit and 'M' is a minutes digit. This function converts the string 
    // to a double in degrees only. The below conversion is based on the number of digits 
    // for each part of the coordinate. Based on the format above, there are three digits 
    // each for the integer portion of degrees and two for minutes, a decimal point, and 
    // five digits for the fractional part of the minutes. Since the final product is a 
    // double expressed in degrees, 'deg_int' holds the conversion of the first three digits 
    // and 'deg_frac' holds the conversion of the minutes which makes up the 
    // fractional/decimal part of the degrees. 
    
    int32_t deg_int = CLEAR; 
    int32_t deg_frac = CLEAR; 
    uint8_t lon_index = CLEAR; 

    // Convert the integer portion of the degrees and minutes, both of which are two 
    // digit/characters in length. The minutes get converted to a scaled value to 
    // prepare for adding the fractional/decimal part of the minute. 
    deg_int += (int32_t)char_to_int(m8q_driver_data.pos_data.lon[lon_index], 
                                    BYTE_1 - lon_index); 
    
    while (++lon_index < BYTE_3)
    {
        deg_int += (int32_t)char_to_int(m8q_driver_data.pos_data.lon[lon_index], 
                                        BYTE_2 - lon_index); 
        deg_frac += (int32_t)char_to_int(m8q_driver_data.pos_data.lon[lon_index + BYTE_2], 
                                         BYTE_7 - lon_index); 
    }

    // Bypass the decimal point character 
    lon_index += BYTE_3; 

    // Convert the fractional part of the minute and add it to the scaled value of minutes. 
    // The latitude string contains 10 bytes so that is where the conversion stops. The 
    // fractional part of the minute is 5 characters in length. 
    do
    {
        deg_frac += (int32_t)char_to_int(m8q_driver_data.pos_data.lon[lon_index], 
                                         BYTE_10 - lon_index); 
    }
    while (++lon_index < BYTE_11); 

    // If the longitude coordinate is in the western hemisphere then change the sign on the 
    // value. The returned value is meant to be interpretted without needing the E/W 
    // indicator. 
    if (m8q_get_position_EW_dev() == W_UP_CHAR) 
    {
        deg_int = ~deg_int + 1; 
        deg_frac = ~deg_frac + 1; 
    }

    // Calculate and return the final degree value. This requires changing the scaled 
    // minutes value into a fractional/decimal degrees value. 
    return ((double)deg_int) + (((double)deg_frac) / (pow(SCALE_10, BYTE_5)*MIN_TO_DEG)); 
}


// Get longitude coordinate string 
M8Q_STATUS m8q_get_position_lon_str_dev(
    uint8_t *lon_str, 
    uint8_t lon_str_len)
{
    if (lon_str_len < BYTE_11)
    {
        return M8Q_DATA_BUFF_OVERFLOW; 
    }

    memcpy((void *)lon_str, (void *)m8q_driver_data.pos_data.lon, BYTE_11); 
    return M8Q_OK; 
}


// Get East/West hemisphere 
uint8_t m8q_get_position_EW_dev(void)
{
    return m8q_driver_data.pos_data.EW[BYTE_0]; 
}


// Get navigation status 
uint16_t m8q_get_position_navstat_dev(void)
{
    return (m8q_driver_data.pos_data.navStat[BYTE_0] << SHIFT_8) | 
            m8q_driver_data.pos_data.navStat[BYTE_1]; 
}


// Get acceptable navigation status - returns true for valid position lock, false otherwise 
uint8_t m8q_get_position_navstat_lock_dev(void)
{
    // A valid position lock is indicated by navigation statuses: G2, G3, D2, D3. The status 
    // is a 16-bit value and the 4 most significant bits of the lowest byte is unique to 
    // these valid statuses (0xXX3X). These bits can be checked instead of each status 
    // individually. 
    uint8_t ns = (uint8_t)m8q_get_position_navstat_dev() & FILTER_4_MSB; 
    uint8_t ns_check = M8Q_NAVSTAT_G3 & FILTER_4_MSB; 

    if (ns != ns_check)
    {
        return FALSE; 
    }

    return TRUE; 
}


// Get UTC time 
M8Q_STATUS m8q_get_time_utc_time_dev(
    uint8_t *utc_time, 
    uint8_t utc_time_len)
{
    if (utc_time_len < BYTE_9)
    {
        return M8Q_DATA_BUFF_OVERFLOW; 
    }

    memcpy((void *)utc_time, (void *)m8q_driver_data.time_data.time, BYTE_9); 
    return M8Q_OK; 
}


// Get UTC date 
M8Q_STATUS m8q_get_time_utc_date_dev(
    uint8_t *utc_date, 
    uint8_t utc_date_len)
{
    if (utc_date_len < BYTE_6)
    {
        return M8Q_DATA_BUFF_OVERFLOW; 
    }

    memcpy((void *)utc_date, (void *)m8q_driver_data.time_data.date, BYTE_6); 
    return M8Q_OK; 
}

//=======================================================================================


//=======================================================================================
// Read and write functions (private) 

// Read the data stream size 
M8Q_STATUS m8q_read_ds_size_dev(
    uint16_t *data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 
    uint8_t address = M8Q_REG_0XFD; 
    uint8_t num_bytes[BYTE_2]; 

    // Send the data size register address to the device then read the data high and 
    // low bytes of the data size. 
    i2c_status |= m8q_write_dev(&address, BYTE_1); 
    i2c_status |= m8q_read_dev(num_bytes, BYTE_2); 

    if (i2c_status)
    {
        *data_size = CLEAR; 
        return M8Q_READ_FAULT; 
    }

    // Format the data into the data size 
    *data_size = ((uint16_t)num_bytes[BYTE_0] << SHIFT_8) | (uint16_t)num_bytes[BYTE_1]; 

    if (!(*data_size))
    {
        return M8Q_NO_DATA_AVAILABLE; 
    }

    return M8Q_OK; 
}


// Flush/clear the data stream - no data stored or returned 
M8Q_STATUS m8q_flush_ds_dev(
    uint16_t max_buff_len, 
    uint16_t stream_len)
{
    I2C_STATUS i2c_status; 
    uint8_t stream_data[max_buff_len]; 
    uint16_t stream_index = CLEAR; 
    uint16_t buff_len = max_buff_len; 

    // Read the data stream up until the buffer length and do this repeatedly until 
    // the entire stream has been read. 
    while (stream_index < stream_len)
    {
        if ((stream_len - stream_index) < max_buff_len)
        {
            buff_len = stream_len - stream_index; 
        }

        i2c_status = m8q_read_dev(stream_data, buff_len); 

        if (i2c_status)
        {
            return M8Q_READ_FAULT; 
        }

        stream_index += max_buff_len; 
    }

    return M8Q_DATA_BUFF_OVERFLOW; 
}


// Read the whole data stream and sort/store the data 
M8Q_STATUS m8q_read_sort_ds_dev(
    uint16_t stream_len)
{
    I2C_STATUS i2c_status; 
    uint8_t stream_data[stream_len]; 
    M8Q_MSG_TYPE msg_type = M8Q_MSG_INVALID; 
    uint16_t stream_index = CLEAR; 
    uint8_t msg_offset = CLEAR; 

    // Read the whole data stream 
    i2c_status = m8q_read_dev(stream_data, stream_len); 

    if (i2c_status)
    {
        return M8Q_READ_FAULT; 
    }

    while (stream_index < stream_len)
    {
        // Identify the message type 
        msg_type = m8q_msg_id_dev((char *)&stream_data[stream_index], &msg_offset); 

        // Sort the message data as needed 
        if (msg_type == M8Q_MSG_NMEA)
        {
            stream_index += msg_offset + BYTE_1; 

            m8q_nmea_msg_parse_dev(
                &stream_data[stream_index], 
                &stream_index, 
                stream_len, 
                nmea_msg_target.num_param, 
                nmea_msg_target.msg_data); 
        }
        else if (msg_type == M8Q_MSG_UBX)
        {
            stream_index += BYTE_2; 

            m8q_ubx_msg_parse_dev(
                &stream_data[stream_index], 
                &stream_index, 
                stream_len); 
        }
        else 
        {
            return M8Q_UNKNOWN_DATA; 
        }
    }

    return M8Q_OK; 
}


// Read data from the device 
I2C_STATUS m8q_read_dev(
    uint8_t *data_buff, 
    uint16_t data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Initiate the transmission then read the data 
    i2c_status |= m8q_start_trans_dev(I2C_R_OFFSET); 
    i2c_status |= i2c_read(m8q_driver_data.i2c, data_buff, data_size); 

    return i2c_status; 
}


// Send a formatted message to the device 
M8Q_STATUS m8q_write_msg_dev(
    const void *msg, 
    uint8_t msg_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Send the message to the device and generate a stop condition when done. 
    i2c_status |= m8q_write_dev((uint8_t *)msg, msg_size); 
    i2c_stop(m8q_driver_data.i2c); 

    if (i2c_status)
    {
        return M8Q_WRITE_FAULT; 
    }

    return M8Q_OK; 
}


// Write data to the device 
I2C_STATUS m8q_write_dev(
    const uint8_t *data, 
    uint8_t data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Initiate the transmission and send the data 
    i2c_status |= m8q_start_trans_dev(I2C_W_OFFSET); 
    i2c_status |= i2c_write(m8q_driver_data.i2c, data, data_size); 

    return i2c_status; 
}


// Start an I2C transmission 
I2C_STATUS m8q_start_trans_dev(
    i2c_rw_offset_t offset)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Generate a start condition then send the device address with a read or write 
    // offset. 
    i2c_status |= i2c_start(m8q_driver_data.i2c); 
    i2c_status |= i2c_write_addr(m8q_driver_data.i2c, M8Q_I2C_8_BIT_ADDR + offset); 
    i2c_clear_addr(m8q_driver_data.i2c); 

    return i2c_status;  
}

//=======================================================================================


//=======================================================================================
// Message processing (private) 

// Message identification 
M8Q_MSG_TYPE m8q_msg_id_dev(
    const char *msg, 
    uint8_t *msg_offset)
{
    uint8_t msg_index = CLEAR; 
    uint8_t id_check_status = CLEAR; 

    // Check for the start of an NMEA message 
    if (*msg == nmea_msg_start)
    {
        // Check for a PUBX message ID 
        if (str_compare(nmea_pubx_msg_id, msg, BYTE_1))
        {
            // Check for a valid PUBX message format 
            id_check_status = m8q_msg_id_check_dev(msg, 
                                                   (void *)nmea_pubx_msgs, 
                                                   NMEA_PUBX_NUM_MSGS, 
                                                   sizeof(nmea_msg_format_t), 
                                                   BYTE_6, 
                                                   &msg_index); 
            if (id_check_status)
            {
                *msg_offset = BYTE_8; 
                nmea_msg_target.num_param = nmea_pubx_msgs[msg_index].nmea_msg_data.num_param; 
                nmea_msg_target.msg_data = nmea_pubx_msgs[msg_index].nmea_msg_data.msg_data; 

                return M8Q_MSG_NMEA; 
            }
        }
        else 
        {
            // Check for a standard NMEA message ID 
            id_check_status = m8q_msg_id_check_dev(msg, 
                                                   (void *)&nmea_std_msg_id[0][0], 
                                                   NMEA_STD_ID_NUM, 
                                                   NMEA_STD_ID_LEN, 
                                                   BYTE_1, 
                                                   &msg_index); 
            if (id_check_status)
            {
                // Check for a valid standard NMEA message format 
                id_check_status = m8q_msg_id_check_dev(msg, 
                                                       (void *)nmea_std_msgs, 
                                                       NMEA_STD_NUM_MSGS, 
                                                       sizeof(nmea_msg_format_t), 
                                                       BYTE_3, 
                                                       &msg_index); 
                if (id_check_status)
                {
                    *msg_offset = BYTE_6; 
                    nmea_msg_target.num_param = nmea_std_msgs[msg_index].nmea_msg_data.num_param; 
                    nmea_msg_target.msg_data = nmea_std_msgs[msg_index].nmea_msg_data.msg_data; 

                    return M8Q_MSG_NMEA; 
                }
            }
        }
    }
    // Check for the start of a UBX config message 
    else if (str_compare(ubx_msg_sync_str, msg, BYTE_0))
    {
        // Check for a valid UBX class string 
        id_check_status = m8q_msg_id_check_dev(msg, 
                                               (void *)ubx_msg_class[0].ubx_msg_class_str, 
                                               UBX_CLASS_NUM, 
                                               sizeof(ubx_msg_class_t), 
                                               BYTE_5, 
                                               &msg_index); 
        if (id_check_status)
        {
            return M8Q_MSG_UBX; 
        }
    }
    // Check for the start of a received UBX message 
    else if (str_compare((char *)ubx_msg_sync_data, msg, BYTE_0))
    {
        // Check for a valid UBX class byte 
        id_check_status = m8q_msg_id_check_dev(msg, 
                                               (void *)&ubx_msg_class[0].ubc_msg_class_data, 
                                               UBX_CLASS_NUM, 
                                               sizeof(ubx_msg_class_t), 
                                               BYTE_2, 
                                               &msg_index); 
        if (id_check_status)
        {
            return M8Q_MSG_UBX; 
        }
    }

    return M8Q_MSG_INVALID; 
}


// Message identification helper function 
uint8_t m8q_msg_id_check_dev(
    const char *msg, 
    const void *ref_msg, 
    uint8_t num_compare, 
    uint8_t max_size, 
    uint8_t offset, 
    uint8_t *msg_index)
{
    for (uint8_t i = CLEAR; i < num_compare; i++)
    {
        if (str_compare((char *)(ref_msg + i*max_size), msg, offset))
        {
            *msg_index = i; 
            return TRUE; 
        }
    }

    return FALSE; 
}


// Incoming NMEA message parse 
void m8q_nmea_msg_parse_dev(
    const uint8_t *nmea_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len, 
    uint8_t arg_num, 
    uint8_t **data)
{
    // Local variables 
    uint8_t param_index = CLEAR; 
    uint8_t param_len = CLEAR; 
    uint8_t data_index = CLEAR; 
    uint8_t msg_byte = *nmea_msg; 

    // Check if the message has a data record in the driver. If not, then count the length of 
    // the message and return. 
    if (data == NULL)
    {
        m8q_nmea_msg_count_dev(nmea_msg, stream_index, stream_len); 
        return; 
    }

    // There is a data record to store the message. Offset the message to the first data 
    // byte, get the length of the first data array to fill and start parsing the message 
    // into it's data fields and storing them in the data record. 
    param_len = data[data_index + BYTE_1] - data[data_index]; 

    while ((*stream_index)++ < stream_len)
    {
        // Check for the end of the NMEA message parameters 
        if (msg_byte != AST_CHAR)
        {
            // Check for a comma - a comma is the separation between parameters 
            if (msg_byte != COMMA_CHAR)
            {
                // Message parameter byte seen. Store the byte in the data record if there 
                // is room for it (so not to exceed parameter allocated memory). 
                if (param_index < param_len)
                {
                    data[data_index][param_index++] = msg_byte; 
                }
            }
            else 
            {
                // End of message parameter. If the message data is less than the memory 
                // allocated to store it then terminate the data so old data is not mixed 
                // in. Proceed to check of there are any remainding parameters to fill. 
                if (param_index < param_len)
                {
                    data[data_index][param_index] = NULL_CHAR; 
                }

                if (++data_index >= arg_num)
                {
                    m8q_nmea_msg_count_dev(++nmea_msg, stream_index, stream_len); 
                    break; 
                }

                // If there are additional parameters to store then get the next parameter 
                // storage size and reset the parameter index. 
                param_index = CLEAR; 
                param_len = data[data_index + BYTE_1] - data[data_index]; 
            }
        }
        else 
        {
            // End of message parameters seen. If the message data is less than the memory 
            // allocated to store it then terminate the data so old data is not mixed in. 
            // Storing is done so check the remaining message bytes and exit the loop. 
            if (param_index < param_len)
            {
                data[data_index][param_index] = NULL_CHAR; 
            }

            m8q_nmea_msg_count_dev(++nmea_msg, stream_index, stream_len); 
            break; 
        }

        msg_byte = *(++nmea_msg); 
    }
}


// Incoming NMEA message counter 
void m8q_nmea_msg_count_dev(
    const uint8_t *nmea_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len)
{
    while ((*stream_index)++ < stream_len)
    {
        if (*nmea_msg++ == NL_CHAR)
        {
            break; 
        }
    }
}


// Incoming UBX message parse 
void m8q_ubx_msg_parse_dev(
    const uint8_t *ubx_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len)
{
    uint16_t counter = BYTE_2; 
    uint16_t max_count; 
    uint16_t class_ID = (*ubx_msg << SHIFT_8) | *(ubx_msg + BYTE_1); 

    // Check for an ACK or NAK message. 
    // Right now the driver does not utilize received UBX messages other than for 
    // acknowledging UBX config messages sent to the device so ACK and NAK messages are 
    // checked for manually. If this changes then the method for checking for UBX 
    // message types will also change. 
    if (class_ID == ACK_ACK)
    {
        m8q_driver_data.ack_msg_count++; 
    }
    else if (class_ID == ACK_NAK)
    {
        m8q_driver_data.nak_msg_count++; 
    }

    ubx_msg += BYTE_2; 
    *stream_index += BYTE_4; 
    max_count = ((*ubx_msg) | (*(ubx_msg + BYTE_1) << SHIFT_8)) + BYTE_4; 

    while (*stream_index < stream_len)
    {
        if (counter++ >= max_count)
        {
            break; 
        }
        
        (*stream_index)++; 
    }
}

//=======================================================================================


//=======================================================================================
// Device configuration - config message formatting and writing (private) 

// Send NMEA configuration messages 
M8Q_STATUS m8q_nmea_config_dev(
    const char *config_msg, 
    uint8_t num_args, 
    uint8_t field_offset, 
    uint8_t msg_len)
{
    // Config message validation variables 
    const char *msg_ptr = config_msg + field_offset; 
    char msg_char = CLEAR; 
    uint8_t msg_field_count = CLEAR; 
    uint8_t msg_term_flag = CLEAR; 

    // Config message formatting variables 
    uint16_t checksum = CLEAR; 
    char msg_str[msg_len + M8Q_NMEA_END_MSG]; 
    uint8_t msg_str_len = CLEAR; 

    // Check that a comma separates the address field from the data fields 
    if (*msg_ptr++ != COMMA_CHAR)
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Check the number of message fields, that the fields contain only valid characters, 
    // and that a message termination character ('*') is present. 
    while (*msg_ptr != NULL_CHAR)
    {
        msg_char = *msg_ptr++; 

        // Commas separate message fields 
        if (msg_char == COMMA_CHAR)
        {
            msg_field_count++; 
        }
        // An asterisks signifies the end of the message 
        else if (msg_char == AST_CHAR)
        {
            // The asterisks must be the end of the message 
            if (*msg_ptr == NULL_CHAR)
            {
                msg_field_count++; 
                msg_term_flag++; 
            }
            break; 
        }
        // Check for an invalid character in the config message 
        else if ((msg_char < ZERO_CHAR) || (msg_char > NINE_CHAR))
        {
            if ((msg_char < A_UP_CHAR) || (msg_char > Z_UP_CHAR))
            {
                if ((msg_char < A_LO_CHAR) || (msg_char > Z_LO_CHAR))
                {
                    if ((msg_char != PERIOD_CHAR) && (msg_char != MINUS_CHAR))
                    {
                        return M8Q_INVALID_CONFIG; 
                    }
                }
            }
        }
    }

    if ((!msg_term_flag) || (msg_field_count != num_args))
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Calculate a message checksum and append its character representation onto the 
    // message contents. Send the formatted message to the device. 
    checksum = m8q_nmea_checksum_dev(config_msg); 

    msg_str_len = (uint8_t)sprintf(msg_str, "%s%c%c\r\n", config_msg, 
                                                          (char)(checksum >> SHIFT_8), 
                                                          (char)(checksum)); 
    
    if (msg_str_len < 0)
    {
        return M8Q_INVALID_CONFIG; 
    }

    return m8q_write_msg_dev((void *)msg_str, msg_str_len); 
}


// NMEA message checksum calculation 
uint16_t m8q_nmea_checksum_dev(
    const char *msg)
{
    // Local variables 
    uint16_t xor_result = CLEAR; 
    uint16_t cs_hi_byte = CLEAR; 
    uint16_t cs_lo_byte = CLEAR; 
    uint16_t checksum = CLEAR; 

    // Perform and exlusive OR (XOR) on the NMEA message to calculate the checksum. 
    // Make sure the start character ('$') is ignored. If the code makes it to this 
    // point then the message being checked will have a termination asterisks. 
    while (*(++msg) != AST_CHAR) 
    {
        xor_result ^= (uint16_t)(*msg);
    }

    // Convert the checksum to character representation 
    cs_hi_byte = (xor_result & FILTER_4_MSB) >> SHIFT_4; 
    cs_lo_byte = (xor_result & FILTER_4_LSB); 

    cs_hi_byte = (cs_hi_byte <= MAX_CHAR_DIGIT) ? (cs_hi_byte + NUM_TO_CHAR_OFFSET) : 
                                                  (cs_hi_byte + HEX_TO_LET_CHAR); 
    cs_lo_byte = (cs_lo_byte <= MAX_CHAR_DIGIT) ? (cs_lo_byte + NUM_TO_CHAR_OFFSET) : 
                                                  (cs_lo_byte + HEX_TO_LET_CHAR); 
    
    checksum |= (cs_hi_byte << SHIFT_8) | cs_lo_byte; 

    return checksum; 
}


// Send UBX configuration messages 
M8Q_STATUS m8q_ubx_config_dev(
    const char *config_msg, 
    uint8_t max_msg_len)
{
    // Local variables 
    const char *msg_ptr = config_msg + BYTE_7; 
    uint16_t pl_len = CLEAR; 
    uint8_t msg_data[max_msg_len]; 
    memset((void *)msg_data, CLEAR, sizeof(msg_data)); 
    uint8_t msg_len = CLEAR; 

    // Check that a comma separates the class and ID fields 
    if (*msg_ptr++ != COMMA_CHAR)
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Check for a correctly formatted ID. Note that due to the number of UBX message 
    // IDs per class, and the number of non-unique IDs across classes, the ID is only 
    // checked for formatting and not validity. Inputting an existing ID is left as the 
    // applications responsibility. 
    if (!ubx_config_id_check_dev(msg_ptr))
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Check for a correctly formatted payload length and get its value. This value will 
    // be used to check the payload of the message. 
    msg_ptr += BYTE_3; 
    if (!ubx_config_len_check_dev(msg_ptr, &pl_len))
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Check that the payload matches the specified length and that it's formatted 
    // correctly. Note that the payload contents are not verified. It is left to the 
    // application to set the payload contents as needed. 
    msg_ptr += BYTE_5; 
    if (!ubx_config_payload_check_dev(msg_ptr, pl_len))
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Convert the message string to the correct format, calculate and append the 
    // checksum, and calculate the total formatted message length. 
    ubx_config_msg_convert_dev(config_msg, msg_data, &msg_len); 

    // Send the formatted message to the device 
    return m8q_write_msg_dev((void *)msg_data, msg_len); 
}


// Check for a correctly formatted message ID input 
uint8_t ubx_config_id_check_dev(
    const char *msg_id)
{
    uint8_t byte_count = CLEAR; 
    uint8_t comma_term = CLEAR; 

    ubx_config_field_check_dev(msg_id, &byte_count, &comma_term); 

    if ((byte_count != BYTE_2) || (comma_term != COMMA_CHAR))
    {
        return FALSE; 
    }

    return TRUE; 
}


// Check for a correctly formatted message length input and get the length value 
uint8_t ubx_config_len_check_dev(
    const char *msg_pl_len, 
    uint16_t *pl_len)
{
    uint8_t byte_count = CLEAR; 
    uint8_t comma_term = CLEAR; 
    const char *len_str0 = msg_pl_len; 
    const char *len_str1 = msg_pl_len + BYTE_2; 
    uint8_t len_byte0, len_byte1; 
    
    ubx_config_field_check_dev(msg_pl_len, &byte_count, &comma_term); 

    if ((byte_count != BYTE_4) || (comma_term != COMMA_CHAR))
    {
        return FALSE; 
    }

    // Calculate the value to the length 
    len_byte0 = ubx_config_byte_convert_dev(len_str0); 
    len_byte1 = ubx_config_byte_convert_dev(len_str1); 
    *pl_len = ((len_byte1 << SHIFT_8) | len_byte0); 

    return TRUE; 
}


// Check that the payload is of correct length and format 
uint8_t ubx_config_payload_check_dev(
    const char *msg_payload, 
    uint16_t pl_len)
{
    uint8_t pl_count = CLEAR; 
    uint8_t char_count = CLEAR; 
    uint8_t term_char = CLEAR; 
    uint8_t asterisk_term = CLEAR; 

    // Go through the config message payload and check that each data field has an even 
    // number of bytes, each data field is terminated by a comma or asterisks, there are no 
    // invalid characters in the message and that the number of bytes in the payload matches 
    // the specified payload length. 
    // The payload gets looped through one data field at a time and each data field must 
    // have at least one byte (or two message characters) in it. The payload length specifies 
    // the number of bytes in the payload so if every payload field has one byte then the 
    // specified payload length is the maximum number of times we need to loop through the 
    // payload. 

    for (uint8_t i = CLEAR; i < pl_len; i++)
    {
        ubx_config_field_check_dev(msg_payload, &char_count, &term_char); 

        if (!term_char)
        {
            // Invalid character or end of string seen before a valid termination 
            // character. 
            return FALSE; 
        }
        else 
        {
            // Data field has valid characters and a proper termination. Check that the 
            // number of message characters is even (two message characters create one 
            // formatted message byte), and if so then increment the payload counter and 
            // check if the end of the message has been reached. 
            
            if (char_count % 2)
            {
                return FALSE; 
            }

            pl_count += (char_count / 2); 
            
            if (term_char == AST_CHAR)
            {
                asterisk_term++; 
                break; 
            }
            
            // 'char_count' will account of the message data field and the +1 moves the 
            // message pointer past the termination character. 
            msg_payload += char_count + 1; 
            char_count = CLEAR; 
            term_char = CLEAR; 
        }
    }

    if ((pl_count != pl_len) || (!asterisk_term))
    {
        return FALSE; 
    }

    return TRUE; 
}


// Convert the message string into a sendable message 
void ubx_config_msg_convert_dev(
    const char *msg_str, 
    uint8_t *msg_data, 
    uint8_t *msg_len)
{
    // Local variables 
    uint8_t *msg_data_copy = msg_data; 
    uint8_t checksum_len = CLEAR; 
    uint8_t checksum_A = CLEAR; 
    uint8_t checksum_B = CLEAR; 

    // Convert the message string into a valid format 
    while (*msg_str != AST_CHAR)
    {
        if (*msg_str == COMMA_CHAR)
        {
            msg_str++; 
        }
        else 
        {
            *msg_data_copy++ = ubx_config_byte_convert_dev(msg_str); 
            msg_str += 2; 
            (*msg_len)++; 
        }
    }

    // Calculate the checksum. Exclude the sync characters from the calculation. See the 
    // datasheet for information about calculating the checksum. 
    msg_data += BYTE_2; 
    checksum_len = *msg_len - BYTE_2; 

    for (uint8_t i = CLEAR; i < checksum_len; i++)
    {
        checksum_A += *msg_data++; 
        checksum_B += checksum_A; 
    }

    // The last two bytes of the message are the checksum 
    *msg_data_copy++ = checksum_A; 
    *msg_data_copy = checksum_B; 

    // Add two more to the message length to account for the checksum 
    *msg_len += BYTE_2; 
}


// Check the size and format of a message field 
void ubx_config_field_check_dev(
    const char *msg_field, 
    uint8_t *byte_count, 
    uint8_t *term_char)
{
    while (*msg_field != NULL_CHAR)
    {
        if (*msg_field == COMMA_CHAR)
        {
            *term_char = COMMA_CHAR; 
            break; 
        }

        if (*msg_field == AST_CHAR)
        {
            *term_char = AST_CHAR; 
            break; 
        }

        if (!ubx_config_valid_char_dev(*msg_field)) 
        {
            break; 
        }
        
        (*byte_count)++; 
        msg_field++; 
    }
}


// Check for a valid UBX message character input 
uint8_t ubx_config_valid_char_dev(
    char msg_char)
{
    // UBX configuration messages only contain hexadecimal characters (i.e. 
    // "0123456789ABCDF") because UBX messages are made up of only integer values. 
    // Negative and floating point values can still be sent as UBX payloads in an integer 
    // representation. How payloads are interpretted depends on the message. 

    if ((msg_char < ZERO_CHAR) || (msg_char > NINE_CHAR))
    {
        if ((msg_char < A_UP_CHAR) || (msg_char > F_UP_CHAR))
        {
            return FALSE; 
        }
    }

    return TRUE; 
}


// Convert two hex string characters to a value 
uint8_t ubx_config_byte_convert_dev(
    const char *msg_bytes)
{
    uint8_t byte0 = *msg_bytes++; 
    uint8_t byte1 = *msg_bytes; 
    uint8_t nibble0, nibble1; 

    nibble1 = (byte0 <= NINE_CHAR) ? byte0 - NUM_TO_CHAR_OFFSET : 
                                     byte0 - HEX_TO_LET_CHAR; 
    nibble0 = (byte1 <= NINE_CHAR) ? byte1 - NUM_TO_CHAR_OFFSET : 
                                     byte1 - HEX_TO_LET_CHAR; 

    return ((nibble1 << SHIFT_4) | nibble0); 
}

//=======================================================================================






// Old Code 






//=======================================================================================
// Enums 

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
// Prototypes 

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
    if (!arg_num) 
    {
        return; 
    }

    // Increment to the first data field 
    msg += start_byte; 

    // Calculate the first data field length 
    arg_len = *(&data[data_index] + 1) - data[data_index];  

    // Read and parse the message 
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
                if (arg_index < arg_len) 
                {
                    data[data_index][arg_index] = NULL_CHAR; 
                } 

                // Increment jagged array index 
                data_index++; 

                // Exit if the storage array has been exceeded 
                if (data_index >= arg_num) 
                {
                    break; 
                } 

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
            if (arg_index < arg_len) 
            {
                data[data_index][arg_index] = NULL_CHAR; 
            }
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

                for (uint8_t i = 0; i < M8Q_NMEA_END_MSG; i++) 
                {
                    *msg_ptr++ = (uint8_t)term_str[i]; 
                }

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
        if (checksum_char <= MAX_CHAR_DIGIT) checksum_char += NUM_TO_CHAR_OFFSET; 
        else checksum_char += HEX_TO_LET_CHAR; 
        checksum |= (uint16_t)(checksum_char << SHIFT_8*(1-i)); 
    }

    return checksum; 
}


// UBX config function 
void m8q_ubx_config(
    uint8_t *input_msg)
{
    // Local variables 
    uint16_t m8q_status = M8Q_FAULT_NONE; 
    uint8_t *msg_ptr = input_msg; 
    uint8_t config_msg[M8Q_MSG_MAX_LEN];   // Formatted UBX message to send to the receiver 
    uint16_t checksum = CLEAR; 
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

                for (uint8_t i = 0; i < BYTE_4; i++) 
                {
                    *msg_ptr++ = ZERO_CHAR; 
                } 

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
                    if (input_msg_len > BYTE_17) 
                    {
                        msg_len_pl_check = input_msg_len - BYTE_17; 
                    }

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
                        if (!M8Q_USER_CONFIG) 
                        {
                            m8q_status |= (SET_BIT << M8Q_FAULT_UBX_RESP); 
                        }
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

                if ((!msg_index) || char_count || (comma_count == 2)) 
                {
                    break; 
                }
            }

            msg_index++; 
        }
        else
        {
            if (!char_count) 
            {
                status = M8Q_UBX_MSG_CONV_SUCC; 
            }
            
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
