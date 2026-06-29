#include "myRadio_gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"

RADIO_GPIO_CALLBACK gpioCallback;

extern SPI_HandleTypeDef hspi2;

//---------------------------射频SPI驱动部分 (STM32F4 HAL)---------------------
void BOARD_SPI_NSS_H(void)
{
    HAL_GPIO_WritePin(RF_SX126x_SPI_CSN_PORT, RF_SX126x_SPI_CSN_PIN, GPIO_PIN_SET);
}
void BOARD_SPI_NSS_L(void)
{
    HAL_GPIO_WritePin(RF_SX126x_SPI_CSN_PORT, RF_SX126x_SPI_CSN_PIN, GPIO_PIN_RESET);
}

//---------------------------射频驱动IO部分---------------------
void RF_SX126x_IO1_H(void)
{
    // IO1/DIO1 is input (EXTI), no need to set
}
void RF_SX126x_IO1_L(void)
{
    // IO1/DIO1 is input (EXTI), no need to set
}
void RF_SX126x_IO3_H(void)
{
    // IO3/DIO3 not used on this board (controlled internally by SX126x)
}
void RF_SX126x_IO3_L(void)
{
    // IO3/DIO3 not used on this board
}
void RF_SX126x_RST_H(void)
{
    HAL_GPIO_WritePin(RF_SX126x_RST_PORT, RF_SX126x_RST_PIN, GPIO_PIN_SET);
}
void RF_SX126x_RST_L(void)
{
    HAL_GPIO_WritePin(RF_SX126x_RST_PORT, RF_SX126x_RST_PIN, GPIO_PIN_RESET);
}
void RF_SX126x_EXT_PA_RE_H(void)
{
    HAL_GPIO_WritePin(RF_SX126x_EXTPA_RE_PORT, RF_SX126x_EXTPA_RE_PIN, GPIO_PIN_SET);
}
void RF_SX126x_EXT_PA_RE_L(void)
{
    HAL_GPIO_WritePin(RF_SX126x_EXTPA_RE_PORT, RF_SX126x_EXTPA_RE_PIN, GPIO_PIN_RESET);
}
void RF_SX126x_EXT_PA_TE_H(void)
{
    HAL_GPIO_WritePin(RF_SX126x_EXTPA_TE_PORT, RF_SX126x_EXTPA_TE_PIN, GPIO_PIN_SET);
}
void RF_SX126x_EXT_PA_TE_L(void)
{
    HAL_GPIO_WritePin(RF_SX126x_EXTPA_TE_PORT, RF_SX126x_EXTPA_TE_PIN, GPIO_PIN_RESET);
}
uint8_t READ_RF_SX126x_IO1(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(RF_SX126x_IO1_PORT, RF_SX126x_IO1_PIN);
}
uint8_t READ_RF_SX12xx_DIO3(void)
{
    // DIO3 not mapped on this board
    return 0;
}
uint8_t READ_RF_SX126x_BUSY(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(RF_SX126x_BUSY_PORT, RF_SX126x_BUSY_PIN);
}

/**
 * @brief  DIO1 EXTI中断回调 (由HAL_GPIO_EXTI_Callback调用)
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == RF_SX126x_IO1_PIN)
    {
        if (gpioCallback)
        {
            gpioCallback(1);
        }
    }
}

/**
 * @brief 射频GPIO初始化
 *        GPIO和SPI已由CubeMX的MX_GPIO_Init()和MX_SPI2_Init()完成
 *        此处仅需配置NSS引脚为GPIO输出模式，并使能DIO1的EXTI中断
 */
void myRadio_gpio_init(RADIO_GPIO_CALLBACK cb)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 将PB12从SPI2_NSS AF模式重新配置为普通GPIO输出（软件NSS控制） */
    GPIO_InitStruct.Pin = RF_SX126x_SPI_CSN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(RF_SX126x_SPI_CSN_PORT, &GPIO_InitStruct);

    /* NSS默认拉高 */
    BOARD_SPI_NSS_H();

    /* 先设置回调，再使能中断 */
    gpioCallback = cb;
    
    /* 清除可能挂起的 EXTI0 中断标志 */
    __HAL_GPIO_EXTI_CLEAR_IT(RF_SX126x_IO1_PIN);
    
    /* 使能DIO1(PC0)的EXTI中断 */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/**
 * @brief  SPI读写一个字节
 */
uint8_t myRadioSpi_rwByte(uint8_t byteToWrite)
{
    uint8_t rxData = 0;
    HAL_SPI_TransmitReceive(&hspi2, &byteToWrite, &rxData, 1, 100);
    return rxData;
}

/**
 * @brief  SPI写一个字节数组
 */
void myRadioSpi_wBuffer(uint8_t* pData, uint8_t len)
{
    if(len == 0) return;
    
    HAL_SPI_Transmit(&hspi2, pData, len, 100);
}

/**
 * @brief  SPI读一个字节数组
 */
void myRadioSpi_rBuffer(uint8_t* pData, uint8_t len)
{
    if(len == 0) return;

    HAL_SPI_Receive(&hspi2, pData, len, 100);
}