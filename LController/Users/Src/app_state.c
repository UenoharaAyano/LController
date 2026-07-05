#include "app_state.h"

Flag_S flag;
Temp_S temp;

void FlagInit(void)
{
    // 初始化标志位
		flag.RobotMode =false;
    flag.LCDfirstshow = true;
    flag.JSfast_slow = true;
    flag.TJCvalrefresh = false;
    flag.TJCpage=false;
    flag.clawupflag=false;
}
void TempInit(void)
{
    // 初始化临时变量
    // temp.POWERflashtic = 100; // 电源刷新计数
		temp.key_num=0;
		temp.ToggleNum=0x0F;
    temp.ToggleNumlast = 0;
    temp.L1xlast = 0;
    temp.L1ylast = 0;
    temp.Rxlast = 0;
    temp.Rylast = 0;
    temp.L1xNow = 0;
    temp.L1yNow = 0;
    temp.RxNow = 0;
    temp.RyNow = 0;
}
