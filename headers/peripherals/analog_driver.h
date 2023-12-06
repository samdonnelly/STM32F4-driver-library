/**
 * @file analog_driver.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog (ADC) data functions 
 * 
 * @version 0.1
 * @date 2022-09-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _ANALOG_DRIVER_H_
#define _ANALOG_DRIVER_H_

//=======================================================================================
// Includes 

// Toolkit 
#include "stm32f411xe.h"
#include "tools.h"

// Drivers 
#include "gpio_driver.h"
#include "timers.h"

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief ADC clock prescalar 
 * 
 * @details The prescalar controls the speed at which the ADC circuitry operates. This 
 *          prescalar divides the APB2 clock to get the ADCCLK speed. The ADC has a 
 *          maximum clock frequency so check the datasheet before choosing a prescalar. 
 */
typedef enum {
    ADC_PCLK2_2,   // PLCK2 divided by 2 
    ADC_PCLK2_4,   // PLCK2 divided by 4 
    ADC_PCLK2_6,   // PLCK2 divided by 6 
    ADC_PCLK2_8    // PLCK2 divided by 8 
} adc_prescalar_t; 


/**
 * @brief ADC channel number 
 * 
 * @details Each ADC has up to 18 channels that it serves. Use this to specify which 
 *          channel to configure during initialization. 
 */
typedef enum {
    ADC_CHANNEL_0, 
    ADC_CHANNEL_1, 
    ADC_CHANNEL_2, 
    ADC_CHANNEL_3, 
    ADC_CHANNEL_4, 
    ADC_CHANNEL_5, 
    ADC_CHANNEL_6, 
    ADC_CHANNEL_7, 
    ADC_CHANNEL_8, 
    ADC_CHANNEL_9, 
    ADC_CHANNEL_10, 
    ADC_CHANNEL_11, 
    ADC_CHANNEL_12, 
    ADC_CHANNEL_13, 
    ADC_CHANNEL_14, 
    ADC_CHANNEL_15, 
    ADC_CHANNEL_16, 
    ADC_CHANNEL_17, 
    ADC_CHANNEL_18 
} adc_channel_t; 


/**
 * @brief Sampling cycles selection 
 * 
 * @details The ADC can be configured to sample for a certain period of time. Fewer sample 
 *          cycles is faster but less accurate and vice versa for more sample cycles. This 
 *          enum defines the available sample cycles. 
 */
typedef enum {
    ADC_SMP_3,     // 3 cycles 
    ADC_SMP_15,    // 15 cycles
    ADC_SMP_28,    // 28 cycles
    ADC_SMP_56,    // 56 cycles
    ADC_SMP_84,    // 84 cycles
    ADC_SMP_112,   // 112 cycles
    ADC_SMP_144,   // 144 cycles
    ADC_SMP_480    // 480 cycles
} adc_smp_cycles_t; 


/**
 * @brief Resolution selection 
 * 
 * @details The ADC can be configured for different resolutions. Higher resolution reads 
 *          (more accurate/precise reads) take more clock cycles and vice versa for lower 
 *          resolutions. 
 */
typedef enum {
    ADC_RES_12,   // 12-bit resolution (15 ADCCLK cycles) 
    ADC_RES_10,   // 10-bit resolution (13 ADCCLK cycles) 
    ADC_RES_8,    // 8-bit resolution (11 ADCCLK cycles) 
    ADC_RES_6     // 6-bit resolution (9 ADCCLK cycles) 
} adc_res_t;


/**
 * @brief Sequence number 
 * 
 * @details When using SCAN mode the ADC reads all the conversions in the order defined within 
 *          its sequence. This enum is used to define both the number that a conversion is 
 *          within the sequence and total sequence length/size. 
 */
typedef enum {
    ADC_SEQ_1 = 1, 
    ADC_SEQ_2,  
    ADC_SEQ_3,  
    ADC_SEQ_4,  
    ADC_SEQ_5,  
    ADC_SEQ_6,  
    ADC_SEQ_7,  
    ADC_SEQ_8,  
    ADC_SEQ_9,  
    ADC_SEQ_10, 
    ADC_SEQ_11, 
    ADC_SEQ_12, 
    ADC_SEQ_13, 
    ADC_SEQ_14, 
    ADC_SEQ_15, 
    ADC_SEQ_16 
} adc_seq_num_t; 


