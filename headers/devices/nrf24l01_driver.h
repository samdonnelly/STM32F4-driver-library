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

#include "stm32f411xe.h"
#include "tools.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Data handling 
#define NRF24L01_RF_CH_MAX 0x7D        // RF channel frequency max setting 
#define NRF24L01_MAX_PAYLOAD_LEN 32    // Max data packet size (data size + data) 
#define NRF24l01_ADDR_WIDTH 5          // Address width 

//=======================================================================================


//=======================================================================================
// Enums 
/**
 * @brief Data rate to use 
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
    NRF24L01_DP_5 
} nrf24l01_data_pipe_t; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief nRF24L01 initialization 
 * 
 * @details Initialization function for both PTX and PRX devices. Specific PTX/PRX 
 *          configuration is done with the user configuration functions below. This 
 *          function configures the data record and the device registers to their default 
 *          value. This must be called before using the rest of the driver. 
 *          
 *          NOTE: The device can run onto SPI up to 10Mbps. Ensure the SPI initialized 
 *                has a speed less than or equal to this. 
 *          
 *          NOTE: the timer must be a timer that increments every 1us so that the timer 
 *                delay functions can be used. 
 * 
 * @param spi : SPI port used for the device 
 * @param gpio_ss : GPIO port for the slave aelect pin 
 * @param ss_pin : slave select pin number 
 * @param gpio_en : GPIO port used for the enable pin 
 * @param en_pin : enable pin number 
 * @param timer : timer port used for the driver 
 */
void nrf24l01_init(
    SPI_TypeDef *spi, 
    GPIO_TypeDef *gpio_ss, 
    pin_selector_t ss_pin, 
    GPIO_TypeDef *gpio_en, 
    pin_selector_t en_pin, 
    TIM_TypeDef *timer); 

//=======================================================================================


//=======================================================================================
// User configuration functions 

/**
 * @brief Configure a devices PTX settings 
 * 
 * @details Removes the device from any active mode and updates its PTX settings before 
 *          putting it back into an active mode. 
 * 
 * @param tx_addr : 5 byte address used by the PTX device 
 */
void nrf24l01_ptx_config(const uint8_t *tx_addr); 


/**
 * @brief Configure a devices PRX settings 
 * 
 * @details Removes the device from any active mode and updates its PRX settings before 
 *          putting it back into an active mode. 
 * 
 * @param rx_addr : 5 byte address used by the PRX device 
 * @param pipe_num : data pipe number 
 */
void nrf24l01_prx_config(
    const uint8_t *rx_addr, 
    nrf24l01_data_pipe_t pipe_num); 


/**
 * @brief Set frequency channel 
 * 
 * @details Removes the device from any active mode and updates the RF channel before 
 *          putting it back into an active mode. Note that the PTX and PRX devices must 
 *          be on the same channel in order to communicate. The channel set will be: 
 *          --> 2400 MHz + 'rf_ch_freq' 
 * 
 * @param rf_ch_freq : RF channel 
 */
void nrf24l01_set_rf_channel(uint8_t rf_ch_freq); 


/**
 * @brief RF data rate set 
 * 
 * @details Removes the device from any active mode and updates the data rate before 
 *          putting it back into an active mode. 
 * 
 * @param rate : data rate to use 
 */
void nrf24l01_set_rf_dr(nrf24l01_data_rate_t rate); 


/**
 * @brief Set power output 
 * 
 * @details Removes the device from any active mode and updates the power output before 
 *          putting it back into an active mode. 
 * 
 * @param rf_pwr : power output level 
 */
void nrf24l01_set_rf_pwr(nrf24l01_rf_pwr_t rf_pwr); 


/**
 * @brief Enter low power mode - power down 
 * 
 * @details Removes the device from any active mode and sets the PWR_UP bit low to go to 
 *          the power down state. 
 */
void nrf24l01_pwr_down(void); 


/**
 * @brief Exit low power mode - power up 
 * 
 * @details Sets the PWR_UP bit high to exit the power down state and puts the device 
 *          back into an active mode. Note that this function has a short, blocking 
 *          delay (~1.5ms) to allow the device' startup state to pass. 
 */
void nrf24l01_pwr_up(void); 

//=======================================================================================


//=======================================================================================
// User getters 

