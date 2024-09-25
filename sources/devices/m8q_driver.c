/**
 * @file m8q_driver.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS driver 
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
#include "i2c_comm.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Device parameters 
#define M8Q_I2C_8_BIT_ADDR 0x84   // Receiver I2C address (default : 0x42 << 1) 

// Register addresses 
#define M8Q_REG_0XFD 0xFD         // Available data bytes (high byte) register 

// NMEA message format 
#define NMEA_START 0x24           // '$' --> NMEA protocol start character 
#define NMEA_PUBX_NUM_MSGS 5      // Number of NMEA PUBX messages 
#define NMEA_STD_ID_NUM 5         // Number of standard NMEA message IDs 
#define NMEA_STD_ID_LEN 3         // Maximum length of a standard NMEA message ID 
#define NMEA_STD_NUM_MSGS 19      // Number of standard NMEA formatters 
#define NMEA_MSG_FORMAT_LEN 4     // Maximum length of an NMEA message formatter 
#define M8Q_NMEA_END_MSG 6        // Length of string to append to NMEA message after payload 

// UBX message format 
#define UBX_CLASS_NUM 14          // Number of UBX classes 
#define UBX_CLASS_LEN 3           // Maximum length of a UBX class 
#define UBX_CFG_INDEX 4           // Location of CFG message info in ubx_msg_class 

// UBX ACK class message 
#define ACK_ACK 0x0501            // ACK-ACK message class and ID 
#define ACK_NAK 0x0500            // ACK-NAK message class and ID 
#define ACK_TIMEOUT 10            // ACK message check counter before timeout 

// Other 
#define EOM_BYTE 1                // End of memory byte - helps find size of message fields 

//=======================================================================================


//=======================================================================================
// Enums 

// M8Q message type 
typedef enum {
    M8Q_MSG_INVALID, 
    M8Q_MSG_NMEA, 
    M8Q_MSG_UBX, 
    M8Q_MSG_RTCM 
} m8q_msg_type_t; 


// Number of fields in an NMEA message 
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
// Stored messages 

// NMEA POSITION message fields  
typedef struct m8q_nmea_pos_s 
{
    uint8_t time    [BYTE_9];     // UTC time 
    uint8_t lat     [BYTE_11];    // Latitude 
    uint8_t NS      [BYTE_1];     // North/South indicator 
    uint8_t lon     [BYTE_12];    // Longitude 
    uint8_t EW      [BYTE_1];     // East/West indicator 
    uint8_t altRef  [BYTE_9];     // Altitude above user datum ellipsoid 
    uint8_t navStat [BYTE_2];     // Navigation status 
    uint8_t hAcc    [BYTE_5];     // Horizontal accuracy estimate 
    uint8_t vAcc    [BYTE_5];     // Vertical accuracy estimate 
    uint8_t SOG     [BYTE_9];     // Speed over ground 
    uint8_t COG     [BYTE_6];     // Course over ground 
    uint8_t vVel    [BYTE_6];     // Vertical velocity (+ downwards) 
    uint8_t diffAge [BYTE_1];     // Age of differential corrections 
    uint8_t HDOP    [BYTE_5];     // Horizontal dilution of precision 
    uint8_t VDOP    [BYTE_5];     // Vertical dilution of precision 
    uint8_t TDOP    [BYTE_5];     // Time dilution of precision 
    uint8_t numSvs  [BYTE_2];     // Number of satellites used in the navigation solution 
    uint8_t res     [BYTE_1];     // Reserved --> 0 
    uint8_t DR      [BYTE_1];     // DR used 
    uint8_t eom     [BYTE_1];     // End of memory --> used for finding field length 
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
    uint8_t eom      [BYTE_1];     // End of memory --> used for finding field length 
} 
m8q_nmea_time_t;

//=======================================================================================


//=======================================================================================
// Data record 

// Driver data record 
typedef struct m8q_driver_data_s
{
    // Peripherals 
    I2C_TypeDef *i2c; 
    GPIO_TypeDef *pwr_save_gpio; 
    GPIO_TypeDef *tx_ready_gpio; 

    // Pins 
    pin_selector_t pwr_save;       // Interrupt pin (for low power mode) 
    pin_selector_t tx_ready;       // TX-Ready pin 
    
    // Messages 
    m8q_nmea_pos_t pos_data;       // POSITION message 
    m8q_nmea_time_t time_data;     // TIME message 
    uint8_t ack_msg_count;         // ACK-ACK message counter 
    uint8_t nak_msg_count;         // ACK-NAK message counter 

    // Other 
    uint16_t data_buff_limit; 
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
static uint8_t *position[NMEA_NUM_FIELDS_POSITION + EOM_BYTE] = 
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
static uint8_t *time[NMEA_NUM_FIELDS_TIME + EOM_BYTE] = 
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
// Datatypes 

typedef uint8_t M8Q_MSG_TYPE; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Flush/clear the M8Q data stream 
 * 
 * @details Reads all the data in the device stream but doesn't store any of it. This 
 *          function is used when the data stream size is larger than the maximum 
 *          specified buffer size. The data stream size that can be read from the device 
 *          indicates the combined size of all messages, not the number or length of 
 *          individual messages, meaning you can't choose to read individual messages 
 *          using the data stream length alone. Because of this, you must read the whole 
 *          data stream, and if the available buffer is too small then it's likely 
 *          message data will be lost reading to the length of the buffer. Instead, 
 *          the data stream is purposely flushed or cleared so it will be zero and 
 *          reading can continue. 
 *          
 *          Note that if this function successfully reads the data stream then an 
 *          overflow status will be returned. This is for use by the parent function 
 *          to indicate to a user that data was flushed. 
 * 
 * @param stream_len : length of the data stream 
 * @return M8Q_STATUS : status of the read operation 
 */
