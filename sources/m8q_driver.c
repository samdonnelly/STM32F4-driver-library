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

// Device drivers 
#include "m8q_driver.h"

//=======================================================================================


// TODO 
// - Reduce the amount of terminal outputs 
// - m8q_check_data_size is providing unreliable results - troubleshoot 
//     - Maybe I'm not using it correctly? 
// - Remove the pointer to buffer in m8q_read --> the point of the data records and 
//   getters is to only get the information you need, not the while string. Create a 
//   global (to this driver only) record to store UBC response messages (ubx_config) so 
//   you can get rid of the pointer to buffer argument. 


//=======================================================================================
// Message processing functions 

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

//=======================================================================================


//=======================================================================================
// Message configuration functions 

/**
 * @brief M8Q NMEA config function 
 * 
 * @details 
 * 
 * @param i2c : pointer to the I2C port used 
 * @param msg : pointer to the input message buffer  
 */
void m8q_nmea_config(
    I2C_TypeDef *i2c, 
    uint8_t *msg); 


/**
 * @brief M8Q NMEA checkusm calculation 
 * 
 * @details Calculates the NMEA config message checksum to be sent along with the message 
 *          to the receiver using an exlusive OR (XOR) operation on all bytes of the 
 *          message string. 
 * 
 * @param msg : pointer to message buffer  
 * @return CHECKSUM : checksum of an NMEA message to be sent to the receiver 
 */
CHECKSUM m8q_nmea_checksum(
    uint8_t *msg); 


/**
 * @brief M8Q UBX config function 
 * 
 * @details 
 * 
 * @param i2c : pointer to the I2C port used 
 * @param input_msg : pointer to the input message buffer 
 */
void m8q_ubx_config(
    I2C_TypeDef *i2c, 
    uint8_t *input_msg); 


/**
 * @brief M8Q UBX message conversion 
 * 
 * @details 
 * 
 * @param i2c 
 * @param input_msg_len 
 * @param input_msg_start 
 * @param input_msg 
 * @param new_msg_byte_count 
 * @param new_msg 
 * @return UBX_MSG_STATUS 
 */
UBX_MSG_STATUS m8q_ubx_msg_convert(
    I2C_TypeDef *i2c, 
    uint8_t input_msg_len,
    uint8_t input_msg_start, 
    uint8_t *input_msg, 
    uint16_t *new_msg_byte_count, 
    uint8_t *new_msg); 


/**
 * @brief M8Q UBX checksum calculation 
 * 
 * @details 
 * 
 * @param msg : pointer to message buffer 
 * @param len : Length of checksum calculation range 
 * @return CHECKSUM : checksum of a UBX message to be sent to the receiver 
 */
CHECKSUM m8q_ubx_checksum(
    uint8_t *msg, 
    uint16_t len); 

//=======================================================================================


//=======================================================================================
// Messages 

