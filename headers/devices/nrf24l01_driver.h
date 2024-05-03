/**
 * @file nrf24l01_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module driver interface 
 * 
 * @version 0.1
 * @date 2023-07-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _NRF24L01_DRIVER_H_ 
#define _NRF24L01_DRIVER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define NRF24L01_RF_CH_MAX 0x7D        // RF channel frequency max setting 
#define NRF24L01_MAX_PAYLOAD_LEN 32    // Max data packet size (data size + data) 
#define NRF24l01_ADDR_WIDTH 5          // Address width 

//=======================================================================================


//=======================================================================================
// Enums 

// nRF24L01 driver status 
typedef enum {
    NRF24L01_OK,            // No problem with the nRF24L01 device 
    NRF24L01_INVALID_PTR,   // Invalid pointer provided to function 
    NRF24L01_WRITE_FAULT,   // A problem occurred while writing via SPI 
    NRF24L01_READ_FAULT     // A problem occurred while reading via SPI 
} nrf24l01_status_t; 


/**
 * @brief Data rate 
 * 
 * @details A slower data rate will allow for longer range communication (better receiver 
 *          sensitivity). A higher data rate has lower average current consumption and 
 *          reduced probability of on-air collisions. The transmitter and receiver must 
 *          have the same data rate set in order to communicate with one another, the same 
 *          goes with the channel frequency. 
 */
typedef enum {
    NRF24L01_DR_1MBPS, 
    NRF24L01_DR_2MBPS, 
    NRF24L01_DR_250KBPS 
} nrf24l01_data_rate_t; 


/**
 * @brief Power output level 
 */
typedef enum {
    NRF24L01_RF_PWR_18DBM, 
    NRF24L01_RF_PWR_12DBM, 
    NRF24L01_RF_PWR_6DBM, 
    NRF24L01_RF_PWR_0DBM
} nrf24l01_rf_pwr_t; 


/**
 * @brief TX/RX mode setter 
 */
typedef enum {
    NRF24L01_TX_MODE, 
    NRF24L01_RX_MODE
} nrf24l01_mode_select_t; 


/**
 * @brief Power mode 
 */
typedef enum {
    NRF24L01_PWR_DOWN, 
    NRF24L01_PWR_UP 
} nrf24l01_pwr_mode_t; 


/**
 * @brief Data pipe number 
 */
typedef enum {
    NRF24L01_DP_0, 
    NRF24L01_DP_1, 
    NRF24L01_DP_2, 
    NRF24L01_DP_3, 
    NRF24L01_DP_4, 
    NRF24L01_DP_5, 
    NRF24L01_RX_FIFO_EMPTY = 7   // Not an actual pipe. Used for matching register values. 
} nrf24l01_data_pipe_t; 

//=======================================================================================


//=======================================================================================
// Datatypes 

typedef uint8_t NRF24L01_STATUS; 
typedef nrf24l01_data_pipe_t DATA_PIPE; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief nRF24L01 general initialization 
 * 
 * @details Initialization function for both TX and RX devices that configures the driver 
 *          data record and device registers. This must be called before the device can be 
 *          used. After this function is called, the PTX and/or the PRX functions should 
 *          be called depending on the application. After that is done the device must be 
 *          powered up before it can be used for the first time. 
 *          
 *          NOTE: The device can run onto SPI up to 10Mbps. Ensure the SPI initialized 
 *                has a speed less than or equal to this. 
 *          
 *          NOTE: the timer must be a timer that increments every 1us so that the timer 
 *                delay functions can be used. 
 * 
 * @see nrf24l01_ptx_config 
 * @see nrf24l01_prx_config 
 * @see nrf24l01_pwr_up 
 * 
 * @param spi : SPI port used for the device 
 * @param gpio_ss : GPIO port for the slave select pin 
 * @param ss_pin : slave select pin number 
 * @param gpio_en : GPIO port used for the enable pin 
 * @param en_pin : enable pin number 
 * @param timer : timer port used for the driver 
 * @param rf_ch_freq : initial RF channel 
 * @param data_rate : initial data rate to use 
 * @param rf_pwr : initial power output level 
 * @return NRF24L01_STATUS : init (write/read operation) status 
 */
NRF24L01_STATUS nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer, 
    uint8_t rf_ch_freq, 
    nrf24l01_data_rate_t data_rate, 
    nrf24l01_rf_pwr_t rf_pwr); 


/**
 * @brief Configure a devices PTX settings 
 * 
 * @details Devices that will be sending data can call this function to update TX 
 *          settings. If the device is operating on default settings then this 
 *          function may not be needed. Once settings are configured as needed, the 
 *          device must be powered up (if not already) before it can be used. This 
 *          function can be called at any point if TX settings need to be changed. 
 *          
 *          Settings updated in this function: 
 *          - TX_ADDR --> TX address. This address must match one of the data pipe 
 *                        addresses in an RX device for the RX device to receive data. 
 * 
 * @see nrf24l01_pwr_up 
 * 
 * @param tx_addr : 5 byte data pipe address the TX device sends to 
 */