#if DEV_CODE 

/**
 * @brief ADC parameter configuration 
 * 
 * @details This is used for disabling or enabling features of the ADC when setting it up. 
 *          The ADC initialization functions take this as an argument for a number of 
 *          features. These features include: 
 *          
 *          End of Conversion (EOC) 
 *          - 0 --> EOC bit set after each sequence of regular conversions 
 *          - 1 --> EOC bit set after each regular conversion 
 *          
 *          End of Conversion (EOC) interrupt 
 *          - 0 --> Disable EOC interrupt 
 *          - 1 --> Enable EOC interrupt 
 *          
 *          Scan mode 
 *          - 0 --> Disable scan mode 
 *          - 1 --> Enable scan mode 
 *          
 *          Continuous mode 
 *          - 0 --> Disable continuous mode (set to single conversion mode) 
 *          - 1 --> Enable continuous mode 
 *          
 *          DMA mode 
 *          - 0 --> Disable DMA for ADC 
 *          - 1 --> Enable DMA for ADC 
 *          
 *          DMA disable selection 
 *          - 0 --> No new DMA request is issued after the last transfer 
 *          - 1 --> DMA requets are issued as long as data are coverted 
 *          
 *          Watchdog mode 
 *          - 0 --> Disable the watchdog 
 *          - 1 --> Enable the watchdog 
 *          
 *          Watchdog single channel selection 
 *          - 0 --> Disable the watchdog single channel selection 
 *          - 1 --> Enable the watchdog single channel selection 
 *          
 *          Watchdog interrupt 
 *          - 0 --> Disable the watchdog interrupt 
 *          - 1 --> Enable the watchdog interrupt 
 *          
 *          Overrun interrupt 
 *          - 0 --> Disable the overrun interrupt 
 *          - 1 --> Enable the overrun interrupt 
 * 
 * @see adc_port_init 
 * @see adc_wd_init 
 */
typedef enum {
    ADC_PARAM_DISABLE, 
    ADC_PARAM_ENABLE 
} adc_param_config_t; 

#else   // DEV_CODE 

/**
 * @brief EOC selection 
 */
typedef enum {
    ADC_EOC_SEQ,   // EOC bit set after each sequence of regular conversions 
    ADC_EOC_EACH   // EOC bit set after each regular conversion 
} adc_eoc_config_t; 


/**
 * @brief EOC interrupt selection 
 */
typedef enum {
    ADC_EOC_INT_DISABLE,   // Disable EOC interrupt 
    ADC_EOC_INT_ENABLE     // Enable EOC interrupt 
} adc_eoc_int_t; 


/**
 * @brief Scan mode selection 
 */
typedef enum {
    ADC_SCAN_DISABLE,   // Disable scan mode 
    ADC_SCAN_ENABLE     // Enable scan mode 
} adc_scan_t; 


/**
 * @brief Continuous mode selection 
 */
typedef enum {
    ADC_CONT_DISABLE,   // Disable continuous mode (single conversion mode) 
    ADC_CONT_ENABLE     // Enable continuous mode 
} adc_cont_t; 


/**
 * @brief DMA mode selection 
 */
typedef enum {
    ADC_DMA_DISABLE,   // Disable DMA for ADC 
    ADC_DMA_ENABLE     // Enable DMA for ADC 
} adc_dma_t; 


/**
 * @brief DMA Disable selection 
 */
typedef enum {
    ADC_DDS_DISABLE,    // No new DMA request is issued after the last transfer 
    ADC_DDS_ENABLE      // DMA requets are issued as long as data are coverted 
} adc_dds_t; 


/**
 * @brief Watchdog selection 
 */
typedef enum {
    ADC_WD_DISABLE,   // Disable the watchdog 
    ADC_WD_ENABLE     // Enable the watchdog 
} adc_wd_t; 


/**
 * @brief Watchdog single channel selection 
 */
