#include "fifo_queue.h"
#include "stdlib.h"
#include "string.h"


Queue *U2msgRxQueue;
Queue *U2msgTxQueue;
Queue *TJCMsgRxQueue;

Queue *newQueue()
{
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  if (!queue)
    return NULL;

  queue->front = 0;           // 队前端
  queue->rear = 0;            // 队后端
  queue->size = QUEUEMAXSIZE; // 队最大容量
  queue->count = 0;           // 队当前容量

  return queue;
}

bool isFull(Queue *queue) { return queue->count >= queue->size; }

bool isEmpty(Queue *queue) { return queue->count == 0; }
/*单个字节复制入队*/
bool enQueue(Queue *queue, uint8_t ch)
{
  if (queue->count < queue->size)
  {
    queue->data[queue->rear] = ch;
    queue->rear = (queue->rear + 1) % queue->size;
    queue->count++;
    return true;
  }
  else
  {
    return false;
  }
}
/*固定长度复制入队*/
bool enQueueLen(Queue *queue, uint8_t *ch, int len)
{
  if (queue->rear < queue->size - len && queue->count < queue->size - len)
  {
    memcpy(&queue->data[queue->rear], ch, len * sizeof(uint8_t));
    queue->rear = (queue->rear + len) % queue->size;
    queue->count += len;
    return true;
  }
  else
  {
    for (int i = 0; i < len; i++)
    {
      if (!enQueue(queue, *(ch + i)))
        return false;
    }
    return true;
  }
}
/*单个字节地址出队*/
bool deQueue(Queue *queue, uint8_t *ch)
{
  if (queue->count > 0)
  {
    *ch = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->size;
    queue->count--;
    return true;
  }
  else
    return false;
}
/*固定长度地址出队*/
bool deQueueLen(Queue *queue, uint8_t *ch, int Len)
{
  for (int i = 0; i < Len; i++)
  {
    if (!deQueue(queue, (ch + i)))
      return false;
  }
  return true;
}
/*清空队列*/
void clearQueue(Queue *queue)
{
  queue->front = 0;
  queue->rear = 0;
  queue->count = 0;
}
/*删除队列*/
void deleteQueue(Queue *queue) { free(queue); }

/*初始化三个全局队列*/
bool QueueInit()
{
    U2msgRxQueue = newQueue();
    U2msgTxQueue = newQueue();
    TJCMsgRxQueue = newQueue();
    if (!U2msgRxQueue || !U2msgTxQueue || !TJCMsgRxQueue)
        return false;
    return true;
}
