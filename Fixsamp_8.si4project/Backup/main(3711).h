#ifndef __MAIN_H__
#define __MAIN_H__

#include "STC32G.h"

#include "stdio.h"
#include "intrins.h"
#include "string.h"



//#define  FOR_DEBUG

typedef 	unsigned char	BOOL;
#define     TRUE    1
#define     FALSE   0

typedef 	unsigned char	BYTE;
typedef 	unsigned short	WORD;
typedef 	unsigned long	DWORD;

typedef 	unsigned char	u8;
typedef 	unsigned short	u16;
typedef 	unsigned long	u32;

typedef 	unsigned int	uint;


#define alt_u8   BYTE
#define alt_u16 WORD
#define alt_u32 DWORD  


#define bool BYTE
#define true 1
#define false 0

#define BOOL BYTE
#define TRUE 1
#define FALSE 0

#define SENSOR_COUNT 4
#define CHANNLE_NUM  8
#define FLOW_METER_CNT   9



#include "IrDA.h"
#include "uart.h"
#include "EepRom.h"
#include "MwPro.h"
#include "LcdApp.h"
#include "ModBus.h"



typedef struct
{
    WORD  Sign;        // 0xA55A
    BYTE  Address;
    BYTE  res1;
    //HOST_SENSOR_PARAM SenParam[SENSOR_COUNT];   // 14*4 = 56 * 4 = 224
    //HOST_ALRAM_PARA   AlmParam[SENSOR_COUNT];   //  6*4 = 24 * 4 = 96
    WORD  res2;
       
    BYTE  BkLight;      // 背光亮度 1-100
    
    BYTE  SampMode;     // 取样模式
    WORD  SampTime;     // 取样时间
    float SampVol;      // 取样体积
    float SampFlow[CHANNLE_NUM];  // 取样流量
    
    DWORD TotleTime; 
    float TotleFlow;

    BYTE  AlarmThres;   // 报警阈值：%

    BYTE  Enable;       // 通道使能：1位指示一个通道

    WORD  Sum;
}SYS_PARAM;

typedef struct
{
    bool  Running;
    DWORD RunTime;

    float Flow[CHANNLE_NUM];
    float Volume[CHANNLE_NUM];   // 取样体积 
    float TotleFlow;            // 总流量
    float TotleVol;              // 总取样体积
}RUN_STATUS;



#define MAIN_Fosc        11059200UL    // 11.0592M
//#define MAIN_Fosc        35000000UL    // 35M

#define SysTick     9216   // 10ms    // 次/秒, 系统滴答频率, 在4000~16000之间

#define Timer0_Reload   (65536UL - SysTick)     //Timer 0 中断频率

#define OFFLINE_TIME  600

#define OUT_IO_COUNT      5
//#define LED_GREEN         0
//#define LED_YELLOW        1
//#define LED_RED           2
#define LIGHT_BLUE        0
#define LIGHT_YELLOW      1
#define GAS_BUMP          2
#define EX_FAN            3
#define ALARM_SOUND       4



#define MODE_TIME  1                // 定时取样 
#define MODE_VOL   2                // 定量取样
#define MODE_MAN   3                // 手动取样


// output port 

#define ALM_CH_DISABLE    0
#define ALM_FLOW_NOR      1
#define ALM_FLOW_LOW      2
#define ALM_FLOW_HIGH     3
#define ALM_FLOW_ABNOR    4


#define LED_FLASH_TIME    500   // ms


#define RUN_LED(x) (x)?(P4 |= (1<<7)):(P4 &= ~(1<<7))     // 板载LED

// 三色LED
//#define GRE_LED(x) (x)?(P0 |= (1<<2)):(P0 &= ~(1<<2))     // 绿
//#define YEL_LED(x) (x)?(P0 |= (1<<3)):(P0 &= ~(1<<3))     // 黄
//#define RED_LED(x) (x)?(P0 |= (1<<4)):(P0 &= ~(1<<4))     // 红