typedef enum {
    ADC_WD_SC_DISABLE,   // Disable the watchdog single channel selection 
    ADC_WD_SC_ENABLE     // Enable the watchdog single channel selection 
} adc_wd_sc_t; 


/**
 * @brief Watchdog interrupt selection 
 */
typedef enum {
    ADC_WD_INT_DISABLE,   // Disable the watchdog interrupt 
    ADC_WD_INT_ENABLE     // Enable the watchdog interrupt 
} adc_awdie_t; 


/**
 * @brief Overrun interrupt selection 
 */
typedef enum {
    ADC_OVR_INT_DISABLE,   // Disable the overrun interrupt 
    ADC_OVR_INT_ENABLE     // Enable the overrun interrupt 
} adc_ovrie_t; 

#endif   // DEV_CODE 

/**
 * @brief ADC driver status 
 */
typedef enum {
    ADC_OK, 
    ADC_INVALID_PTR, 
    ADC_TIMEOUT 
} adc_status_t; 

//=======================================================================================


//=======================================================================================
// Data types 

typedef uint8_t ADC_STATUS; 

//=======================================================================================


//=======================================================================================
// Initialization 

/**
 * @brief ADC1 clock enable 
 * 
 * @details 
 * 
 * @param rcc 
 * @return ADC_STATUS : ADC1 clock enable status 
 */
ADC_STATUS adc1_clock_enable(
    RCC_TypeDef *rcc); 


/**
 * @brief ADC port initialization 
 * 
 * @details This functions defines the behavior of an ADC port (ex. ADC1). All the channels 
 *          within the ADC port follow this configuration. This function gets called once 
 *          for each port. See the enum listed below for argument inputs. 
 * 
 * @see adc_prescalar_t
 * @see adc_res_t
 * @see adc_eoc_config_t
 * @see adc_scan_t
 * @see adc_cont_t
 * @see adc_dma_t
 * @see adc_dds_t
 * @see adc_eoc_int_t
 * @see adc_ovrie_t
 * 
 * @param adc : pointer to the ADC port to configure 
 * @param adc_common : pointer to the ADC ports common control register 
 * @param prescalar : prescalar to define the ADCCLK 
 * @param resolution : conversion resolution 
 * @param eoc : end of conversion behavior 
 * @param scan : scan mode selection 
 * @param cont : continuous mode selection 
 * @param dma : DMA mode selection 
 * @param dds : DMA disable behavior 
 * @param eocie : end of conversion interrupt selection 
 * @param ovrie : overrun behavior selection 
 * @return ADC_STATUS : port initialization status 
 */
#if DEV_CODE 
ADC_STATUS adc_port_init(
    ADC_TypeDef *adc, 
    ADC_Common_TypeDef *adc_common, 
    adc_prescalar_t prescalar, 
    adc_res_t resolution, 
    adc_param_config_t eoc, 
    adc_param_config_t eocie, 
    adc_param_config_t scan, 
    adc_param_config_t cont, 
    adc_param_config_t dma, 
    adc_param_config_t dds, 
    adc_param_config_t ovrie); 
#else   // DEV_CODE 
void adc_port_init(
    ADC_TypeDef *adc, 
    ADC_Common_TypeDef *adc_common, 
    adc_prescalar_t prescalar, 
    adc_res_t resolution, 
    adc_eoc_config_t eoc, 
    adc_scan_t scan, 
    adc_cont_t cont, 
    adc_dma_t dma, 
    adc_dds_t dds, 
    adc_eoc_int_t eocie, 
    adc_ovrie_t ovrie); 
#endif   // DEV_CODE 


/**
 * @brief ADC pin initialization 
 * 
 * @details This defines a pin/channel used for the ADC conversions and the sample time 
 *          for each channel. Note that ADC channels are mapped to specific pins by default 
 *          so check the datasheet to see which channel coresponds to what pin. This 
 *          function is called once for each pin/channel used. 
 * 
 * @see pin_selector_t
 * @see adc_channel_t
 * @see adc_smp_cycles_t
 * 
 * @param adc : pointer to the ADC port to configure 
 * @param gpio : pointer to the GPIO port of the pin used 
 * @param adc_pin : pin number of the pin used 
 * @param adc_channel : channel that corresponds to the pin used 
 * @param smp : sample time of the channel 
 * @return ADC_STATUS : ADC pin initialization status 
 */
