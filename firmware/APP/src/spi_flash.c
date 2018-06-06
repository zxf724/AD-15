/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file spi_flash.c
 * @author ����
 * @version V1.0
 * @date 2016.4.1
 * @brief spi_flash ���������ļ�.
 *
 * **********************************************************************
 * @note
 * 2016.12.26 ���ӽ���mutex����.
 *
 * **********************************************************************
 */



/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#define SFLASH_WP_OFF()             nrf_gpio_pin_set(W25_WP_PIN)
#define SFLASH_WP_ON()              nrf_gpio_pin_clear(W25_WP_PIN)

#define SFLASH_CS_H()               nrf_gpio_pin_set(W25_CS_PIN)
#define SFLASH_CS_L()               nrf_gpio_pin_clear(W25_CS_PIN)

#define ADDR_ALLOW(addr)            (addr >= SECTOR_ADDR(SECTOR_SAFE_NUM) && addr < SFlash_Info.Capacity)

/* Private variables ---------------------------------------------------------*/
static uint32_t             SwapSector_Num;
SPI_Flash_Info_t            SFlash_Info;
static const nrf_drv_spi_t mspi = NRF_DRV_SPI_INSTANCE(0);  /**< SPI instance. */

/* Private function prototypes -----------------------------------------------*/
static BOOL SFlash_PageWrite(uint32_t addr, uint8_t* wdat, uint32_t len);
static void SFlash_WriteEn(void);
static BOOL SFlash_WriteProtect(void);
static BOOL SFlash_UnWriteProtect(void);
static BOOL SFlash_Get(void);
static void SFlash_Release(void);
static uint8_t SFlash_RW(uint8_t c);
static BOOL SFlash_ReadInfo(void);
static BOOL SFlash_WaitForWriteEnd(uint32_t ms);

static void SFlash_Console(int argc, char* argv[]);

/* Exported functions --------------------------------------------------------*/

/**
 * spi flash ������ʼ��
 * @param h_spi  spi HAL�ľ��
 */
void SFlash_Init(void) {
//	SFLASH_CS_H();

	nrf_drv_spi_config_t spi_config;

	nrf_drv_spi_uninit(&mspi);
	spi_config.frequency = NRF_DRV_SPI_FREQ_8M;
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
	spi_config.mode = NRF_DRV_SPI_MODE_0;
	spi_config.orc = 0xFF;
	spi_config.irq_priority = 0;

	spi_config.miso_pin = SPI_MISO_PIN;
	spi_config.mosi_pin = SPI_MOSI_PIN;
	spi_config.sck_pin = SPI_SCK_PIN;
	spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
	nrf_drv_spi_init(&mspi, &spi_config, NULL);

	SFlash_WriteProtect();
	SFlash_Wakeup();
	SFlash_ReadInfo();

	SwapSector_Num = SFlash_Info.SectorCount - SECTOR_SWAP_NUM;

	CMD_ENT_DEF(flash, SFlash_Console);
	Cmd_AddEntrance(CMD_ENT(flash));

	DBG_LOG("SPI falsh Init.");
}

/**
 * ��ȡflash�������ĵ�ַ.
 * @return ���ؽ��������ĵ�ַ
 */
BOOL SFlash_GetSwapSector(uint32_t* addr) {
	BOOL r = FALSE;

	if (SFlash_Get()) {
		SwapSector_Num++;
		if (SwapSector_Num >= SFlash_Info.SectorCount - 1) {
			SwapSector_Num = SFlash_Info.SectorCount - SECTOR_SWAP_NUM;
		}
		SFlash_Release();
		r = TRUE;
	}
	*addr = SECTOR_ADDR(SwapSector_Num);
	return r;
}

/**
 *  ��SPI FLASH.
 * @param addr �����ĵ�ַ
 * @param rdat �������ݱ����ָ��
 * @param len  �����ĳ���
 * @return �����ɹ�����TRUE
 */