M8Q_STATUS m8q_flush_ds(uint16_t stream_len); 


/**
 * @brief Read the M8Q data stream and store the data 
 * 
 * @details This function reads the whole data stream from the device, then identifies 
 *          each message in the stream and calls the appropriate message parsing function 
 *          as needed. If the message does not match any known message types then the 
 *          function returns an unknown data status. This function is called by 
 *          m8q_read_data if the data stream size is within the maximum allowed buffer 
 *          size. 
 *          
 *          This function is separate from m8q_read_data because a stack allocated buffer 
 *          the size of the data stream needs to be created but can only be done once the 
 *          data stream size is known. m8q_read_data reads the stream size and passes it 
 *          to this function which can then create the buffer. 
 * 
 * @see m8q_read_data 
 * @see m8q_msg_id 
 * @see m8q_nmea_msg_parse 
 * @see m8q_ubx_msg_parse 
 * 
 * @param stream_len : length of the data stream 
 * @return M8Q_STATUS : status of the read operation 
 */
M8Q_STATUS m8q_read_sort_ds(uint16_t stream_len); 


/**
 * @brief Read data from the device 
 * 
 * @details Reads data of a given size from the device using the I2C driver functions 
 *          and stores it in the buffer. Called by numerous functions that require 
 *          data from the device. 
 * 
 * @param data_buff : buffer to store the read data 
 * @param data_size : data size to be read from the device 
 * @return I2C_STATUS : status of the I2C read operation 
 */
I2C_STATUS m8q_read(
    uint8_t *data_buff, 
    uint16_t data_size); 


/**
 * @brief Send a formatted message to the device 
 * 
 * @details Used specifically for sending messages to the device. Stops the I2C 
 *          transmission after sending the data. Uses the m8q_write function to 
 *          send the message. 
 * 
 * @see m8q_write 
 * 
 * @param msg : buffer that contains the formatted message to send 
 * @param msg_size : size of the message being sent 
 * @return M8Q_STATUS : status of the write operation 
 */
M8Q_STATUS m8q_write_msg(
    const void *msg, 
    uint8_t msg_size); 


/**
 * @brief Write data to the device 
 * 
 * @details Writes data of a given size from the buffer to the device using the I2C 
 *          driver functions. Called when needing to specify the device register 
 *          address or when sending a message to the device. 
 * 
 * @param data : buffer that contains the data to send 
 * @param data_size : size of the data to send 
 * @return I2C_STATUS : status of the I2C write operation 
 */
I2C_STATUS m8q_write(
    const uint8_t *data, 
    uint8_t data_size); 


/**
 * @brief Start an I2C transmission 
 * 
 * @details Starts an I2C transmission which is used by both read and write operations. 
 *          The offset is used in the I2C address to set up either a read or write 
 *          operation. 
 * 
 * @see i2c_rw_offset_t 
 * 
 * @param offset : read or write offset to be used with the device I2C address 
 * @return I2C_STATUS : status of the I2C operations 
 */
I2C_STATUS m8q_start_trans(i2c_rw_offset_t offset); 


/**
 * @brief Message identification 
 * 
 * @details Takes in either an incoming or outgoing message and attempts to identify 
 *          the type of message based on the message processing information above. 
 *          This helps to determine if a message is valid before using it but also 
 *          provides message information if there is a match. For example, for 
 *          incoming NMEA messages, if the message has a data record in the driver 
 *          then a pointer to the data record is set. 
 *          
 *          For NMEA messages, the byte at which the message payload starts changes 
 *          depending on if it's a PUBX or standard message. This offset is set in 
 *          msg_offset if the message type is NMEA. 
 * 
 * @param msg : buffer that stores the message 
 * @param msg_offset : Byte at which to start processing a valid message 
 * @return M8Q_MSG_TYPE : returns the message type - see m8q_msg_type_t 
 */
