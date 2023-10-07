#ifndef CMD_CMD_H_
#define CMD_CMD_H_


#define   UART_DATA_TIMEOUT      100
#define   SENSOR_CMD_TIMEOUT     200

#define   HEAD                      0x02       //帧包头字节
#define   TAIL                      0x03       //帧结尾字节

#define  LEN  1
#define  CMD  4
#define  DAT  5

//命令宏
#define CMD_DEV_CON                   0x43    //‘C’//联络
#define CMD_WRITE_DETER_PARA_W        0x57    // 'W'     //
#define CMD_WRITE_ALARM_PARA_B        0x42   //  'B'     //
#define CMD_SET_DEV_NUM               0x48    //‘H’
#define CMD_READ_DOSE                 0x56    //‘V’
#define CMD_READ_PUSLE                0x50    //‘P’
#define CMD_READ_ALARM_PARA           0x46    //‘F’
#define CMD_READ_DETER_PARA_R         0x52    //‘R’
#define CMD_READ_DETER_PARA_J         0x4A    //‘J’

#define CMD_READ_TEMP_T               0x54    //‘T’ 温度查询
#define CMD_READ_RANGE_Q              0x51    //‘Q’ 量程查询
#define CMD_READ_EDITION_M            0x4D    //‘M’版本查询
#define CMD_SOUND         'A'    // 声音控制
#define CMD_LED           'Y'    // 报警灯
#define CMD_CERTAINKEY    'Z'    // 报警确认按钮
#define CMD_POWER         'N'    // 开关机
#define CMD_INPUT         'I'    // IO状态
#define CMD_BATTERY       'G'    // 电池电量
#define CMD_VERSION       'M'    // 版本

#define CMD_OUT_4_20MA     0x03     // 4_20MA 输出控制
#define CMD_GET_4_20MA     0x04     // 读4_20MA 输入


#pragma pack(1)



typedef union
{
    alt_u8 ByteWhole;
    struct
    {   
        alt_u8 bit0:1;
        alt_u8 bit1:1;
        alt_u8 bit2:1;
        alt_u8 bit3:1;
        alt_u8 bit4:1;
        alt_u8 bit5:1;
        alt_u8 bit6:1;
        alt_u8 bit7:1;
    }BIT;
}BYTE_BIT;


// 0关闭   1打开 2闪烁
typedef struct
{
    char StateLed_Green;      // 0
    char StateLed_Yellow;     // 1
    char StateLed_Red;        // 2
    char AlarmLed_RED;        // 3
    char AlarmLed_Yellow;     // 4
}LED_PARAM;


typedef struct 
{
    float DOSE_RATE_ALARM_1;//剂量率报警1
    float DOSE_RATE_ALARM_2;//剂量率报警2
    float CUM_DOSE_RATE_ALARM_1;//累积剂量率报警1
    float CUM_DOSE_RATE_ALARM_2;//累积剂量率报警2
    float INVALID_ALRAM_1;//失效一级报警
    float INVALID_ALRAM_2;//失效二级报警
}HOST_ALRAM_PARA;// 主机报警参数

typedef struct 
{
    float LOW_REVISE_COE_A;//低量程修正系数A
    float LOW_REVISE_COE_B;//低量程修正系数B
    #if 1
    float LOW_REVISE_COE_C;//低量程修正系数C
    float HIG_REVISE_COE_A;//高量程修正系数A
    float HIG_REVISE_COE_B;//高量程修正系数B
    float HIG_REVISE_COE_C;//高量程修正系数C
    float SUPER_REVISE_COE_A;//超高量程修正系数A
    float SUPER_REVISE_COE_B;//超高量程修正系数B
    float SUPER_REVISE_COE_C;//超高量程修正系数C
    #endif
    alt_u32 DET_THR_1;//探测器阈值1
    alt_u32 DET_THR_2;//探测器阈值2
    alt_u32 DET_THR_3;//探测器阈值3
    alt_u32 DET_TIME;//探测器时间
    alt_u32 HV_THR;//高压阈值阈值
}HOST_SENSOR_PARAM;//写探测器参数