void nrf24l01_ptx_config(const uint8_t *tx_addr); 


/**
 * @brief Configure a devices PRX settings 
 * 
 * @details Devices that will be receiving data can call this function to update RX 
 *          settings. If the device is operating on default settings then this 
 *          function may not be needed. Once settings are configured as needed, the 
 *          device must be powered up (if not already) before it can be used. This 
 *          function can be called at any point if RX settings need to be changed. 
 *          
 *          Settings updated in this function: 
 *          - EN_RXADDR ---> Data pipe to enable. Multiple data pipes can be enabled 
 *                           at a time but this function must be called for each pipe 
 *                           being configured. 
 *          - RX_ADDR_PX --> RX data pipe address. The specified data pipe will be 
 *                           assigned the specified address. A TX device must send data 
 *                           with a matching address in order for the data pipe to 
 *                           receive it. Note that the addresses for data pipes 0-1 
 *                           can be configured up to 5 bytes long whereas only the 
 *                           least significant byte of the address for pipes 2-5 can 
 *                           be configured. The other bytes in the addresses of pipes 
 *                           2-5 will be the same as specified for pipe 1. 
 * 
 * @see nrf24l01_pwr_up 
 * 
 * @param rx_addr : Data pipe address 
 * @param pipe_num : data pipe number 
 */
void nrf24l01_prx_config(
    const uint8_t *rx_addr, 
    nrf24l01_data_pipe_t pipe_num); 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Data available status 
 * 
 * @details Returns the data pipe number for the payload available for reading from the 
 *          RX FIFO. If no data is available then an RX FIFO empty value will be returned. 
 *          This function will read from the status register to get the return value. This 
 *          function can be used to check for data before reading the payload. 
 *          
 *          If data is not read or flushed from the RX FIFO then incoming data will 
 *          accumulate. If the RX FIFO fills up, it will discard new incoming data so it's 
 *          important to read or flush data when it arrives. 
 * 
 * @see nrf24l01_data_pipe_t 
 * 
 * @param pipe_num : pipe number to check 
 * @return DATA_PIPE : data pipe for available payload 
 */
DATA_PIPE nrf24l01_data_ready_status(void); 


/**
 * @brief Receive payload 
 * 
 * @details If the RX FIFO has data available in one of the data pipes then the data gets 
 *          read and stored in the buffer. Data in the RX FIFO means the device received 
 *          a payload from a TX device. To get data from a TX device, the RX device must 
 *          have the same frequency channel and data rate as the TX device. 
 *          
 *          Note that the maximum data size for each RX FIFO slot is 32 bytes. This is the 
 *          number of bytes read regardless of the received data length so the buffer 
 *          must be long enough to accomodate this. 
 * 
 * @param read_buff : buffer to store the received payload 
 */
void nrf24l01_receive_payload(uint8_t *read_buff); 


/**
 * @brief Send payload 
 * 
 * @details Writes the contents of the buffer to the TX FIFO as a payload. This payload 
 *          gets sent by the device as a radio message at the configured frequency 
 *          channel, power and data rate. This function checks if the device has sent 
 *          the message and a fault will be returned if the data was never sent. After 
 *          sending data, the TX FIFO will be flushed so there no leftover data. 
 *          
 *          Note that the maximum data size for each TX FIFO slot is 32 bytes. Data 
 *          longer than this will be truncated. 
 * 
 * @param data_buff : buffer that contains data to be sent 
 * @return uint8_t : write operation status 
 */
NRF24L01_STATUS nrf24l01_send_payload(const uint8_t *data_buff); 


//==================================================
// RF_CH register 

/**
 * @brief RF_CH register read 
 * 
 * @details Must be called to get the latest RF_CH value from the device. 
 * 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_rf_ch_read(void); 


/**
 * @brief Get RF channel 
 * 
 * @details Returns the RF channel frequency stored in the data record. Unless the 
 *          frquency is unable to update in the device then the data record value will 
 *          be the same as the value used by the device. The frequency that the device 
 *          is using can be updated in the data record by calling the channel read 
 *          function. This value added to 2400 MHz provides the channel frquency (MHz). 
 * 
 * @see nrf24l01_rf_ch_read 
 * 
 * @return uint8_t : RF channel (MHz) before adding 2400 MHz 
 */
uint8_t nrf24l01_get_rf_ch(void); 