#if DEV_CODE 
ADC_STATUS adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp); 
#else   // DEV_CODE 
void adc_pin_init(
    ADC_TypeDef *adc, 
    GPIO_TypeDef *gpio, 
    pin_selector_t adc_pin, 
    adc_channel_t adc_channel, 
    adc_smp_cycles_t smp); 
#endif   // DEV_CODE 


/**
 * @brief ADC watchdog initialization 
 * 
 * @details Configures the watchdog for the ADC. This function is called once for each ADC
 *          port but it can also not be called at all if the watchdog is not used. The 
 *          watchdog monitors for over and under voltages on a channel and can be used to 
 *          trigger an interupt when it happens. 
 * 
 * @see adc_wd_t
 * @see adc_wd_sc_t
 * @see adc_channel_t
 * @see adc_awdie_t
 * 
 * @param adc : pointer to the ADC port to configure 
 * @param wd : watchdog behavior seletion 
 * @param wdsc : watchdog single channel selection 
 * @param channel : channel for the watchdog 
 * @param hi_thresh : upper voltage threshold that triggers the watchdog 
 * @param lo_thresh : lower voltage threshold that triggers the watchdog 
 * @param awdie : watchdog interrupt selection 
 * @return ADC_STATUS : ADC watchdog initialization status 
 */
#if DEV_CODE 
ADC_STATUS adc_wd_init(
    ADC_TypeDef *adc, 
    adc_param_config_t wd, 
    adc_param_config_t wdsc, 
    adc_channel_t channel, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh, 
    adc_param_config_t awdie); 
#else   // DEV_CODE 
void adc_wd_init(
    ADC_TypeDef *adc, 
    adc_wd_t wd, 
    adc_wd_sc_t wdsc, 
    adc_channel_t channel, 
    uint16_t hi_thresh, 
    uint16_t lo_thresh, 
    adc_awdie_t awdie); 
#endif   // DEV_CODE 


/**
 * @brief Channel conversion sequence 
 * 
 * @details Defines a single channels position in the conversion sequence. Conversion sequences 
 *          are only relevant (and necessary) in scan mode. In this mode the ADC will convert 
 *          the channel defined at sequence position 1, then once done will automatically 
 *          proceed to convert sequence position 2 and so on until the end of the defined 
 *          sequence. This function is called during the initialization sequence only if scan 
 *          mode is being used. A single ADC channel can be assigned to multiple sequence 
 *          positions by calling this function multiples times for the same channel in 
 *          different positions. 
 * 
 * @see adc_channel_t
 * @see adc_seq_num_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param channel : channel being assigned a sequence position 
 * @param seq_num : sequence position of the channel 
 * @return ADC_STATUS : ADC channel conversion sequence initialization status 
 */
#if DEV_CODE 
ADC_STATUS adc_seq(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_seq_num_t seq_num); 
#else   // DEV_CODE 
void adc_seq(
    ADC_TypeDef *adc, 
    adc_channel_t channel, 
    adc_seq_num_t seq_num); 
#endif   // DEV_CODE 


/**
 * @brief Regular channel sequence length setter 
 * 
 * @details If using a sequence (scan mode), this function must be called once after the ADC 
 *          sequence has been defined using the adc_seq function. This specifies the length of 
 *          the defined sequence so the ADC knows when to stop conversions. 
 * 
 * @see adc_seq 
 * 
 * @param adc : pointer to the ADC port used 
 * @param seq_len : number of conversions in the regular channel conversion sequence 
 * @return ADC_STATUS : ADC sequence length setter initialization status 
 */
#if DEV_CODE 
ADC_STATUS adc_seq_len_set(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len); 
#else   // DEV_CODE 
void adc_seq_len_set(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len); 
#endif   // DEV_CODE 

//=======================================================================================


//=======================================================================================
// User functions 

