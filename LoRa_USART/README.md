## 这是LoRa模块转串口输出/接收的工程文件，用于接收控制器发送的指令并将其转换为串口输出。

### 功能
- 接收控制器发送的指令，将指令转换为串口输出至主控板
- 通过同一个串口接收主控板发送的指令，将接收到的指令转换为LoRa信号发送出去
- 通过LED指示工作状态：初始化完成后，LED3常亮。当成功通过串口发送接收到的指令时，LED2闪烁一次。当成功发送LoRa信号时，LED1闪烁一次。
- 与主控板通信的串口请用USART2，USART4预留用于调试等用途。

### 架构
- 使用STM32F072C8T6作为主控芯片。引脚的命名与初始化配置已完成，请参考Core目录下的代码。
- 库文件在Driver目录下，使用HAL库。
- LoRa模块的驱动代码在radio目录下，其中有可供调用的函数，用于初始化LoRa模块、发送指令、接收指令等。

### 1、进入低功耗
调用`./radio/myRadio.c/myRadio_abort()`函数，将射频模块进入低功耗模式，进入低功耗模式后，射频模块将不接收任何无线信号，只有当射频模块进入接收状态后，才能再次接收无线信号。

```c
void myRadio_abort(void)
{
    if (rf_handle == 0)
    {
        return;
    }
    RF_SX126x_EXT_PA_TO_IDLE();
    State = LOWPOWER;
    rf_workProcess = RF_PRC_SLEEP;
    Radio.Sleep();
}
```

### 2、射频模块进入接收状态
调用`./radio/myRadio.c/myRadio_receiver()`函数，将射频模块进入接收状态，进入接收状态后，射频模块将接收周围环境中的无线信号，并可以接收数据包。在接收状态或者发送过程不能重新调用该函数，发送需要等待发送完成才能再次调用该函数，不然会打断无线发送。接收到无线数据后，射频模块会通过`DIO1`引脚产生输出个信号，然后在`./radio/myRadio.c/myRadio_process(void)`函数中读取fifo中的数据包。
休眠唤醒已在底层实现`.\radio\sx126x.c\void SX126xCheckDeviceReady( void )`，每次操作`SX126xWriteCommand(...)`或者`SX126xReadCommand(...)`都会调用`SX126xCheckDeviceReady`，休眠状态下可以直接调用`myRadio_receiver`进入接收状态。
```c
void myRadio_receiver(void)
{ 
    if (rf_handle == 0)
    {
        return;
    }
    RF_SX126x_EXT_PA_TO_RX();
    Radio.Rx( rf_rxTimeout ); 
    rf_workProcess = RF_PRC_RX;
}
```
### 3、射频模块发送数据包
调用`./radio/myRadio.c/myRadio_transmit(rfTxPacket_ts *packet)`函数，将射频模块进入发送状态，进入发送状态后，射频模块将发送数据包，发送完成后才可以调用接收函数进入接收状态，发送完成后，射频模块会通过`IRQ`引脚产生输出个信号，然后在`./radio/myRadio.c/myRadio_process(void)`函数中读取发送完成状态。
休眠唤醒已在底层实现`.\radio\sx126x.c\void SX126xCheckDeviceReady( void )`，每次操作`SX126xWriteCommand(...)`或者`SX126xReadCommand(...)`都会调用`SX126xCheckDeviceReady`。休眠状态下可以直接调用`myRadio_transmit`发送无线数据
```c
void myRadio_transmit(rfTxPacket_ts *packet)
{
    if (rf_handle == 0)
    {
        return;
    }
    RF_SX126x_EXT_PA_TO_TX();

    rf_workProcess = RF_PRC_TX;
    Radio.Send( packet->payload, packet->len );
}
```
### 4、射频初始化
调用`./radio/myRadio.c/myRadio_init(int agr0, void *agr1_ptr)`函数，将射频模块初始化，初始化完成后，射频模块将进入接收状态，该函数会初始化一个默认的参数，如果需要自定义参数，比如频率信道，发射功率，无线波特率等参数，可以调用相关函数接口重新设置：
- `./radio/myRadio.c/myRadio_setFrequency(uint32_t freq)`：设置射频模块工作频率
- `./radio/myRadio.c/myRadio_setTxPower(uint8_t power)`：设置射频模块发射功率
- `./radio/myRadio.c/void myRadio_setBaudrate(uint32_t br)`：设置射频模块无线波特率

有源晶体参数设置：
```c
if ((chipType >= DVTP_VG2379S433N0SA) && (chipType <= DVTP_VGdd79S915N0SA))
{
    SX126xEnableTxco(true);
}
```
用户可以直接调用`SX126xEnableTxco(true);`设置即可。
初始化完成后就可以进入接收状态或者发送无线数据包了，具体使用方法可以参考示例代码。
```c
    myRadio_gpio_init(myRadio_gpioCallback);
    
/**-------------------------radio init----------------------------------**/
    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError; 
    
    if ((chipType >= DVTP_VG2379S433N0SA) && (chipType <= DVTP_VGdd79S915N0SA))
    {
        SX126xEnableTxco(true);
    }
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );

    
// #if defined( USE_MODEM_LORA )
    if (getLoRaSta())
    {
        Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
        
        Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    0, true, 0, 0, LORA_IQ_INVERSION_ON, false );
    }
    // #elif defined( USE_MODEM_FSK )
    else
    {        
        
        Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                    FSK_DATARATE, 0,
                                    FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                    true, 0, 0, 0, 3000 );
        
        Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                    0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                    0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, false,
                                    0, 0,false, false);
    // #else
    }
//     #error "Please define a frequency band in the compiler options."
// #endif
    RF_SX126x_EXT_PA_TO_IDLE();
/**-------------------------radio init end----------------------------------**/
    if ((rfRxCallBack )agr1_ptr)
    {
        rxCb = (rfRxCallBack )agr1_ptr;
    }
    rf_handle = 0xe5;
```
### 5、射频底层执行
调用`./radio/myRadio.c/myRadio_process(void)`函数，该函数需要放在主函数中不断判断检测是否有中断触发（可以放在while循环中执行），然后根据中断标志来解析处理。状态处理可以直接在相应的位置处理，或者通过回调函数`rxCb`将结果返回上一层处理
```c
void myRadio_process(void)
{
    if (rf_handle == 0)
    {
        return;
    }
    Radio.IrqProcess();
}
```
在初始化的时候已经通过回调注册了几个回调函数
```c
RadioEvents.TxDone = OnTxDone;
RadioEvents.RxDone = OnRxDone;
RadioEvents.TxTimeout = OnTxTimeout;
RadioEvents.RxTimeout = OnRxTimeout;
RadioEvents.RxError = OnRxError; 
```
当有相关状态触发时，会通过回调函数将状态回调到`./radio/myRadio.c`中对应的函数中执行。

### 6、无线波特率设置
可以通过`void myRadio_setBaudrate(uint32_t br)`来设置。
- `void myRadio_setBaudrate(uint32_t br)`：通过数组选择定义好的几组扩频因子、带宽、码率的组合即可
