#include "touchscreen.h"
#include "usart.h"
#include "fifo_queue.h"
#include "string.h"
#include "input.h"
#include "robot_msg.h"
#include "cmsis_os2.h"
#include "app_state.h"
#include "app_init.h"

Queue *TJCMsgQueue;
TjcMsg tjcMsg = {.getEnd= true ,.getHead= false ,.head= 0 ,.endIndex=0 ,.tempData= {0} };
uint8_t msgSendTemp[50];
uint8_t TxtmsgSendTemp[50];

uint8_t MapState[12];	// 储存2区地图的状态
uint8_t Mapindex=0;		// 2区地图索引，用来标记格子的序号
uint32_t total = 0; 	// 暂存24位状态数据

void R1str_Deal(TjcMsg *tjcMsg)	// 用来处理字符串反馈"有"数据的情况
{	
	if(tjcMsg->tempData[0]==0x31)	//R1T
		MapState[Mapindex] = 1;
	else if(tjcMsg->tempData[0]==0x32)	//R2T
		MapState[Mapindex] = 2;
	else if(tjcMsg->tempData[0]==0x46)	//False
		MapState[Mapindex] = 3;
	Mapindex++;
}

/*
	小端模式				
	                   对应序号：	   1  2  3  4
	地图序号设置		控制包.Map[0]=(00 01 10 11 )
	1 2 3			   对应序号：	   5  6  7  8
	4 5 6			   控制包.Map[1]=(00 01 10 11 )
	7 8 9			   对应序号：	   9  10  11  12
	10 11 12		   控制包.Map[2]=(00 01 10 11 )
*/
void Map_Deal()
{
	for(int i=0;i<12;i++)
		total |= (uint32_t)MapState[i] << (2 * (11 - i));
    r1controlmsg.Map[0] = (total >> 16) & 0xFF; // 高8位 → 第1个字节		
    r1controlmsg.Map[1] = (total >> 8) & 0xFF;  // 中8位 → 第2个字节		
    r1controlmsg.Map[2] = total & 0xFF;         // 低8位 → 第3个字节
    total=0;
}

void GetMsg(TjcMsg *tjcMsg, char msg)
{
    if (tjcMsg->getHead)
    { // 得到包头
			if (msg == end[tjcMsg->endIndex]&&(tjcMsg->dataflag ||tjcMsg->head!=num))	// 新增处理num负数反馈时出现并非包尾的FF的情况，click和string不变
        { // 检测包尾
            tjcMsg->endIndex++;
            if (tjcMsg->endIndex == strlen(end))
            {
                tjcMsg->getEnd = true;
                tjcMsg->getHead = false;
                tjcMsg->endIndex = 0;
                if (tjcMsg->head==string && tjcMsg->datalen==0)	// 分支出来处理字符串反馈只有包头包尾没有数据的情况
                {
                	MapState[Mapindex]=0;
                	Mapindex++;
                }
                if(Mapindex==12)	
                {	// 当状态数组填满时，开始处理状态数组
                	Map_Deal();
                	Mapindex=0;
                }
            }
        }
				else
				    MsgDeal(tjcMsg, msg);
    }
    if (tjcMsg->getEnd)
    {
        if (MsgInHead(msg))  
        { // 检测包头并储存
            tjcMsg->head = msg;
            tjcMsg->datalen = 0;
            tjcMsg->getEnd = false;
            tjcMsg->getHead = true;
            if(msg ==num)
            tjcMsg->dataflag=false;
        }
    }
}

// 检测包头
bool MsgInHead(char msg)
{
    for (size_t i = 0; i < num_valid_value; i++)
    {
        if (valid_value[i] == msg)
        {
            return true;
        }
    }
    return false;
}
// 双态开关和数值反馈处理
uint8_t dualswitchvalIndex = 0; // 双控开关反馈索引
uint8_t msgbackIndex = 1;

