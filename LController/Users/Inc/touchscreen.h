#ifndef TJC_H
#define TJC_H

#include "main.h"
#include "stdbool.h"

static char end[4] = {0xFF, 0xFF, 0xFF, '\0'};// \0是前后字符串结束符

// X-Macro，使用宏管理一个枚举体，并将枚举体的值放入valid_value数组中
// 使用X-Macro技术维护同步的枚举体 HeadSet 和数组 valid_value
// 包头用来识别是点击信号还是字符串信号还是数据信号
#define HEAD_VALUES    \
    HEAD(click, 0x65)  \
    HEAD(string, 0x70) \
    HEAD(num, 0x71)

#define HEAD(name, value) name = value,
enum HeadSet {
    HEAD_VALUES
};
#undef HEAD // 移除前面的宏定义
/*
上面的枚举等价于
enum HeadSet {
    click = 0x65,
    string = 0x70
    num = 0x71
};
*/
/*
重新定义HEAD_VALUES只输出 value
HEAD_VALUES展开值为
0x65
0x70
0x71
static const int valid_value[] = {0x65, 0x70};
*/
static const int valid_value[] = {
#define HEAD(name, value) value,
    HEAD_VALUES
#undef HEAD
};

static const int num_valid_value = sizeof(valid_value) / sizeof(valid_value[0]); // 元素数

typedef struct TjcMsg {
    bool getEnd;
    bool getHead;
    bool dataflag;
    char head;
    uint8_t datalen;
    uint8_t endIndex; // 结束标记的索引
    char tempData[16];
} TjcMsg;

/*getEnd=1,getHead=0--wait head
get head-->getEnd=0,getHead=1-->wait data
get data-->get
*/
void GetMsg(TjcMsg *tjcMsg, char msg);
bool MsgInHead(char msg);
void MsgDeal(TjcMsg *tjcMsg, char msg);
void MsgSend(const char *msgSend);
void TxtmsgSend(const char *msgSend);
void numtotxt_send(uint8_t num, uint8_t *arradd);
void powerSend(uint16_t power);
void R1str_Deal(TjcMsg *tjcMsg);
void Map_Deal();
void valSend(uint8_t num, int16_t val);
// void float_valsend(uint8_t num, float val);
void R1_TJC_val_show();
void R1C1val_Deal(TjcMsg *tjcMsg);
void R1C2val_Deal(TjcMsg *tjcMsg);
void R1C1dualswitch_deal(TjcMsg *tjcMsg);
void R1C2dualswitch_deal(TjcMsg *tjcMsg);

// void R2_TJC_val_show();
// void R2dualswitchval_deal(TjcMsg *tjcMsg);
// void R2val_Deal(TjcMsg *tjcMsg);

extern TjcMsg tjcMsg;
#endif