// NMEA POSITION message fields  
typedef struct m8q_nmea_pos_s 
{
    uint8_t time    [BYTE_9];     // UTC time 
    uint8_t lat     [BYTE_11];    // Latitude 
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
    uint8_t numSvs  [BYTE_2];     // Number of satellites ued in the navigation solution 
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


// NMEA message data 
typedef struct m8q_msg_data_s
{
    m8q_nmea_pos_t  pos_data;     // POSITION message 
    m8q_nmea_time_t time_data;    // TIME message 
} 
m8q_msg_data_t; 


// NMEA message data instance 
static m8q_msg_data_t m8q_msg_data; 


// NMEA POSITION message 
static uint8_t* position[M8Q_NMEA_POS_ARGS+1] = 
{ 
    m8q_msg_data.pos_data.time, 
    m8q_msg_data.pos_data.lat, 
    m8q_msg_data.pos_data.NS, 
    m8q_msg_data.pos_data.lon, 
    m8q_msg_data.pos_data.EW, 
    m8q_msg_data.pos_data.altRef, 
    m8q_msg_data.pos_data.navStat, 
    m8q_msg_data.pos_data.hAcc, 
    m8q_msg_data.pos_data.vAcc,
    m8q_msg_data.pos_data.SOG,
    m8q_msg_data.pos_data.COG,
    m8q_msg_data.pos_data.vVel,
    m8q_msg_data.pos_data.diffAge,
    m8q_msg_data.pos_data.HDOP,
    m8q_msg_data.pos_data.VDOP,
    m8q_msg_data.pos_data.TDOP,
    m8q_msg_data.pos_data.numSvs,
    m8q_msg_data.pos_data.res,
    m8q_msg_data.pos_data.DR, 
    m8q_msg_data.pos_data.eom 
}; 

// NMEA TIME message 
static uint8_t* time[M8Q_NMEA_TIME_ARGS+1] = 
{ 
    m8q_msg_data.time_data.time, 
    m8q_msg_data.time_data.date, 
    m8q_msg_data.time_data.utcTow, 
    m8q_msg_data.time_data.utcWk, 
    m8q_msg_data.time_data.leapSec, 
    m8q_msg_data.time_data.clkBias, 
    m8q_msg_data.time_data.clkDrift, 
    m8q_msg_data.time_data.tpGran, 
    m8q_msg_data.time_data.eom 
}; 

//=======================================================================================


//=======================================================================================
// Initialization 

// M8Q initialization 
void m8q_init(
    I2C_TypeDef *i2c, 
    uint8_t msg_num, 
    uint8_t msg_index, 
    uint8_t *config_msgs)
{
    // TODO consider making a file for general purpose inits that apply to everything 
    // Enable GPIOC clock - RCC_AHB1ENR register, bit 2 - this is needed for GPIO to work 
    RCC->AHB1ENR |= (SET_BIT << SHIFT_2);

    // Configure a GPIO output for low power mode 
    // TODO figure out which GPIO to use that also works 
    gpio_init(GPIOC, PIN_10, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO);
    gpio_write(GPIOC, GPIOX_PIN_10, GPIO_HIGH);

    // Configure a GPIO input for TX_READY 
    gpio_init(GPIOC, PIN_11, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PD);

    // Send configuration messages 
    for (uint8_t i = 0; i < msg_num; i++)
    {
        // Print message to terminal for verification 
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, "Config message: "); 
        uart_sendstring(USART2, (char *)(config_msgs + i*msg_index)); 
        uart_send_new_line(USART2); 

        // Identify the message type 
        switch (*(config_msgs + i*msg_index))
        {
            case M8Q_NMEA_START:  // NMEA message 
                m8q_nmea_config(i2c, (config_msgs + i*msg_index)); 
                break;

            case M8Q_UBX_SYNC1:  // UBX message 
                m8q_ubx_config(i2c, (config_msgs + i*msg_index)); 
                break;
            
            default:  // Unknown config message 
                uart_sendstring(USART2, "Unknown message type.\r\n"); 
                break;
        }
    }
}

//=======================================================================================


//=======================================================================================
// NMEA Read 

// Read a message from the M8Q 
M8Q_READ_STAT m8q_read(
    I2C_TypeDef *i2c, 
    uint8_t *data)
{    
    // Local variables 
    M8Q_READ_STAT read_status = M8Q_READ_INVALID; 
    uint8_t data_check = 0; 

    // Check for a valid data stream 
    m8q_check_data_stream(i2c, &data_check); 

    switch (data_check)
    {
        case M8Q_NO_DATA:  // No data stream available 
            break;

        case M8Q_NMEA_START:  // Start of NMEA message 
            // Capture the byte checked in the message response 
            *data++ = data_check;

            // Generate a start condition 
            i2c_start(i2c); 

            // Send the device address with a read offset 
            i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET); 
            i2c_clear_addr(i2c);  

            // Read the rest of the data stream until "\r\n" 
            i2c_read_to_term(i2c, data, M8Q_NMEA_END_PAY, I2C_4_BYTE); 

            // Parse the message data into its data record 
            m8q_nmea_sort(data); 

            read_status = M8Q_READ_NMEA; 
            break;
        
        case M8Q_UBX_START:  // Start of UBX message 
            // Capture the byte checked in the message response 
            *data++ = data_check; 

            // Generate a start condition 
            i2c_start(i2c); 

            // Send the device address with a read offset 
            i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET); 
            i2c_clear_addr(i2c); 

            // Read the rest of the UBX message 
            i2c_read_to_len(i2c, 
                            M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET, 
                            data, 
                            M8Q_UBX_LENGTH_OFST-BYTE_1, 
                            M8Q_UBX_LENGTH_LEN, 
                            M8Q_UBX_CS_LEN); 

            read_status = M8Q_READ_UBX; 
            break; 

        default:  // Unknown data stream 
            break;
    }

    return read_status; 
}