BOOL SFlash_Read(uint32_t addr, uint8_t* rdat, uint32_t len) {
//    uint32_t i = 0;
	BOOL r = FALSE;

	if (SFlash_Get()) {
		SFLASH_CS_L();

		SFlash_RW(SFLASH_CMD_READ);
		SFlash_RW((addr & 0xFF0000) >> 16);
		SFlash_RW((addr & 0xFF00) >> 8);
		SFlash_RW(addr & 0xFF);

		// for (i = 0; i < len; i++) {
		//     *(rdat + i) = SFlash_RW(SFLASH_CMD_DUMMY);
		// }
		nrf_drv_spi_transfer(&mspi, NULL, 0, rdat, len);
		SFLASH_CS_H();
		SFlash_Release();
		r = TRUE;
	}
	return r;
}

/**
 * �е�ַ���SPI flashд�뺯��
 * @param addr д��ĵ�ַ
 * @param wdat ��д�������
 * @param len  ��д������ݳ���
 * @return ����д��״̬
 */
BOOL SFlash_Write(uint32_t addr, uint8_t* wdat, uint32_t len) {
	BOOL r = FALSE;

	if (ADDR_ALLOW(addr)) {
		r = SFlash_Write_NotCheck(addr, wdat, len);
	}
	return r;
}

/**
 * �޵�ַ���SPI flashд�뺯��,����������д��SPI flash��ȫ����.
 * @param addr д��ĵ�ַ
 * @param wdat ��д�������
 * @param len  ��д������ݳ���
 * @return ����д��״̬
 */
BOOL SFlash_Write_NotCheck(uint32_t addr, uint8_t* wdat, uint32_t len) {
	uint32_t rem = 0, wlen = 0;

	if (wdat == NULL || len == 0 || (len + addr) >= SFlash_Info.Capacity) {
		return FALSE;
	}

	if (SFlash_Get()) {
		SFlash_UnWriteProtect();

		rem = addr % SFLASH_PAGE_SIZE;
		if (rem > 0) {
			rem = SFLASH_PAGE_SIZE - rem;
			if (rem > len - wlen) {
				rem = len - wlen;
			}
			SFlash_PageWrite(addr, wdat, rem);
			wlen += rem;
			addr += rem;
			wdat += rem;
		}

		while (wlen < len) {
			rem = SFLASH_PAGE_SIZE;
			if (rem > len - wlen) {
				rem = len - wlen;
			}
			if (SFlash_PageWrite(addr, wdat, rem) == FALSE) {
				break;
			}
			addr += rem;
			wdat += rem;
			wlen += rem;
		}
		SFlash_WriteProtect();
		SFlash_Release();
	}
	return (wlen == len) ? TRUE : FALSE;
}

/**
 * SPI FLASH ��Ƭ����.
 * @return ���ز������
 */
BOOL SFlash_EraseChip(void) {
	BOOL r = FALSE;

	if (SFlash_Get()) {
		SFlash_UnWriteProtect();

		SFlash_WriteEn();
		SFLASH_CS_L();
		SFlash_RW(SFLASH_CMD_BE);
		SFLASH_CS_H();
		r = SFlash_WaitForWriteEnd(30000);

		SFlash_WriteProtect();
		SFlash_Release();
	}
	return r;
}

/**
 * ����ַ����SPI FLASH ��������.
 * @param addr ������������ʼ��ַ
 * @param nums ����������������
 * @return ���ز������
 */
BOOL SFlash_EraseSectors(uint32_t addr, uint16_t nums) {
	BOOL r = FALSE;

	if (ADDR_ALLOW(addr)) {
		r = SFlash_EraseSectors_NotCheck(addr, nums);
	}
	return r;
}

/**
 * �޵�ַ����SPI FLASH ��������,������������ȫ����
 * @param addr ������������ʼ��ַ
 * @param nums ����������������
 * @return ���ز������
 */
