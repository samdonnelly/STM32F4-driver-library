/**
 * @file stm32f4x.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Common register info for all STM32F4 models 
 * 
 * @details This allows drivers to use register types without being tied to a specific 
 *          device. 
 * 
 * @version 0.1
 * @date 2024-12-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _STM32F4X_H_ 
#define _STM32F4X_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "system_config.h" 

//=======================================================================================


//=======================================================================================
// Board selection 

#if STM32F4_05xx 
  #include "stm32f405xx.h"
#elif STM32F4_15xx 
  #include "stm32f415xx.h"
#elif STM32F4_07xx 
  #include "stm32f407xx.h"
#elif STM32F4_17xx 
  #include "stm32f417xx.h"
#elif STM32F4_27xx 
  #include "stm32f427xx.h"
#elif STM32F4_37xx 
  #include "stm32f437xx.h"
#elif STM32F4_29xx 
  #include "stm32f429xx.h"
#elif STM32F4_39xx 
  #include "stm32f439xx.h"
#elif STM32F4_01xC 
  #include "stm32f401xc.h"
#elif STM32F4_01xE 
  #include "stm32f401xe.h"
#elif STM32F4_10Tx 
  #include "stm32f410tx.h"
#elif STM32F4_10Cx 
  #include "stm32f410cx.h"
#elif STM32F4_10Rx 
  #include "stm32f410rx.h"
#elif STM32F4_11xE 
  #include "stm32f411xe.h"
#elif STM32F4_46xx 
  #include "stm32f446xx.h"
#elif STM32F4_69xx 
  #include "stm32f469xx.h"
#elif STM32F4_79xx 
  #include "stm32f479xx.h"
#elif STM32F4_12Cx 
  #include "stm32f412cx.h"
#elif STM32F4_12Zx 
  #include "stm32f412zx.h"
#elif STM32F4_12Rx 
  #include "stm32f412rx.h"
#elif STM32F4_12Vx 
  #include "stm32f412vx.h"
#elif STM32F4_13xx 
  #include "stm32f413xx.h"
#elif STM32F4_23xx 
  #include "stm32f423xx.h"
#else
 #error "Please select first the target STM32F4xx device used in your application" \
        "(in stm32f4x.h file)"
#endif

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _STM32F4X_H_ 
