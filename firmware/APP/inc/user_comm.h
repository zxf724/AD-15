/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file uaer_comm.h
 * @author ����
 * @version V1.0
 * @date 2016.4.1
 * @brief �û�����ͷ�ļ�.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USER_COMM_H
#define _USER_COMM_H

/* Includes ------------------------------------------------------------------*/


#include "prjlib.h"
#include "user_uart.h"
#include "spi_flash.h"

#include "datasave.h"

   
   
   
   


/* Exported define -----------------------------------------------------------*/
/*���Դ��ںţ�Ϊ0ʱ�رմ�ӡ���*/


/*DEBUG ��Ϣ�ȼ���̬������ʹ��*/
#define LOG_LEVEL_DYNAMIC   1

/*DEBUG
  ��ϢĬ�ϵȼ�,������Ϊ�������ֺ����չ��*/
#define LOG_LEVEL_DEF       4

/*GPRS�Ĵ��ں�*/
#define GPRS_UART_PORT      2

/*RFID���ߴ��ں�*/
#define RFID_UART_PORT      3

/*����DFU���BKP�Ĵ���*/
#define DFU_BKP             (BKP->DR10)

/*���帴λ��ʶ����BKP*/
#define NRST_BKP            (BKP->DR9)
#define IWDG_BKP            (BKP->DR8)
#define SWRST_BKP           (BKP->DR7)
#define PORRST_BKP          (BKP->DR6)

/*ʹ��flash��д����*/
#define FLASH_WRP_EN        1

/*ʹ��Ӳ�����Ź�*/
#define IWDG_HW_EN          1

/*�汾�Ŷ���*/
//#define PROJECT             "AD-02"
//#define VERSION             "AD-02_FM_V1.01"
//#define VERSION_HARDWARE    "AD-02_HD_V1.0"
//#define VERSION_GSM         "SIM800C"

/*UART���ջ���Ĵ�С������Ϊ2���ݴη�ֵ*/
#define UART1_RECEVIE_BUFFER_SIZE   2048
#define UART2_RECEVIE_BUFFER_SIZE   2048
#define UART3_RECEVIE_BUFFER_SIZE   1024

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define DBG_LEVEL_ERR           1
#define DBG_LEVEL_WAR           2
#define DBG_LEVEL_LOG           3
#define DBG_LEVEL_INFO          4
#define DBG_LEVEL_DEBUG         5
#define DBG_LEVEL_TEMP          6

/* Exported macro ------------------------------------------------------------*/

/*����DEBUG��Ϣ*/


/*DEBUG ��Ϣ�ȼ���̬*/
#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_ERR))
#define DBG_ERR(format, ...)    CMD_Printf("error> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_ERR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_WAR))
#define DBG_WAR(format, ...)    CMD_Printf("warring> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_WAR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_LOG))
#define DBG_LOG(format, ...)    CMD_Printf("log> "format"\r\n", ##__VA_ARGS__)

#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_INFO))
#define DBG_INFO(format, ...)   CMD_Printf("inf> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_INFO(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_DEBUG))
#define DBG_DBG(format, ...)    CMD_Printf("dbg> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_DBG(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_TEMP))
#define DBG_TEMP(format, ...)   CMD_Printf("temp> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_TEMP(format, ...)
#endif



#define THROW(str)
#define DBG_HEX(dat, len)
#define DBG_PRINT(level, format, ...)
#define DBG_PRINTBUF(level, format, buf, len)

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/



#endif
