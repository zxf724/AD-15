/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file mqtt_conn.h
 * @author ����
 * @version V1.0
 * @date 2016.12.20
 * @brief MQTT���ӹ������ļ�.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _MQTT_CONN_H
#define _MQTT_CONN_H


/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"
#include "MQTTPacket.h"
#include "MQTTClient.h"

/* Exported define -----------------------------------------------------------*/
/*MQTT����ʹ��*/
#define MQTT_DEBUG                      0

/*Э�鴦���泤��*/
#define MQTT_TX_BUFF_SIZE               2048
#define MQTT_RX_BUFF_SIZE               2048

/*MQTT���ݷ��͵ĳ�ʱʱ�䣬��λ����*/
#define MQTT_TIMEOUT_DEF                5000

/*MQTT�����������,ʵ�ʷ��ͼ��Ϊ����ֵ��һ�룬��λ��*/
#define MQTT_PING_INVT_DEF              90

/*����ʧ�����¼�Ȩ��ʧ�ܴ���*/
#define CONNECT_FAIL_REAUTH             5
/*����ʧ�ܳ�ʱʱ��,��λ��*/
#define CONNECT_FAIL_TIMEOUT            30

#define MQTT_TASK_STK_SIZE              512
#define MQTT_TASK_PRIO                  osPriorityBelowNormal
#define MQTT_SEND_Q_SIZE                8

/* Exported types ------------------------------------------------------------*/

/*MQTT���Ӳ���*/
typedef struct
{
    char  *MQTT_Server;
    uint16_t MQTT_Port;
    char  *MQTT_ClientID;
    char  *MQTT_UserName;
    char  *MQTT_PassWord;
} MQTT_ConnParam_t;

/*MQTT���Ļص���������*/
typedef void (*Arrived_t)(uint8_t *data, uint16_t len);

/*MQTT����ص�����*/
typedef void (*MQTT_TaskPollFun)(void);

/*MQTT��Ϣ���ݻص�����*/
typedef void (*MQTT_MsgDataFun)(char *topic,  uint8_t *payload, uint16_t len);

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void MQTT_Conn_Init(void);

void MQTT_SetHookFun(MQTT_TaskPollFun poll, MQTT_MsgDataFun sendfail);

BOOL MQTT_IsConnected(void);
BOOL MQTT_IsDataFlow(void);

int16_t MQTT_SendData(uint8_t *dat, uint16_t len);
int16_t MQTT_ReadData(uint8_t *dat, uint16_t len);

BOOL Publish_MQTT(char const *topic, Qos qos, uint8_t *payload, uint16_t len);
BOOL Subscribe_MQTT(char const *topic, Qos qos, Arrived_t fun);

#endif
