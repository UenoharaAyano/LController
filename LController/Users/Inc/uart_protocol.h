#ifndef __DATAPROCESS_H__
#define __DATAPROCESS_H__

#include "robot_msg.h"
#include "main.h"
#include <stdbool.h>

#define ARRSIZE 128
#define HEADMAXSIZE 2
#define ENDMAXSIZE 2

// 一般顺序：Head + Id + Data + Sum + CRC + End
typedef enum
{
    USART_NO_CHECK = 0x00,
    USART_ID_CHECK = 0x01,  // 8bits
    USART_SUM_CHECK = 0x02, // 8bits
    USART_CRC_CHECK = 0x03, // 32bits
} USARTCheckMode_E;

typedef struct USARTDataFrame_S
{
    uint8_t head[HEADMAXSIZE]; // 包头
		uint8_t headlen;
    uint8_t tail[ENDMAXSIZE];  // 包尾
		uint8_t taillen;
	  uint8_t checkmode;  // 校验模式
    uint8_t arr[ARRSIZE]; // 编码得到的包的缓冲区
    int sendlen; // arr索引
} USARTDataFrame_S;

bool UartDMAStart();
void Usart2DataFrameInit(uint8_t mode);
uint8_t *dataEncode(MsgId_E msgType, USARTDataFrame_S *UDataFrame);
extern USARTDataFrame_S U2DataFrame;
extern const uint8_t checkLen[4];
#endif