/**
 * @brief Data ready status 
 * 
 * @details Returns that status of the RX FIFO for a given pipe number. If true it means 
 *          there is data available to be read for that pipe. 
 *          
 *          It's important to read data from the RX FIFO when it's available. Data will 
 *          fill up in the RX FIFO and if the FIFO becomes full then new incoming data 
 *          will be discarded and therefore lost. 
 * 
 * @param pipe_num : pipe number to check 
 * @return uint8_t : RX FIFO data status 
 */
uint8_t nrf24l01_data_ready_status(nrf24l01_data_pipe_t pipe_num); 


/**
 * @brief Get power mode 
 * 
 * @details Reads and returns the current power mode of the device. 
 * 
 * @return nrf24l01_pwr_mode_t : current power mode 
 */
nrf24l01_pwr_mode_t nrf24l01_get_pwr_mode(void); 


/**
 * @brief Get active mode 
 * 
 * @details Reads and returns the active mode of the device. 
 * 
 * @return nrf24l01_mode_select_t : current active mode 
 */
nrf24l01_mode_select_t nrf24l01_get_mode(void); 


/**
 * @brief Get RF channel 
 * 
 * @details Reads and returns the RF channel of the device. Note that the returned value 
 *          is in MHz and it should be added to 2400 MHz to get the true channel number. 
 * 
 * @return uint8_t : RF channel (MHz) before adding 2400 MHz 
 */
uint8_t nrf24l01_get_rf_ch(void); 


/**
 * @brief Get RF data rate 
 * 
 * @details Reads, formats and returns the data rate of the device. 
 * 
 * @return nrf24l01_data_rate_t : current data rate 
 */
nrf24l01_data_rate_t nrf24l01_get_rf_dr(void); 


/**
 * @brief Get power output 
 * 
 * @details Reads and returns the power output level of the device. 
 * 
 * @return nrf24l01_rf_pwr_t : current power output level 
 */
nrf24l01_rf_pwr_t nrf24l01_get_rf_pwr(void); 

//=======================================================================================


//=======================================================================================
// User data functions 

/**
 * @brief Receive payload 
 * 
 * @details If data is available for the specified pipe number, then read the RX FIFO 
 *          contents and store it in the buffer. This function can't be used while in 
 *          in low power mode. 
 *          
 *          NOTE: 'read_buff' must be at least 30 bytes long. This is the longest 
 *                possible data packet that can be received so if 'read_buff' is smaller 
 *                than this some data could be lost. 
 * 
 * @param read_buff : buffer to store the received payload 
 * @param pipe_num : pipe number to read from 
 */
void nrf24l01_receive_payload(
    uint8_t *read_buff, 
    nrf24l01_data_pipe_t pipe_num); 


/**
 * @brief Send payload 
 * 
 * @details Sends the payload stored in the buffer out over the device' RF channel. This 
 *          function can't be used while in low power mode. 
 *          
 *          This function will put the device into TX mode just long enough to send the 
 *          payload out. The device is not supposed to remain in TX mode for longer than 
 *          4ms so once a single packet has been sent the device is put back into RX mode. 
 *          
 *          The device has 3 separate 32-byte TX FIFOs. This means the data between each 
 *          FIFO is not connected. When sending payloads you can send up to 32 bytes to 
 *          the device at once because that is the capacity of a single FIFO. However, 
 *          this driver caps the data size at 30 bytes to make room for data length and 
 *          NULL termination bytes. 
 *          
 *          This function determines the length of the payload passed in data_buff so 
 *          that it doesn't have to be specified by the application. However, if the 
 *          length of the payload is too large, not all the data will be sent (see note 
 *          below). Determining payload length is handled here and not left to the 
 *          application because if this device is used to send data that doesn't have a 
 *          predefined length then the length of the data would have to be determined 
 *          anyway. 
 *          
 *          NOTE: The max data length that can be sent at one time (one call of this 
 *                function) is 30 bytes. The device FIFO supports 32 bytes but the first 
 *                byte is used to store the data length and the data is terminated with a 
 *                NULL character. 
 * 
 * @param data_buff : buffer that contains data to be sent 
 * @return uint8_t : status of the send operation - a 1 is returned if successful 
 */
uint8_t nrf24l01_send_payload(const uint8_t *data_buff); 

//=======================================================================================


//=======================================================================================
// Status 

/**
 * @brief nrF24L01 clear device driver status code 
 */
void nrf24l01_clear_status(void); 


/**
 * @brief nrF24L01 get device driver status code 
 * 
 * @return uint8_t : driver status 
 */
uint8_t nrf24l01_get_status(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _NRF24L01_DRIVER_H_ 
