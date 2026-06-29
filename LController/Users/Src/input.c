#include "input.h"
#include "filter.h"
#include "uart_protocol.h"
#include "adc.h"
#include "cmsis_os2.h"
#include "app_state.h"
#include "robot_msg.h"
#include "touchscreen.h"
#include "screen_ui.h"
#include "math.h"
#include <stdlib.h>
/***********************************key GPIO User Define Begin********************************/
__IO Key_S Key[19];

void KeyInit()
{
    Key[K_C1] = (Key_S){GPIOB, GPIO_PIN_6};
    Key[K_C2] = (Key_S){GPIOB, GPIO_PIN_7};
    Key[K_C3] = (Key_S){GPIOB, GPIO_PIN_8};
    Key[K_C4] = (Key_S){GPIOB, GPIO_PIN_9};
    Key[K_R1] = (Key_S){GPIOB, GPIO_PIN_3};
    Key[K_R2] = (Key_S){GPIOB, GPIO_PIN_4};
    Key[K_R3] = (Key_S){GPIOB, GPIO_PIN_5};
    Key[bottom_1] = (Key_S){GPIOC, GPIO_PIN_6};
    Key[bottom_2] = (Key_S){GPIOC, GPIO_PIN_7};
    Key[bottom_3] = (Key_S){GPIOA, GPIO_PIN_11};
    Key[bottom_4] = (Key_S){GPIOA, GPIO_PIN_12};
    Key[TOP_LEFT] = (Key_S){GPIOC, GPIO_PIN_14};
    Key[TOP_RIGHT] = (Key_S){GPIOC, GPIO_PIN_13};
    Key[JOYSTICK_LEFT] = (Key_S){GPIOB, GPIO_PIN_10};   // 左摇杆按键
    Key[JOYSTICK_RIGHT] = (Key_S){GPIOA, GPIO_PIN_15};  // 右摇杆按键
    Key[TOGGLE_1] = (Key_S){GPIOC, GPIO_PIN_8};
    Key[TOGGLE_2] = (Key_S){GPIOA, GPIO_PIN_8};
    Key[TOGGLE_3] = (Key_S){GPIOA, GPIO_PIN_9};
    Key[TOGGLE_4] = (Key_S){GPIOA, GPIO_PIN_10};
}
/***********************************key GPIO User Define end********************************/

// Read key state
GPIO_PinState KeyRead(Key_E key)
{
    return HAL_GPIO_ReadPin(Key[key].GPIOx, Key[key].GPIO_Pin);
}
void KRSet(Key_E key)
{
    HAL_GPIO_WritePin(Key[key].GPIOx, Key[key].GPIO_Pin, GPIO_PIN_SET);
}
void KRReset(Key_E key)
{
    HAL_GPIO_WritePin(Key[key].GPIOx, Key[key].GPIO_Pin, GPIO_PIN_RESET);
}
/*
bottom_1---13
bottom_2---14
bottom_3---15
bottom_4---16
TOP_LEFT---17
TOP_RIGHT---18
JOYSTICK_LEFT---19
JOYSTICK_RIGHT---20
TOGGLE_1---1000 1111
TOGGLE_2---0100 1111
TOGGLE_3---0010 1111
TOGGLE_4---0001 1111
  1  2  3  4
  5  6  7  8
  9  10 11 12
*/
uint8_t _tic = 0;
uint8_t keyturn;
uint8_t KeyScan() 	//矩阵扫描
{
    static uint32_t last_key_state = 0; // 用位图保存上一次扫描到的所有有效按键状态
    uint32_t current_key_state = 0;     // 当前周期的所有按键状态位图
    uint8_t current_pressed_key = 0;    // 当前扫描到的物理按键号
    
    // 扫描矩阵按键
    for (__IO Key_E keyRow = K_R1; keyRow <= K_R3+1; keyRow++)
    {
        KRReset(keyRow);
        for (__IO Key_E key = K_C1; key <= K_C4; key++)
        {
            if (KeyRead(key) == 0) // 按键被按下
            {
                current_pressed_key = (uint8_t)((key - K_C1 + 1) + 4 * (keyRow - K_R1));
                current_key_state |= (1 << current_pressed_key);
            }
        }
        KRSet(keyRow);
    }

    // 扫描底部独立按键与摇杆按键
    for (Key_E key = bottom_1; key <= JOYSTICK_RIGHT; key++)
    {
        if (KeyRead(key) == 0) // 按键被按下
        {
            current_pressed_key = (uint8_t)(key + 6);
            current_key_state |= (1 << current_pressed_key);
        }
    }

    // 边缘检测（即当前是1，上次是0的位）
    uint32_t newly_pressed = current_key_state & ~last_key_state;
    
    last_key_state = current_key_state;

    // 找出触发了哪个按键（这里只返回扫描到的第一个新按下的按键）
    if (newly_pressed != 0)
    {
        for (int i = 1; i <= 20; i++) 
        {
            if (newly_pressed & (1 << i))
            {
                return i; // 返回按键编号
            }
        }
    }
    
    return 0; // 没有新的按键按下
}
uint8_t ToggleScan()
{
    static uint8_t last_stable = 0x0F;
    static uint8_t last_raw    = 0x0F;
    static uint8_t stable_cnt  = 0;

    uint8_t raw = 0x0F;

    // 拨动开关原始读取
    if(KeyRead(TOGGLE_1) == 0) raw += 0x80;
    if(KeyRead(TOGGLE_2) == 0) raw += 0x40;
    if(KeyRead(TOGGLE_3) == 0) raw += 0x20;
    if(KeyRead(TOGGLE_4) == 0) raw += 0x10;

    // 软件消抖：连续 3 次读到相同值才确认变化
    if (raw == last_raw)
    {
        if (++stable_cnt >= 3)
        {
            last_stable = raw;
            stable_cnt = 3;  // 防溢出
        }
    }
    else
    {
        stable_cnt = 0;
    }
    last_raw = raw;

    return last_stable;
}
/*霍尔遥感扫描*/
uint16_t ADC_buffer[5];
// LPF left1_LPF;
// LPF left2_LPF;
// LPF right1_LPF;
// LPF right2_LPF;
uint8_t JSdataInitnum = 0;
JS L1;
JS R;
/*
PCB引脚定义左右交换，实际映射：
0--右摇杆y (PC1 - ADC_CHANNEL_12)
1--右摇杆x (PC2 - ADC_CHANNEL_11) -> 角速度控制
2--左摇杆y (PA1 - ADC_CHANNEL_1)
3--左摇杆x (PA0 - ADC_CHANNEL_0)  -> X/Y速度控制
4--电量  (PC3 - ADC_CHANNEL_13)
*/

