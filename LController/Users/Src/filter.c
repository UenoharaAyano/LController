#include "filter.h"
#include "assert.h"
#include "string.h"
/*平均滤波器
mf 滤波句柄
windowSize 窗宽
*/

LPF left1_LPF;
LPF left2_LPF;
LPF right1_LPF;
LPF right2_LPF;

void InitMeanFilter(MF *mf, float windowSize)
{
    assert(windowSize > 0);
    assert(windowSize <= 10);
    mf->windowSize = windowSize;
    memset(mf->data, 0, sizeof(mf->data));
}

void InitLowPassFilter(LPF *lpf, float Ts, float fc)
{
    lpf->Ts = Ts;
    lpf->fc = fc;
    lpf->alpha = lpf->Ts / (lpf->Ts + 1 / (2 * PI * lpf->fc));
    lpf->outputLast = 0;
}

void InitHighPassFilter(HPF *hpf, float Ts, float fc)
{
    hpf->Ts = Ts;
    hpf->fc = fc;
    hpf->alpha = 1 / (1 + 2 * PI * hpf->fc * hpf->Ts);
    hpf->inputLast = 0;
    hpf->outputLast = 0;
}
//带通
void InitBandPassFilter(BPF *bpf, float Ts, float fc1, float fc2)
{
    bpf->Ts = Ts;
    bpf->fc1 = fc1;
    bpf->fc2 = fc2;
    bpf->alpha1 = 1 / (1 + 2 * PI * bpf->fc1 * bpf->Ts);
    bpf->alpha2 = bpf->Ts / (bpf->Ts + 1 / (2 * PI * bpf->fc2));
    bpf->inputLast = 0;
    bpf->outputLast = 0;
}

float MeanFilter(MF *mf, float input)
{
    mf->data[0] = input;
    memcpy(&mf->data[1], &mf->data[0], (mf->windowSize - 1) * sizeof(float));
    float sum = 0;
    for (int i = 0; i < mf->windowSize; i++)
    {
        sum += mf->data[i];
    }
    return sum / mf->windowSize;
}

float LowPassFilter(LPF *lpf, float input)
{
    float output = lpf->alpha * input + (1 - lpf->alpha) * lpf->outputLast;
    lpf->outputLast = output;
    return output;
}

float HighPassFilter(HPF *hpf, float input)
{
    float output = hpf->alpha * hpf->outputLast + hpf->alpha * (input - hpf->inputLast);
    hpf->inputLast = input;
    hpf->outputLast = output;
    return output;
}

float BandPassFilter(BPF *bpf, float input)
{
    float output1 = bpf->alpha1 * bpf->outputLast + bpf->alpha1 * (input - bpf->inputLast);
    float output2 = bpf->alpha2 * output1 + (1 - bpf->alpha2) * bpf->outputLast;
    bpf->inputLast = input;
    bpf->outputLast = output2;
    return output2;
}
