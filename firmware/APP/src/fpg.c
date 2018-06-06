/**
	******************************************************************************
	* @file    fpg.c
	* @author  ����
	* @version V1.0
	* @date    2017.12.15
	* @brief   ָ�ƴ�������������غ���.
	*
	******************************************************************************
	*/


/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void fpg_SendCmd(uint8_t cmd, uint8_t* arg, uint16_t arglen);
static void fpg_SendData(uint8_t type, uint8_t* data, uint16_t datalen);
static uint8_t fpg_ReadWaitAck(uint8_t* arg,  uint16_t timeout);
static uint8_t fpg_ReadWaitData(uint8_t* buff, uint16_t buflen, uint16_t timeout);
static void fpg_Console(int argc, char* argv[]);

/* Exported functions --------------------------------------------------------*/

/**
 * ָ�ƴ�������ʼ��
 */
void FPG_init(void) {
	CMD_ENT_DEF(fpg, fpg_Console);
	Cmd_AddEntrance(CMD_ENT(fpg));

#if DEBUG == 0
	nrf_uart_txrx_pins_set(NRF_UART0, FPG_TX_PIN, FPG_RX_PIN);
#endif

	DBG_LOG("FPG init.");
}

/**
 * ¼��ͼ��
 * ����˵����̽����ָ��̽�⵽��¼��ָ��ͼ�����ImageBuffer������ȷ�����ʾ¼��ɹ���
 * ����ָ�ȡ�
 *
 * @return ¼��ɹ�����TRUE
 */