M8Q_MSG_TYPE m8q_msg_id(
    const char *msg, 
    uint8_t *msg_offset); 


/**
 * @brief Message identification helper function 
 * 
 * @details Used exclusively by m8q_msg_id to help compare a message against message 
 *          types. If a match is found then this function returns true. 
 * 
 * @see m8q_msg_id 
 * 
 * @param msg : buffer that contains the messages 
 * @param ref_msg : buffer that stores the message type to check the message against 
 * @param num_compare : number of message types to check 
 * @param max_size : size of the data type that stores ref_msg - used for incrementing 
 * @param offset : point in the message at which to start the comparison (bytes) 
 * @param msg_index : index of the reference message where a match was found 
 * @return uint8_t : returns true for match, false otherwise 
 */
uint8_t m8q_msg_id_check(
    const char *msg, 
    const void *ref_msg, 
    uint8_t num_compare, 
    uint8_t max_size, 
    uint8_t offset, 
    uint8_t *msg_index); 


/**
 * @brief Incoming NMEA message parse 
 * 
 * @details If an incoming NMEA message is identified in the data stream then this 
 *          function is called to process the message. If the message does not have a 
 *          data record within the driver then no processing is done but the message 
 *          length is counted to update the data stream index. If there is a data 
 *          record then the message payload is sorted and stored in the record 
 *          so it can be used by the driver getter functions. 
 * 
 * @see m8q_read_sort_ds 
 * @see m8q_nmea_msg_count 
 * 
 * @param nmea_msg : buffer for the data stream that points to the NMEA message 
 * @param msg_index : data stream index - gets updated with processing 
 * @param stream_len : max length of the data stream 
 * @param arg_num : number of message payload parameters 
 * @param data : pointer to the data record buffer that can store message data 
 */
void m8q_nmea_msg_parse(
    const uint8_t *nmea_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len, 
    uint8_t arg_num, 
    uint8_t **data); 


/**
 * @brief Incoming NMEA message counter 
 * 
 * @details Counts the remaining length of the NMEA message being processed so the 
 *          data stream index can be updated to the next message. Used only by 
 *          m8q_nmea_msg_parse to count a message with no data record or count 
 *          the remaining bytes after the payload of a message with a data record. 
 * 
 * @see m8q_nmea_msg_parse 
 * 
 * @param nmea_msg : buffer for the data stream that points to the NMEA message 
 * @param stream_index : data stream index 
 * @param stream_len : max length of the data stream 
 */
void m8q_nmea_msg_count(
    const uint8_t *nmea_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len); 


/**
 * @brief Incoming UBX message parse 
 * 
 * @details If an incoming UBX message is identified in the data stream then this 
 *          function is called to process the message. Right now the only processing 
 *          done by this function is checking if the message is an ACK or NAK message 
 *          as a confirmation of CFG messages sent to the device. All UBX messages 
 *          have their length counted so the data stream index can be updated. 
 * 
 * @param ubx_msg : buffer for the data stream that points to the UBX message 
 * @param stream_index : data stream index 
 * @param stream_len : max length of the data stream 
 */
void m8q_ubx_msg_parse(
    const uint8_t *ubx_msg, 
    uint16_t *stream_index, 
    uint16_t stream_len); 


/**
 * @brief Send NMEA configuration messages 
 * 
 * @details Takes in an NMEA configuration message from the application level, checks 
 *          the formatting of the message, then finds the checksum and terminates the 
 *          message before sending it to the device. If the message is not formatted 
 *          correctly then the function will abort and the message will not be sent. 
 *          The purpose of this function is to ensure the device is not set up 
 *          incorrectly due to an inproper message format. See m8q_send_msg for more 
 *          info on formatted messages. 
 *          
 *          Note: In this context "configuration message" is not referring to CFG 
 *                messages specifically. It's simply referring to messages from the 
 *                user that get sent to the device that set up the device as needed, 
 *                which could include CFG messages. 
 * 
 * @see m8q_send_msg 
 * 
 * @param config_msg : buffer that contains the message 
 * @param num_args : number of parameters in the message payload 
 * @param field_offset : message payload offset from beginning of message 
 * @param max_msg_len : max possible length of the message 
 */
M8Q_STATUS m8q_nmea_config(
    const char *config_msg, 
    uint8_t num_args, 
    uint8_t field_offset, 
    uint8_t max_msg_len); 


