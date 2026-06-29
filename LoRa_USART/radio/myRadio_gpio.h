#ifndef __MYRADIO_GPIO_H_
#define __MYRADIO_GPIO_H_

#include <stdint.h>
#include "main.h"

typedef void (*RADIO_GPIO_CALLBACK)(uint8_t index);

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

#define RF_SX126x_EXT_PA_TO_TX() {RF_SX126x_EXT_PA_TE_H();RF_SX126x_EXT_PA_RE_L();}
#define RF_SX126x_EXT_PA_TO_RX() {RF_SX126x_EXT_PA_TE_L();RF_SX126x_EXT_PA_RE_H();}
#define RF_SX126x_EXT_PA_TO_IDLE() {RF_SX126x_EXT_PA_TE_L();RF_SX126x_EXT_PA_RE_L();}

void myRadio_gpio_init(RADIO_GPIO_CALLBACK cb);
uint8_t myRadioSpi_rwByte(uint8_t byteToWrite);
void myRadioSpi_wBuffer(uint8_t* pData, uint8_t len);
void myRadioSpi_rBuffer(uint8_t* pData, uint8_t len);
void BOARD_SPI_NSS_H(void);
void BOARD_SPI_NSS_L(void);

#define SpiReadWrite(p) myRadioSpi_rwByte(p)
#define SpiWriteData(p1, p2) myRadioSpi_wBuffer(p1, p2)
#define SpiReadData(p1, p2) myRadioSpi_rBuffer(p1, p2)

#endif