uint8_t FPG_PS_GetImage(void) {
	fpg_SendCmd(PS_GetImage, NULL, 0);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * ��������
 * ����˵������ImageBuffer�е�ԭʼͼ������ָ�������ļ�����CharBuffer1��CharBuffer2
 *
 * @param charbuffer 1Ϊ������CharBuffer1��2ΪCharBuffer2
 * @return ���ɳɹ�����TRUE
 */
uint8_t FPG_PS_GenChar(uint8_t charbuffer) {
	fpg_SendCmd(PS_GenChar, &charbuffer, 1);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * ��ȷ�ȶ���öָ������
 * ����˵������ȷ�ȶ�CharBuffer1 ��CharBuffer2
 * �е������ļ�
 *
 * @param pscore �÷�
 * @return ָ��ƥ�䷵��TRUE
 */
uint8_t FPG_PS_Match(uint16_t* pscore) {
	uint8_t buf[2], ret;

	fpg_SendCmd(PS_Match, NULL, 0);
	ret = fpg_ReadWaitAck(buf, 1000);
	if (ret == 0) {
		*pscore = (buf[0] << 8) | buf[1];
	}
	return ret;
}

/**
 * ����ָ��
 * ����˵������CharBuffer1��CharBuffer2�е������ļ����������򲿷�ָ�ƿ⡣����������
 * �򷵻�ҳ�롣
 *
 * @param charbuffer 1Ϊ������CharBuffer1��2ΪCharBuffer2
 * @param startpage  ��ʼҳ
 * @param pagenum    ҳ��
 * @param pageid     ҳ��
 * @param pscore     �÷�
 * @return �����ɹ�����TRUE
 */
uint8_t FPG_PS_Search(uint8_t charbuffer, uint16_t startpage, uint16_t pagenum, uint16_t* pageid, uint16_t* pscore) {
	uint8_t buf[5], ret;

	buf[0] = charbuffer;
	buf[1] = (uint8_t)(startpage >> 8);
	buf[2] = (uint8_t)(startpage >> 0);
	buf[3] = (uint8_t)(pagenum >> 8);
	buf[4] = (uint8_t)(pagenum >> 0);

	fpg_SendCmd(PS_Search, buf, 5);
	ret = fpg_ReadWaitAck(buf, 3000);
	if (ret == 0 && pageid != NULL && pscore != NULL) {
		*pageid = (buf[0] << 8) | buf[1];
		*pscore = (buf[2] << 8) | buf[3];
	}
	return ret;
}

/**
 * �ϲ�����������ģ�壩�� PS_RegModel
 * ����˵������CharBuffer1��CharBuffer2�е������ļ��ϲ�����ģ�壬�������
 * CharBuffer1��CharBuffer2
 *
 * @return �ϲ��ɹ�����TRUE
 */
uint8_t FPG_PS_RegModel(void) {
	fpg_SendCmd(PS_RegModel, NULL, 0);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * ����ģ�壺PS_StoreChar
 * ����˵������CharBuffer1��CharBuffer2�е�ģ���ļ��浽PageID��flash���ݿ�λ�á�
 * ���������BufferID(��������)��PageID��ָ�ƿ�λ�úţ�
 *
 * @param charbuffer ��������
 * @param pageid     ָ�ƿ�λ�ú�
 * @return ����ɹ��ɹ�����TRUE
 */
uint8_t FPG_PS_StoreChar(uint8_t charbuffer, uint16_t pageid) {
	uint8_t buf[3], ret;

	buf[0] = charbuffer;
	buf[1] = (uint8_t)(pageid >> 8);
	buf[2] = (uint8_t)(pageid >> 0);

	fpg_SendCmd(PS_StoreChar, buf, 3);
	ret = fpg_ReadWaitAck(NULL, 1000);
	return ret;
}

/**
 * ����ģ�壺PS_LoadChar
 * ����˵������flash���ݿ���ָ��ID�ŵ�ָ��ģ����뵽ģ�建����CharBuffer1��
 * CharBuffer2
 *
 * @param charbuffer ��������
 * @param pageid     ָ�ƿ�ģ���
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_LoadChar(uint8_t charbuffer, uint16_t pageid) {
	uint8_t buf[3];

	buf[0] = charbuffer;
	buf[1] = (uint8_t)(pageid >> 8);
	buf[2] = (uint8_t)(pageid >> 0);

	fpg_SendCmd(PS_LoadChar, buf, 3);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * �ϴ�������ģ�壺PS_UpChar
 * ����˵�����������������е������ļ��ϴ�����λ��
 *
 * @param charbuffer ��������
 * @param data ������ָ��
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_UpChar(uint8_t charbuffer, uint8_t* data) {
	uint8_t ret;

	fpg_SendCmd(PS_UpChar, &charbuffer, 1);
	ret = fpg_ReadWaitAck(NULL, 1000);
	if (ret == 0) {
		if (fpg_ReadWaitData(data, 256, 1000) == 0) {
			fpg_ReadWaitData(data + 256, 256, 1000);
		}
	}
	return ret;
}

/**
 * ����������ģ�壺PS_DownChar
 * ����˵������λ�����������ļ���ģ���һ������������
 *
 * @param charbuffer ��������
 * @param data       д�������
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_DownChar(uint8_t charbuffer, uint8_t* data) {
	uint8_t ret;

	fpg_SendCmd(PS_DownChar, &charbuffer, 1);
	ret = fpg_ReadWaitAck(NULL, 1000); 
	if (ret == 0) {
		fpg_SendData(PS_TYPE_DATA_LAST, data, 256);
		/*��ʱ����UART FIFO���*/
		nrf_delay_ms(10);
		fpg_SendData(PS_TYPE_DATA_LAST, data + 256, 256);
	}
	return ret;
}

/**
 * �ϴ�ͼ��PS_UpImage
 * ����˵������ͼ�񻺳����е������ϴ�����λ��
 *
 * @return ִ�гɹ�����TRUE
 */
BOOL FPG_PS_UpImage(void) {
	fpg_SendCmd(PS_UpImage, NULL, 0);
	if (fpg_ReadWaitAck(NULL, 1000) == 0) {
		//����ͼ���ļ�����
		return TRUE;
	}
	return FALSE;
}

/**
 * ����ͼ��PS_DownImage
 * ����˵������λ������ͼ�����ݸ�ģ��
 *
 * @return ִ�гɹ�����TRUE
 */
BOOL FPG_PS_DownImage(void) {
	fpg_SendCmd(PS_DownImage, NULL, 0);
	if (fpg_ReadWaitAck(NULL, 1000) == 0) {
		//����ͼ���ļ�����
		return TRUE;
	}
	return FALSE;
}

/**
 * ɾ��ģ�壺PS_DeletChar
 * ����˵����ɾ��flash ���ݿ���ָ��ID�ſ�ʼ��N��ָ��ģ��
 *
 * @param pageid ָ�ƿ�ģ���
 * @param number ɾ����ģ�����
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_DeletChar(uint16_t pageid, uint16_t number) {
	uint8_t buf[4];

	buf[0] = (uint8_t)(pageid >> 8);
	buf[1] = (uint8_t)(pageid >> 0);
	buf[2] = (uint8_t)(number >> 8);
	buf[3] = (uint8_t)(number >> 0);

	fpg_SendCmd(PS_DeletChar, buf, 4);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * ���ָ�ƿ⣺PS_Empty
 * ����˵����ɾ��flash ���ݿ�������ָ��ģ��
 *
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_Empty(void) {
	fpg_SendCmd(PS_Empty, NULL, 0);
	return fpg_ReadWaitAck(NULL, 2000);
}

/**
 * дϵͳ�Ĵ�����PS_WriteReg
 * ����˵����дģ��Ĵ���
 *
 * @param reg    �Ĵ������
 * @param res    д������
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_WriteReg(uint8_t reg, uint8_t res) {
	uint8_t buf[2];

	buf[0] = (uint8_t)(reg);
	buf[1] = (uint8_t)(res);

	fpg_SendCmd(PS_WriteReg, buf, 2);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * ��ϵͳ����������PS_ReadSysPara
 * ����˵������ȡģ��Ļ��������������ʣ�����С�ȣ�
 * ������ǰ16 ���ֽڴ����ģ��Ļ���ͨѶ��������Ϣ����Ϊģ��Ļ�������
 *
 * @param par    �����Ĳ���
 * @return ִ�гɹ�����TRUE
 */
BOOL FPG_PS_ReadSysPara(FPG_SysPara_t* par) {
	uint8_t buf[16];

	memset(buf, 0, 12);

	fpg_SendCmd(PS_ReadSysPara, NULL, 0);
	if (fpg_ReadWaitAck(buf, 1000) == 0) {
		DBG_LOG("FPG_PS_ReadSysPara:%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x,%#02x",
						buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
		//��ȷ�ϲ���
		return TRUE;
	}
	return FALSE;
}

/**
 * �Զ�ע��ģ�壺PS_Enroll
 * ����˵�����ɼ�һ��ָ��ע��ģ�壬��ָ�ƿ���������λ���洢�����ش洢ID
 *
 * @param pageid ע���ҳ��
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_Enroll(uint16_t* pageid) {
	uint8_t buf[2], ret;

	fpg_SendCmd(PS_Enroll, NULL, 0);
	ret = fpg_ReadWaitAck(buf, 3000);
	if (ret == 0) {
		*pageid = (buf[0] << 8) | buf[1];
	}
	return ret;
}

/**
 *  �Զ���ָ֤�ƣ�PS_Identify
 * ����˵�����Զ��ɼ�ָ�ƣ���ָ�ƿ�������Ŀ��ģ�岢�����������
 * ���Ŀ��ģ��ͬ��ǰ�ɼ���ָ�ƱȶԵ÷ִ�����߷�ֵ������Ŀ��ģ��Ϊ����
 * ���������Բɼ�����������Ŀ��ģ��Ŀհ�����
 *
 * @param pageid ҳ��
 * @param pscore �÷�
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_Identify(uint16_t* pageid, uint16_t* pscore) {
	uint8_t buf[4], ret;

	fpg_SendCmd(PS_Identify, NULL, 0);
	ret = fpg_ReadWaitAck(buf, 3000);
	if (ret == 0) {
		*pageid = (buf[0] << 8) | buf[1];
		*pscore = (buf[2] << 8) | buf[3];
	}
	return ret;
}

/**
 * ���ÿ��PS_SetPwd
 * ����˵��������ģ�����ֿ���
 *
 * @param pwd    ����
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_SetPwd(uint32_t pwd) {
	uint8_t buf[4];

	buf[0] = ((uint8_t)(pwd >> 24));
	buf[1] = ((uint8_t)(pwd >> 18));
	buf[2] = ((uint8_t)(pwd >> 8));
	buf[3] = ((uint8_t)pwd);

	fpg_SendCmd(PS_SetPwd, buf, 4);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * ��֤���PS_VfyPwd
 * ����˵������֤ģ�����ֿ���
 *
 * @param pwd    ����
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_VfyPwd(uint32_t pwd) {
	uint8_t buf[4];

	buf[0] = ((uint8_t)(pwd >> 24));
	buf[1] = ((uint8_t)(pwd >> 18));
	buf[2] = ((uint8_t)(pwd >> 8));
	buf[3] = ((uint8_t)pwd);

	fpg_SendCmd(PS_VfyPwd, buf, 4);
	return fpg_ReadWaitAck(NULL, 1000);
}

/**
 * �����������PS_GetRandomCode
 * ����˵������оƬ����һ������������ظ���λ��
 *
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_GetRandomCode(uint32_t* radom) {
	uint8_t buf[4], ret;

	fpg_SendCmd(PS_GetRandomCode, NULL, 0);
	ret = fpg_ReadWaitAck(buf, 1000);
	if (ret == 0) {
		*radom = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3] << 0);
	}
	return ret;
}

/**
 * ��flash��Ϣҳ��PS_ReadINFpage
 * ����˵������ȡFLASH Information Page ���ڵ���Ϣҳ(512bytes)
 *
 * @return ִ�гɹ�����TRUE
 */
BOOL FPG_PS_ReadINFpage(void) {
	fpg_SendCmd(PS_ReadINFpage, NULL, 0);
	if (fpg_ReadWaitAck(NULL, 1000) == 0) {
		//��������
		return TRUE;
	}
	return FALSE;
}

/**
 * ����������PS_HighSpeedSearch
 * ����˵������CharBuffer1 ��CharBuffer2 �е������ļ��������������򲿷�ָ�ƿ⡣��
 * ���������򷵻�ҳ�롣
 * ��ָ����ڵ�ȷ������ָ�ƿ��У��ҵ�¼ʱ�����ܺõ�ָ�ƣ���ܿ��������
 * �����
 *
 * @param charbuffer 1Ϊ������CharBuffer1��2ΪCharBuffer2
 * @param startpage  ��ʼҳ
 * @param pagenum    ҳ��
 * @param pageid     ҳ��
 * @param pscore     �÷�
 * @return �����ɹ�����TRUE
 */
uint8_t FPG_PS_HighSpeedSearch(uint8_t charbuffer, uint16_t startpage, uint16_t pagenum, uint16_t* pageid, uint16_t* pscore) {
	uint8_t buf[5], ret;

	buf[0] = charbuffer;
	buf[1] = (uint8_t)(startpage >> 8);
	buf[2] = (uint8_t)(startpage >> 0);
	buf[3] = (uint8_t)(pagenum >> 8);
	buf[4] = (uint8_t)(pagenum >> 0);

	fpg_SendCmd(PS_HighSpeedSearch, buf, 5);
	ret = fpg_ReadWaitAck(buf, 1000);
	if (ret == 0) {
		*pageid = (buf[0] << 8) | buf[1];
		*pscore = (buf[2] << 8) | buf[3];
	}
	return ret;
}

/**
 * ����Чģ�������PS_ValidTempleteNum
 * ����˵��������Чģ�����
 *
 * @param number ������ģ�����
 * @return ִ�гɹ�����TRUE
 */
uint8_t FPG_PS_ValidTempleteNum(uint16_t* number) {
	uint8_t buf[2], ret;

	fpg_SendCmd(PS_ValidTempleteNum, NULL, 0);
	ret = fpg_ReadWaitAck(buf, 200);
	if (ret == 0 && number != NULL) {
		*number = (buf[0] << 8) | buf[1];
	}
	return ret;
}

/**
 * ��ȡָ�ƴ�����������Ϣ�ַ�
 *
 * @param retnum ���ص���ֵ
 * @return �����ַ���ָ��
 */
char* FPG_GetResultString(uint8_t retnum) {
	switch (retnum) {
		case 0x00:
			return "ָ��ִ����ϻ� OK";
		case 0x01:
			return "���ݰ����մ���";
		case 0x02:
			return "��������û����ָ";
		case 0x03:
			return "¼��ָ��ͼ��ʧ��";
		case 0x04:
			return "ָ��ͼ��̫�ɡ�̫��������������";
		case 0x05:
			return "ָ��ͼ��̫ʪ��̫ģ��������������";
		case 0x06:
			return "ָ��ͼ��̫�Ҷ�����������";
		case 0x07:
			return "ָ��ͼ����������������̫�٣������̫С��������������";
		case 0x08:
			return "ָ�Ʋ�ƥ��";
		case 0x09:
			return "û������ָ��";
		case 0x0a:
			return "�����ϲ�ʧ��";
		case 0x0b:
			return "����ָ�ƿ�ʱ��ַ��ų���ָ�ƿⷶΧ";
		case 0x0c:
			return "��ָ�ƿ��ģ��������Ч";
		case 0x0d:
			return "�ϴ�����ʧ��";
		case 0x0e:
			return "ģ�鲻�ܽ��պ������ݰ�";
		case 0x0f:
			return "�ϴ�ͼ��ʧ��";
		case 0x10:
			return "ɾ��ģ��ʧ��";
		case 0x11:
			return "���ָ�ƿ�ʧ��";
		case 0x12:
			return "���ܽ���͹���״̬";
		case 0x13:
			return "�����ȷ";
		case 0x14:
			return "ϵͳ��λʧ��";
		case 0x15:
			return "��������û����Чԭʼͼ��������ͼ��";
		case 0x16 :
			return "��������ʧ��";
		case 0x17:
			return "����ָ�ƻ����βɼ�֮����ָû���ƶ���";
		case 0x18:
			return "��д FLASH ����";
		case 0xf0:
			return "�к������ݰ���ָ���ȷ���պ��� 0xf0 Ӧ��";
		case 0xf1:
			return "�к������ݰ���ָ�������� 0xf1 Ӧ��";
		case 0xf2:
			return "��д�ڲ� FLASH ʱ��У��ʹ���";
		case 0xf3:
			return "��д�ڲ� FLASH ʱ������ʶ����";
		case 0xf4:
			return "��д�ڲ� FLASH ʱ�������ȴ���";
		case 0xf5:
			return "��д�ڲ� FLASH ʱ�����볤��̫��";
		case 0xf6:
			return "��д�ڲ� FLASH ʱ����д FLASH ʧ��";
		case 0x19:
			return "δ�������";
		case 0x1a:
			return "��Ч�Ĵ�����";
		case 0x1b:
			return "�Ĵ����趨���ݴ����";
		case 0x1c:
			return "���±�ҳ��ָ������";
		case 0x1d:
			return "�˿ڲ���ʧ��";
		case 0x1e:
			return "�Զ�ע�ᣨenroll��ʧ��";
		case 0x1f:
			return "ָ�ƿ���";
			/*�Զ�������*/
		case 0xfe:
			return "��λ������У�����";
		case 0xff:
			return "��ʱ";
		default :
			return "ʾ֪����";
	}
}

/* Private function prototypes -----------------------------------------------*/

/**
 * ָ�ƴ��������������
 *
 * @param cmd     ָ��
 * @param arg     ����ֵ
 * @param arglen  �����ĳ���
 */
static void fpg_SendCmd(uint8_t cmd, uint8_t* arg, uint16_t arglen) {
	uint16_t add = 0, len = 0, i = 0;
#if DEBUG == 1
	/*��ʱ�����ڿ���*/
	nrf_delay_ms(10);
//	nrf_uart_txrx_pins_set(NRF_UART0, FPG_TX_PIN, FPG_RX_PIN);
#endif

	app_uart_put((uint8_t)(PS_HEAD >> 8));
	app_uart_put((uint8_t)PS_HEAD);

	app_uart_put((uint8_t)(PS_ADDR_DEF >> 24));
	app_uart_put((uint8_t)(PS_ADDR_DEF >> 18));
	app_uart_put((uint8_t)(PS_ADDR_DEF >> 8));
	app_uart_put((uint8_t)PS_ADDR_DEF);

	app_uart_put(PS_TYPE_CMD);
	len = arglen + 3;
	app_uart_put(len >> 8);
	app_uart_put(len);
	app_uart_put(cmd);

	add = PS_TYPE_CMD;
	add += len;
	add += cmd;
	for (i = 0; i < arglen && arg != NULL; i++) {
		app_uart_put(arg[i]);
		add += arg[i];
	}
	app_uart_put(add >> 8);
	app_uart_put(add);

#if DEBUG == 1
	nrf_delay_ms(10);
	nrf_uart_txrx_pins_set(NRF_UART0, TX_PIN_NUMBER, RX_PIN_NUMBER);
#endif
}

/**
 * ָ�ƴ������������ݰ�
 *
 * @param type    �������ͣ����ݰ����ǽ�����
 * @param data    ����ָ��
 * @param datalen ���ݳ���
 */
static void fpg_SendData(uint8_t type, uint8_t* data, uint16_t datalen) {
	uint16_t add = 0, len = 0, i = 0;

#if DEBUG == 1
	/*��ʱ�����ڿ���*/
	nrf_delay_ms(10);
	nrf_uart_txrx_pins_set(NRF_UART0, FPG_TX_PIN, FPG_RX_PIN);
#endif

	app_uart_put((uint8_t)(PS_HEAD >> 8));
	app_uart_put((uint8_t)PS_HEAD);

	app_uart_put((uint8_t)(PS_ADDR_DEF >> 24));
	app_uart_put((uint8_t)(PS_ADDR_DEF >> 18));
	app_uart_put((uint8_t)(PS_ADDR_DEF >> 8));
	app_uart_put((uint8_t)PS_ADDR_DEF);

	app_uart_put(type);
	len = datalen + 2;
	app_uart_put(len >> 8);
	app_uart_put(len);

	add = type;
	add += len;
	for (i = 0; i < datalen && data != NULL; i++) {
		app_uart_put(data[i]);
		add += data[i];
	}
	app_uart_put(add >> 8);
	app_uart_put(add);

#if DEBUG == 1
	nrf_delay_ms(100);
	nrf_uart_txrx_pins_set(NRF_UART0, TX_PIN_NUMBER, RX_PIN_NUMBER);
#endif
}

/**
 * �ȴ�ָ��Ӧ��
 *
 * @param arg     ��������ָ��
 * @param timeout ��ʱʱ��
 * @return ����ȷ����
 */
static uint8_t fpg_ReadWaitAck(uint8_t* arg,  uint16_t timeout) {
	uint16_t timeindex = 0, index = 0, len = 0, add = 0;
	uint8_t head[10], addbuf[2], rst = 0x00;

#if DEBUG == 1

	nrf_uart_txrx_pins_set(NRF_UART0, FPG_TX_PIN, FPG_RX_PIN);
#endif

	do {
		if (app_uart_get(&head[index]) == NRF_SUCCESS) {
			index++;
			/*�˳������ݰ�*/
			if (index == 2 && (head[0] != (PS_HEAD >> 8) || head[1] != (uint8_t)PS_HEAD)) {
				index = 0;
			}
			if (index == 10 && head[6] == PS_TYPE_ACK) {
				len = (head[7] << 8) | head[8];
				rst = head[9];
				add = head[6] + head[7] + head[8] + head[9];
				/*��������*/
				len -= 3;
				index = 0;
				while (index < len) {
					if (app_uart_get(&arg[index]) == NRF_SUCCESS) {
						add += arg[index];
						index++;
					}
				}
				/*����У��*/
				index = 0;
				while (index < 2) {
					if (app_uart_get(&addbuf[index]) == NRF_SUCCESS) {
						index++;
					}
				}
				/*����У��*/
				if (addbuf[0] != (add >> 8) || addbuf[1] != (uint8_t)add) {
					rst = 0xFE;
				}
				break;
			}
		}
		nrf_delay_ms(1);
		timeindex++;
		WatchDog_Clear();
	} while (timeindex < timeout);

	if (timeindex >= timeout) {
		rst = 0xFF;
	}

#if DEBUG == 1
	nrf_uart_txrx_pins_set(NRF_UART0, TX_PIN_NUMBER, RX_PIN_NUMBER);
#endif
	if (rst != 0) {
		DBG_LOG("fpg_ReadWaitAck error:%s.", FPG_GetResultString(rst));
	}

	return rst;
}

/**
 * ��������
 *
 * @param buff    ��������ָ��
 * @param buflen  ��������󳤶�
 * @param timeout
 * @return �����ɹ�����0,��ʱ����0xFF
 */
static uint8_t fpg_ReadWaitData(uint8_t* buff, uint16_t buflen, uint16_t timeout) {
	uint16_t timeindex = 0, index = 0, len = 0, add = 0;
	uint8_t head[10], addbuf[2], rst = 0x00;

#if DEBUG == 1

	nrf_uart_txrx_pins_set(NRF_UART0, FPG_TX_PIN, FPG_RX_PIN);
#endif

	do {
		if (app_uart_get(&head[index]) == NRF_SUCCESS) {
			index++;
			/*�˳������ݰ�*/
			if (index == 2 && (head[0] != (PS_HEAD >> 8) || head[1] != (uint8_t)PS_HEAD)) {
				index = 0;
			}
			if (index == 9 && (head[6] == PS_TYPE_DATA || head[6] == PS_TYPE_DATA_LAST)) {
				len = (head[7] << 8) | head[8];
				add = head[6] + head[7] + head[8];
				/*��������*/
				len -= 2;
				index = 0;
				if (len > buflen) {
					len = buflen;
					DBG_LOG("fpg_ReadWaitData buf overflow.");
				}
				while (index < len) {
					if (app_uart_get(&buff[index]) == NRF_SUCCESS) {
						add += addbuf[index];
						index++;
					}
				}
				/*����У��*/
				index = 0;
				while (index < 2) {
					if (app_uart_get(&addbuf[index]) == NRF_SUCCESS) {
						index++;
					}
				}
				/*����У��*/
				if (addbuf[0] != (add >> 8) || addbuf[1] != (uint8_t)add) {
					rst = 0xFE;
				}
				break;
			}
		}
		nrf_delay_ms(1);
		timeindex++;
	} while (timeindex < timeout);

	if (timeindex >= timeout) {
		rst = 0xFF;
	}
#if DEBUG == 1
	nrf_uart_txrx_pins_set(NRF_UART0, TX_PIN_NUMBER, RX_PIN_NUMBER);
#endif
	if (rst != 0) {
		DBG_LOG("fpg_ReadWaitData error:%s.", FPG_GetResultString(rst));
	}
	return rst;
}

/**
 *  ָ�ƴ��������Խӿ�
 */
static void fpg_Console(int argc, char* argv[]) {
	uint16_t score = 0, page = 0;

	argv++;
	argc--;

	if (ARGV_EQUAL("poweron")) {
		FPG_EN();
		DBG_LOG("FPG poweron.");
	} else if (ARGV_EQUAL("poweroff")) {
		FPG_DIS();
		DBG_LOG("FPG poweroff.");
	} else if (ARGV_EQUAL("istouch")) {
		DBG_LOG("FPG is touch:%u.", IS_FPG_INT());
	} else if (ARGV_EQUAL("sendcmd")) {
		fpg_SendCmd(uatoi(argv[1]), NULL, 0);
		DBG_LOG("fpg_SendCmd:%u.", uatoi(argv[1]));
	} else if (ARGV_EQUAL("getimage")) {
		DBG_LOG("FPG_PS_GetImage:%u.", FPG_PS_GetImage());
	} else if (ARGV_EQUAL("genchar")) {
		DBG_LOG("FPG_PS_GenChar:%u.", FPG_PS_GenChar(uatoi(argv[1])));
	} else if (ARGV_EQUAL("match")) {
		DBG_LOG("FPG_PS_Match:%u, scroe:%u.", FPG_PS_Match(&score), score);
	} else if (ARGV_EQUAL("search")) {
		DBG_LOG("FPG_PS_Search:%u, pageid:%u, scroe:%u.",
						FPG_PS_Search(uatoi(argv[1]), uatoi(argv[2]), uatoi(argv[3]), &page, &score), page, score);
	} else if (ARGV_EQUAL("regmodel")) {
		DBG_LOG("FPG_PS_RegModel:%u.", FPG_PS_RegModel());
	} else if (ARGV_EQUAL("storechar")) {
		DBG_LOG("FPG_PS_StoreChar:%u.", FPG_PS_StoreChar(uatoi(argv[1]), uatoi(argv[2])));
	} else if (ARGV_EQUAL("loadchar")) {
		DBG_LOG("FPG_PS_LoadChar:%u.", FPG_PS_LoadChar(uatoi(argv[1]), uatoi(argv[2])));
	} else if (ARGV_EQUAL("deletchar")) {
		DBG_LOG("FPG_PS_DeletChar:%u.", FPG_PS_DeletChar(uatoi(argv[1]), uatoi(argv[2])));
	} else if (ARGV_EQUAL("empty")) {
		DBG_LOG("FPG_PS_Empty:%u.", FPG_PS_Empty());
	} else if (ARGV_EQUAL("readsyspara")) {
		FPG_PS_ReadSysPara(NULL);
	} else if (ARGV_EQUAL("enroll")) {
		DBG_LOG("FPG_PS_Enroll:%u, pageid:%u.", FPG_PS_Enroll(&page), page);
	} else if (ARGV_EQUAL("indentify")) {
		DBG_LOG("FPG_PS_Identify:%u, pageid:%u, score:%u.", FPG_PS_Identify(&page, &score), page, score);
	} else if (ARGV_EQUAL("highspeedsearch")) {
		DBG_LOG("FPG_PS_HighSpeedSearch:%u, pageid:%u, scroe:%u.",
						FPG_PS_HighSpeedSearch(uatoi(argv[1]), uatoi(argv[2]), uatoi(argv[3]), &page, &score), page, score);
	} else if (ARGV_EQUAL("validtempletenum")) {
		DBG_LOG("FPG_PS_ValidTempleteNum:%u, Number:%u.", FPG_PS_ValidTempleteNum(&page), page);
	} else if (ARGV_EQUAL("upchar")) {
		DBG_LOG("FPG_PS_UpChar:%u.", FPG_PS_UpChar(uatoi(argv[1]), mulitiBuf));
	} else if (ARGV_EQUAL("downchar")) {
		DBG_LOG("FPG_PS_DownChar:%u.", FPG_PS_DownChar(uatoi(argv[1]), mulitiBuf));
	}
}

/************************ (C) COPYRIGHT  *****END OF FILE****/
