#include "uart_protocol.h"
#include "stdlib.h"
#include "string.h"
#include "robot_msg.h"
#include "app_state.h"
#include "app_init.h"
#include "usart.h"
#include "fifo_queue.h"
#include "touchscreen.h"
#define crcCode 0x80086066  // CRC 校验标准码

const uint8_t checkLen[4] = {0, sizeof(uint8_t), sizeof(uint8_t), sizeof(uint32_t)}; // 不同校验类型校验位长度

/*串口数据结构体句柄*/
USARTDataFrame_S U2DataFrame={.arr={0}};

char bwTemp;
char tjcTemp;
bool UartDMAStart()
{
    HAL_UART_Receive_DMA(&huart2, (uint8_t *)&bwTemp,1);
    HAL_UART_Receive_DMA(&huart3, (uint8_t *)&tjcTemp, 1);
    return true;
};

// 回调函数定义
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2)
	{
		enQueue(U2msgRxQueue, bwTemp);
		HAL_UART_Receive_DMA(&huart2, (uint8_t *)&bwTemp, 1);
	}
	if (huart == &huart3)
	{
		enQueue(TJCMsgRxQueue, tjcTemp);
		HAL_UART_Receive_DMA(&huart3, (uint8_t *)&tjcTemp, 1);
	}
}

//初始化数据包
void Usart2DataFrameInit(uint8_t mode)	//数据检验的模式
{
			U2DataFrame.checkmode=mode;
			U2DataFrame.sendlen=0;
// 			if (flag.RobotMode == 1)
// 			{ // R2

// 			}
// 			else
// 			{ // R1
					// U2DataFrame.head[0] = 0xA5;
					// U2DataFrame.tail[0] =0x5A;
					// U2DataFrame.headlen= 1;
					// U2DataFrame.taillen = 1;
                    U2DataFrame.head[0] = 0xFF;
					U2DataFrame.head[1] = 0xFE;      
					U2DataFrame.headlen = 2;
					U2DataFrame.tail[0] = 0xAA; 
					U2DataFrame.tail[1] = 0xDD;
					U2DataFrame.taillen = 2;
			// }
}


// 和校验
uint8_t checkSum(MsgId_E msgType, uint8_t data[])
{
    uint8_t _sum = 0;
    for (int i = 0; i < MsgLen(msgType); i++)
        _sum += data[i];
    return _sum;
};
// CRC校验
/*
data 0x06                            0000 0110
crc 0xffffffff -> 0x00000006         1111 1111 0000 0110  0110 0011 1010
crc 0x00000006 -> 0x00000003         0x00000003^0x80086066=0x8008606A
crc 0x00000003 -> 0x8008606A
*/
uint32_t checkCRC(MsgId_E msgType, uint8_t data[])
{
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < MsgLen(msgType); i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x00000001)
            {
                crc ^= crcCode;
            }
            crc >>= 1;
        }
    }
    return ~crc;
};

/*
 * 串口数据编码函数 为相应串口数据包添加 包头 ID 数据 校验 包尾
 * @param  msgType  消息类型（枚举）
 * @param  usartData  所用串口数据包句柄(包括包头包尾，缺数据)
 * @return          返回指向编码后数据的指针
 */