// Read the data stream size 
void m8q_check_data_size(
    I2C_TypeDef *i2c, 
    uint16_t *data_size)
{
    // Local variables 
    uint8_t num_bytes[BYTE_2];        // Store the high and low byte of the data size 
    uint8_t address = M8Q_REG_0XFD;   // Address of high byte for the data size 

    // Generate a start condition 
    i2c_start(i2c); 

    // Write the slave address with write access 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + I2C_W_OFFSET); 
    i2c_clear_addr(i2c); 

    // Send the first data size register address to start reading from there 
    i2c_write_master_mode(i2c, &address, I2C_1_BYTE); 

    // Generate another start condition 
    i2c_start(i2c); 

    // Send the device address again with a read offset 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET);  
    i2c_clear_addr(i2c); 

    // Read the data size registers 
    i2c_read_master_mode(i2c, num_bytes, I2C_2_BYTE); 

    // Format the data into the data size 
    *data_size = (uint16_t)((num_bytes[BYTE_0] << SHIFT_8) | num_bytes[BYTE_1]); 
}


// Read the current value at the data stream register 
void m8q_check_data_stream(
    I2C_TypeDef *i2c, 
    uint8_t *data_check)
{
    // Generate a start condition 
    i2c_start(i2c); 

    // Send the device address with a read offset 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + I2C_R_OFFSET); 
    i2c_clear_addr(i2c); 

    // Read the first byte of the data stream 
    i2c_read_master_mode(i2c, data_check, I2C_1_BYTE); 
}

//=======================================================================================


//=======================================================================================
// Write 

// M8Q write 
void m8q_write(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t data_size)
{
    // Generate a start condition 
    i2c_start(I2C1); 

    // Send the device address with a write offset 
    i2c_write_address(I2C1, M8Q_I2C_8_BIT_ADDR + I2C_W_OFFSET); 
    i2c_clear_addr(I2C1); 

    // Send data (at least 2 bytes) 
    i2c_write_master_mode(I2C1, data, data_size); 

    // Generate a stop condition 
    i2c_stop(I2C1); 
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
    uint8_t msg_len = 0; 

    // Calculate message size 
    while (*msg++ != term_char) msg_len++; 

    return msg_len; 
}


// M8Q NMEA message sort 
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


// M8Q NMEA message parse 
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

    // // Calculate the first data field length 
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
                if (arg_index < arg_len) data[data_index][arg_index] = NULL_CHAR; 

                // Increment jagged array index 
                data_index++; 

                // Exit if the storage array has been exceeded 
                if (data_index >= arg_num) break; 

                // Reset arg index and calculate the new argument length 
                arg_index = 0; 
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
// Getters 

// M8Q TX-Ready getter 
uint8_t m8q_get_tx_ready(void)
{
    return gpio_read(GPIOC, GPIOX_PIN_11); 
}


// M8Q latitude getter 
void m8q_get_lat(uint16_t *deg_min, uint32_t *min_frac)
{
    // Local variables 
    uint8_t deg_min_array[M8Q_COO_DATA_LEN];          // Integer portion of the minute 
    uint8_t min_frac_array[M8Q_COO_DATA_LEN+BYTE_1];  // Fractional part of the minute 
    uint8_t lat_length = 2*M8Q_COO_DATA_LEN + BYTE_1;

    // Copy the latitude into integer and fractional parts 
    for (uint8_t i = 0; i < lat_length; i++)
    {
        if (i < (M8Q_COO_DATA_LEN-BYTE_1))
            deg_min_array[i] = position[M8Q_POS_LAT][i]; 
        
        else if (i == (M8Q_COO_DATA_LEN-BYTE_1))
            deg_min_array[i] = NULL_CHAR; 
        
        else if (i < (2*M8Q_COO_DATA_LEN))
            min_frac_array[i-M8Q_COO_DATA_LEN] = position[M8Q_POS_LAT][i]; 
        
        else
            min_frac_array[i-M8Q_COO_DATA_LEN] = NULL_CHAR; 
    }

    // Convert each number 
    sscanf((char *)deg_min_array, "%hu", deg_min); 
    sscanf((char *)min_frac_array, "%lu", min_frac); 
}