/**
 * @brief NMEA message checksum calculation 
 * 
 * @details Calculates and returns the NMEA message checksum for m8q_nmea_config. Refer 
 *          to the device interface manual for details on how the checksum is determined. 
 * 
 * @see m8q_nmea_config 
 * 
 * @param msg : buffer that contains the NMEA message 
 * @return uint16_t : 2-byte checksum 
 */
uint16_t m8q_nmea_checksum(const char *msg); 


/**
 * @brief Send UBX configuration messages 
 * 
 * @details Takes in a UBX configuration message from the application level, checks the 
 *          formatting of the message, converts the message data then finds the checksum 
 *          and terminates the message before sending it to the device. If the message 
 *          is not formatted correctly then the function will abort and the message will 
 *          not be sent. The purpose of this function is to ensure the device is not set 
 *          up incorrectly due to an inproper message format. 
 * 
 *          Note that NMEA messages use ASCII representation but UBX messages do not. UBX 
 *          configuration messages are formatted in ASCII form so they're easier for the 
 *          user to read and write. Before sending the message, the message must be 
 *          converted from ASCII. See m8q_send_msg for more info on formatted messages. 
 *          
 *          Note: In this context "configuration message" is not referring to CFG 
 *                messages specifically. It's simply referring to messages from the 
 *                user that get sent to the device that set up the device as needed, 
 *                which could include CFG messages. 
 * 
 * @see m8q_send_msg 
 * 
 * @param config_msg : buffer that contains the message 
 * @param max_msg_len : max possible length of the message 
 */
M8Q_STATUS m8q_ubx_config(
    const char *config_msg, 
    uint8_t max_msg_len); 


/**
 * @brief UBX message ID check 
 * 
 * @details Checks that the UBX message ID is formatted correctly. Returns true if it is. 
 * 
 * @see m8q_ubx_config 
 * @see ubx_config_field_check 
 * 
 * @param msg_id : buffer that contains the message ID 
 * @return uint8_t : status of the check - true if the ID is correctly formatted 
 */
uint8_t ubx_config_id_check(const char *msg_id); 


/**
 * @brief UBX message payload length check 
 * 
 * @details Checks that the UBX message payload length field is formatted correctly then 
 *          converts the string to a length value and stores it in 'pl_len'. Returns 
 *          true if the format is correct. 
 * 
 * @see m8q_ubx_config 
 * @see ubx_config_field_check 
 * 
 * @param msg_pl_len : buffer that contains the payload length 
 * @param pl_len : buffer to store the converted length value 
 * @return uint8_t : status of the check - true if the length is correctly formatted 
 */
uint8_t ubx_config_len_check(
    const char *msg_pl_len, 
    uint16_t *pl_len); 


/**
 * @brief UBX message payload check 
 * 
 * @details Checks that the UBX message payload is formatted correctly and that the 
 *          amount of data matches the payload length found by ubx_config_len_check. 
 *          Returns true if the format is correct. 
 * 
 * @see m8q_ubx_config 
 * @see ubx_config_len_check 
 * @see ubx_config_field_check 
 * 
 * @param msg_payload : buffer that contains the message payload 
 * @param pl_len : message payload length 
 * @return uint8_t : status of the check - true if the payload is correctly formatted 
 */
uint8_t ubx_config_payload_check(
    const char *msg_payload, 
    uint16_t pl_len); 


/**
 * @brief UBX message convert 
 * 
 * @details If this function is called it means the config message is correctly formatted. 
 *          This function converts the UBX message to its proper format and calculates the 
 *          message checksum. After this, the complete formatted message gets sent to the 
 *          device. 
 *          
 *          Note that NMEA messages use ASCII representation but UBX messages do not. UBX 
 *          configuration messages are formatted in ASCII form so they're easier for the 
 *          user to read and write. Before sending the message, the message must be 
 *          converted from ASCII. See m8q_send_msg for more info on formatted messages. 
 * 
 * @see m8q_ubx_config 
 * 
 * @param msg_str : buffer that contains the config message 
 * @param msg_data : buffer that stores the formatted message 
 * @param msg_len : buffer that stores the formatted message length 
 */
void ubx_config_msg_convert(
    const char *msg_str, 
    uint8_t *msg_data, 
    uint8_t *msg_len); 


/**
 * @brief UBX message field check 
 * 
 * @details Used by various message field check functions to checks for valid UBX 
 *          configuration message characters and counts the length of the message field. 
 *          UBX config messages only contain hexidecimal characters (i.e. 
 *          "0123456789ABCDF") for data, commas to separate message fields, and an 
 *          asterisks to signify the end of a message. Fields must be terminated with 
 *          either a comma (",") or an asterisks ("*"). 
 * 
 * @param msg_field : buffer that contains the message field to check 
 * @param byte_count : counter that keeps track of the field length 
 * @param term_char : gets set to the termination character seen 
 */
