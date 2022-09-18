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

// Standard Libraries 
#if M8Q_USER_CONFIG
#include <stdio.h> 
#endif  // M8Q_USER_CONFIG

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief M8Q NMEA config function 
 * 
 * @details 
 * 
 * @param i2c : pointer to the I2C port used 
 * @param msg : pointer to the message string 
 */
void m8q_nmea_config(
    I2C_TypeDef *i2c, 
    uint8_t *msg); 


/**
 * @brief M8Q NMEA message calculation 
 * 
 * @details Calculates the NMEA config message checksum to be sent along with the message 
 *          to the receiver using an exlusive OR (XOR) operation on all bytes of the 
 *          message string. 
 * 
 * @param msg : pointer to message string 
 * @return NMEA_CHECKSUM : checksum of an NMEA message to be sent to the receiver 
 */
NMEA_CHECKSUM m8q_nmea_checksum(
    uint8_t *msg); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param msg 
 * @param term_char 
 * @return uint8_t 
 */
uint8_t m8q_message_size(
    uint8_t *msg, 
    uint8_t term_char); 

//=======================================================================================


//=======================================================================================
// Initialization 

// 
void m8q_init(void)
{
    // TODO configure GPIO for txReady pin if desired 
}

//=======================================================================================


//=======================================================================================
// NMEA Read 

// Read a message from the M8Q 
NMEA_VALID m8q_read(
    I2C_TypeDef *i2c, 
    uint8_t *data)
{
    // Local variables 
    NMEA_VALID read_status = M8Q_NMEA_READ_INVALID; 
    uint8_t data_check = 0; 

    // Check for a valid data stream 
    m8q_check_data_stream(i2c, &data_check); 

    switch (data_check)
    {
        case M8Q_NO_DATA:  // No data stream available 
            break;

        case M8Q_NMEA_START:  // Valid data stream - start of NMEA message 
            // Generate a start condition 
            i2c_start(i2c); 

            // Send the device address with a read offset 
            i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET); 
            i2c_clear_addr(i2c);  

            // Read the rest of the data stream until "\r\n" 
            i2c_read_to_term(i2c, data, M8Q_NMEA_END_PAY, I2C_4_BYTE); 

            read_status = M8Q_NMEA_READ_VALID; 
            break;

        default:  // Unknown data stream 
            break;
    }

    return read_status; 
}


// Read the NMEA data stream size 
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
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_W_OFFSET); 
    i2c_clear_addr(i2c); 

    // Send the first data size register address to start reading from there 
    i2c_write_master_mode(i2c, &address, I2C_1_BYTE); 

    // Generate another start condition 
    i2c_start(i2c); 

    // Send the device address again with a read offset 
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET);  
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
    i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET); 
    i2c_clear_addr(i2c); 

    // Read the first byte of the data stream 
    i2c_read_master_mode(i2c, data_check, I2C_1_BYTE); 
}

//=======================================================================================


//=======================================================================================
// NMEA write 

// TODO send a save command/mask after writing a CFG message. 

// M8Q write 
void m8q_write(
    I2C_TypeDef *i2c, 
    uint8_t *data, 
    uint8_t data_size)
{
    // Generate a start condition 
    i2c_start(I2C1); 

    // Send the device address with a write offset 
    i2c_write_address(I2C1, M8Q_I2C_8_BIT_ADDR + M8Q_W_OFFSET); 
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

    while (*msg != term_char)
    {
        msg_len++; 
        msg++; 
    }

    return msg_len; 
}

//=======================================================================================


//=======================================================================================
// Getters 
//=======================================================================================


//=======================================================================================
// User Configuration 

#if M8Q_USER_CONFIG

// M8Q user configuration 
void m8q_user_config(
    I2C_TypeDef *i2c)
{
    // Local variables 
    uint8_t nmea_config_msg[M8Q_CONFIG_MSG]; 

    // Check if there is user input waiting 
    if (uart_data_ready(USART2))
    {
        // Read the input 
        uart_getstr(USART2, (char *)nmea_config_msg, UART_STR_TERM_CARRIAGE); 

        // Identify the message type 
        switch (nmea_config_msg[0])
        {
            case M8Q_NMEA_START:  // NMEA message 
                m8q_nmea_config(i2c, nmea_config_msg); 
                break;

            case M8Q_UBX_SYNC1:  // UBX message 
                uart_send_new_line(USART2); 
                uart_sendstring(USART2, "Could be a UBX message\r\n"); 
                m8q_nmea_config_ui(); 
                break;
            
            default:  // Unknown input 
                uart_send_new_line(USART2); 
                uart_sendstring(USART2, "Unknown message type\r\n"); 
                m8q_nmea_config_ui(); 
                break;
        }
    }
}


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
    NMEA_CHECKSUM checksum = 0; 
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
        {
            uart_send_new_line(USART2); 
            uart_sendstring(USART2, "Unsupported PUBX message ID\r\n");
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

                // Pass the message along to the NMEA send function 
                m8q_nmea_write(I2C1, msg, m8q_message_size(msg, NULL_CHAR));

                // Send confirmation message to terminal 
                uart_send_new_line(USART2); 
                uart_sendstring(USART2, "NMEA configuration message sent\r\n"); 
            } 
            else
            {
                uart_send_new_line(USART2); 
                uart_sendstring(USART2, "Invalid formatting of PUBX message\r\n"); 
            }
        }
    }
    else 
    {
        uart_send_new_line(USART2); 
        uart_sendstring(USART2, "Only PUBX messages are supported\r\n"); 
    }

    // Prompt for the next message 
    m8q_nmea_config_ui(); 
}


// M8Q NMEA message calculation 
NMEA_CHECKSUM m8q_nmea_checksum(
    uint8_t *msg)
{
    // Local variables 
    NMEA_CHECKSUM checksum = 0; 
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


// M8Q NMEA config user interface 
void m8q_nmea_config_ui(void)
{
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, ">>> Config message: "); 
}

#endif   // M8Q_USER_CONFIG

//=======================================================================================
