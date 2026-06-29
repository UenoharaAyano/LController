
#ifndef __MYRADIO_GPIO_H_
#define __MYRADIO_GPIO_H_

#include <stdint.h>
#include "main.h"
#include "spi.h"

#define SPI_HARD

typedef void (*RADIO_GPIO_CALLBACK)(uint8_t index);

//-------------射频模块引脚映射 (STM32F4xx HAL)---------------
// SPI由CubeMX配置的SPI2硬件驱动: PB13(SCK), PB14(MISO), PB15(MOSI)
// NSS由软件控制: PB12
#define RF_SX126x_SPI_CSN_PIN       GPIO_PIN_12
#define RF_SX126x_SPI_CSN_PORT      GPIOB

// DIO1用于射频中断响应: PC0 (EXTI Rising)
#define RF_SX126x_IO1_PIN           GPIO_PIN_0
#define RF_SX126x_IO1_PORT          GPIOC

// RST射频芯片复位: PD2
#define RF_SX126x_RST_PIN           LoRa_RST_Pin
#define RF_SX126x_RST_PORT          LoRa_RST_GPIO_Port

// BUSY用于射频芯片工作状态检测: PB11
#define RF_SX126x_BUSY_PIN          LoRa_BUSY_Pin
#define RF_SX126x_BUSY_PORT         LoRa_BUSY_GPIO_Port

// 外部PA/LNA控制: RXEN=PB5, TXEN=PB6
#define RF_SX126x_EXTPA_RE_PIN      LoRa_RXEN_Pin
#define RF_SX126x_EXTPA_RE_PORT     LoRa_RXEN_GPIO_Port
#define RF_SX126x_EXTPA_TE_PIN      LoRa_TXEN_Pin
#define RF_SX126x_EXTPA_TE_PORT     LoRa_TXEN_GPIO_Port
//-------------射频模块引脚映射---------------END

uint8_t READ_RF_SX126x_IO1(void);
uint8_t READ_RF_SX12xx_DIO3(void);
uint8_t READ_RF_SX126x_BUSY(void);
void RF_SX126x_IO1_H(void);
void RF_SX126x_IO1_L(void);
void RF_SX126x_IO3_H(void);
void RF_SX126x_IO3_L(void);
void RF_SX126x_RST_H(void);
void RF_SX126x_RST_L(void);
void RF_SX126x_EXT_PA_RE_H(void);
void RF_SX126x_EXT_PA_RE_L(void);
void RF_SX126x_EXT_PA_TE_H(void);
void RF_SX126x_EXT_PA_TE_L(void);

#define RF_SX126x_EXT_PA_TO_TX() do{ RF_SX126x_EXT_PA_TE_H(); RF_SX126x_EXT_PA_RE_L(); }while(0)
#define RF_SX126x_EXT_PA_TO_RX() do{ RF_SX126x_EXT_PA_TE_L(); RF_SX126x_EXT_PA_RE_H(); }while(0)
#define RF_SX126x_EXT_PA_TO_IDLE() do{ RF_SX126x_EXT_PA_TE_L(); RF_SX126x_EXT_PA_RE_L(); }while(0)

void myRadio_gpio_init(RADIO_GPIO_CALLBACK cb);
uint8_t myRadioSpi_rwByte(uint8_t byteToWrite);
void myRadioSpi_wBuffer(uint8_t* pData, uint8_t len);
void myRadioSpi_rBuffer(uint8_t* pData, uint8_t len);
void BOARD_SPI_NSS_H(void);
void BOARD_SPI_NSS_L(void);

//-------------将封装的API映射到射频模块硬件层---------------
#define SpiReadWrite(p) myRadioSpi_rwByte(p)
#define SpiWriteData(p1, p2) myRadioSpi_wBuffer(p1, p2)
#define SpiReadData(p1, p2) myRadioSpi_rBuffer(p1, p2)
//-------------将封装的API映射到射频模块硬件层---------------END
#endif
