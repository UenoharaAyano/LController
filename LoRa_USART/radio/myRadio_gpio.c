#include "myRadio_gpio.h"
#include "spi.h"
#include "gpio.h"

RADIO_GPIO_CALLBACK gpioCallback;

void BOARD_SPI_NSS_H(void)
{
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
}
void BOARD_SPI_NSS_L(void)
{
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
}

void RF_SX126x_IO1_H(void) {}
void RF_SX126x_IO1_L(void) {}
void RF_SX126x_IO3_H(void) {}
void RF_SX126x_IO3_L(void) {}

void RF_SX126x_RST_H(void)
{
    HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
}
void RF_SX126x_RST_L(void)
{
    HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_RESET);
}
void RF_SX126x_EXT_PA_RE_H(void)
{
    HAL_GPIO_WritePin(LORA_RXEN_GPIO_Port, LORA_RXEN_Pin, GPIO_PIN_SET);
}
void RF_SX126x_EXT_PA_RE_L(void)
{
    HAL_GPIO_WritePin(LORA_RXEN_GPIO_Port, LORA_RXEN_Pin, GPIO_PIN_RESET);
}
void RF_SX126x_EXT_PA_TE_H(void)
{
    HAL_GPIO_WritePin(LORA_TXEN_GPIO_Port, LORA_TXEN_Pin, GPIO_PIN_SET);
}
void RF_SX126x_EXT_PA_TE_L(void)
{
    HAL_GPIO_WritePin(LORA_TXEN_GPIO_Port, LORA_TXEN_Pin, GPIO_PIN_RESET);
}

uint8_t READ_RF_SX126x_IO1(void)
{
    return HAL_GPIO_ReadPin(LORA_DIO1_GPIO_Port, LORA_DIO1_Pin) == GPIO_PIN_SET ? 1 : 0;
}
uint8_t READ_RF_SX12xx_DIO3(void)
{
    return 0;
}
uint8_t READ_RF_SX126x_BUSY(void)
{
    return HAL_GPIO_ReadPin(LORA_BUSY_GPIO_Port, LORA_BUSY_Pin) == GPIO_PIN_SET ? 1 : 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == LORA_DIO1_Pin)
    {
        if (gpioCallback)
        {
            gpioCallback(1);
        }
    }
}

void myRadio_gpio_init(RADIO_GPIO_CALLBACK cb)
{
    gpioCallback = cb;
    BOARD_SPI_NSS_H();
    RF_SX126x_RST_H();
    RF_SX126x_EXT_PA_TO_IDLE();
}

uint8_t myRadioSpi_rwByte(uint8_t byteToWrite)
{
    uint8_t rxData = 0;
    HAL_SPI_TransmitReceive(&hspi1, &byteToWrite, &rxData, 1, 100);
    return rxData;
}

void myRadioSpi_wBuffer(uint8_t* pData, uint8_t len)
{
    if(len == 0) return;
    
    HAL_SPI_Transmit(&hspi1, pData, len, 500);
}

void myRadioSpi_rBuffer(uint8_t* pData, uint8_t len)
{
    if(len == 0) return;

    HAL_SPI_Receive(&hspi1, pData, len, 500);
}
