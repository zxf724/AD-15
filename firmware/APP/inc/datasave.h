/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file Datasave.h
 * @author ����
 * @version V1.0
 * @date 2016.12.15
 * @brief ҵ���߼�������ͷ�ļ�.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _DATASAVE_H
#define _DATASAVE_H


/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"




#define    UmbrellaID_Num      100

/* Exported types ------------------------------------------------------------*/
/*MQTT���Ӳ���*/
typedef struct
{
    char  MQTT_Server[32];
    char  MQTT_ClientID[48];
    char  MQTT_UserName[48];
    uint16_t MQTT_Port;
    uint16_t MQTT_Timout;
    uint16_t MQTT_PingInvt;
} MQTT_Param_t;

/*GPRS����*/
typedef struct
{
    char APN[16];
    char APN_User[16];
    char APN_PWD[16];
} GPRS_WorkParam_t;

/*��������*/
typedef struct
{
    uint16_t version;
    uint16_t crc;
    MQTT_Param_t mqtt;
    GPRS_WorkParam_t gprs;
    uint32_t StartLogAddr;
    uint16_t UmbrellaCount;
    uint32_t UmbrellaID[UmbrellaID_Num];
} CommParam_t;




/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define PARAM_LEGAL(par)    (isalnum(par[0]) && (strlen(par) < OBJ_LEN(par)))

#define SAVE_ADDR()         (SECTOR_ADDR(DATA_SAVE_SECTOR))
#define SAVE_ADDR_MAX()     (SECTOR_ADDR(DATA_SAVE_SECTOR + SECTOR_DATA_SAVE_SIZE))

/* Exported variables --------------------------------------------------------*/
extern CommParam_t  CommParam;

/* Exported functions --------------------------------------------------------*/
void DataSave_Init(void);

void WorkParam_Init(void);
BOOL WorkParam_Save(void);

#endif


