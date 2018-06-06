/**
    ******************************************************************************
    * @file    touchpad.c
    * @author  ����
    * @version V1.0
    * @date    2017.12.15
    * @brief   ����������������غ���.
    *
    ******************************************************************************
    */


/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static const nrf_drv_twi_t mtwi = NRF_DRV_TWI_INSTANCE(1);

/* Private function prototypes -----------------------------------------------*/
static void touchpad_Console(int argc, char* argv[]);

/* Exported functions --------------------------------------------------------*/

/**
 * ָ�ƴ�������ʼ��
 */
void TouchPad_init(void) {

  nrf_drv_twi_config_t const config = {
    .scl                = TWI_SCL_PIN,
    .sda                = TWI_SDA_PIN,
    .frequency          = NRF_TWI_FREQ_100K,
    .interrupt_priority = APP_IRQ_PRIORITY_HIGH
  };

  nrf_drv_twi_init(&mtwi, &config, NULL, NULL);
  nrf_drv_twi_enable(&mtwi);

  CMD_ENT_DEF(touchpad, touchpad_Console);
  Cmd_AddEntrance(CMD_ENT(touchpad));

	TouchPad_WriteThreshold(32);

  DBG_LOG("touchpad init.");
}

/**
 * ����������ֵ
 * 
 * @return ���ش�������ֵ�������������ذ���ֵ
 */
char TouchPad_ReadKey(void) {
  uint16_t key = TouchPad_SacnKeys();
  if (key == KEY_1) return '1';
  if (key == KEY_2) return '2';
  if (key == KEY_3) return '3';
  if (key == KEY_4) return '4';
  if (key == KEY_5) return '5';
  if (key == KEY_6) return '6';
  if (key == KEY_7) return '7';
  if (key == KEY_8) return '8';
  if (key == KEY_9) return '9';
  if (key == KEY_0) return '0';
  if (key == KEY_2A) return '*';
  if (key == KEY_23) return '#';

  return 0;
}

/**
 * ɨ�谴��
 * 
 * @return ���ذ����룬bit0-bit11��Ӧ����������KEY1-KEY12
 */
uint16_t TouchPad_SacnKeys(void) {
  uint16_t ret = 0;
  uint8_t buf[3] = { 0, 0, 0 };

  nrf_drv_twi_rx(&mtwi, 0xA0 >> 1, &buf[0], 3);
  if (buf[2] == buf[0] + buf[1] + 1) {
    ret = buf[0] | (buf[1] << 8);
  } else {
    DBG_LOG("TouchPad_ReadKey Check error. %#x %#x %#x",
            buf[0], buf[1], buf[2]);
  }
  DBG_LOG("TouchPad_ReadKey:%#x", ret);
  return ret;

}

/**
 * д�����ż�ֵ
 * 
 * @param thd    �ż�ֵ����Χ8-255
 */
void TouchPad_WriteThreshold(uint8_t thd) {
  uint8_t buf[12];

  if (thd >= 8) {
    memset(buf, thd, 12);
    nrf_drv_twi_tx(&mtwi, 0xA0 >> 1, &buf[0], 12, false);
    DBG_LOG("TouchPad_WriteThreshold:%#x", thd);
  }
}

/* Private function prototypes -----------------------------------------------*/

/**
 *  ָ�ƴ��������Խӿ�
 */
static void touchpad_Console(int argc, char* argv[]) {
  argv++;
  argc--;

  if (ARGV_EQUAL("isint")) {
    DBG_LOG("Touchpad INT:%d.", IS_TP_INT());
  } else if (ARGV_EQUAL("scan")) {
    TouchPad_SacnKeys();
  } else if (ARGV_EQUAL("read")) {
    DBG_LOG("TouchPad_ReadKey:%c", TouchPad_ReadKey());
  } else if (ARGV_EQUAL("threshold")) {
    TouchPad_WriteThreshold(uatoi(argv[1]));
  } else if (ARGV_EQUAL("testi2c")) {
    uint8_t tmp;
    tmp = uatoix(argv[2]);
    nrf_drv_twi_tx(&mtwi, uatoix(argv[1]), &tmp, 1, false);
  }
}

/************************ (C) COPYRIGHT  *****END OF FILE****/
