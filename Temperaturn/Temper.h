#ifndef _TEMPER_H_
#define  _TEMPER_H_

#define SysTick_1     11000
#define N 12

#define TEMPTABCOUNT     (sizeof(temp_table) / sizeof(temp_table[0]))

#define D_SCALE   10      //结果放大倍数, 放大10倍就是保留一位小数 
#define Timer1_Reload   (65536UL - SysTick_1)

extern uint TEMPER;
extern uint TEMPER_Val;

void Adc_Init();
void ADC_Temp();
int Get_Temperature(uint adc);
uint Get_ADC12bitResult(u8 channel);



#endif