uint8_t *dataEncode(MsgId_E msgType, USARTDataFrame_S *UDataFrame)
{
    UDataFrame->sendlen=0;
	// 添加包头
    for (int i = 0; i < U2DataFrame.headlen; i++)
    {
        UDataFrame->arr[i] = UDataFrame->head[i];
    }
    UDataFrame->sendlen += U2DataFrame.headlen;
//		// ID检验
//    if (UDataFrame->checkmode == USART_ID_CHECK)
//    {
//        UDataFrame->arr[UDataFrame->sendlen] = msgType;
//        UDataFrame->sendlen += checkLen[USART_ID_CHECK];
//    }
    // 把对应类型数据添加进数据包
    int check_index = UDataFrame->sendlen;
    memcpy((&UDataFrame->arr[UDataFrame->sendlen]), (char *)MsgAddr(msgType), MsgLen(msgType));
    UDataFrame->sendlen += MsgLen(msgType);
    // 和校验
    if (UDataFrame->checkmode == USART_SUM_CHECK)
    {
        UDataFrame->arr[UDataFrame->sendlen] = checkSum(msgType, &UDataFrame->arr[check_index]);
        UDataFrame->sendlen += checkLen[USART_SUM_CHECK];
    }
    // CRC校验
//    if (UDataFrame->checkmode == USART_CRC_CHECK)
//    {
//        uint32_t crc = checkCRC(msgType, &UDataFrame->arr[check_index]);
//        memcpy(&UDataFrame->arr[UDataFrame->sendlen], &crc, (size_t)checkLen[USART_CRC_CHECK]);
//        UDataFrame->sendlen += checkLen[USART_SUM_CHECK];
//    }
    // 添加包尾
    for (int i = 0; i < U2DataFrame.taillen; i++)
    {
        UDataFrame->arr[(UDataFrame->sendlen)+ i] = UDataFrame->tail[i];
    }
    UDataFrame->sendlen += U2DataFrame.taillen;
		
    // 返回已经编码好的数据指针
    return UDataFrame->arr;
}


/*
 * 串口数据解码函数   temp添加进数据包
 * @param  temp  读取到的单个字节数据
 * @param  msgType  消息类型（枚举）
 * @param  usartData  所用串口数据包句柄
 * @return          返回解码结果
 */
//bool dataDecode(uint8_t temp, MsgId_E msgType, USARTData_S *usartData)
//{
//    bool if_not_end; // 是否接收完整
//    if_not_end = usartData->rec_len < MsgLen(msgType) + usartData->check_len;
//    if (if_not_end)
//    {
//        usartData->rec_arr[usartData->rec_len] = temp;
//        usartData->rec_len++;
//        UpdateHeadEnd(temp, usartagreement.TAIL_Len, usartData->rec_end_arr);              // 添加包尾
//        if (!memcmp(usartData->rec_end_arr, usartData->end_char, usartagreement.TAIL_Len)) // 校验包尾
//            usartData->get_end = true;
//    }
//    else
//    {
//        clearFlag(usartData); // 清空标志位
//    }
//    if (usartData->get_end)
//    {
//        clearFlag(usartData);
//        memcpy((char *)MsgAddr(msgType), &usartData->rec_arr[0], MsgLen(msgType));
//        return true;
//    }
//    return false;
//}

// 读取数据包
//void ReadMsg(uint8_t temp, USARTData_S *usartData)
//{
//    if (usartData->get_head)
//    {
//        if ((usartData->check_mode & USART_ID_CHECK) && !usartData->get_head_id)
//        {
//            usartData->head_id = temp;
//            usartData->get_head_id = true;
//        } // 得到Id
//        else if (dataDecode(temp, usartData->head_id, usartData)) // 解码
//        {
//            int index = MsgLen(usartData->head_id); // 这类数据长度
//            if (usartData->check_mode & USART_SUM_CHECK)
//            {
//                uint8_t sum = checkSum(usartData->head_id, usartData->rec_arr);
//                if (sum != usartData->rec_arr[MsgLen(usartData->head_id)])
//                {
//                    clearFlag(usartData);
//                    return;
//                }
//                index += checkLen[USART_SUM_CHECK];
//            }
//            if (usartData->check_mode & USART_CRC_CHECK)
//            {
//                uint32_t crc = checkCRC(usartData->head_id, usartData->rec_arr);
//                uint32_t crc_rec;
//                memcpy(&crc_rec, &usartData->rec_arr[index], checkLen[USART_CRC_CHECK]);
//                if (crc != crc_rec)
//                {
//                    clearFlag(usartData);
//                    return;
//                }
//                index += checkLen[USART_SUM_CHECK];
//            }
//            memcpy((char *)MsgAddr(usartData->head_id), usartData->rec_arr, MsgLen(usartData->head_id));
//            clearFlag(usartData);
//        }
//    }
//    else
//    {
//        getHeadMsg(temp, usartData);
//    }
//}