BOOL SFlash_EraseSectors_NotCheck(uint32_t addr, uint16_t nums) {
	uint8_t cmd = 0;
	uint32_t addrend = 0, block = 0, waitforerase = 0;

	if (nums == 0) {
		return FALSE;
	}

	addr = addr / SFLASH_SECTOR_SIZE;
	addr *= SFLASH_SECTOR_SIZE;

	addrend = addr + nums * SFLASH_SECTOR_SIZE;

	if (SFlash_Get()) {
		SFlash_UnWriteProtect();
		while (addr < addrend) {
			if ((addrend - addr >= SFLASH_64K_SIZE) && (addr % SFLASH_64K_SIZE == 0)) {
				block = SFLASH_64K_SIZE;
				cmd = SFLASH_CMD_64KE;
				waitforerase = 3000;
			} else if ((addrend - addr >= SFLASH_32K_SIZE) && (addr % SFLASH_32K_SIZE == 0)) {
				block = SFLASH_32K_SIZE;
				cmd = SFLASH_CMD_32KE;
				waitforerase = 1600;
			} else {
				block = SFLASH_SECTOR_SIZE;
				cmd = SFLASH_CMD_SE;
				waitforerase = 400;
			}
			SFlash_WriteEn();
			SFLASH_CS_L();
			SFlash_RW(cmd);
			SFlash_RW((addr & 0xFF0000) >> 16);
			SFlash_RW((addr & 0xFF00) >> 8);
			SFlash_RW(addr & 0xFF);
			SFLASH_CS_H();
			if (SFlash_WaitForWriteEnd(waitforerase) == FALSE) {
				break;
			}
			addr += block;
		}
		SFlash_WriteProtect();
		SFlash_Release();
	}
	return (addr == addrend) ? TRUE : FALSE;
}


/**
 * flash����
 * @return
 */
BOOL SFlash_Sleep(void) {
	BOOL r = FALSE;

	if (SFlash_Get()) {
		SFLASH_CS_L();
		SFlash_RW(SFLASH_CMD_PWR_DOWN);
		SFLASH_CS_H();

		/*��ʱ*/
		SFlash_RW(SFLASH_CMD_DUMMY);
		SFlash_RW(SFLASH_CMD_DUMMY);

		SFlash_Release();
		r = TRUE;
	}
	return r;
}

/**
 * FLASH����
 * @return
 */
BOOL SFlash_Wakeup(void) {
	BOOL r = FALSE;

	if (SFlash_Get()) {
		SFLASH_CS_L();
		SFlash_RW(SFLASH_CMD_RELEASE_PWR_DOWN);
		SFLASH_CS_H();

		/*��ʱ*/
		SFlash_RW(SFLASH_CMD_DUMMY);
		SFlash_RW(SFLASH_CMD_DUMMY);

		SFlash_Release();
		r = TRUE;
	}
	return r;
}

/* Private function prototypes -----------------------------------------------*/

/**
 * �ޱ���SPI FLASHҳ���,������ʹ��ǰ���Ƚ��д����.
 * @param addr д��ĵ�ַ
 * @param wdat ��д�������
 * @param len  ��д������ݳ���
 * @return ����д��״̬
 */
static BOOL SFlash_PageWrite(uint32_t addr, uint8_t* wdat, uint32_t len) {
	uint16_t i = 0;
	BOOL r = FALSE;

	if (wdat != NULL && len > 0 && len <= SFLASH_PAGE_SIZE) {

		SFlash_WriteEn();
		SFLASH_CS_L();
		SFlash_RW(SFLASH_CMD_WRITE);
		SFlash_RW((addr & 0xFF0000) >> 16);
		SFlash_RW((addr & 0xFF00) >> 8);
		SFlash_RW(addr & 0xFF);

		for (i = 0; i < len; i++) {
			// SFlash_RW(*(wdat + i));
			nrf_drv_spi_transfer(&mspi, wdat, len, NULL, 0);
		}
		SFLASH_CS_H();

		/*�ȴ���̣���ʱ100����*/
		r = SFlash_WaitForWriteEnd(100);
	}
	return r;
}

/**
	* @brief  SPI FLASHдʹ������.
	*/
static void SFlash_WriteEn(void) {
	SFLASH_CS_L();
	SFlash_RW(SFLASH_CMD_WREN);
	SFLASH_CS_H();
}

/**
 * SPI FLASHд����.
 * @return ����ִ�н��.
 */