/**
 * @brief Set frequency channel 
 * 
 * @details Sets the RF channel freuency in the data record. To update the frequency 
 *          used by the device, the RF channel write function must be called. The 
 *          frequency channel is the value passed to this function + 2400 MHz. 
 * 
 * @see nrf24l01_rf_ch_write 
 * 
 * @param rf_ch_freq : RF channel (MHz) 
 */
void nrf24l01_set_rf_ch(uint8_t rf_ch_freq); 


/**
 * @brief RF_CH register write 
 * 
 * @details Must be called to update the device's frequency channel. 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_rf_ch_write(void); 

//==================================================


//==================================================
// RF_SETUP register 

/**
 * @brief RF_SETUP register read 
 * 
 * @details Must be called to get the latest RF_SETUP register contents from the device. 
 * 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_rf_setup_read(void); 


/**
 * @brief Get RF data rate 
 * 
 * @details Returns the data rate from the RF_SETUP data record. To get the data rate 
 *          being used by the device, the RF_SETUP register read function must be called. 
 *          However, unless the RF_SETUP register fails to update using the RF_SETUP 
 *          register write function, the value in the data record will match the value 
 *          used by the device. 
 * 
 * @see nrf24l01_rf_setup_read 
 * 
 * @return nrf24l01_data_rate_t : current data rate 
 */
nrf24l01_data_rate_t nrf24l01_get_rf_setup_dr(void); 


/**
 * @brief Get power output 
 * 
 * @details Returns the power output from the RF_SETUP data record. To get the power 
 *          output being used by the device, the RF_SETUP register read function must 
 *          be called. However, unless the RF_SETUP register fails to update using the 
 *          RF_SETUP register write function, the value in the data record will match 
 *          the value used by the device. 
 * 
 * @see nrf24l01_rf_setup_read 
 * 
 * @return nrf24l01_rf_pwr_t : current power output level 
 */
nrf24l01_rf_pwr_t nrf24l01_get_rf_setup_pwr(void); 


/**
 * @brief RF data rate set 
 * 
 * @details Sets the data rate in the data record. To update the data rate used by the 
 *          device, the RF_SETUP register write function must be called. 
 * 
 * @see nrf24l01_rf_setup_write 
 * 
 * @param rate : data rate to use 
 */
void nrf24l01_set_rf_setup_dr(nrf24l01_data_rate_t rate); 


/**
 * @brief Set power output 
 * 
 * @details Sets the power output in the data record. To update the power output used 
 *          by the device, the RF_SETUP register write function must be called. 
 * 
 * @see nrf24l01_rf_setup_write 
 * 
 * @param rf_pwr : power output level 
 */
void nrf24l01_set_rf_setup_pwr(nrf24l01_rf_pwr_t rf_pwr); 


/**
 * @brief RF_SETUP register write 
 * 
 * @details Must be called to update the the RF_SETUP register in the device. 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_rf_setup_write(void); 

//==================================================


//==================================================
// CONFIG register 

/**
 * @brief CONFIG register read 
 * 
 * @details Must be called to get the latest CONFIG register value from the device. 
 * 
 * @return NRF24L01_STATUS : read operation status 
 */
NRF24L01_STATUS nrf24l01_config_read(void); 


/**
 * @brief Get power mode 
 * 
 * @details Returns the power mode from the CONFIG data record. To get the power mode 
 *          being used by the device, the CONFIG register read function must be called. 
 *          However, unless the CONFIG register fails to update using the CONFIG 
 *          register write function, the value in the data record will match the value 
 *          used by the device. 
 * 
 * @return nrf24l01_pwr_mode_t : current power mode 
 */
nrf24l01_pwr_mode_t nrf24l01_get_config_pwr_mode(void); 


/**
 * @brief Get active mode 
 * 
 * @details Returns the active mode from the CONFIG data record. To get the active mode 
 *          of the device, the CONFIG register read function must be called. However, 
 *          unless the CONFIG register fails to update using the CONFIG register write 
 *          function, the value in the data record will match the value used by the device. 
 * 
 * @return nrf24l01_mode_select_t : current active mode 
 */
nrf24l01_mode_select_t nrf24l01_get_config_mode(void); 


/**
 * @brief Enter low power mode - power down 
 * 
 * @details Removes the device from any active mode then sets and writes the PWR_UP bit 
 *          low to go to the power down state. 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_pwr_down(void); 


/**
 * @brief Exit low power mode - power up 
 * 
 * @details Sets and writes the PWR_UP and PRIM_RX bits high to exit the power down state 
 *          and enter RX mode. Note that if successfuly, this function will have a short 
 *          blocking delay (~1.5ms) to allow the device' startup state to pass. 
 * 
 * @return NRF24L01_STATUS : write operation status 
 */
NRF24L01_STATUS nrf24l01_pwr_up(void); 

//==================================================

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_DRIVER_H_ 