void R1C1dualswitch_deal(TjcMsg *tjcMsg)
{
    // 双态开关val接收
    switch (dualswitchvalIndex)
    {
    case 2:
		Press(Protect);
        dualswitchvalIndex = 0;
        return;
    case 3:
        Press(Zero);
        dualswitchvalIndex = 0;
        return;
    case 4:
		Press(InstPro);
        dualswitchvalIndex = 0;
        return;
	case 5:
		Press(takeplan);
		dualswitchvalIndex = 0;
		return;
	case 6:
		Press(ArmStand);
		dualswitchvalIndex = 0;
		return;
	case 7:
		Press(MoveWeap);
		dualswitchvalIndex = 0;
		return;
	case 8:
		Press(ThrowWeap);
		dualswitchvalIndex = 0;
		return;
	case 9:
		Press(Trace);
		dualswitchvalIndex = 0;
		return;
	case 10:
		BoolChange(LockAngle, !Boolback(LockAngle));
		dualswitchvalIndex =0;
		return;
	case 11:
		BoolChange(LockPoint, !Boolback(LockPoint));
		dualswitchvalIndex =0;
		return;
	case 12:
		BoolChange(ITL, !Boolback(ITL));
		dualswitchvalIndex =0;
		return;
	case 13:
		BoolChange(Arm, !Boolback(Arm));
		dualswitchvalIndex =0;
		return;
	case 14:
		BoolChange(trustswitch, !Boolback(trustswitch));
		dualswitchvalIndex =0;
		return;
	case 15:
		BoolChange(PutCubeNum, !Boolback(PutCubeNum));
		dualswitchvalIndex =0;
		return;
    default:
        break;
    }
}

void R1C2dualswitch_deal(TjcMsg *tjcMsg)
{
    switch (dualswitchvalIndex)
    {
	case 1:
		BoolChange(TakeCubePose, !Boolback(TakeCubePose));
		dualswitchvalIndex = 0;
		return;
    case 2:
		BoolChange(DTtest, !Boolback(DTtest));
        dualswitchvalIndex = 0;
        return;
    case 3:
        BoolChange(TakeCube23, !Boolback(TakeCube23));
        dualswitchvalIndex = 0;
        return;
    case 4:
		BoolChange(direction, !Boolback(direction));
        dualswitchvalIndex = 0;
        return;
	case 5:
		BoolChange(xchaway, !Boolback(xchaway));
		dualswitchvalIndex = 0;
		return;
	case 6:
		BoolChange(Auto, !Boolback(Auto));
		dualswitchvalIndex = 0;
		return;
	case 7:
		BoolChange(Updown, !Boolback(Updown));
		dualswitchvalIndex = 0;
		return;
	case 24:
		BoolChange(relocation, !Boolback(relocation));
		dualswitchvalIndex = 0;
		return;
    default:
        break;
    }
}

void R1C1val_Deal(TjcMsg *tjcMsg)
{

    // 数值输入接收
	switch (msgbackIndex)
	{
		case 1:
		{
			memcpy(&r1controlmsg.s1, tjcMsg->tempData, 1);
			msgbackIndex++;
			break;
		}
		case 2:
		{
			memcpy(&r1controlmsg.s2, tjcMsg->tempData, 1);
			msgbackIndex++;
			break;
		}
//		case 3:
//		{
////			memcpy(&r1controlmsg.area, tjcMsg->tempData, 1);
//			msgbackIndex++;
//			break;
//		}
		case 3:
		{
			memcpy(&r1controlmsg.pilenum, tjcMsg->tempData, 1);
			msgbackIndex++;
			break;
		}
		case 4:
		{
			memcpy(&r1controlmsg.gridnum, tjcMsg->tempData, 1);
			msgbackIndex++;
			break;
		}
		case 5:
		{
			memcpy(&r1controlmsg.tex, tjcMsg->tempData, 2);
			msgbackIndex++;
			break;
		}
		case 6:
		{
			memcpy(&r1controlmsg.tey, tjcMsg->tempData, 2);
			msgbackIndex++;
			break;
		}
		case 7:
		{
			memcpy(&r1controlmsg.angle, tjcMsg->tempData, 2);
			msgbackIndex =1;
			break;
		}
		default:
			break;
	}
}