void ubx_config_field_check(
    const char *msg_field, 
    uint8_t *byte_count, 
    uint8_t *term_char); 


/**
 * @brief UBX message data character check 
 * 
 * @details Used by ubx_config_field_check to check if a message data character is within 
 *          "A"-"F", or "0"-"9". The function will return true if the data character is 
 *          within bounds. 
 * 
 * @see ubx_config_field_check 
 * 
 * @param msg_char : UBX message character/byte 
 * @return uint8_t : Status of the check - true if character is within bounds 
 */
uint8_t ubx_config_valid_char(char msg_char); 


/**
 * @brief UBX message byte conversion 
 * 
 * @details Converts ASCII represented hexidecimal numbers into their byte value. UBX 
 *          config messages are formatted in ASCII to allow the user to better read 
 *          and write out messages. However, actualy UBX messages are not ASCII 
 *          formatted and must be converted to a usable value. This function takes in 
 *          two characters that represent the low and high nibbles of a hexidecimal 
 *          number and converts them to their number form (ex. "A8" --> 0xA8). 
 * 
 * @param msg_bytes : buffer that stores two characters that represent a number 
 * @return uint8_t : converted value 
 */
uint8_t ubx_config_byte_convert(const char *msg_bytes); 

//=======================================================================================


//=======================================================================================
// Initialization 