// 指示灯
//#define RED_LIGHT(x) (x)?(P4 |= (1<<1)):(P4 &= ~(1<<1))   // 红灯
#define YEL_LIGHT(x) (x)?(P3 |= (1<<3)):(P3 &= ~(1<<3))   // 黄灯
#define BLU_LIGHT(x) (x)?(P3 |= (1<<2)):(P3 &= ~(1<<2))   // 蓝灯


#define STOP_M(x)   (P6 & (1<<6))                           //急停控制

#define PW_MAIN(x)    (x)?(P6 |= (1<<4)):(P6 &= ~(1<<4))   // 主电源控制

#define RS485_EN1(x)  (x)?(P0 |= (1<<4)):(P0 &= ~(1<<4)) 
#define RS485_EN2(x)  (x)?(P5 |= (1<<2)):(P5 &= ~(1<<2))

#define BUMP_M(x)   (x)?(P6 |= (1<<7)):(P6 &= ~(1<<7))     //真空泵控制
#define FANS_M(x)   (x)?(P5 |= (1<<0)):(P5 &= ~(1<<0))     //风扇控制
#define ALARM(x)    (x)?(P3 |= (1<<4)):(P3 &= ~(1<<4))      // 报警音

#define VALVE0(x)   (x)?(P4 |=(1<<1)):(P4 &= ~(1<<1))       //电磁阀0控制
#define VALVE1(x)   (x)?(P4 |=(1<<2)):(P4 &= ~(1<<2))       //电磁阀1控制
#define VALVE2(x)   (x)?(P4 |=(1<<3)):(P4 &= ~(1<<3))       //电磁阀2控制
#define VALVE3(x)   (x)?(P4 |=(1<<4)):(P4 &= ~(1<<4))       //电磁阀3控制
#define VALVE4(x)   (x)?(P2 |=(1<<0)):(P2 &= ~(1<<0))       //电磁阀4控制
#define VALVE5(x)   (x)?(P2 |=(1<<1)):(P2 &= ~(1<<1))       //电磁阀5控制
#define VALVE6(x)   (x)?(P2 |=(1<<2)):(P2 &= ~(1<<2))       //电磁阀6控制
#define VALVE7(x)   (x)?(P2 |=(1<<3)):(P2 &= ~(1<<3))       //电磁阀7控制


#define POWER_LOCK()      (P4 & (1<<0))                     // 开关机锁
#define LOCK_BIT()        (1<<0)

#define CMD_OUT_CTL      'O'            // 0x4F
#define CMD_IN_STATUE    'I'            // 0x49
#define CMD_GET_VER      'M'            // 0x56
#define CMD_DATA_TT      'T'            // 0x54 数据透传
#define CMD_SET_ADD      'H'            // 0x48


#define KEY_ALM_CFM      3
#define POWER_ON               0xAA
#define POWER_OFF              0x55


#define RED       0
#define YELLOW    1
#define BLUE      2

extern SYS_PARAM xdata SysParam;
extern RUN_STATUS xdata RunStatus;
extern float SimFlow;
extern alt_u8 g_Output[OUT_IO_COUNT];
extern BYTE ChannelError[FLOW_METER_CNT];





void WriteParam();
void Delay(WORD ms);

void Error();
void OutCtl(alt_u8 id, alt_u8 st);

void ParamDef();

void Init();
void SaveParam();

void GetValve();
void CheckValve();  
void CloseValve();

void OpenPump();
void ClosePump();
void CheckAlarm();
void UpdataUI();

//void MainTask();
void DevRun();
void StartSamp();
void StopSamp(bool Auto);
void SendReadFlowCmd(BYTE ch);
void AbnorAlarm();
void SyncModBusDev();

void DebugMsg(char *msg);
void DebugInt(int msg);
void DumpCmd(BYTE *dat, BYTE len);

#endif