void R1C2val_Deal(TjcMsg *tjcMsg)
{
	// 数值输入接收
	switch (msgbackIndex)
	{
		case 1:
		{
			memcpy(&r1controlmsg.R2claw, tjcMsg->tempData, 1);
			msgbackIndex =1;
			break;
		}
		default:
		break;
	}
}

/*
void R2dualswitchval_deal(TjcMsg *tjcMsg)
{
	    // 双态开关val接收
    switch (dualswitchvalIndex)
    {
    case 1:
    {
        Press(LockPoint2);
        dualswitchvalIndex = 0;
        return;
    }
    case 2:
    {
        BoolChange (LockAngle2, !Boolback(LockAngle2));
        dualswitchvalIndex = 0;
        return;
    }
		case 3:
    {
        BoolChange (ITL2, !Boolback(ITL2));
        dualswitchvalIndex = 0;
        return;
    }
		case 4:
    {
        Press(InstSt);
        dualswitchvalIndex = 0;
        return;
    }
		case 5:
    {
        Press(WheelSt);
        dualswitchvalIndex = 0;
        return;
    }
    default:
        break;
    }
}

void R2val_Deal(TjcMsg *tjcMsg)
{

    // 数值输入接收
    switch (msgbackIndex)
    {
    case 1:
    {
        memcpy(&r2controlmsg.X, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 2:
    {
        memcpy(&r2controlmsg.Y, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 3:
    {
        memcpy(&r2controlmsg.angle, tjcMsg->tempData, 2);
        msgbackIndex++;
        break;
    }
    case 4:
    {
//        int32_t temp;
//        memcpy(&temp, tjcMsg->tempData, 4);
//        r2controlmsg.Z_angle = (float)temp / 10000;
				memcpy(&r2controlmsg.singlepath, tjcMsg->tempData, 1);
        msgbackIndex++;
        break;
    }
    case 5:
    {
//			 int32_t temp;
//        memcpy(&temp, tjcMsg->tempData, 4);
//        r2controlmsg.Go_angle= (float)temp / 10000;
//        msgbackIndex = 1;
				memcpy(&r2controlmsg.headnum, tjcMsg->tempData, 1);
        msgbackIndex++;
        break;
    }
		case 6:
		{
				memcpy(&r2controlmsg.mode, tjcMsg->tempData, 1);
        msgbackIndex =1;
				break;
		}
    default:
        break;
    }
}
*/

void MsgDeal(TjcMsg *tjcMsg, char msg)
{
	tjcMsg->tempData[tjcMsg->datalen] = msg;
	tjcMsg->datalen++;
	if(tjcMsg->head == click && tjcMsg->datalen==3)
  	{
		if (tjcMsg->tempData[0] == 0x01)	 //control1页面开关反馈处理
		{
			dualswitchvalIndex=tjcMsg->tempData[1];
			R1C1dualswitch_deal(tjcMsg);
			return;
		}
		if (tjcMsg->tempData[0] == 0x02)	 // control2页面开关反馈处理
		{
			dualswitchvalIndex=tjcMsg->tempData[1];
			R1C2dualswitch_deal(tjcMsg);
			return;
		}
    }
    if (tjcMsg->head == string && tjcMsg->datalen==1)	// 如果是字符串反馈
    { 
		R1str_Deal(tjcMsg);
		return;
    }
    if (tjcMsg->head == num && tjcMsg->datalen==4)	// 如果是数值反馈
    { 		
		if (!flag.TJCpage)	 //control1页面数值反馈处理
		{
    		tjcMsg->dataflag=true;
			R1C1val_Deal(tjcMsg);
			return;
		}
		if (flag.TJCpage)	 // control2页面数值反馈处理
		{
    		tjcMsg->dataflag=true;
			R1C2val_Deal(tjcMsg);
			return;
		}
    }
}
void MsgSend(const char *msgSend)
{
    size_t dataLen = strlen(msgSend);
    memset(msgSendTemp, 0, 50);
    memcpy(msgSendTemp, msgSend, dataLen);
    msgSendTemp[dataLen] = '\0';
    strcat((char *)msgSendTemp, end); // 拼接数据和包尾
    HAL_UART_Transmit_DMA(&huart3, msgSendTemp, dataLen + strlen(end));
}

