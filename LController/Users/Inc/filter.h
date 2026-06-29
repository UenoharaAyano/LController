#ifndef UTILITY_H
#define UTILITY_H

#include "main.h"
#define PI 3.1415926535897932384626433832795028841971

typedef struct MeanFilter
{
    int windowSize;
    float data[10];
} MF;

typedef struct LowPassFilter
{
    float fc;         // 截止频率
    float Ts;         // 采样周期
    float alpha;      // 滤波系数
    float outputLast; // 上一输出值
} LPF;

typedef struct HighPassFilter
{
    float fc;         // 截止频率
    float Ts;         // 采样周期
    float alpha;      // 滤波系数
    float inputLast;  // 上一输入值
    float outputLast; // 上一输出值
} HPF;

typedef struct BandPassFilter
{
    float fc1;
    float fc2;
    float Ts;
    float alpha1;
    float alpha2;
    float inputLast;
    float outputLast;
} BPF;

typedef struct KalmanFilter
{
    float X_last;
    float X_mid;
    float X_now;
    float P_mid;
    float P_now;
    float P_last;
    float kg;
    float A;
    float Q;
    float R;
    float H;
} KF;

void InitMeanFilter(MF *mf, float windowSize);
void InitLowPassFilter(LPF *lpf, float Ts, float fc);
void InitHighPassFilter(HPF *hpf, float Ts, float fc);
void InitBandPassFilter(BPF *bpf, float Ts, float fc1, float fc2);

float MeanFilter(MF *mf, float input);
float LowPassFilter(LPF *lpf, float input);
float HighPassFilter(HPF *hpf, float input);
float BandPassFilter(BPF *bpf, float input);

extern LPF left1_LPF;
extern LPF left2_LPF;
extern LPF left3_LPF;
extern LPF left4_LPF;
extern LPF right1_LPF;
extern LPF right2_LPF;

#endif