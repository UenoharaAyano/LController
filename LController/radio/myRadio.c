#include "main.h"
#include "myRadio.h"
#include "myRadio_gpio.h"

/**-------------------------radio include----------------------------------**/
#include "radio.h"
#include "crc.h"
#include "sx126x-board.h"
#include "sx126x.h"
/**-------------------------radio include end----------------------------------**/

static int8_t rfTxPower;
static uint32_t rfFrequence;
static uint32_t rfBaudrate;
static rfRxCallBack rxCb;
static uint8_t rfRxBuffer[255];
static uint32_t rf_handle;
static uint8_t rf_workProcess;
static uint8_t chipType;
/**-------------------------radio params----------------------------------**/
static uint32_t rf_rxTimeout = 0;   //=0. no time out
                                    //=0xffffffff, into continuous
                                    //=other, time out active, unit: ms

const loraBaudrateFrame_ts loraBaudrateFrame[MAX_RF_BAUDRATE_COUNT] = 
{
    {//91.55bps,SF=12,BW=62.5kHz(0),CR=4
    .SpreadingFactor = 12,        
    .SignalBw = 0,               
    .ErrorCoding = 4,            
    },
    {//610.35bps,SF=10,BW=125kHz(1),CR=4
    .SpreadingFactor = 10,        
    .SignalBw = 1,               
    .ErrorCoding = 4,            
    },
    {//1220.7bps,SF=10,BW=250kHz(2),CR=4
    .SpreadingFactor = 10,        
    .SignalBw = 2,               
    .ErrorCoding = 4,            
    },
    {//2441.41bps,SF=10,BW=500kHz(3),CR=4
    .SpreadingFactor = 10,        
    .SignalBw = 3,               
    .ErrorCoding = 4,            
    },
    {//5022.32bps,SF=9,BW=500kHz(3),CR=3
    .SpreadingFactor = 9,        
    .SignalBw = 3,               
    .ErrorCoding = 3,            
    },
    {//12500bps,SF=8,BW=500kHz(3),CR=1
    .SpreadingFactor = 8,                                                                                
    .SignalBw = 3,               
    .ErrorCoding = 1,            
    },
    {//37500bps,SF=6,BW=500kHz(3),CR=1
    .SpreadingFactor = 6,        
    .SignalBw = 3,               
    .ErrorCoding = 1,            
    },
    {//62500bps,SF=5,BW=500kHz(3),CR=1
    .SpreadingFactor = 5,        
    .SignalBw = 3,               
    .ErrorCoding = 1,            
    },
};
// #define USE_MODEM_LORA
#define REGION_CN779

#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 // Hz

#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                433000000 // Hz

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 // Hz

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 // Hz

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_US915_HYBRID )

#define RF_FREQUENCY                                915000000 // Hz

#else

    #error "Please define a frequency band in the compiler options."

#endif

#define TX_OUTPUT_POWER                             22        // 22 dBm

uint16_t  crc_value;

bool LoRaOn = true;
/*!
 * Radio events function pointer
 */

#define LORA_BANDWIDTH                              1         // [0: 62.5 kHz, 
                                                              //  1: 125 kHz,
                                                              //  2: 250 kHz,
                                                              //  3: 500 kHz]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,       
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

// #elif defined( USE_MODEM_FSK )
// #define FSK_FDEV                                    25e3      // Hz 
// #define FSK_DATARATE                                9600      // bps
// #define FSK_BANDWIDTH                               100e3     // Hz >> DSB in sx126x
// #define FSK_AFC_BANDWIDTH                           100e3     // Hz
// #define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
// #define FSK_FIX_LENGTH_PAYLOAD_ON                   false
#define FSK_FDEV                                    5000      // Hz 
#define FSK_DATARATE                                4800      // bps
#define FSK_BANDWIDTH                               20000     // Hz >> DSB in sx126x
#define FSK_AFC_BANDWIDTH                           50000     // Hz
#define FSK_PREAMBLE_LENGTH                         3         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false
// #define FSK_FDEV                                    47000      // Hz 
// #define FSK_DATARATE                                100000      // bps
// #define FSK_BANDWIDTH                               194000     // Hz >> DSB in sx126x
// #define FSK_AFC_BANDWIDTH                           50000     // Hz
// #define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
// #define FSK_FIX_LENGTH_PAYLOAD_ON                   false
// #else
//     #error "Please define a modem in the compiler options."
// #endif

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            0xffff
#define BUFFER_SIZE                                 64 // Define the payload size here