static BOOL SFlash_WriteProtect(void) {
	SFlash_WriteEn();

	SFLASH_WP_OFF();
	SFLASH_CS_L();
	SFlash_RW(SFLASH_CMD_WRSR);
	SFlash_RW(0x9E);
	SFLASH_CS_H();
	SFLASH_WP_ON();

	return SFlash_WaitForWriteEnd(100);
}

/**
 * SPI FLASH�������.
 * @return ����ִ�н��.
 */
static BOOL SFlash_UnWriteProtect(void) {
	SFlash_WriteEn();

	SFLASH_WP_OFF();
	SFLASH_CS_L();
	SFlash_RW(SFLASH_CMD_WRSR);
	SFlash_RW(0x82);
	SFLASH_CS_H();
	SFLASH_WP_ON();

	return SFlash_WaitForWriteEnd(100);
}

/**
 * ��ȡ SPI Flash,����ȡmutex.
 * @return ����mutex��ȡ�����
 */
static BOOL SFlash_Get(void) {
#if MFRC_USE == 1

	if (NRF_SPI0->ENABLE == 0) {
		nrf_gpio_cfg_input(SPI_MISO_PIN, NRF_GPIO_PIN_NOPULL);
		NRF_SPI0->ENABLE = 1;
		SFlash_RW(SFLASH_CMD_DUMMY);
	}
#endif
	return TRUE;
}

/**
 * �ͷ� SPI Flash,���ͷ�mutex������CS����.
 */
static void SFlash_Release(void) {
	SFLASH_CS_H();
	SFlash_RW(SFLASH_CMD_DUMMY);

#if MFRC_USE == 1
	NRF_SPI0->ENABLE = 0;
	nrf_gpio_pin_clear(SPI_SCK_PIN);
	nrf_gpio_pin_clear(SPI_MOSI_PIN);
	nrf_gpio_cfg_input(SPI_MISO_PIN, NRF_GPIO_PIN_PULLDOWN);
#endif
}

/**
 * SPI FLASH��дһ���ֽ�.
 * @param c    д����ֽ�
 * @return ���ص��ֽ�
 */
static uint8_t SFlash_RW(uint8_t c) {
	uint8_t tc = c, rc = 0;

	nrf_drv_spi_transfer(&mspi, &tc, 1, &rc, 1);
	return rc;
}

/**
 * SPI FLASH��оƬ��Ϣ.
 * @return �����ɹ�����TRUE.
 */
static BOOL SFlash_ReadInfo(void) {
	BOOL r = FALSE;

	/*��flash�����Ϣ*/
	if (SFlash_Get()) {

		/*���������豸ID*/
		SFLASH_CS_L();
		SFlash_RW(SFLASH_CMD_RDID);
		SFlash_Info.ManFID = SFlash_RW(SFLASH_CMD_DUMMY);
		SFlash_Info.Type = SFlash_RW(SFLASH_CMD_DUMMY);
		SFlash_Info.DevID = SFlash_RW(SFLASH_CMD_DUMMY);
		SFLASH_CS_H();

		/*֧��25Q, 25Pϵ��  SPI FLASH*/
		if (SFlash_Info.ManFID == 0xEF) {
			/*֧��25Q80 - 25Q256*/
			if (SFlash_Info.DevID >= 0x14 && SFlash_Info.DevID <= 0x19) {
				SFlash_Info.Capacity = 0x100000 * (1 << (SFlash_Info.DevID - 0x14));
				SFlash_Info.PageSize = SFLASH_PAGE_SIZE;
				SFlash_Info.SectorSize = SFLASH_SECTOR_SIZE;
				SFlash_Info.SectorCount = SFlash_Info.Capacity / SFLASH_SECTOR_SIZE;

				DBG_LOG("SPI flash config OK, id:0x%X, Sector count:%d, Capacity: %dKB.",
								SFlash_Info.DevID,
								SFlash_Info.SectorCount,
								SFlash_Info.Capacity / 1024);
				r =  TRUE;
			} else {
				DBG_LOG("Not Support This Capacity Flash.");
			}
		} else {
			DBG_LOG("Not Support This Firm Flash:FID 0x%02X.", SFlash_Info.ManFID);
		}
		SFlash_Release();
	}
	return r;
}