// Device initialization 
M8Q_STATUS m8q_init(
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
        init_status = m8q_send_msg(config_msgs, max_msg_size); 

        if (init_status)
        {
            break; 
        }

        if (*(config_msgs + BYTE_6) == 
            *(ubx_msg_class[UBX_CFG_INDEX].ubx_msg_class_str + BYTE_1))
        {
            ack_timeout = ACK_TIMEOUT; 

            do
            {
                if (!m8q_read_data())
                {
                    ack_status = m8q_get_ack_status(); 

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


// Low power pin initialization 
M8Q_STATUS m8q_pwr_pin_init(
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
    m8q_clear_low_pwr(); 

    return M8Q_OK; 
}


// TX ready pin initialization 
M8Q_STATUS m8q_txr_pin_init(
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
// User functions 

// Read and store relevant message data 
M8Q_STATUS m8q_read_data(void)
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
    read_status = m8q_read_ds_size(&stream_len); 

    if (!read_status)
    {
        read_status = (stream_len > m8q_driver_data.data_buff_limit) ? 
                      m8q_flush_ds(stream_len) : m8q_read_sort_ds(stream_len); 
    }

    return read_status; 
}


// Read and return the data stream contents 
M8Q_STATUS m8q_read_ds(
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
    read_status = m8q_read_ds_size(&stream_len); 

    if (!read_status)
    {
        if (stream_len >= buff_size)
        {
            read_status = m8q_flush_ds(stream_len); 
        }
        else 
        {
            i2c_status = m8q_read(data_buff, stream_len); 

            if (i2c_status)
            {
                read_status = M8Q_READ_FAULT; 
            }

            data_buff += stream_len; 
            *data_buff = NULL_CHAR; 
        }
    }

    return read_status; 
}


// Read data stream size 
M8Q_STATUS m8q_read_ds_size(uint16_t *data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 
    uint8_t address = M8Q_REG_0XFD; 
    uint8_t num_bytes[BYTE_2]; 

    // Send the data size register address to the device then read the data high and 
    // low bytes of the data size. 
    i2c_status |= m8q_write(&address, BYTE_1); 
    i2c_status |= m8q_read(num_bytes, BYTE_2); 

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


// Return the ACK/NAK message counter status 
uint16_t m8q_get_ack_status(void)
{
    uint16_t ack_status = (m8q_driver_data.nak_msg_count << SHIFT_8) | 
                           m8q_driver_data.ack_msg_count; 
    return ack_status; 
}


// Send a message to the device 
M8Q_STATUS m8q_send_msg(
    const char *write_msg, 
    uint8_t max_msg_size)
{
    if (write_msg == NULL)
    {
        return M8Q_INVALID_PTR; 
    }

    M8Q_MSG_TYPE msg_type = M8Q_MSG_INVALID; 
    M8Q_STATUS init_status = M8Q_OK; 
    uint8_t msg_offset = CLEAR; 

    // Identify the message type. If the message is identifiable then attempt to configure 
    // it and send it to the device. 
    msg_type = m8q_msg_id(write_msg, &msg_offset); 

    if (msg_type == M8Q_MSG_NMEA)
    {
        init_status = m8q_nmea_config(write_msg, 
                                      nmea_msg_target.num_param, 
                                      msg_offset, 
                                      max_msg_size); 
    }
    else if (msg_type == M8Q_MSG_UBX)
    {
        init_status = m8q_ubx_config(write_msg, max_msg_size); 
    }
    else
    {
        init_status = M8Q_INVALID_CONFIG; 
    }

    return init_status; 
}


// Get TX ready status 
GPIO_STATE m8q_get_tx_ready(void)
{
    return gpio_read(m8q_driver_data.tx_ready_gpio, (SET_BIT << m8q_driver_data.tx_ready)); 
}


// Enter low power mode 
void m8q_set_low_pwr(void)
{
    gpio_write(m8q_driver_data.pwr_save_gpio, (SET_BIT << m8q_driver_data.pwr_save), GPIO_LOW); 
}


// Exit low power mode 
void m8q_clear_low_pwr(void)
{
    gpio_write(m8q_driver_data.pwr_save_gpio, (SET_BIT << m8q_driver_data.pwr_save), GPIO_HIGH); 
}


// Get latitude coordinate 
double m8q_get_position_lat(void)
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
    if (m8q_get_position_NS() == S_UP_CHAR) 
    {
        deg_int = ~deg_int + 1; 
        deg_frac = ~deg_frac + 1; 
    }

    // Calculate and return the final degree value. This requires changing the scaled 
    // minutes value into a fractional/decimal degrees value. 
    return ((double)deg_int) + (((double)deg_frac) / (pow(SCALE_10, BYTE_5)*MIN_TO_DEG)); 
}


// Get latitude coordinate string 
M8Q_STATUS m8q_get_position_lat_str(
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
uint8_t m8q_get_position_NS(void)
{
    return m8q_driver_data.pos_data.NS[BYTE_0]; 
}


// Get longitude coordinate 
double m8q_get_position_lon(void)
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
                                    BYTE_2 - lon_index); 
    
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
    if (m8q_get_position_EW() == W_UP_CHAR) 
    {
        deg_int = ~deg_int + 1; 
        deg_frac = ~deg_frac + 1; 
    }

    // Calculate and return the final degree value. This requires changing the scaled 
    // minutes value into a fractional/decimal degrees value. 
    return ((double)deg_int) + (((double)deg_frac) / (pow(SCALE_10, BYTE_5)*MIN_TO_DEG)); 
}


// Get longitude coordinate string 
M8Q_STATUS m8q_get_position_lon_str(
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
uint8_t m8q_get_position_EW(void)
{
    return m8q_driver_data.pos_data.EW[BYTE_0]; 
}


// Get navigation status 
uint16_t m8q_get_position_navstat(void)
{
    return (m8q_driver_data.pos_data.navStat[BYTE_0] << SHIFT_8) | 
            m8q_driver_data.pos_data.navStat[BYTE_1]; 
}


// Get acceptable navigation status 
uint8_t m8q_get_position_navstat_lock(void)
{
    // A valid position lock is indicated by navigation statuses: G2, G3, D2, D3. The status 
    // is a 16-bit value and the 4 most significant bits of the lowest byte is unique to 
    // these valid statuses (0xXX3X). These bits can be checked instead of each status 
    // individually. 
    uint8_t ns = (uint8_t)m8q_get_position_navstat() & FILTER_4_MSB; 
    uint8_t ns_check = M8Q_NAVSTAT_G3 & FILTER_4_MSB; 

    if (ns != ns_check)
    {
        return FALSE; 
    }

    return TRUE; 
}

//=======================================================================================
// 
//=======================================================================================
// Get speed over ground (SOG) value 
uint32_t m8q_get_position_sog(void)
{
    uint32_t sog = CLEAR; 
    uint8_t sog_index = CLEAR, sog_len = CLEAR; 

    // Find the size of the SOG string 
    while (m8q_driver_data.pos_data.SOG[sog_index] != NULL_CHAR)
    {
        if (m8q_driver_data.pos_data.SOG[sog_index++] != PERIOD_CHAR)
        {
            sog_len++; 
        }
    }

    sog_index = CLEAR; 

    // Convert the SOG string to a scaled integer 
    while (sog_len)
    {
        if (m8q_driver_data.pos_data.SOG[sog_index] != PERIOD_CHAR)
        {
            sog += char_to_int(m8q_driver_data.pos_data.SOG[sog_index], --sog_len); 
        }

        sog_index++; 
    }
    
    return sog; 
}


// Get speed over ground (SOG) string 
M8Q_STATUS m8q_get_position_sog_str(
    uint8_t *sog_str, 
    uint8_t sog_str_len)
{
    uint8_t sog_size = (uint8_t)sizeof(m8q_driver_data.pos_data.SOG); 

    if (sog_str_len < sog_size)
    {
        return M8Q_DATA_BUFF_OVERFLOW; 
    }

    memcpy((void *)sog_str, (void *)m8q_driver_data.pos_data.SOG, sog_size); 
    return M8Q_OK; 
}


// Get UTC time 
M8Q_STATUS m8q_get_time_utc_time(
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
M8Q_STATUS m8q_get_time_utc_date(
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
// Read and write functions 

// Flush/clear the M8Q data stream 
M8Q_STATUS m8q_flush_ds(uint16_t stream_len)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Initiate the transmission then get the data stream without storing any data 
    i2c_status |= m8q_start_trans(I2C_R_OFFSET); 
    i2c_status |= i2c_clear(m8q_driver_data.i2c, stream_len); 

    if (i2c_status)
    {
        return M8Q_READ_FAULT; 
    }

    return M8Q_DATA_BUFF_OVERFLOW; 
}


// Read the M8Q data stream and store the data 
M8Q_STATUS m8q_read_sort_ds(uint16_t stream_len)
{
    I2C_STATUS i2c_status; 
    uint8_t stream_data[stream_len]; 
    M8Q_MSG_TYPE msg_type = M8Q_MSG_INVALID; 
    uint16_t stream_index = CLEAR; 
    uint8_t msg_offset = CLEAR; 

    // Read the whole data stream 
    i2c_status = m8q_read(stream_data, stream_len); 

    if (i2c_status)
    {
        return M8Q_READ_FAULT; 
    }

    while (stream_index < stream_len)
    {
        // Identify the message type 
        msg_type = m8q_msg_id((char *)&stream_data[stream_index], &msg_offset); 

        // Sort the message data as needed 
        if (msg_type == M8Q_MSG_NMEA)
        {
            stream_index += msg_offset + BYTE_1; 

            m8q_nmea_msg_parse(
                &stream_data[stream_index], 
                &stream_index, 
                stream_len, 
                nmea_msg_target.num_param, 
                nmea_msg_target.msg_data); 
        }
        else if (msg_type == M8Q_MSG_UBX)
        {
            stream_index += BYTE_2; 

            m8q_ubx_msg_parse(
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
I2C_STATUS m8q_read(
    uint8_t *data_buff, 
    uint16_t data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Initiate the transmission then read the data 
    i2c_status |= m8q_start_trans(I2C_R_OFFSET); 
    i2c_status |= i2c_read(m8q_driver_data.i2c, data_buff, data_size); 

    return i2c_status; 
}


// Send a formatted message to the device 
M8Q_STATUS m8q_write_msg(
    const void *msg, 
    uint8_t msg_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Send the message to the device and generate a stop condition when done. 
    i2c_status |= m8q_write((uint8_t *)msg, msg_size); 
    i2c_stop(m8q_driver_data.i2c); 

    if (i2c_status)
    {
        return M8Q_WRITE_FAULT; 
    }

    return M8Q_OK; 
}


// Write data to the device 
I2C_STATUS m8q_write(
    const uint8_t *data, 
    uint8_t data_size)
{
    I2C_STATUS i2c_status = I2C_OK; 

    // Initiate the transmission and send the data 
    i2c_status |= m8q_start_trans(I2C_W_OFFSET); 
    i2c_status |= i2c_write(m8q_driver_data.i2c, data, data_size); 

    return i2c_status; 
}


// Start an I2C transmission 
I2C_STATUS m8q_start_trans(i2c_rw_offset_t offset)
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
// Message processing 

// Message identification 
M8Q_MSG_TYPE m8q_msg_id(
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
            id_check_status = m8q_msg_id_check(msg, 
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
            id_check_status = m8q_msg_id_check(msg, 
                                               (void *)&nmea_std_msg_id[0][0], 
                                               NMEA_STD_ID_NUM, 
                                               NMEA_STD_ID_LEN, 
                                               BYTE_1, 
                                               &msg_index); 
            if (id_check_status)
            {
                // Check for a valid standard NMEA message format 
                id_check_status = m8q_msg_id_check(msg, 
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
        id_check_status = m8q_msg_id_check(msg, 
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
        id_check_status = m8q_msg_id_check(msg, 
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
uint8_t m8q_msg_id_check(
    const char *msg, 
    const void *ref_msg, 
    uint8_t num_compare, 
    uint8_t max_size, 
    uint8_t offset, 
    uint8_t *msg_index)
{
    for (uint8_t i = CLEAR; i < num_compare; i++)
    {
        if (str_compare((char *)ref_msg + i*max_size, msg, offset))
        {
            *msg_index = i; 
            return TRUE; 
        }
    }

    return FALSE; 
}


// Incoming NMEA message parse 
void m8q_nmea_msg_parse(
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
        m8q_nmea_msg_count(nmea_msg, stream_index, stream_len); 
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
                    m8q_nmea_msg_count(++nmea_msg, stream_index, stream_len); 
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

            m8q_nmea_msg_count(++nmea_msg, stream_index, stream_len); 
            break; 
        }

        msg_byte = *(++nmea_msg); 
    }
}


// Incoming NMEA message counter 
void m8q_nmea_msg_count(
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
void m8q_ubx_msg_parse(
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
// Device configuration - config message formatting and writing 

// Send NMEA configuration messages 
M8Q_STATUS m8q_nmea_config(
    const char *config_msg, 
    uint8_t num_args, 
    uint8_t field_offset, 
    uint8_t max_msg_len)
{
    // Config message validation variables 
    const char *msg_ptr = config_msg + field_offset; 
    char msg_char = CLEAR; 
    uint8_t msg_field_count = CLEAR; 
    uint8_t msg_term_flag = CLEAR; 

    // Config message formatting variables 
    uint16_t checksum = CLEAR; 
    char msg_str[max_msg_len + M8Q_NMEA_END_MSG]; 
    int16_t msg_str_len = CLEAR; 

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
    checksum = m8q_nmea_checksum(config_msg); 

    msg_str_len = sprintf(msg_str, "%s%c%c\r\n", config_msg, 
                                                 (char)(checksum >> SHIFT_8), 
                                                 (char)(checksum)); 
    
    if (msg_str_len < 0)
    {
        return M8Q_INVALID_CONFIG; 
    }

    return m8q_write_msg((void *)msg_str, (uint8_t)msg_str_len); 
}


// NMEA message checksum calculation 
uint16_t m8q_nmea_checksum(const char *msg)
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
M8Q_STATUS m8q_ubx_config(
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
    if (!ubx_config_id_check(msg_ptr))
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Check for a correctly formatted payload length and get its value. This value will 
    // be used to check the payload of the message. 
    msg_ptr += BYTE_3; 
    if (!ubx_config_len_check(msg_ptr, &pl_len))
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Check that the payload matches the specified length and that it's formatted 
    // correctly. Note that the payload contents are not verified. It is left to the 
    // application to set the payload contents as needed. 
    msg_ptr += BYTE_5; 
    if (!ubx_config_payload_check(msg_ptr, pl_len))
    {
        return M8Q_INVALID_CONFIG; 
    }

    // Convert the message string to the correct format, calculate and append the 
    // checksum, and calculate the total formatted message length. 
    ubx_config_msg_convert(config_msg, msg_data, &msg_len); 

    // Send the formatted message to the device 
    return m8q_write_msg((void *)msg_data, msg_len); 
}


// UBX message ID check 
uint8_t ubx_config_id_check(const char *msg_id)
{
    uint8_t byte_count = CLEAR; 
    uint8_t comma_term = CLEAR; 

    ubx_config_field_check(msg_id, &byte_count, &comma_term); 

    if ((byte_count != BYTE_2) || (comma_term != COMMA_CHAR))
    {
        return FALSE; 
    }

    return TRUE; 
}


// UBX message payload length check 
uint8_t ubx_config_len_check(
    const char *msg_pl_len, 
    uint16_t *pl_len)
{
    uint8_t byte_count = CLEAR; 
    uint8_t comma_term = CLEAR; 
    const char *len_str0 = msg_pl_len; 
    const char *len_str1 = msg_pl_len + BYTE_2; 
    uint8_t len_byte0, len_byte1; 
    
    ubx_config_field_check(msg_pl_len, &byte_count, &comma_term); 

    if ((byte_count != BYTE_4) || (comma_term != COMMA_CHAR))
    {
        return FALSE; 
    }

    // Calculate the value to the length 
    len_byte0 = ubx_config_byte_convert(len_str0); 
    len_byte1 = ubx_config_byte_convert(len_str1); 
    *pl_len = ((len_byte1 << SHIFT_8) | len_byte0); 

    return TRUE; 
}


// UBX message payload check 
uint8_t ubx_config_payload_check(
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
        ubx_config_field_check(msg_payload, &char_count, &term_char); 

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
            
            // 'char_count' will account for the message data field and the +1 moves the 
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


// UBX message convert 
void ubx_config_msg_convert(
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
            *msg_data_copy++ = ubx_config_byte_convert(msg_str); 
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


// UBX message field check 
void ubx_config_field_check(
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

        if (!ubx_config_valid_char(*msg_field)) 
        {
            break; 
        }
        
        (*byte_count)++; 
        msg_field++; 
    }
}


// UBX message data character check 
uint8_t ubx_config_valid_char(char msg_char)
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


// UBX message byte conversion 
uint8_t ubx_config_byte_convert(const char *msg_bytes)
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