/**
 * @brief Turn ADC on 
 * 
 * @details Enables the ADC. This is needed before the ADC can operate. The ADC must not be 
 *          enabled while configuring the ADC settings. Note that there is a short, blocking 
 *          delay within this function to allow time for the ADC to stabilize after being 
 *          enabled. 
 * 
 * @param adc : pointer to the ADC port used 
 * @return ADC_STATUS : ADC on status 
 */
#if DEV_CODE 
ADC_STATUS adc_on(
    ADC_TypeDef *adc); 
#else   // DEV_CODE 
void adc_on(
    ADC_TypeDef *adc); 
#endif   // DEV_CODE 


/**
 * @brief Turn ADC off 
 * 
 * @details Disables the ADC and puts it in power down mode. This can be used during times 
 *          when the ADC is not needed and must be used if changing ADC settings. By default 
 *          the ADC is disabled on statup so if the ADC needs to be disabled then this function 
 *          only needs to be called after adc_on has been called. 
 * 
 * @param adc : pointer to the ADC port used 
 * @return ADC_STATUS : ADC off status 
 */
#if DEV_CODE 
ADC_STATUS adc_off(
    ADC_TypeDef *adc); 
#else   // DEV_CODE 
void adc_off(
    ADC_TypeDef *adc); 
#endif   // DEV_CODE 


/**
 * @brief Start an ADC conversion 
 * 
 * @details Starts the ADC conversion(s). This function only needs to be called when using DMA 
 *          to convert ADC values. If using continuous mode then this function needs to only 
 *          be called once after enabling the ADC. If using non-continuous mode then this 
 *          function needs to be called every time you want a converison or sequence of 
 *          conversions to happen. Note that this function has no effect (conversion won't 
 *          start) if the ADC is not enabled. 
 * 
 * @see adc_on 
 * 
 * @param adc : pointer to the ADC port used 
 * @return ADC_STATUS : ADC start status 
 */
#if DEV_CODE 
ADC_STATUS adc_start(
    ADC_TypeDef *adc); 
#else   // DEV_CODE 
void adc_start(
    ADC_TypeDef *adc); 
#endif   // DEV_CODE 


/**
 * @brief Read a single ADC conversion 
 * 
 * @details This function allows for converting and reading a single ADC conversion on a 
 *          specified channel. This function is only to be used in single conversion mode 
 *          without DMA. <br>
 * 
 *          Note that this function clears any pre-existing sequences so if using this then 
 *          a conversion sequence shouldn't be defined during initialization. This function 
 *          can be used in systems with a single or multiple ADC channels but each channel 
 *          conversion is only performed periodically and in no particualr order. 
 * 
 * @see adc_channel_t 
 * 
 * @param adc : pointer to the ADC port used 
 * @param channel : channel to perform and read an ADC conversion 
 * @return uint16_t : ADC conversion value 
 */
uint16_t adc_read_single(
    ADC_TypeDef *adc, 
    adc_channel_t channel); 


/**
 * @brief Scan all ADC conversion in the sequence 
 * 
 * @details This function allows for converting and reading all ADC conversions defined in 
 *          a sequence. This function is only to be used in scan mode, with continuous mode 
 *          disabled, and without DMA. <br> 
 *          
 *          During ADC initialization, a sequence of conversions needs to be defined in order
 *          for all the data to be read. The sequence length and buffer size passed to the 
 *          function need to match the sequence length defined in initialization to prevent 
 *          loss of data. 
 * 
 * @see adc_seq
 * @see adc_seq_num_t
 * 
 * @param adc : pointer to the ADC port used 
 * @param seq_len : length of the ADC sequence defined 
 * @param adc_data : pointer to a buffer to store the conversion data 
 * @return ADC_STATUS : ADC scan status 
 */
#if DEV_CODE 
ADC_STATUS adc_scan_seq(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len, 
    uint16_t *adc_data); 
#else   // DEV_CODE 
void adc_scan_seq(
    ADC_TypeDef *adc, 
    adc_seq_num_t seq_len, 
    uint16_t *adc_data); 
#endif   // DEV_CODE 

//=======================================================================================

#endif  // _ANALOG_DRIVER_H_