/**
 * SPI FLASH�ȴ�д���.
 * @param ms     �ȴ���ʱ��,��λms
 * @return �����ӳ�ʱ����д��ɷ���TRUE.
 */
static BOOL SFlash_WaitForWriteEnd(uint32_t ms) {
	uint8_t res = 0;
	uint32_t volatile t = 0;

	SFLASH_CS_L();
	SFlash_RW(SFLASH_CMD_RDSR);

	res = SFlash_RW(SFLASH_CMD_DUMMY);
	do {
		res = SFlash_RW(SFLASH_CMD_DUMMY);
		if ((res & SFLASH_WIP_FLAG)) {
			nrf_delay_us(99);
//			WatchDog_Clear();
			t++;
		} else {
			break;
		}
	} while (t <= ms * 10);
	SFLASH_CS_H();

	return ((res & SFLASH_WIP_FLAG) ? FALSE : TRUE);
}

/**
 * SPI falsh��������
 * @param argc ����������
 * @param argv �����б�
 */
static void SFlash_Console(int argc, char* argv[]) {
	BOOL r = FALSE;
	uint32_t addr = 0, j = 0, len = 0;
	char* p = NULL, buf[128];

	argc--;
	argv++;

	if (strcmp(argv[0], "CS") == 0) {
		j = uatoi(argv[1]);
		if (j) {
			SFLASH_CS_H();
		} else {
			SFLASH_CS_L();
		}
		DBG_LOG("SPI flash CS set:%d", j);
	} else if (strcmp(argv[0], "RW") == 0) {
		p = argv[1];
		j = SFlash_RW(uatoi(p));
		DBG_LOG("SPI flash read write res:%d", j);
	} else if (strcmp(argv[0], "erase") == 0) {
		p = argv[1];
		if (strcmp(p, "all") == 0) {
			DBG_LOG("SPI flash erase chip begin.");
			SFlash_EraseChip();
			DBG_LOG("SPI flash erase chip OK.");
		} else if (isxdigit(*p)) {
			if (Cmd_ArgFind(&argv[2], "-x") >= 0) {
				addr = uatoix(p);
			} else {
				addr = uatoi(p);
			}
			if (Cmd_ArgFind(&argv[2], "-d") >= 0) {
				r = SFlash_EraseSectors_NotCheck(addr, 1);
			} else {
				r = SFlash_EraseSectors(addr, 1);
			}
			DBG_LOG("SPI flash erase sector address 0x%02X res:%d.", addr, (int)r);
		}
	} else if (strcmp(argv[0], "swap") == 0) {
		SFlash_GetSwapSector(&addr);
		DBG_LOG("SPI swap address 0x%X now.", addr);
	} else if (ARGV_EQUAL("init")) {
		DBG_LOG("SPI init test.");
		SFlash_Init();
	} else if (strcmp(argv[0], "read") == 0) {
		if (Cmd_ArgFind(&argv[3], "-x") >= 0) {
			addr = uatoix(argv[1]);
			len = uatoix(argv[2]);
		} else {
			addr = uatoi(argv[1]);
			len = uatoi(argv[2]);
		}
		if (len > 127) {
			len = 127;
		}
		if (SFlash_Read(addr, (uint8_t*)buf, len)) {
			buf[len] = 0;
			DBG_LOG("SPI flash read address 0x%02X:%s", addr, buf);
		}
	} else if (strcmp(argv[0], "write") == 0) {
		if (Cmd_ArgFind(&argv[3], "-x") >= 0) {
			addr = uatoix(argv[1]);
		} else {
			addr = uatoi(argv[1]);

		}
		len = strlen(argv[2]);
		r = (Cmd_ArgFind(&argv[3], "-d") >= 0) ? TRUE : FALSE;
		DBG_LOG("SPI flash write address 0x%02X begin.", addr);
		if (r) {
			SFlash_Write_NotCheck(addr, (uint8_t*)argv[2], len);
		} else {
			SFlash_Write(addr, (uint8_t*)argv[2], len);
		}
		DBG_LOG("SPI flash write OK, end address:0x%0X, length:%d.", addr, len);
	}
}