States_t State = LOWPOWER;

static RadioEvents_t RadioEvents;
static uint32_t txTimeoutCount;
extern void RadioOnDioIrq( void );
void OnTxDone( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnTxTimeout( void );
void OnRxTimeout( void );
void OnRxError( void );
void setLoRaSta( bool sta );
bool getLoRaSta(void);
/**-------------------------radio params end----------------------------------**/
void myRadio_delay(uint32_t time_ms)
{
    HAL_Delay(time_ms);
}
/**
 * @brief IO口中断回调
 *      IO口产生中断后会执行该函数
 *      用于接收射频工作的中断响应
 * 
 * @param index 
 */
void myRadio_gpioCallback(uint8_t index)
{
    RadioOnDioIrq();
}
/**
 * @brief 射频初始化
 * 
 * @param agr0 
 * @param agr1_ptr 无线工作状态响应回调
 *          产生回调给外部使用，@rfRxCallBack
 */
void myRadio_init(int agr0, void *agr1_ptr)
{
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
    // Radio.SetChannel( RF_FREQUENCY );    //此处屏蔽，由外部设置，myRadio_init之后再单独设置频率
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
}
/**
 * @brief 射频底层执行程序
 *      要放在主循环中执行
 * 
 */
void myRadio_process(void)
{
    if (rf_handle == 0)
    {
        return;
    }
    if (State != TX && State != RX)
    {
        return;
    }
    Radio.IrqProcess();
}
/**
 * @brief 退出射频进入休眠
 *  注意：@README.md的开发注意事项
 */
void myRadio_abort(void)
{
    if (rf_handle == 0)
    {
        return;
    }
    RF_SX126x_EXT_PA_TO_IDLE();
    State = LOWPOWER;
    Radio.Sleep();
}
/**
 * @brief 获取射频工作中心频率
 * 
 * @return uint32_t 
 */
uint32_t myRadio_getFrequency(void)
{
    if (rf_handle == 0)
    {
        return 0;
    }
    return rfFrequence;
}
/**
 * @brief 设置射频工作中心频率
 * 
 * @param freq 
 *      具体频点，单位：Hz
 */
void myRadio_setFrequency(uint32_t freq)
{
    if (rf_handle == 0)
    {
        return;
    }
    rfFrequence = freq;
    Radio.SetChannel(freq);
}
/**
 * @brief 获取发射功率
 * 
 * @return int8_t 
 */
int8_t myRadio_getTxPower(void)
{
    if (rf_handle == 0)
    {
        return 0;
    }
    return rfTxPower;
}
/**
 * @brief 设置发射功率
 * 
 * @param power 
 *          单位：dbm
 */
void myRadio_setTxPower(int8_t power)
{
    if (rf_handle == 0)
    {
        return;
    }
    rfTxPower = power;
    SX126xSetRfTxPower(rfTxPower);
}
/**
 * 获取射频波特率
 * @param : br->
*/
uint32_t myRadio_getBaudrate(void)
{
    if (rf_handle == 0)
    {
        return 0;
    }
    return rfBaudrate;
}
/**
 * 设置射频波特率
 * @param : br->
*/
void myRadio_setBaudrate(uint32_t br)
{
    if (rf_handle == 0)
    {
        return;
    }
    rfBaudrate = br;
    if (getLoRaSta())
    {
        Radio.SetTxConfig( MODEM_LORA, rfTxPower, 0, loraBaudrateFrame[br].SignalBw,
                                    loraBaudrateFrame[br].SpreadingFactor, loraBaudrateFrame[br].ErrorCoding,
                                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    true, 0, 0, LORA_IQ_INVERSION_ON, 0 );
        
        Radio.SetRxConfig( MODEM_LORA, loraBaudrateFrame[br].SignalBw, loraBaudrateFrame[br].SpreadingFactor,
                                    loraBaudrateFrame[br].ErrorCoding, 0, LORA_PREAMBLE_LENGTH,
                                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    0, true, 0, 0, LORA_IQ_INVERSION_ON, false );
    }
}
void myRadio_setRfParams(uint8_t sf, uint8_t bw, uint8_t cr)
{
    if (rf_handle == 0)
    {
        return;
    }
    if (getLoRaSta())
    {
        Radio.SetTxConfig( MODEM_LORA, rfTxPower, 0, bw,
                                    sf, cr,
                                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    true, 0, 0, LORA_IQ_INVERSION_ON, 0 );
        
        Radio.SetRxConfig( MODEM_LORA, bw, sf,
                                    cr, 0, LORA_PREAMBLE_LENGTH,
                                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    0, true, 0, 0, LORA_IQ_INVERSION_ON, false );
        printf( "bw=%d, sf=%d, cr=%d\n", bw, sf, cr);
    }
}
/**
 * @brief 设置模组型号
 * 
 * @param type 
 */
void myRadio_setChipType(uint8_t type)
{
    chipType = type;
}
/**
 * @brief 获取模组型号
 * 
 * @return uint8_t 
 */
uint8_t myRadio_getChipType(void)
{
    return chipType;
}
int16_t myRadio_getRssi(void)
{
    return Radio.Rssi(MODEM_LORA);
}
/**
 * @brief 无线发送数据包
 * 
 * @param packet 
 */
void myRadio_transmit(rfTxPacket_ts *packet)
{
    if (rf_handle == 0)
    {
        return;
    }
    RF_SX126x_EXT_PA_TO_TX();
    State = TX;
    Radio.Send( packet->payload, packet->len );
    packet->absTime = Radio.TimeOnAir( MODEM_LORA, packet->len );
}
/**
 * @brief 进入无线接收
 * 
 */
void myRadio_receiver(void)
{ 
    if (rf_handle == 0)
    {
        return;
    }
    State = RX;
    RF_SX126x_EXT_PA_TO_RX();
    Radio.Rx( rf_rxTimeout ); 
}
void myRadio_setCtrl(controlMode_te mode, uint32_t value)
{
    if (rf_handle == 0)
    {
        return;
    }
    switch (mode)
    {
    case RADIO_EXT_CONTROL_TX_UNMODULATED:
    {
        setLoRaSta(false);
        myRadio_init(0, 0);
        RF_SX126x_EXT_PA_TO_TX();
        Radio.SetTxContinuousWave( rfFrequence, rfTxPower, 0 );
    }
        break;
    case RADIO_EXT_CONTROL_TX_MODULATED:
    {
        setLoRaSta(false);
        myRadio_init(0, 0);
        RF_SX126x_EXT_PA_TO_TX();
        Radio.SetTxContinuousWave( rfFrequence, rfTxPower, 0 );
    }
        break;
    case RADIO_EXT_CONTROL_RX_SENSITIVITY:
    {
        setLoRaSta(false);
        myRadio_init(0, 0);
        RF_SX126x_EXT_PA_TO_RX();
        Radio.SetChannel(rfFrequence);
        Radio.Rx( RX_TIMEOUT_VALUE ); 
    }
        break;
    
    default:
        break;
    }
}

/**-------------------------radio funtion----------------------------------**/
void setLoRaSta( bool sta )
{
    LoRaOn = sta;
}
bool getLoRaSta(void)
{
    return LoRaOn;
}

void OnTxDone( void )
{
    rfRxPacket_ts rfRxPacket;
    RF_SX126x_EXT_PA_TO_IDLE();
    State = LOWPOWER;
    if (rxCb)
    {
        rxCb(TX_STA_SECCESS, rfRxPacket);
    }
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    rfRxPacket_ts rfRxPacket;
    if (size > MAX_RF_PACKET_LEN)
    {
        size = MAX_RF_PACKET_LEN;
    }
    State = LOWPOWER;
    memset(rfRxPacket.payload, 0, sizeof(rfRxPacket.payload));
    memcpy(rfRxPacket.payload, payload, size);
    rfRxPacket.len = size;
    rfRxPacket.rssi = rssi;
    rfRxPacket.snr = snr;
    if (rxCb)
    {
        rxCb(RX_STA_SECCESS, rfRxPacket);
    }
}

void OnTxTimeout( void )
{
    rfRxPacket_ts rfRxPacket;
    State = TX_TIMEOUT;
    if (rxCb)
    {
        rxCb(TX_STA_ERROR, rfRxPacket);
    }
}

void OnRxTimeout( void )
{
    rfRxPacket_ts rfRxPacket;
    State = RX_TIMEOUT;
    if (rxCb)
    {
        rxCb(RX_STA_TIMEOUT, rfRxPacket);
    }
}

void OnRxError( void )
{
    rfRxPacket_ts rfRxPacket;
    State = RX_ERROR;
    if (rxCb)
    {
        rxCb(RX_STA_PAYLOAD_ERROR, rfRxPacket);
    }
}
/**-------------------------radio funtion end----------------------------------**/
