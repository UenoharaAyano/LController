#ifndef MSG_TYPE_H
#define MSG_TYPE_H

#include "main.h"
#include "stdbool.h"

// 数据包ID 消息类型
typedef enum
{
    R1all_ID = 0x01,
    R2all_ID = 0x02,
} MsgId_E;

#define R1BoolNum 43
#define R1BackBoolNum 4
#define R1ControlMsgLen sizeof(R1ControlMsg_S)	// 包的字节大小，不包括包头包尾
#define R1BackMsgLen sizeof(R1BackMsg_S)

// #define R2BoolNum 25

typedef enum
{
    /*********R1  bool值 */
  	Zero, 		//回零——[0]
	Protect, 	//保护
	Enable, 	//使能
	Trace,		//寻迹
	Reset,		//重置
	LockPoint,	//锁点
	ITL,		//叉锁
	Process,    //流程

	ActEn,		//Act使能——[1]
	TakeRod,	//取杆
	StoreRod,	//存杆
	Attack,		//攻击
	MoveWeap,	//移武器
	TakeCube,	//取块
	PutCube,	//放块
	LiftUp,		//抬升

	TakeCube23,	//取块2/3——[2]
	PassCube,	//递块
	ArmStand,	//臂直立
	Updown,		//顺/逆时针
	ActReset,	//Act复位
	PutCubeNum,	//放块号
	LcdRefresh,	//LCD刷新
	LockAngle,	//锁角

	ThrowWeap,  //放弃武器——[3]
	SetZero,	//置零
	TakeCubePose,//取块姿势
	TakeRodPre,	//取杆预备
	Arm,		//用的臂
	InstPro,	//机构保护
	DTtest,		//动态调试
	PreTakeCube,//预取块
	
	PrePutCube,	//预放块——[4]
	ArmReturn,	//臂回零
	Step,		//步进
	Auto,		//自动0/1
	direction,	//正取类型
	cubexcha,	//块互递
	xchaway,	//互递方式
	prepass,	//预互递

	trustswitch,//置信开关——[5]
	takeplan,	//取块规划	
	relocation, //重定位

    /*********R2  bool值 */
	
} ConBool;

#pragma pack(1)		// 以CPU运行效率为代价，修改结构体对齐数
typedef struct R1ControlMsg_S	// 26.6.3--42个bool值--42字节
{
    uint8_t BOOL[R1BoolNum/8+1]; 
    uint8_t area;	// 区域
    uint8_t pilenum;// 桩号
    uint8_t gridnum;// 九宫格号
	uint8_t Map[3];	// 2区地图状态
	uint8_t s1;
	uint8_t s2;
	int8_t R2claw;	// R2爪子状态
	short angle;
	short tex;	
	short tey;
	short setx;
	short sety;
	short setw;
	short kp;
	short ki;
	short kd;
	short kp1;	
	short ki1;
	short kd1;
	short kp2;
	short ki2;
	short kd2;
} R1ControlMsg_S;

typedef struct R1BackMsg_S	// 26.6.3--4个bool值--33字节
{
  	uint8_t BackBool[1];
	uint8_t plancube1;	//规划方块1
	uint8_t plancube2;	//规划方块2
	short ang0;
	short ang1;
	short ang2;
	short ang3;
	short Tasksequence;	//任务序列
	float disX;
	float disY;
	float Yaw;
	float pitch;
	float YIS;  
} R1BackMsg_S;

/*
typedef struct R2ControlMsg_S // 22字节
{
		uint8_t BOOL[R2BoolNum/8 +1];
		uint8_t mode;			
		uint8_t singlepath; //单个路径
		uint8_t headnum;	  //武器头序号
    uint8_t Map[3];     //二区地图状态
		short Vx;     
		short Vy;
		short Vw;
		short X;
		short Y;
		short angle;
} R2ControlMsg_S;

typedef struct R2BackMsg_S  // 22字节
{
		uint8_t grid;				//格子
		uint8_t upperarm;		//大臂
		uint8_t takehead;		//取头
		uint8_t wheelstate;	//底盘状态
    uint8_t cubetype;   //方块类型
    uint8_t nowgrid;    //所在格子
		short X;
		short Y;
		short FL;
		short FR;
		short BR;
		short BL;
		float angle;
} R2BackMsg_S;
*/

#pragma pack()

void BoolChange(ConBool Type, bool value);
bool Boolback(ConBool Type);	//返回指定位的bool值

void Press(ConBool type);

int MsgLen(MsgId_E msgType);
int MsgAddr(MsgId_E msgType);

extern R1ControlMsg_S r1controlmsg;
extern R1BackMsg_S r1backmsg;
// extern R2ControlMsg_S r2controlmsg;
// extern R2BackMsg_S r2backmsg;

#endif