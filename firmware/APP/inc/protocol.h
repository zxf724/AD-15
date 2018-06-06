/**
  ******************************************************************************
  * @file    protocol.h
  * @author  ����
  * @version V1.0
  * @date    2015.12.31
  * @brief   Header file of protocol
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _protocol_H
#define _protocol_H



/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"


/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define MSG_TYPE_CMD			1
#define MSG_TYPE_DATA			3
#define CMD_TIME_RALIB		        0x35
#define CMD_BORROW_UMBRELLA		0x1A
#define CMD_DFU				0x2E
#define CMD_DFU_CLEAR_F			0x3E
#define CMD_Single          0X36
#define CMD_Double          0X29
#define RFID_CMD                0x55
//#define CMD_AUTH					0x00
//#define CMD_FUN_OFF				0x03
//#define CMD_FUN_ON				0x04
//#define CMD_REQ_BAT				0x05
//#define CMD_REQ_STATE			0x06
//#define CMD_REQ_OPENLOG		0x07
//#define CMD_FPG_MANAGE		0x08
//#define CMD_MCARD_MANAGE	0x09
//#define CMD_PWD_MANAGE		0x10
//#define CMD_SET_AUTH			0x15
//#define CMD_FATORY_RESET  0x13

/*Э����մ������ջ���ջ��Ŀ�������������Ϊ2��ڤ�η�ֵ */
#define PROTOCOL_REC_STACK_MAX  16

/*�ְ����ݻ������󳤶�*/
#define MULITI_PACKET_LEN_MAX		512

/*�������Ӻ������ݳ�ʱ�Ͽ�ʱ�䣬��λ��*/
#define BLE_CONNECT_TIMEOUT			180
    
typedef struct 
{
  uint8_t lock_num;
  uint8_t lock_state;
}lock_t;
    

extern BOOL isAuthOK;
extern uint32_t AuthOK_TS;
extern uint8_t mulitiBuf[];
extern uint8_t Random_Running[];
extern uint8_t Key_Running[];

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

void Protocol_Init(void);

void Protocol_NewDate(uint8_t *dat, uint8_t len);
void Protocol_DateProcPoll(void);

void Send_Cmd(uint8_t cmd, uint8_t *arg, uint8_t len);
void Send_CmdDefaultkey(uint8_t cmd, uint8_t* arg, uint8_t len);
void Send_MulitiPacket(uint16_t index, uint8_t *data, uint8_t len);

void Report_SetM1(BOOL ret, uint32_t uid);
void Report_SetFPG(BOOL ret, uint8_t pageid);
uint8_t* Create_Key(uint8_t* random);
uint8_t* Create_Random(void);
void AesData_decrypt(uint8_t* data, uint8_t* key, uint16_t len);
void ReBack(uint8_t *dat,uint8_t cmd,uint8_t * param,uint8_t len);
void encrypt_data(uint8_t *dat,uint8_t len);
uint8_t Handle_Single_Lock(void);
uint8_t Handle_Double_Lock(void);
uint32_t getWordFromStr(uint8_t *str);







#endif /* _protocol_H */

/************************ (C)  *****END OF FILE****/