typedef struct 
{
    float DOSE_RATE;//剂量率
    float ACC_DOSE_RATE;//累积剂量
    float WAITINGFORUSE;//备用
    alt_u32 PULSE1;//脉冲1
    alt_u32 PULSE2;//脉冲2

    /*
    bit7   剂量率一级报警、
    bit6位 剂量率二级报警、
    bit5位 累计剂量率一级报警、
    bit4位 累计剂量率二级报警、
    bit3位 过载报警、       
    bit2位 失效一级报警、
    bit1位 失效二级报警、
    bit0位 低本底故障
    */
    BYTE_BIT ALARM_STATUS;//报警状态
    //INF_SOUND_CERTAIN CERTAIN_KEY;//按钮报
    alt_u32 PULSE3;//脉冲3 -- 临界探头专用
}HOST_COUNT_PULSE;//计数脉冲


//系统配置参数
typedef struct
{
	float Canshu1;	 //低量程通道校准因子C
	float Canshu2;	// 高量程通道校准因子C 	
	char yuzhi1[4];	//
	char yuzhi2[4];	//
	char PingHuaShiJian[4];//平滑时间,发送的时候需要转换为4字节ASC字符发送
}SENSOR_PARAM;

//系统配置参数
typedef struct
{
	float DI_A;	//低量程通道校准因子A		
	float DI_B;	// 低量程通道校准因子B 	
	float DI_C;	//低量程通道校准因子C	=	SYS_PRAM.Canshu1
	float GAO_A;	// 高量程通道校准因子A		
	float GAO_B;	// 高量程通道校准因子B 
	float GAO_C;	// 高量程通道校准因子C 	=	SYS_PRAM.Canshu2
}SENSOR_JIAOZHUNYINZI;

//报警信息
typedef struct
{
    float  DoseRatePreAlarm;  	//剂量率一级报警(uSv/h)
    float  DoseRateAlarm;  		//剂量率二级报警(uSv/h)
    float  DosePreAlarm;  		//累计剂量预警阀值
    float  DoseAlarm;  			//累计剂量报警阀值
}SENSOR_ALARM;

//计数结构体    
typedef struct
{
    float  DoseRate;  
    float  Dose;
	alt_u8 State;	
    //状态字: ‘１’为有效，‘０’为无效，
    //  bit7: 剂量率一级报警、
    //  bit6: 剂量率二级报警、
    //  bit5: 累计剂量率一级报警、
    //  bit4: 累计剂量率二级报警、
    //  bit3: 过载报警
    //  bit2: 低本底故障，
    //  bit１、０位未定义
}SENSOR_DOSERATE;


typedef struct
{
    BYTE Charging;
    WORD Vol;       // 电压/10:保留一位小数 
}BAT_INFO;



#pragma pack()


bool ConnectSensor(alt_u8 Addr);
bool ReadDoseRate(alt_u8 Addr);

bool ReadAlarmParam(alt_u8 Addr);
bool WriteAlarmParam(alt_u8 Addr);
bool ReadSensorParam(alt_u8 Addr);
bool WriteSensorParam(alt_u8 Addr);


void ClearRecvData(UART_DATA *pUartData);
bool ValidFrame(UART_DATA *pUartData);
void MakeFrame(UART_DATA *pUartData, alt_u8 Addr, alt_u8 Cmd, alt_u8 *dat, alt_u8 length);
bool WaitSensorAck(alt_u8 Addr, alt_u8 Cmd);

bool HndPcFrame();
bool HndPcCmd();
void SendPcCmd(alt_u8 Addr, alt_u8 Cmd, alt_u8 *dat, alt_u8 length);


bool SoundCtl(alt_u8 Ctl);
bool LedCtl(alt_u8 *led);
bool DevVer(alt_u8 Addr);
bool Out4_20ma(BYTE index, BYTE val);
bool Read4_20ma();

bool ReadBatVol();

#endif /* CMD_CMD_H_ */