// M8Q North/South getter 
uint8_t m8q_get_NS(void)
{
    return *(position[M8Q_POS_NS]); 
}


// M8Q longitude getter 
void m8q_get_long(uint16_t *deg_min, uint32_t *min_frac)
{
    // Local variables 
    uint8_t deg_min_array[M8Q_COO_DATA_LEN+BYTE_1];   // Integer portion of the minute 
    uint8_t min_frac_array[M8Q_COO_DATA_LEN+BYTE_1];  // Fractional part of the minute 
    uint8_t lat_length = 2*M8Q_COO_DATA_LEN + BYTE_2;

    // Copy the latitude into integer and fractional parts 
    for (uint8_t i = 0; i < lat_length; i++)
    {
        if (i < M8Q_COO_DATA_LEN)
            deg_min_array[i] = position[M8Q_POS_LON][i]; 
        
        else if (i == M8Q_COO_DATA_LEN)
            deg_min_array[i] = NULL_CHAR; 
        
        else if (i < (2*M8Q_COO_DATA_LEN + BYTE_1))
            min_frac_array[i-(M8Q_COO_DATA_LEN + BYTE_1)] = position[M8Q_POS_LON][i]; 
        
        else
            min_frac_array[i-(M8Q_COO_DATA_LEN + BYTE_1)] = NULL_CHAR; 
    }

    // Convert each number 
    sscanf((char *)deg_min_array, "%hu", deg_min); 
    sscanf((char *)min_frac_array, "%lu", min_frac); 
}


// M8Q East/West getter 
uint8_t m8q_get_EW(void)
{
    return *(position[M8Q_POS_EW]);
}


// M8Q navigation status getter 
uint16_t m8q_get_navstat(void)
{
    // Local variables 
    uint16_t navstat_high = 0; 
    uint16_t navstat_low = 0; 

    // Format the status 
    navstat_high = position[M8Q_POS_NAVSTAT][BYTE_0] << SHIFT_8; 
    navstat_low = position[M8Q_POS_NAVSTAT][BYTE_1]; 

    return (navstat_high | navstat_low); 
}


// M8Q time getter 
void m8q_get_time(uint8_t *utc_time)
{
    for (uint8_t i = 0; i < 9; i++)
        *utc_time++ = time[M8Q_TIME_TIME][i]; 
}


// M8Q date getter 
void m8q_get_date(uint8_t *utc_date)
{
    for (uint8_t i = 0; i < 6; i++)
        *utc_date++ = time[M8Q_TIME_DATE][i]; 
}

//=======================================================================================


//=======================================================================================
// Setters 

// M8Q Low Power Mode setter 
void m8q_set_low_power(gpio_pin_state_t pin_state)
{
    gpio_write(GPIOC, GPIOX_PIN_10, pin_state);
}

//=======================================================================================


//=======================================================================================
// User Configuration Mode 

#if M8Q_USER_CONFIG

// M8Q user configuration 
void m8q_user_config(
    I2C_TypeDef *i2c)
{
    // Local variables 
    uint8_t config_msg[2*M8Q_CONFIG_MSG]; 

    // Check if there is user input waiting 
    if (uart_data_ready(USART2))
    {
        // Read the input 
        uart_getstr(USART2, (char *)config_msg, UART_STR_TERM_CARRIAGE); 

        uart_send_new_line(USART2); 

        // Identify the message type 
        switch (config_msg[0])
        {
            case M8Q_NMEA_START:  // NMEA message 
                m8q_nmea_config(i2c, config_msg); 
                break;

            case M8Q_UBX_SYNC1:  // UBX message 
                m8q_ubx_config(i2c, config_msg); 
                break;
            
            default:  // Unknown input 
                uart_send_new_line(USART2); 
                uart_sendstring(USART2, "Unknown message type\r\n"); 
                break;
        }

        // Prompt the user for the next message 
        m8q_nmea_config_ui();
    }
}


