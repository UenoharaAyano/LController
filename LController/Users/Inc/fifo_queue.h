#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H
#include "main.h"
#include <stdbool.h>

#define QUEUEMAXSIZE 640 // 队列的最大容量

// 队列结构体定义
typedef struct Queue
{
    int front;                  // 队列头索引
    int rear;                   // 队列尾索引
    int size;                   // 队列的总容量
    int count;                  // 队列中当前元素的数量
    uint8_t data[QUEUEMAXSIZE]; // 队列存储的数据
} Queue;

// 创建一个新的队列
Queue *newQueue(void);

// 所需队列初始化
bool QueueInit();

// 检查队列是否已满
bool isFull(Queue *queue);

// 检查队列是否为空
bool isEmpty(Queue *queue);

// 向队列中添加一个元素
bool enQueue(Queue *queue, uint8_t ch);

// 向队列中添加多个元素
bool enQueueLen(Queue *queue, uint8_t *ch, int len);

// 从队列中移除一个元素
bool deQueue(Queue *queue, uint8_t *ch);

// 从队列中移除多个元素
bool deQueueLen(Queue *queue, uint8_t *ch, int Len);

// 清空队列
void clearQueue(Queue *queue);

// 删除队列并释放内存
void deleteQueue(Queue *queue);

// 全局队列变量声明
extern Queue *U2msgRxQueue;
extern Queue *U2msgTxQueue;
extern Queue *TJCMsgRxQueue; 
#endif