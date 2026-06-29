## 使用说明
* 第一次使用需要在keyboard.h 中将joystickmeasureFlag置为1，debug程序，先静置等待采取中间值，然后将三个摇杆（带壳）向四周转动到极限位置 监测keyboard.c中L1Val、L2Val、RVal的值并记录 将其在代码中修改为记录值
* 默认按键 (1.1版本)
    * 左摇杆按键(19) - 切换快/慢模式
    * 右摇杆按键(20) - 切换单/双摇杆模式 + 刷新LCD显示
    * 第4个拨杆 - 摇杆大小幅度
    * 按下左上摇杆复位 切换为R1模式
    * 按下左下摇杆复位 切换为R2模式
* 摇杆模式说明
    * **单摇杆模式**: 左摇杆1控制XY速度，左摇杆2不使用
    * **双摇杆模式**: 左摇杆1控制Y速度，左摇杆2控制X速度

## 版本说明 (1.1)
### 硬件变更
* **删除蓝牙模块(BW16)**，改用LoRa无线模块
* **GPIO引脚变更：**
    | 功能 | 1.0版本 | 1.1版本 | 备注 |
    |------|---------|---------|------|
    | WIFI_NRST | PC0 | - | 删除(蓝牙模块) |
    | Input_LK1 | PB10 | Input_LK (PB10) | 合并为一个 |
    | Input_LK2 | PB11 | - | 被LoRa_BUSY占用 |
    | LoRa_DIO1 | - | PC0 | 新增 |
    | LoRa_RXEN | - | PB0 | 新增 |
    | LoRa_TXEN | - | PB1 | 新增 |
    | LoRa_BUSY | - | PB11 | 新增 |
    | LoRa_RST | - | PD2 | 新增 |
    | LORA_NSS | - | PB12 | 新增(软件定义) |

### 软件变更
* keyboard.h: 删除 `JOYSTICK_LEFT2`，保留 `JOYSTICK_LEFT`
* keyboard.c: 更新 `KeyInit()` 函数适配新GPIO定义
* 按键编号调整：左摇杆按键=19，右摇杆按键=20（原为19/20/21）

## 自定义索引
* 按键引脚定义 
    * 名称在keyboard.h   key Name User Define Begin
    * 引脚在keyboard.c   key GPIO User Define Begin
    * 功能在keyboard.c   key Function User Define Begin
* LoRa通信 (已从蓝牙BW16迁移至LoRa模块)
    * LoRa配置在 LoRa_Radio/lora_config.h (频率、功率、扩频因子等)
    * LoRa通信接口在 Users/Src/lora_comm.c
    * 接收到数据包的处理在 lora_comm.c 中 R1_LoRaTranMsgDeal / R2_LoRaTranMsgDeal
    * 发送的数据包在msg_type.h
* 串口协议
    * 包头包尾在data_process.c  Usart Agreement User Define Begin
* 小屏界面修改在LCD.c中
* 基本临时变量和标志位在Variables.c
* STATUS
  * LED三色灯效果在WS2812b.c   LED style User Define Begin
  * 蜂鸣器在LCD.c   Beep(uint8_t times, uint16_t delaytime)
* TJC 串口屏 用上位机改