// M8Q NMEA config user interface 
void m8q_nmea_config_ui(void)
{
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, ">>> Config message: "); 
}

#endif   // M8Q_USER_CONFIG

//=======================================================================================


//=======================================================================================
// Message configuration functions 

// M8Q NMEA config function 
void m8q_nmea_config(
    I2C_TypeDef *i2c, 
    uint8_t *msg)
{
    // Local variables 
    uint8_t *msg_ptr = msg; 
    uint8_t msg_args = 0; 
    uint8_t msg_arg_count = 0; 
    uint8_t msg_arg_mask = 0; 
    CHECKSUM checksum = 0; 
    char term_str[M8Q_NMEA_END_MSG]; 

    // Check message header and ID 
    if (str_compare("$PUBX,", (char *)msg, BYTE_0))
    {
        // RATE (ID=40) 
        if (str_compare("40,", (char *)msg, BYTE_6)) 
            msg_args = M8Q_NMEA_RATE_ARGS; 
        
        // CONFIG (ID=41) 
        else if (str_compare("41,", (char *)msg, BYTE_6)) 
            msg_args = M8Q_NMEA_CONFIG_ARGS; 

        // Unsupported message ID 
        else
            uart_sendstring(USART2, "Unsupported PUBX message ID.\r\n");

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

                // Pass the message along to the NMEA send function 
                m8q_write(I2C1, msg, m8q_message_size(msg, NULL_CHAR));

                // Send confirmation message to terminal 
                uart_sendstring(USART2, "NMEA configuration message sent.\r\n"); 
            } 
            else
                uart_sendstring(USART2, "Invalid formatting of PUBX message.\r\n"); 
        }
    }
    else 
        uart_sendstring(USART2, "Only PUBX messages are supported.\r\n"); 
}


// M8Q NMEA message calculation 
CHECKSUM m8q_nmea_checksum(
    uint8_t *msg)
{
    // Local variables 
    CHECKSUM checksum = 0; 
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
        if (checksum_char <= HEX_NUM_TO_LET) checksum_char += HEX_TO_NUM_CHAR; 
        else checksum_char += HEX_TO_LET_CHAR; 
        checksum |= (uint16_t)(checksum_char << SHIFT_8*(1-i)); 
    }

    return checksum; 
}


