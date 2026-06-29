#include "robot_msg.h"
#include "app_state.h"
#include "cmsis_os.h"

R1ControlMsg_S r1controlmsg;
R1BackMsg_S r1backmsg;
// R2ControlMsg_S r2controlmsg;
// R2BackMsg_S r2backmsg;

int MsgLen(MsgId_E msgType)	// 控制包字节长度
{
//  if(msgType== R1all_ID) 
		return sizeof(r1controlmsg);
	// else
	// 	return sizeof(r2controlmsg);
}
int MsgAddr(MsgId_E msgType)	// 控制包地址
{
//  if(msgType== R1all_ID)
		return (uint32_t)&r1controlmsg;
	// else
	// 	return (uint32_t)&r2controlmsg;
}

void BoolChange(ConBool Type, bool value)
{
    if(flag.RobotMode == 0)
        r1controlmsg.BOOL[Type / 8] = (r1controlmsg.BOOL[Type / 8] & ~(1 << (Type % 8))) | ((value & 0x01) << (Type % 8));
    // else // R2
    //     r2controlmsg.BOOL[(Type -R1BoolNum) / 8] = (r2controlmsg.BOOL[(Type - R1BoolNum) / 8] & ~(1 << ((Type - R1BoolNum) % 8))) | ((value & 0x01) << ((Type - R1BoolNum) % 8));
}
bool Boolback(ConBool Type)
{
//    if(flag.RobotMode == 0)
        return (r1controlmsg.BOOL[Type / 8] >> (Type % 8)) & 0x01;
    // else // R2
    //     return (r2controlmsg.BOOL[(Type - R1BoolNum) / 8] >> ((Type - R1BoolNum) % 8)) & 0x01;
}
void Press(ConBool type)
{
    BoolChange(type, 1);
    osDelay(20);
    BoolChange(type, 0);
}