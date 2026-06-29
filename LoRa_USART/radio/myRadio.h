#ifndef __MY_RADIO_H
#define __MY_RADIO_H
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_RF_PACKET_LEN 256


typedef enum
{
    RF_TX_PWR_N_9,
    RF_TX_PWR_N_8,
    RF_TX_PWR_N_7,
    RF_TX_PWR_N_6,
    RF_TX_PWR_N_5,
    RF_TX_PWR_N_4,
    RF_TX_PWR_N_3,
    RF_TX_PWR_N_2,
    RF_TX_PWR_N_1,
    RF_TX_PWR_P_0,
    RF_TX_PWR_P_1,
    RF_TX_PWR_P_2,
    RF_TX_PWR_P_3,
    RF_TX_PWR_P_4,
    RF_TX_PWR_P_5,
    RF_TX_PWR_P_6,
    RF_TX_PWR_P_7,
    RF_TX_PWR_P_8,
    RF_TX_PWR_P_9,
    RF_TX_PWR_P_10,
    RF_TX_PWR_P_11,
    RF_TX_PWR_P_12,
    RF_TX_PWR_P_13,
    RF_TX_PWR_P_14,
    RF_TX_PWR_P_15,
    RF_TX_PWR_P_16,
    RF_TX_PWR_P_17,
    RF_TX_PWR_P_18,
    RF_TX_PWR_P_19,
    RF_TX_PWR_P_20,
    RF_TX_PWR_P_21,
    RF_TX_PWR_P_22,
    RF_TX_PWR_MAX_COUNT,
}rf_txPwr_te;
typedef enum
{
    DVTP_VG2379S433N0S1,
    DVTP_VG2379S490N0S1,
    DVTP_VG2373S868N0S1,
    DVTP_VG2373S915N0S1,
    DVTP_VGdd79S170N0S1,
    DVTP_VGdd79S433N0S1,
    DVTP_VGdd79S490N0S1,
    DVTP_VGdd79S868N0S1,
    DVTP_VGdd79S915N0S1,
    DVTP_VG2379S433X0M1,
    DVTP_VG2379S490X0M1,
    DVTP_VG2373S868X0M1,
    DVTP_VG2373S915X0M1,
    DVTP_VGdd79S170X0M1,
    DVTP_VGdd79S433X0M1,
    DVTP_VGdd79S490X0M1,
    DVTP_VGdd79S868X0M1,
    DVTP_VGdd79S915X0M1,
    DVTP_VG2379S433X0M2,
    DVTP_VG2379S490X0M2,
    DVTP_VG2373S868X0M2,
    DVTP_VG2373S915X0M2,
    DVTP_VGdd79S170X0M2,
    DVTP_VGdd79S433X0M2,
    DVTP_VGdd79S490X0M2,
    DVTP_VGdd79S868X0M2,
    DVTP_VGdd79S915X0M2,
    DVTP_VG2379S433N0SA,
    DVTP_VG2379S490N0SA,
    DVTP_VG2373S868N0SA,
    DVTP_VG2373S915N0SA,
    DVTP_VGdd79S170N0SA,
    DVTP_VGdd79S433N0SA,
    DVTP_VGdd79S490N0SA,
    DVTP_VGdd79S868N0SA,
    DVTP_VGdd79S915N0SA,
    DVTP_MAX_COUNT,
}deviceType_te;

typedef enum
{
    RF_BAUDRATE_90,    
    RF_BAUDRATE_610,   
    RF_BAUDRATE_1220,  
    RF_BAUDRATE_2440,  
    RF_BAUDRATE_5000,  
    RF_BAUDRATE_12500, 
    RF_BAUDRATE_37500, 
    RF_BAUDRATE_62500, 
    MAX_RF_BAUDRATE_COUNT,    //
}rfBaudrate_te;
typedef enum
{
    FREQ_BAND_315,
    FREQ_BAND_433,
    FREQ_BAND_490,
    FREQ_BAND_868,
    FREQ_BAND_915,
    MAX_FREQ_BAND_COUNT,
}freqBand_te;
//! \brief Structure for the TX Packet
typedef struct
{
    uint8_t rmvAddr[8];              //
    uint32_t absTime;                //
    uint8_t len;                     //
    uint8_t payload[MAX_RF_PACKET_LEN];       //
} rfTxPacket_ts;

typedef struct
{
    uint8_t rmvAddr[8];              //
    int16_t rssi;                     //
    int8_t snr;                     //
    uint32_t absTime;                //
    uint32_t rxTimeout;              //
    uint8_t len;                     //
    uint8_t payload[MAX_RF_PACKET_LEN];
} rfRxPacket_ts;
typedef struct
{
    uint8_t SignalBw;                   // LORA [0: 7.8 kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,
                                        // 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved]  
    uint8_t SpreadingFactor;            // LORA [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
    uint8_t ErrorCoding;                // LORA [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
}loraBaudrateFrame_ts;
typedef enum 
{
    RADIO_EXT_CONTROL_TX_UNMODULATED,   //单载波功率测试
    RADIO_EXT_CONTROL_RX_SENSITIVITY,   //接收灵敏度测试
    RADIO_EXT_CONTROL_TX_MODULATED,   //连续调制测试
}controlMode_te;
typedef enum
{
    RX_STA_SECCESS,
    RX_STA_TIMEOUT,
    RX_STA_PAYLOAD_ERROR,
    TX_STA_SECCESS,
    TX_STA_ERROR,
}rxStatus_te;
typedef void (*rfRxCallBack)(uint8_t status, rfRxPacket_ts packet);
extern const loraBaudrateFrame_ts loraBaudrateFrame[MAX_RF_BAUDRATE_COUNT];

void myRadio_init(int agr0, void *agr1_ptr);
void myRadio_abort(void);
uint32_t myRadio_getFrequency(void);
void myRadio_setFrequency(uint32_t freq);
int8_t myRadio_getTxPower(void);
void myRadio_setTxPower(int8_t power);
uint32_t myRadio_getBaudrate(void);
void myRadio_setBaudrate(uint32_t br);
void myRadio_setRfParams(uint8_t sf, uint8_t bw, uint8_t cr);
void myRadio_setChipType(uint8_t type);
uint8_t myRadio_getChipType(void);
int16_t myRadio_getRssi(void);
void myRadio_transmit(rfTxPacket_ts *packet);
void myRadio_receiver(void);
void myRadio_setCtrl(controlMode_te mode, uint32_t value);
void myRadio_process(void);
#endif