// M8Q UBX config function 
void m8q_ubx_config(
    I2C_TypeDef *i2c, 
    uint8_t *input_msg)
{
    // Local variables 
    uint8_t *msg_ptr = input_msg;         // Copy of pointer to input message buffer 
    uint8_t config_msg[M8Q_CONFIG_MSG];   // Formatted UBX message to send to the receiver 
    uint8_t resp_msg[M8Q_CONFIG_MSG];     // UBX message response from the receiver 
    CHECKSUM checksum = 0;                // Formatted UBX message checksum 

    // User inputs 
    uint8_t input_msg_len = m8q_message_size(input_msg, CR_CHAR); 
    uint8_t msg_id_input = 0; 
    uint8_t pl_len_input[M8Q_UBX_LENGTH_LEN]; 
    uint8_t pl_arg_input[M8Q_CONFIG_MSG]; 

    // Formatters 
    uint16_t pl_len = 0; 
    uint16_t byte_count = 0; 
    uint8_t format_ok = 0; 

    // Check the sync characters and class 
    if (str_compare("B5,62,06,", (char *)input_msg, BYTE_0))
    {
        // Validate the ID formatting 
        if (m8q_ubx_msg_convert(i2c, BYTE_2, BYTE_9, input_msg, &byte_count, &msg_id_input))
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
                if (m8q_ubx_msg_convert(i2c, BYTE_4, BYTE_12, 
                                        input_msg, &byte_count, pl_len_input))
                {
                    // Format the payload length 
                    pl_len = (pl_len_input[1] << SHIFT_8) | pl_len_input[0]; 

                    // Check the argument format 
                    if (input_msg_len < BYTE_17) 
                        input_msg_len++;  // correction for variable payload length 
                    
                    byte_count = 0;  // Reset the byte count to check payload length 
                    
                    if (m8q_ubx_msg_convert(i2c, input_msg_len-BYTE_17, BYTE_17,
                                            input_msg, &byte_count, pl_arg_input))
                    {                        
                        if (pl_len == byte_count)
                            format_ok++; 
                        
                        else 
                            uart_sendstring(USART2, "Payload length doesn't match size.\r\n");
                    } 
                    else
                        uart_sendstring(USART2, "Invalid payload format.\r\n");
                }
                else
                    uart_sendstring(USART2, "Invalid payload length format.\r\n");
            }

            if (format_ok)
            {
                // Convert the input message to the proper UBX message format 
                if (m8q_ubx_msg_convert(i2c, input_msg_len, BYTE_0, 
                                        input_msg, &byte_count, config_msg))
                {
                    // Calculate the checksum 
                    checksum = m8q_ubx_checksum(config_msg, M8Q_UBX_MSG_FMT_LEN+pl_len); 

                    // Add the checksum to the end of the message buffer 
                    config_msg[M8Q_UBX_HEADER_LEN+pl_len] = (uint8_t)(checksum >> SHIFT_8); 
                    config_msg[M8Q_UBX_HEADER_LEN+pl_len+BYTE_1] = (uint8_t)(checksum); 

                    // Send the UBX message 
                    m8q_write(i2c, config_msg, M8Q_UBX_HEADER_LEN + pl_len + M8Q_UBX_CS_LEN); 

                    // Read the UBX CFG response 
                    while(m8q_read(i2c, resp_msg) != M8Q_READ_UBX); 

                    // Communicate the results 
                    uart_sendstring(USART2, "UBX configuration message sent.\r\n");

                    pl_len = (resp_msg[M8Q_UBX_LENGTH_OFST+1] << SHIFT_8) | 
                                                        resp_msg[M8Q_UBX_LENGTH_OFST];  

                    if (resp_msg[M8Q_UBX_CLASS_OFST] == M8Q_UBX_ACK_CLASS)
                    {
                        if (resp_msg[M8Q_UBX_ID_OFST] == M8Q_UBX_ACK_ID)
                            uart_sendstring(USART2, "Message acknowledged.\r\n"); 
                        else
                            uart_sendstring(USART2, "Message not acknowledged.\r\n"); 
                    }
                    else
                    {
                        for (uint8_t i = 0; i < (M8Q_UBX_HEADER_LEN+pl_len+M8Q_UBX_CS_LEN); i++)
                        {
                            uart_send_integer(USART2, (int16_t)resp_msg[i]); 
                            uart_send_new_line(USART2);
                        }
                    }
                }
                else
                    uart_sendstring(USART2, "Message conversion failed. Check format.\r\n");
            } 
        } 
        else 
            uart_sendstring(USART2, "Invalid ID format.\r\n"); 
    }
    else
        uart_sendstring(USART2, "Unknown message type.\r\n"); 
}


// M8Q UBX message convert 
UBX_MSG_STATUS m8q_ubx_msg_convert(
    I2C_TypeDef *i2c, 
    uint8_t input_msg_len, 
    uint8_t input_msg_start, 
    uint8_t *input_msg, 
    uint16_t *new_msg_byte_count, 
    uint8_t *new_msg)
{
    // Local variable 
    UBX_MSG_STATUS status = M8Q_UBX_MSG_CONV_FAIL; 
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
                    low_nibble -= HEX_TO_NUM_CHAR; 
                
                else if ((low_nibble >= A_CHAR) && (low_nibble <= F_CHAR)) 
                    low_nibble -= HEX_TO_LET_CHAR; 
                
                else 
                    break; 
                
                // Format two characters into one byte 
                if (char_count)  // Format byte
                {
                    *new_msg_byte_count += 1; 
                    *new_msg++ = (high_nibble << SHIFT_4) | low_nibble; 
                }

                else  // Store the byte 
                    high_nibble = low_nibble; 

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


// M8Q UBX checksum calculation 
CHECKSUM m8q_ubx_checksum(
    uint8_t *msg, 
    uint16_t len)
{
    // Local variables 
    uint8_t checksum_A = 0; 
    uint8_t checksum_B = 0; 
    CHECKSUM checksum = 0; 

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
