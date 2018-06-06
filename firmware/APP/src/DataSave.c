
/**
 * *********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file DataSave.c
 * @version V1.0
 * @date 2016.12.16
 * @brief ���ݴ洢������.
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author ����
 */



/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
CommParam_t  CommParam;

/* Private function prototypes -----------------------------------------------*/
static void datasave_Console(int argc, char *argv[]);

/* Exported functions --------------------------------------------------------*/
/**
 * ҵ�����ʼ��
 */
void DataSave_Init(void)
{
    CMD_ENT_DEF(datasave, datasave_Console);
    Cmd_AddEntrance(CMD_ENT(datasave));

    DBG_LOG("DataSave Init.");
}

/**
 * ����������ʼ������洢�����д��Ϊ�գ���ָ�Ĭ�ϲ���.
 */
void WorkParam_Init(void)
{
    uint16_t crc = 0;
    uint32_t addr = 0;

    addr = SECTOR_ADDR(WORK_PARAM_SECTOR);
    SFlash_Read(addr, (uint8_t *)&CommParam, OBJ_LEN(CommParam_t));

    crc = CRC_16(0, (uint8_t *)&(CommParam)+4, OBJ_LEN(CommParam_t) - 4);
    if (CommParam.crc == crc && crc != 0 && crc != BIT16_MAX) {
        DBG_LOG("Work parameter sector1 load OK.");
        return;
    } else {
//        addr = SECTOR_ADDR(WORK_PARAM_SECTOR + 1);
        SFlash_Read(addr, (uint8_t *)&CommParam, OBJ_LEN(CommParam_t));
        crc = CRC_16(0, (uint8_t *)&(CommParam)+4, OBJ_LEN(CommParam_t) - 4);
    }
    if (CommParam.crc == crc && crc != 0 && crc != BIT16_MAX) {
        DBG_LOG("Work parameter sector2 load OK.");
        return;
    }
    if (CommParam.version == BIT16_MAX || CommParam.version == 0) {
//        addr = SECTOR_ADDR(WORK_PARAM_SECTOR);
        SFlash_Read(addr, (uint8_t *)&CommParam, OBJ_LEN(CommParam_t));
        CommParam.version = 0;
        DBG_LOG("NO Work parameter!!!");
    } else {
        DBG_LOG("Work parameter Break!!!");
    }
    /*������߰汾ʱ�ظ���������*/
    if (CommParam.version > 1) {
        CommParam.version = 0;
    }

    if (CommParam.version < 1) {
        DBG_LOG("Work version1 default.");
        CommParam.version = 1;

        /*��ʼ��mqtt����*/
        CommParam.mqtt.MQTT_Port = 1883;
        strcpy(CommParam.mqtt.MQTT_ClientID, "80000001");
        CommParam.UmbrellaCount = 100;
        strcpy(CommParam.mqtt.MQTT_Server, "139.199.175.39");
        strcpy(CommParam.mqtt.MQTT_UserName, "afusmart");
//        WorkParam.mqtt.MQTT_Timout = MQTT_TIMEOUT_DEF;
//        WorkParam.mqtt.MQTT_PingInvt = MQTT_PING_INVT_DEF;

        /*��ʼ��GPRS����*/
        CommParam.gprs.APN[0] = 0;
        CommParam.gprs.APN_User[0] = 0;
        CommParam.gprs.APN_PWD[0] = 0;

        /*��ʼ��������־��¼*/
        CommParam.StartLogAddr = 0;


        DBG_LOG("Work parameter verison1 default.");
    }
    WorkParam_Save();
}

/**
 * ���������洢
 * @return
 */
BOOL WorkParam_Save(void)
{
    uint16_t crc = 0;
    uint32_t addr = 0, len = OBJ_LEN(CommParam_t);

    crc = CRC_16(0, (uint8_t *)&(CommParam)+4, len - 4);
    CommParam.crc = crc;

    addr = SECTOR_ADDR(WORK_PARAM_SECTOR + 1);
    SFlash_EraseSectors_NotCheck(addr, 1);
    SFlash_Write_NotCheck(addr, (uint8_t *)&CommParam, len);

    addr = SECTOR_ADDR(WORK_PARAM_SECTOR);
    SFlash_EraseSectors_NotCheck(addr, 1);
    return SFlash_Write_NotCheck(addr, (uint8_t *)&CommParam, len);
}

/* Private function prototypes -----------------------------------------------*/

/**
 * ���ݴ洢��������
 * @param argc ����������
 * @param argv �����б�
 */
static void datasave_Console(int argc, char *argv[])
{
    argv++;
    argc--;
    if (ARGV_EQUAL("CommParam")) {
        DBG_LOG("CommParam length:%u", OBJ_LEN(CommParam));
    }
   if (ARGV_EQUAL("deviceid")) {
      if (argv[1] != NULL) {
         strcpy(CommParam.mqtt.MQTT_ClientID, argv[1]);
        WorkParam_Save();
        }
        DBG_LOG("Device ID:%s",CommParam.mqtt.MQTT_ClientID );
	}
}