void TxtmsgSend(const char *msgSend)	// 发送文本控件的文字,需要加双引号
{
    uint8_t dataLen =17;	// "control1.t0.txt=\""的长度是17
    memset(TxtmsgSendTemp, 0, 50);	// 初始化数组
	memcpy(TxtmsgSendTemp, "control1.t0.txt=\"",17);
    strcat((char *)TxtmsgSendTemp, msgSend);
	dataLen +=strlen(msgSend);	// 此时dataLen指向magsend的结束符
    TxtmsgSendTemp[dataLen++] = '\"';
    TxtmsgSendTemp[dataLen] = '\0';	// 此时dateLen指向要拼接包尾的字符串的结束符
    strcat((char *)TxtmsgSendTemp, end); // 拼接数据和包尾
    HAL_UART_Transmit_DMA(&huart3, TxtmsgSendTemp, dataLen + strlen(end));	
}

void numtotxt_send(uint8_t num, uint8_t *arradd)	// 数值转文本发送，适用于文本控件显示多位二级制数的情况
{
	uint8_t dataLen = 17;
	memset(TxtmsgSendTemp, 0, 50);
	if(num==1)	// 对应t1控件，一位
	{
		memcpy(TxtmsgSendTemp, "control1.t1.txt=\"",17);
		TxtmsgSendTemp[dataLen++]=(arradd[0]& 0x08)?'1':'0';
	}
	else if(num==2)	// 对应t2控件，三位
	{
		memcpy(TxtmsgSendTemp, "control1.t2.txt=\"",17);
		TxtmsgSendTemp[dataLen++]=(arradd[0]& 0x04)?'1':'0';
		TxtmsgSendTemp[dataLen++]=(arradd[0]& 0x02)?'1':'0';
		TxtmsgSendTemp[dataLen++]=(arradd[0]& 0x01)?'1':'0';
	}
	else
		return;
	TxtmsgSendTemp[dataLen++] = '\"';
	TxtmsgSendTemp[dataLen] = '\0';
	strcat((char *)TxtmsgSendTemp, end); // 拼接数据和包尾
	HAL_UART_Transmit_DMA(&huart3, TxtmsgSendTemp, dataLen + strlen(end));
}

// void powerSend(uint16_t power)	// 作为进度条的数据
// {
//     memset(msgSendTemp, 0, 50);
//     memcpy(msgSendTemp, "Power.val=", 10);
//     msgSendTemp[10] = power / 10 + '0';
//     msgSendTemp[11] = power % 10 + '0';
//     msgSendTemp[12] = '\0';
//     msgSendTemp[12 + strlen(end)] = '\0';
//     strcat((char *)msgSendTemp, end); // 拼接数据和包尾
//     HAL_UART_Transmit_DMA(&huart3, msgSendTemp, 12 + strlen(end));
// }

