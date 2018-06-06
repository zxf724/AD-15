/**
  ******************************************************************************
  * @file    COMMAND.h
  * @author  ����
  * @version V1.0
  * @date    2015.12.31
  * @brief   Header file of command
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _COMMAND_H
#define _COMMAND_H



/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"

/** @addtogroup firmwave_F1_MCU
  * @{
  */


/** @defgroup COMMAND
  * @{
  */


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/

/** @defgroup COMMAND_Exported_Constants COMMAND Exported Constants
  * @{
  */
#define COMMAND_MAX        UART_RX_BUF_SIZE

    
    
extern uint8_t  RFID_DATA[32];
extern uint8_t umbrellaID_num;
    
extern uint8_t  RFID_DATA_Flag;
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/** @addtogroup COMMAND_Exported_Functions
  * @{
  */
void Command_Init(void);

void CommandReceive_Poll(void);

void RfidReceive_Poll(void) ;

void Receive_Poll(void);
/**
  * @}
  */



/**
  * @}
  */

/**
  * @}
  */



#endif /* _COMMAND_H */

/************************ (C)  *****END OF FILE****/