bool JoystickInit()
{
    InitLowPassFilter(&left1_LPF, 100, 100);
    InitLowPassFilter(&left2_LPF, 100, 100);
    InitLowPassFilter(&right1_LPF, 100, 100);
    InitLowPassFilter(&right2_LPF, 100, 100);

    // 开启ADC的DMA传输 (5个通道)
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_buffer, 5) == HAL_ERROR)
        return false;
    else
    {
        while (JSdataInitnum++ < 20)  // 前20次采样作为滤波器初始化
        {
            L1.x = LowPassFilter(&left1_LPF, ADC_buffer[2]);
            L1.y = LowPassFilter(&left2_LPF, ADC_buffer[3]);
            R.x = LowPassFilter(&right1_LPF, ADC_buffer[1]);
            R.y = LowPassFilter(&right2_LPF, ADC_buffer[0]);
        }
        
        return true;
    }
}
// 存储摇杆极限值
// xmin xmax ymin ymax xmid ymid
int16_t L1val[6] = {23, 4095, 16, 4056, 2050, 1950};
int16_t Rval[6] = {58, 4080, 104, 4095, 2100, 2150};

#if joystickmeasureFlag == 1
// 静止采样数
uint8_t L1_index = 0;
uint8_t R_index = 0;
#endif

JScontrol_S JScontrolmsg;
uint8_t Dead_zone = 120; // 摇杆死区
short K = 1600;
short Kw = 6200;        // 角速度系数
// 左边摇杆 - 控制X/Y速度
void joystickLeft_scan()
{
    // 读取左摇杆ADC值 (PA0-ADC_CH0, PA1-ADC_CH1)
    L1.x = LowPassFilter(&left2_LPF, ADC_buffer[3]);
    L1.y = LowPassFilter(&left1_LPF, ADC_buffer[2]);
    // L1.x = ADC_buffer[2];
    // L1.y = ADC_buffer[3];

    #if joystickmeasureFlag == 1
        if (L1_index == 0)
        { // 第一次先清零
            L1_index++;
            L1val[0] = 2048;
            L1val[1] = 2048;
            L1val[2] = 2048;
            L1val[3] = 2048;
            L1val[4] = 0;
            L1val[5] = 0;
        }
        else if (L1_index < 51)
        { // 静置采样50个点取均值作为手柄中心位置
            L1_index++;
            L1val[4] += (L1.x / 50);
            L1val[5] += (L1.y / 50);
        }
        else
        { // 将手柄推到底作圆周运动，读取极限值
            L1val[0] = (L1.x < L1val[0]) ? L1.x : L1val[0];
            L1val[2] = (L1.y < L1val[2]) ? L1.y : L1val[2];
            L1val[1] = (L1.x > L1val[1]) ? L1.x : L1val[1];
            L1val[3] = (L1.y > L1val[3]) ? L1.y : L1val[3];
        }
    #else
        if (flag.JSfast_slow)
            K = 1600;
        else
            K = 800;
        // 将L1.x与L1.y映射到-K~K
        // X轴控制
        int32_t range_X_pos = L1val[1] - L1val[4];
        if (range_X_pos < 500) range_X_pos = 1800;
        
        int32_t range_X_neg = L1val[4] - L1val[0];
        if (range_X_neg < 500) range_X_neg = 1800;

        if (L1.x > L1val[4])
        {
            if (abs(K * (L1.x - L1val[4]) / range_X_pos) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velX = 0;
            else
                JScontrolmsg.velX = -K * (L1.x - L1val[4]) / range_X_pos - Dead_zone;
        }
        else
        {
            if (abs(K * (L1val[4] - L1.x) / range_X_neg) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velX = 0;
            else
                JScontrolmsg.velX = K * (L1val[4] - L1.x) / range_X_neg + Dead_zone;
        }

        // Y轴控制
        int32_t range_Y_pos = L1val[3] - L1val[5];
        if (range_Y_pos < 500) range_Y_pos = 1800;
        
        int32_t range_Y_neg = L1val[5] - L1val[2];
        if (range_Y_neg < 500) range_Y_neg = 1800;

        if (L1.y > L1val[5])
        {
            if (abs(K * (L1.y - L1val[5]) / range_Y_pos) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velY = 0;
            else
                JScontrolmsg.velY = K * (L1.y - L1val[5]) / range_Y_pos - Dead_zone;
        }
        else
        {
            if (abs(K * (L1val[5] - L1.y) / range_Y_neg) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.velY = 0;
            else
                JScontrolmsg.velY = -K * (L1val[5] - L1.y) / range_Y_neg + Dead_zone;
        }
        JScontrolmsg.velY = -JScontrolmsg.velY; // Y轴方向取反
    #endif
}
// 右边摇杆--作为角速度控制
void joystickRight_scan()
{
    // 读取右摇杆ADC值 (PC1-ADC_CH11, PC2-ADC_CH12)
    R.x = LowPassFilter(&right1_LPF, ADC_buffer[1]);
    R.y = LowPassFilter(&right2_LPF, ADC_buffer[0]);

    #if joystickmeasureFlag == 1
        if (R_index == 0)
        { // 第一次先清零
            R_index++;
            Rval[0] = 2048;
            Rval[1] = 2048;
            Rval[2] = 2048;
            Rval[3] = 2048;
            Rval[4] = 0;
            Rval[5] = 0;
        }
        else if (R_index < 51)
        { // 静置采样10个点取均值作为手柄中心位置
            R_index++;
            Rval[4] += (R.x / 50);
            Rval[5] += (R.y / 50);
        }
        else
        { // 将手柄推到底作圆周运动，读取极限值
            Rval[0] = (R.x < Rval[0]) ? R.x : Rval[0];
            Rval[2] = (R.y < Rval[2]) ? R.y : Rval[2];
            Rval[1] = (R.x > Rval[1]) ? R.x : Rval[1];
            Rval[3] = (R.y > Rval[3]) ? R.y : Rval[3];
        }
    #else
        int32_t range_R_pos = Rval[1] - Rval[4];
        if (range_R_pos < 500) range_R_pos = 1800;
        
        int32_t range_R_neg = Rval[4] - Rval[0];
        if (range_R_neg < 500) range_R_neg = 1800;

        if (R.x > Rval[4])
        {
            if (abs(Kw * (R.x - Rval[4]) / range_R_pos) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.angW = 0;
            else
                JScontrolmsg.angW = -Kw * (R.x - Rval[4]) / range_R_pos + Dead_zone;
        }
        else
        {
            if (abs(Kw * (Rval[4] - R.x) / range_R_neg) < Dead_zone) // 防止摇杆抖动
                JScontrolmsg.angW = 0;
            else
                JScontrolmsg.angW = Kw * (Rval[4] - R.x) / range_R_neg - Dead_zone;
        }
    #endif
}

// 电量检测
// uint8_t power_scan()
// {
//     uint8_t power = (ADC_buffer[4] - MIN_power) * 100 / (MAX_power - MIN_power);
//     if (power >= 100)
//         return 99;
//     return power;
// }

/***********************************key Function User Define Begin********************************/

// R1按键功能定义
void R1_key_function(uint8_t keynum)
{
    switch (keynum)
    {
    case 1:
    {
        Press(prepass);
		TxtmsgSend("预互递");
        break;
    }
    case 5:
    {
        Press(PreTakeCube);
		TxtmsgSend("预取块");
        break;
    }
    case 9:
    {
        Press(TakeRodPre);
		TxtmsgSend("预取杆");
        break;
    }
    case 2:
    {
        Press(cubexcha);
		TxtmsgSend("块互递");
        break;
    }
    case 6:
    {
        Press(TakeCube);
		TxtmsgSend("取块");
        break;
    }
    case 10:
    {
        Press(TakeRod);
		TxtmsgSend("取杆");
        break;
    }
    case 3:
    {
       Press(LcdRefresh);
		TxtmsgSend("LCD刷新");
       break;
    }
    case 7:
    {
        Press(PrePutCube);
		TxtmsgSend("预放块");
        break;
    }
    case 11:
    {
        Press(StoreRod);
		TxtmsgSend("存杆");
        break;
    }
    case 4:
    {
        Press(Attack);
		TxtmsgSend("攻击");
        break;
    }
    case 8:
    {
        Press(PutCube);
		TxtmsgSend("放块");
        break;
    }
    case 12:
    {
        Press(Step);
		TxtmsgSend("步进");
        break;
    }
    case 13:
    { // 底部1
        Press(Reset);
		TxtmsgSend("重置"); 
        break;
    }
    case 14:
    { // 底部2
        Press(SetZero);
		TxtmsgSend("置零");
        break;
    }
    case 15:
    { // 底部3
        Press(ArmReturn);
		TxtmsgSend("臂回零");
        break;
    }
    case 16:
    { // 底部4
        Press(ActReset);
        TxtmsgSend("Act复位");
        break;
    }
    case 17:	// 切换串口屏界面
    { // 左上
		if(flag.TJCpage)	// 0表示当前在control1界面，1表示在control2界面
		{
			MsgSend("page control1");
			flag.TJCpage=false;
		}
		else
		{
			MsgSend("page control2");
			flag.TJCpage=true;
		}
		osDelay(10);
		break;
	}
    case 18:
    { // 右上
		if(r1controlmsg.area==4)	// 点击区域 +1，0-4的循环		
			r1controlmsg.area=0;
		else
			r1controlmsg.area++;
		valSend(0, r1controlmsg.area);	// 单独向TJC发送显示值，因为统一反馈显示函数需要等flag.TJCvalrefresh才会执行
		osDelay(10);
        break;
    }
    case 19:
    { // 左摇杆按键 - 快慢模式切换
        flag.JSfast_slow = !flag.JSfast_slow;
        osDelay(20);
        Beep(1, 50);
        if (flag.JSfast_slow)
            TxtmsgSend("\"快模式 \"");
        else
            TxtmsgSend("\"慢模式 \"");
        break;
    }
    case 20:
    { // 右边摇杆按键 - 刷新TFT副屏
		flag.LCDfirstshow=true;
		osDelay(20);
        Beep(1, 50);
        break;
    }
    default:
        break;
    }
}
void R1_toggle_function(uint8_t togglenum)
{
    // 提取各个拨杆的状态 (0 或 1)
    uint8_t toggle1 = (togglenum & 0x80) >> 7;
    uint8_t toggle2 = (togglenum & 0x40) >> 6;
    uint8_t toggle3 = (togglenum & 0x20) >> 5;
    uint8_t toggle4 = (togglenum & 0x10) >> 4;

    // 第1个拨杆
    BoolChange(Enable, toggle1);
    MsgSend(toggle1 ? "control1.q0.picc=3" : "control1.q0.picc=2");
    osDelay(5);

    // 第2个拨杆
    BoolChange(ActEn, toggle2);
    MsgSend(toggle2 ? "control1.q1.picc=3" : "control1.q1.picc=2");
    osDelay(5);

    // 第3个拨杆
    BoolChange(Process, toggle3);
    MsgSend(toggle3 ? "control1.q2.picc=3" : "control1.q2.picc=2");
    osDelay(5);
    
    // 第4个拨杆
    BoolChange(LiftUp, toggle4);
    MsgSend(toggle4 ? "control1.q3.picc=3" : "control1.q3.picc=2");
    osDelay(5);

}
// R2按键功能定义

/***********************************key Function User Define End********************************/
