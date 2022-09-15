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
// Message identification 
//=======================================================================================


//=======================================================================================
// NMEA Read 

// Read an NMEA message from the M8Q 
NMEA_VALID m8q_read_nmea(
    I2C_TypeDef *i2c, 
    uint8_t *data)
{
    // Local variables 
    NMEA_VALID read_status = M8Q_NMEA_READ_INVALID; 
    uint8_t data_check = 0; 

    // Check for a valid data stream 
    m8q_check_nmea_stream(i2c, &data_check); 

    // Check the result 
    switch (data_check)
    {
        case M8Q_INVALID_NMEA:  // No data stream available 
            break;

        case M8Q_VALID_NMEA:  // Valid data stream - start of NMEA message 
            // Generate a start condition 
            i2c_start(i2c); 

            // Send the device address with a read offset 
            i2c_write_address(i2c, M8Q_I2C_8_BIT_ADDR + M8Q_R_OFFSET); 
            i2c_clear_addr(i2c);  

            // Read the rest of the data stream until "\r\n" 
            i2c_read_to_term(i2c, data, M8Q_END_NMEA, I2C_4_BYTE); 

            read_status = M8Q_NMEA_READ_VALID; 
            break;

        default:  // Unknown data stream 
            break;
    }

    return read_status; 
}


// Read the NMEA data stream size 
void m8q_read_nmea_ds(
    I2C_TypeDef *i2c, 
    uint16_t *data_size)
{
    // Local variables 
    uint8_t num_bytes[BYTE_2];            // Store the high and low byte of the data size 
    uint8_t address = M8Q_READ_DS_ADDR;   // Address of high byte for the data size 

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


// Check for a valid NMEA data stream 
void m8q_check_nmea_stream(
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
//=======================================================================================


//=======================================================================================
// PUBX read 
//=======================================================================================


//=======================================================================================
// PUBX write 

// TODO send a save command/mask after writing a CFG message. 

//=======================================================================================


//=======================================================================================
// Message processing 
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
            case M8Q_VALID_NMEA: 
                m8q_nmea_config(i2c, nmea_config_msg); 
                break;

            case M8Q_UBX_SYNC1:
                uart_sendstring(USART2, "Could be a UBX message\r\n"); 
                break;
            
            default:
                uart_sendstring(USART2, "Unknown message type\r\n"); 
                break;
        }

        // Print the message to the terminal (for testing) 
        // Send the message to the receiver (application) 
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

    // Check message header 
    if (str_compare("$PUBX,", (char *)msg, BYTE_0))
    {
        // Check message ID 
        if (str_compare("40,", (char *)msg, BYTE_6)) 
            msg_args = M8Q_NMEA_RATE_ARGS; 
        
        else if (str_compare("41,", (char *)msg, BYTE_6)) 
            msg_args = M8Q_NMEA_CONFIG_ARGS; 

        else
            uart_sendstring(USART2, "not a supported PUBX ID\r\n");
        
        // Check the number of message inputs 
        msg_ptr += BYTE_9; 
        while(*msg_ptr != CR_CAHR) 
        {
            if (*msg_ptr == COMMA_CHAR)
            {
                msg_arg_mask = 0; 
            }
            else 
            {
                if (!msg_arg_mask)
                {
                    msg_arg_count++; 
                    msg_arg_mask++; 
                }
            }
            msg_ptr++; 
        }

        // Check if the message is valid 
        if (msg_arg_count == msg_args)
        {
            // Calculate a checksum 
            // m8q_nmea_checksum(msg);
            uart_sendstring(USART2, "Valid PUBX message\r\n"); 

            // Append the checksum and termination characters onto the message 
            // Use the msg_ptr here as it's already at the correct memory location 

            // Pass the message along to the NMEA send function 
        } 
        else
        {
            uart_sendstring(USART2, "Invalid formatting of PUBX message\r\n"); 
        }
    }
    else 
    {
        uart_sendstring(USART2, "not PUBX\r\n"); 
    }
}


// M8Q NMEA message calculation 
NMEA_CHECKSUM m8q_nmea_checksum(
    uint8_t *msg)
{
    // 
}

#endif   // M8Q_USER_CONFIG

//=======================================================================================