void valSend(uint8_t num, int16_t val)	//num表示组件的序号，val表示数值
{
    uint8_t datalen = 9;	// 此时datalen指向"control1."的结束符
    memset(msgSendTemp, 0, 50);
	strcat((char *)msgSendTemp, "control1.");	// 控件反馈全在control1页面
    msgSendTemp[datalen++] = 'n';
    if (num < 10)
    {
        msgSendTemp[datalen++] = num + '0';
    }
    else
    {
        msgSendTemp[datalen++] = num / 10 + '0';
        msgSendTemp[datalen++] = num % 10 + '0';
    }
	msgSendTemp[datalen] = '\0';
    strcat((char *)msgSendTemp, ".val=");
	datalen += strlen(".val=");	// 此时datalen指向.val后面的结束符
    osDelay(10);
    if (val < 0)
    {
        msgSendTemp[datalen++] = '-';
        val = -val; // 取绝对值
    }
    msgSendTemp[datalen++] = val / 10000 + '0';
    msgSendTemp[datalen++] = (val % 10000) / 1000 + '0';
    msgSendTemp[datalen++] = (val % 1000) / 100 + '0';
    msgSendTemp[datalen++] = (val % 100) / 10 + '0';
    msgSendTemp[datalen++] = val % 10 + '0';
    msgSendTemp[datalen] = '\0';
    msgSendTemp[datalen + strlen(end)] = '\0';
    osDelay(10);
    strcat((char *)msgSendTemp, end); // 拼接数据和包尾
    HAL_UART_Transmit_DMA(&huart3, msgSendTemp, datalen + strlen(end));
}

/*
void float_valsend(uint8_t num, float val)
{
    memset(msgSendTemp, 0, 50);
    msgSendTemp[0] = 't';
    msgSendTemp[1] = '0' + num;
    strcat((char *)msgSendTemp, ".txt=\"");
    if (val < 0)
    {
        strcat((char *)msgSendTemp, "-");
        val = -val; // 取绝对值
    }
    uint8_t datalen = strlen((char *)msgSendTemp);
    int16_t intPart = (int16_t)val;                        // 整数部分
    int16_t fracPart = (int16_t)((val - intPart) * 10000); // 小数部分，保留四位小数
    if (intPart < 10)
    {
        msgSendTemp[datalen++] = intPart + '0';
    }
    else
    {
        msgSendTemp[datalen++] = intPart / 100 + '0';
        msgSendTemp[datalen++] = (intPart % 100) / 10 + '0';
        msgSendTemp[datalen++] = intPart % 10 + '0';
    }
    msgSendTemp[datalen++] = '.';
    msgSendTemp[datalen++] = fracPart / 1000 + '0';
    msgSendTemp[datalen++] = (fracPart % 1000) / 100 + '0';
    msgSendTemp[datalen++] = (fracPart % 100) / 10 + '0';
    msgSendTemp[datalen++] = fracPart % 10 + '0';
    strcat((char *)msgSendTemp, "\"");
    osDelay(10);
    MsgSend((char *)msgSendTemp);
}
*/

void R1_TJC_val_show()
{
    numtotxt_send(1, r1backmsg.BackBool);
	osDelay(5);
    numtotxt_send(2, r1backmsg.BackBool);
    osDelay(5);
    valSend(10, r1backmsg.plancube1);
    osDelay(5);
    valSend(11, r1backmsg.plancube2);
    osDelay(5);
	valSend(12, r1backmsg.Tasksequence);
    osDelay(5);
	valSend(13, r1backmsg.ang0);
    osDelay(5);
	valSend(14, r1backmsg.ang1);
    osDelay(5);
	valSend(15, r1backmsg.ang2);
	osDelay(5);
	valSend(16, r1backmsg.ang3);
	osDelay(5);
}

/*
void R2_TJC_val_show()
{
		valSend(6, r2backmsg.FL);
    osDelay(5);
    valSend(7, r2backmsg.FR);
    osDelay(5);
    valSend(8, r2backmsg.BL);
    osDelay(5);
    valSend(9, r2backmsg.BR);
    osDelay(5);
		valSend(10, r2backmsg.grid);
    osDelay(5);
    valSend(11, r2backmsg.upperarm);
    osDelay(5);
    valSend(12, r2backmsg.takehead);
    osDelay(5);
    valSend(13, r2backmsg.wheelstate);
    osDelay(5);
    valSend(14, r2backmsg.cubetype);
    osDelay(5);
    valSend(15, r2backmsg.nowgrid);
    osDelay(5);
}
*/
