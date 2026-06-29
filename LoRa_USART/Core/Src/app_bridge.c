/**
  ******************************************************************************
  * @file           : app_bridge.c
  * @brief          : LoRa <-> UART transparent bridge application logic
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_bridge.h"
#include "usart.h"
#include "gpio.h"

/* Private types -------------------------------------------------------------*/
typedef enum
{
    BRIDGE_STATE_IDLE = 0,
    BRIDGE_STATE_TX_PENDING,
} bridge_state_te;

/* Private variables ---------------------------------------------------------*/
static uint8_t  uart_rx_buf[UART_RX_BUF_SIZE];
static uint8_t  uart_rx_ready;
static uint16_t uart_rx_size;
static rfTxPacket_ts lora_tx_pkt;
static volatile uint8_t bridge_state;

/* LED blink control (non-blocking) */
static volatile uint8_t  led1_active;
static volatile uint32_t led1_on_tick;
static volatile uint8_t  led2_active;
static volatile uint32_t led2_on_tick;

/* Bluetooth-format TX buffer (LoRa RX → BT wrap → USART2 TX) */
static uint8_t  bt_tx_buf[BT_TX_BUF_SIZE];

/* -------------------------------------------------------------------------- */
/*               Bluetooth Packet Format Utilities                             */
/* -------------------------------------------------------------------------- */

/**
  * @brief  Compute Bluetooth-format checksum (accumulated sum, byte overflow).
  * @param  data: pointer to payload data
  * @param  len:  payload length in bytes
  * @retval 8-bit accumulated-sum checksum
  */
static uint8_t BT_ComputeChecksum(const uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}

/**
  * @brief  Pack raw data into Bluetooth format:
  *         [0xA5] [data...] [checksum] [0x5A]
  * @param  raw:     input raw payload
  * @param  raw_len: raw payload length
  * @param  out:     output buffer (must hold raw_len + 3 bytes)
  * @retval total packed length = raw_len + 3
  */
static uint8_t BT_Pack(const uint8_t *raw, uint8_t raw_len, uint8_t *out)
{
    uint8_t checksum = BT_ComputeChecksum(raw, raw_len);

    out[0] = BT_HEADER;
    if (raw_len > 0)
    {
        memcpy(&out[1], raw, raw_len);
    }
    out[raw_len + 1] = checksum;
    out[raw_len + 2] = BT_TAIL;

    return raw_len + BT_OVERHEAD;
}

/**
  * @brief  Unpack Bluetooth-format packet, validate and extract raw data.
  *         Expected format: [0xA5] [data...] [checksum] [0x5A]
  * @param  pkt:     input Bluetooth packet buffer
  * @param  pkt_len: total packet length in bytes
  * @param  out:     output buffer for extracted raw data
  * @retval extracted data length on success, -1 on failure
  */
static int16_t BT_Unpack(const uint8_t *pkt, uint16_t pkt_len, uint8_t *out)
{
    /* Minimum packet: header(1) + checksum(1) + tail(1) = 3 bytes */
    if (pkt_len < BT_OVERHEAD)
    {
        return -1;
    }

    /* Validate header and tail */
    if (pkt[0] != BT_HEADER || pkt[pkt_len - 1] != BT_TAIL)
    {
        return -1;
    }

    uint16_t data_len = pkt_len - BT_OVERHEAD;

    /* Extract data (skip header) */
    if (data_len > 0)
    {
        memcpy(out, &pkt[1], data_len);
    }

    /* Verify checksum */
    uint8_t expected_cs = BT_ComputeChecksum(out, (uint8_t)data_len);
    if (expected_cs != pkt[pkt_len - 2])
    {
        return -1;
    }

    return (int16_t)data_len;
}

/**
  * @brief  Initialize bridge: LED3 ON, start USART2 DMA RX with IDLE detection
  */
void Bridge_Init(void)
{
    bridge_state = BRIDGE_STATE_IDLE;
    uart_rx_ready = 0;
    uart_rx_size = 0;
    led1_active = 0;
    led2_active = 0;

    /* LED3 active-high: always ON after initialization */
    HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_SET);

    /* Start UART reception with DMA + IDLE line detection.
       The HAL will call HAL_UARTEx_RxEventCallback on IDLE. */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_buf, UART_RX_BUF_SIZE);
}

