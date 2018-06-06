/**
  ******************************************************************************
  * @file    fpg.h
  * @author  ����
  * @version V1.0
  * @date    2017.2.15
  * @brief   ָ�ƴ�������������ͷ�ļ�
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FPG_H
#define _FPG_H

/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"

/* Exported define -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    uint8_t stsus_reg;
    uint8_t sensor;
    uint8_t memorysize;
    uint8_t security;
    uint32_t addr;
    uint8_t packetsize;
    uint8_t bdr;
} FPG_SysPara_t;

/* Exported constants --------------------------------------------------------*/
#define PS_GetImage 0x01    //�Ӵ������϶���ͼ�����ͼ�񻺳���
#define PS_GenChar 0x02     //����ԭʼͼ������ָ���������� CharBuffer1 �� CharBuffer2
#define PS_Match 0x03       //��ȷ�ȶ� CharBuffer1 �� CharBuffer2 �е������ļ�
#define PS_Search 0x04      //�� CharBuffer1 �� CharBuffer2 �е������ļ����������򲿷�ָ�ƿ�
#define PS_RegModel 0x05    //�� CharBuffer1 �� CharBuffer2 �е������ļ��ϲ�����ģ����� CharBuffer2
#define PS_StoreChar 0x06   //�������������е��ļ����浽 flash ָ�ƿ���
#define PS_LoadChar 0x07    //�� flash ָ�ƿ��ж�ȡһ��ģ�嵽����������
#define PS_UpChar 0x08      //�������������е��ļ��ϴ�����λ��
#define PS_DownChar 0x09    //����λ������һ�������ļ�������������
#define PS_UpImage 0x0a     //�ϴ�ԭʼͼ��
#define PS_DownImage 0x0b   //����ԭʼͼ��
#define PS_DeletChar 0x0c   // ɾ�� flash ָ�ƿ��е�һ�������ļ�
#define PS_Empty 0x0d       //��� flash ָ�ƿ�
#define PS_WriteReg 0x0e    //дģ��ϵͳ�Ĵ���
#define PS_ReadSysPara 0x0f //��ϵͳ��������
#define PS_Enroll 0x10      //ע��ģ��
#define PS_Identify 0x11    //��ָ֤��
#define PS_SetPwd 0x12      //�����豸���ֿ���
#define PS_VfyPwd 0x13      //��֤�豸���ֿ���
#define PS_GetRandomCode 0x14 //���������
#define PS_ReadINFpage 0x16         //��ȡ FLASH Information Page ����
#define PS_HighSpeedSearch 0x1b     //�������� FLASH
#define PS_ValidTempleteNum 0x1d   //����Чģ�����

#define PS_TYPE_CMD             0x01
#define PS_TYPE_DATA            0x02
#define PS_TYPE_DATA_LAST       0x08
#define PS_TYPE_ACK             0x07
#define PS_HEAD                 0xEF01
#define PS_ADDR_DEF             0xFFFFFFFF
#define PS_PWD_DEF              0x00

#define PACKET_SIZE_32          0
#define PACKET_SIZE_64          1
#define PACKET_SIZE_128         2
#define PACKET_SIZE_256         3

#define REG_BDR                 4
#define REG_LEAVEL              5
#define REG_PACKET_SIZE         6

/* Exported macro ------------------------------------------------------------*/
#define FPG_BDR(bdr)        (bdr / 9600)

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void FPG_init(void);
uint8_t FPG_PS_GetImage(void);
uint8_t FPG_PS_GenChar(uint8_t charbuffer);
uint8_t FPG_PS_Match(uint16_t *pscore);
uint8_t FPG_PS_Search(uint8_t charbuffer, uint16_t startpage, uint16_t pagenum, uint16_t *pageid, uint16_t *pscore);
uint8_t FPG_PS_RegModel(void);
uint8_t FPG_PS_StoreChar(uint8_t charbuffer, uint16_t pageid);
uint8_t FPG_PS_LoadChar(uint8_t charbuffer, uint16_t pageid);
uint8_t FPG_PS_UpChar(uint8_t charbuffer, uint8_t *data);
uint8_t FPG_PS_DownChar(uint8_t charbuffer, uint8_t *data);
BOOL FPG_PS_UpImage(void);
BOOL FPG_PS_DownImage(void);
uint8_t FPG_PS_DeletChar(uint16_t pageid, uint16_t number);
uint8_t FPG_PS_Empty(void);
uint8_t FPG_PS_WriteReg(uint8_t reg, uint8_t res);
BOOL FPG_PS_ReadSysPara(FPG_SysPara_t *par);
uint8_t FPG_PS_Enroll(uint16_t *pageid);
uint8_t FPG_PS_Identify(uint16_t *pageid, uint16_t *pscore);
uint8_t FPG_PS_SetPwd(uint32_t pwd);
uint8_t FPG_PS_VfyPwd(uint32_t pwd);
uint8_t FPG_PS_GetRandomCode(uint32_t *radom);
BOOL FPG_PS_ReadINFpage(void);
uint8_t FPG_PS_HighSpeedSearch(uint8_t charbuffer, uint16_t startpage, uint16_t pagenum, uint16_t *pageid, uint16_t *pscore);
uint8_t FPG_PS_ValidTempleteNum(uint16_t *number);
char* FPG_GetResultString(uint8_t retnum);

#endif


/************************ (C)  *****END OF FILE****/

