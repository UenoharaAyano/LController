/**
  ******************************************************************************
  * @file           : app_bridge.h
  * @brief          : LoRa <-> UART transparent bridge application header
  ******************************************************************************
  */

#ifndef __APP_BRIDGE_H
#define __APP_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "myRadio.h"

/* Defines -------------------------------------------------------------------*/
#define UART_RX_BUF_SIZE       256u
#define LED_BLINK_MS           100u

/* Bluetooth packet format defines */
#define BT_HEADER              0xA5u
#define BT_TAIL                0x5Au
#define BT_OVERHEAD            3u          /* header(1) + checksum(1) + tail(1) */
#define BT_TX_BUF_SIZE         (UART_RX_BUF_SIZE + BT_OVERHEAD)

/* Public API ----------------------------------------------------------------*/

/**
  * @brief  Initialize the bridge application.
  *         - Turns on LED3 (status: initialized)
  *         - Starts USART2 DMA reception with IDLE detection
  */
void Bridge_Init(void);

/**
  * @brief  Main loop process function.
  *         - Checks for pending UART RX data and forwards to LoRa
  *         - Manages LED blink timers
  *         Call this from the while(1) loop in main().
  */
void Bridge_Process(void);

/**
  * @brief  Radio status callback. Registered with myRadio_init().
  *         Called from myRadio_process() context (main loop, NOT ISR).
  *         - RX_STA_SECCESS: forward received payload to USART2, re-enter RX
  *         - TX_STA_SECCESS: blink LED1, re-enter RX, restart UART DMA
  *         - Errors/timeouts: re-enter RX, restart UART DMA
  */
void Bridge_RadioCallback(uint8_t status, rfRxPacket_ts packet);

#ifdef __cplusplus
}
#endif

#endif /* __APP_BRIDGE_H */