/**
  * @brief  Main loop handler: process UART RX data and LED blink timers
  */
void Bridge_Process(void)
{
    /* --- Check for pending UART RX data --- */
    if (uart_rx_ready)
    {
        uart_rx_ready = 0;

        if (bridge_state == BRIDGE_STATE_IDLE && uart_rx_size > 0)
        {
            /* Unpack Bluetooth-format packet to extract raw payload */
            int16_t data_len = BT_Unpack(uart_rx_buf, uart_rx_size,
                                         lora_tx_pkt.payload);

            if (data_len >= 0)
            {
                /* Valid packet: send raw payload over LoRa */
                lora_tx_pkt.len = (uint8_t)data_len;
                bridge_state = BRIDGE_STATE_TX_PENDING;
                myRadio_transmit(&lora_tx_pkt);
            }
            else
            {
                /* Invalid packet (header/tail/checksum mismatch):
                   discard and restart DMA reception immediately */
                HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_buf,
                                             UART_RX_BUF_SIZE);
            }
        }
        /* Note: On success, UART DMA RX will be restarted after LoRa TX completes */
    }

    /* --- LED1 blink timer (LoRa TX success indicator) --- */
    if (led1_active)
    {
        if ((HAL_GetTick() - led1_on_tick) >= LED_BLINK_MS)
        {
            led1_active = 0;
            HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET);
        }
    }

    /* --- LED2 blink timer (USART2 TX success indicator) --- */
    if (led2_active)
    {
        if ((HAL_GetTick() - led2_on_tick) >= LED_BLINK_MS)
        {
            led2_active = 0;
            HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET);
        }
    }
}

/**
  * @brief  Radio status callback (registered via myRadio_init).
  *         Runs in main loop context via myRadio_process() -> Radio.IrqProcess().
  */
void Bridge_RadioCallback(uint8_t status, rfRxPacket_ts packet)
{
    switch (status)
    {
    case RX_STA_SECCESS:
        /* Wrap received LoRa payload in Bluetooth format, forward via USART2 */
        if (packet.len > 0)
        {
            uint8_t pkt_len = BT_Pack(packet.payload, packet.len, bt_tx_buf);
            HAL_UART_Transmit_DMA(&huart2, bt_tx_buf, pkt_len);
        }
        /* Re-enter continuous RX mode */
        myRadio_receiver();
        break;

    case TX_STA_SECCESS:
        /* LoRa TX success: blink LED1 */
        led1_active = 1;
        led1_on_tick = HAL_GetTick();
        HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET);

        /* Re-enter continuous RX mode */
        myRadio_receiver();

        /* Restart UART DMA RX for next packet from main board */
        bridge_state = BRIDGE_STATE_IDLE;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_buf, UART_RX_BUF_SIZE);
        break;

    case TX_STA_ERROR:
        /* TX error: re-enter RX, restart DMA */
        myRadio_receiver();
        bridge_state = BRIDGE_STATE_IDLE;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_buf, UART_RX_BUF_SIZE);
        break;

    case RX_STA_TIMEOUT:
        /* RX timeout — re-enter RX mode. With rf_rxTimeout=0 (continuous),
           this fires rarely; still handle gracefully. */
        myRadio_receiver();
        break;

    case RX_STA_PAYLOAD_ERROR:
        /* RX CRC/payload error — discard, re-enter RX */
        myRadio_receiver();
        break;

    default:
        break;
    }
}

/* -------------------------------------------------------------------------- */
/*                      HAL UART Callback Overrides                           */
/* -------------------------------------------------------------------------- */

/**
  * @brief  UART event callback (ISR context).
  *         Called by HAL when USART2 IDLE line is detected during DMA RX.
  *         Only sets flags — no blocking calls in ISR.
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2 && Size > 0)
    {
        uart_rx_size = Size;
        uart_rx_ready = 1;
    }
}

/**
  * @brief  UART TX complete callback (ISR context, DMA IRQ).
  *         Called after USART2 DMA transmission finishes.
  *         Triggers LED2 blink (non-blocking).
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        led2_active = 1;
        led2_on_tick = HAL_GetTick();
        HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET);
    }
}
