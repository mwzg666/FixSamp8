#include "main.h"
#include "uart.h"
#include "MwPro.h"
#include "Cmd.h"
#include "i2c.h"
#include "mcp4725.h"
#include "ads1110.h"


extern const BYTE VERSION[];
extern alt_u8 g_Output[];
extern DWORD Cps[];


WORD g_CommIdleTime = 0;

UART_DATA xdata g_UartData[1];



// 51单片机是大端的，通过结构体发送的数据要转换为小端
DWORD DwordToSmall(DWORD dat)
{
	BYTE buf[4];
    BYTE t;
    DWORD ret;
    
    memcpy(buf, &dat, 4);
	t = buf[3];
	buf[3] = buf[0];
	buf[0] = t;
	t = buf[2];
	buf[2] = buf[1];
	buf[1] = t;

    memcpy(&ret, buf, 4);
    return ret;
}


WORD WordToSmall(WORD dat)
{
	BYTE buf[2];
    BYTE t;
    WORD ret;
    
    memcpy(buf, &dat, 2);
	t = buf[1];
	buf[0] = buf[1];
	buf[1] = t;
	
    memcpy(&ret, buf, 2);
    return ret;
}



float FloatToSmall(float dat)
{
	BYTE buf[4];
    BYTE t;
    float ret;
    
    memcpy(buf, &dat, 4);
	t = buf[3];
	buf[3] = buf[0];
	buf[0] = t;
	t = buf[2];
	buf[2] = buf[1];
	buf[1] = t;

    memcpy(&ret, buf, 4);
    return ret;
}



void ClearRecvData(UART_DATA *pUartData)
{
    memset(pUartData->RecvBuff, 0, MAX_LENGTH);
    pUartData->RecvLength = 0;
    pUartData->Timer = 0;
}


bool ValidFrame(UART_DATA *pUartData)
{
    BYTE rs;
    alt_u8 lcrc;
    alt_u8 tmp[3] = {0};
    
    FRAME_HEAD FrmHead;
    memcpy(&FrmHead, pUartData->RecvBuff, sizeof(FRAME_HEAD));

    if (FrmHead.Head != HEAD)
    {
        return false;
    }

    if (pUartData->RecvBuff[pUartData->RecvLength-1] != TAIL)
    {
        return false;
    }

    if (FrmHead.Len != pUartData->RecvLength)
    {
        return false;
    }

    lcrc = CheckSum(&pUartData->RecvBuff[1],(BYTE)(pUartData->RecvLength-4));
    rs = BcdToHex(pUartData->RecvBuff[pUartData->RecvLength-3], 
                   pUartData->RecvBuff[pUartData->RecvLength-2]);
    if (lcrc != rs)
    {
        //Error();
        return FALSE;
    }

    return true;
}

void MakeFrame(UART_DATA *pUartData, alt_u8 Addr, alt_u8 Cmd, alt_u8 *dat, alt_u8 length)
{
    alt_u8 lcrc;
        
    FRAME_HEAD FrmHead;
    FrmHead.Head = HEAD;
    FrmHead.Len  = length+8;
    FrmHead.Type = 0;
    FrmHead.Addr = Addr;
    FrmHead.Cmd  = Cmd;

    memcpy(pUartData->SendBuff, &FrmHead, sizeof(FRAME_HEAD));
    if (length > 0)
    {
        memcpy(&pUartData->SendBuff[DAT], dat, length);
    }

    lcrc = CheckSum(&pUartData->SendBuff[1], (BYTE)(FrmHead.Len-4));//计算校验和

    //将校验和转换为两个字节的ASCII
    sprintf((char *)&pUartData->SendBuff[FrmHead.Len-3],"%02X",lcrc);
    pUartData->SendBuff[FrmHead.Len-1] = TAIL;   
    pUartData->SendLength = FrmHead.Len;
}


void SendPcCmd(alt_u8 Addr, alt_u8 Cmd, alt_u8 *dat, alt_u8 length)
{
    MakeFrame(&g_UartData[0], Addr, Cmd, dat, length);
    //DebugMsg("PC<:");
    //PrintData(g_UartData[0].SendBuff,g_UartData[0].SendLength);
    Rs485Send(g_UartData[0].SendBuff,(alt_u8)g_UartData[0].SendLength);

    g_CommIdleTime = 0;
}


bool HndPcFrame()
{
    bool ret = false;
    if (ValidFrame(&g_UartData[0]))
    {
        ret = HndPcCmd();
    }
    ClearRecvData(&g_UartData[0]);
    return ret;
}


bool Out4_20ma(BYTE index, BYTE val)
{
    WORD v = val*100;

    switch(index)
    { 
        //case 1:  MCP4725_OutVol(MCP4725_AV_ADDR, v); break;
        //case 2:  MCP4725_OutVol(MCP4725_HV_ADDR, v); break; 
        //case 1:  MCP4725_OutVol(MCP4725_BL_ADDR, v); break;
        //case 2:  MCP4725_OutVol(MCP4725_HV_ADDR, v); break; 
    }

    SendPcCmd(0, CMD_OUT_4_20MA, NULL, 0);
    return true;
}


bool Read4_20ma()
{
    BYTE ret = 0;
    int Voltage = 0;
    //GetAds1110(I2C_4_20MA_IN, ADS110_4_20mA);

    ret = (BYTE)(Voltage/100);
    SendPcCmd(0,CMD_GET_4_20MA, &ret, 1);
    return true;
}


bool ReadBatVol()
{
    BAT_INFO bi;
    int Voltage = 0;
    //GetAds1110(I2C_BAT_VOL, ADS110_BAT_VOL);

    bi.Vol = (WORD)Voltage;
    //bi.Charging = (BAT_CHARGE() != 0);
    SendPcCmd(0,CMD_BATTERY, (BYTE *)&bi, 3);
    return true;
}


bool HndPcCmd()
{
    bool ret = false;
    FRAME_HEAD FrmHead;
    memcpy(&FrmHead, g_UartData[0].RecvBuff, sizeof(FRAME_HEAD));
    switch(FrmHead.Cmd)
    {
        // Dev cmd
        case CMD_SOUND:    ret = SoundCtl(g_UartData[0].RecvBuff[DAT]); break;
        case CMD_LED:      ret = LedCtl(&g_UartData[0].RecvBuff[DAT]);    break;
        case CMD_OUT_4_20MA:  ret = Out4_20ma(g_UartData[0].RecvBuff[DAT], g_UartData[0].RecvBuff[DAT+1]);    break;
        case CMD_GET_4_20MA:  ret = Read4_20ma();        break;
        case CMD_VERSION:     ret = DevVer(FrmHead.Addr);  break;
        case CMD_BATTERY:     ret = ReadBatVol();        break;

        // Sensor cmd
        case CMD_DEV_CON:        ret = ConnectSensor(FrmHead.Addr);    break;
        case CMD_READ_DOSE:      ret = ReadDoseRate(FrmHead.Addr);     break;
        case CMD_READ_ALARM_PARA:     ret = ReadAlarmParam(FrmHead.Addr);    break;
        case CMD_WRITE_ALARM_PARA_B:  ret = WriteAlarmParam(FrmHead.Addr);    break;
        case CMD_READ_DETER_PARA_R:   ret = ReadSensorParam(FrmHead.Addr);   break;
        case CMD_WRITE_DETER_PARA_W:  ret = WriteSensorParam(FrmHead.Addr);   break;

        //default: ret = FrameRevert(&FrmHead);   break;
    }

    return ret;
}




bool ConnectSensor(alt_u8 Addr)
{
    bool ret = false;
    SendPcCmd(Addr, CMD_DEV_CON, NULL, 0);
    ret = true;
    return ret;    
}


BYTE_BIT GetAlarm(BYTE Addr)
{
    BYTE_BIT ret;
    ret.ByteWhole = 0;
    
    if (Cps[Addr-1] > SysParam.AlmParam[Addr-1].INVALID_ALRAM_2)
    {
        ret.BIT.bit3 = 1;
    }
    else if (Cps[Addr-1] > SysParam.AlmParam[Addr-1].DOSE_RATE_ALARM_2)
    {
        ret.BIT.bit6 = 1;
    }
    else if (Cps[Addr-1] > SysParam.AlmParam[Addr-1].DOSE_RATE_ALARM_1)
    {
        ret.BIT.bit7 = 1;
    }
    
    return ret;
}

bool ReadDoseRate(alt_u8 Addr)
{
    HOST_COUNT_PULSE HostDose;
    
    HostDose.DOSE_RATE = 0;
    HostDose.ACC_DOSE_RATE  = 0;
    HostDose.ALARM_STATUS = GetAlarm(Addr);
    HostDose.PULSE1 = DwordToSmall(Cps[Addr-1]);
    HostDose.PULSE2 = 0;
    HostDose.PULSE3 = 0;

    SendPcCmd(Addr, CMD_READ_DOSE, (alt_u8 *)&HostDose, sizeof(HOST_COUNT_PULSE));
    return true;
}

bool ReadAlarmParam(alt_u8 Addr)
{
    HOST_ALRAM_PARA HostAlarm;
    
    memset(&HostAlarm, 0, sizeof(HOST_ALRAM_PARA));
    
    HostAlarm.DOSE_RATE_ALARM_1 = FloatToSmall(SysParam.AlmParam[Addr-1].DOSE_RATE_ALARM_1);
    HostAlarm.DOSE_RATE_ALARM_2 = FloatToSmall(SysParam.AlmParam[Addr-1].DOSE_RATE_ALARM_2);
    HostAlarm.CUM_DOSE_RATE_ALARM_1 = FloatToSmall(SysParam.AlmParam[Addr-1].CUM_DOSE_RATE_ALARM_1);
    HostAlarm.CUM_DOSE_RATE_ALARM_2 = FloatToSmall(SysParam.AlmParam[Addr-1].CUM_DOSE_RATE_ALARM_2);
    HostAlarm.INVALID_ALRAM_1 = FloatToSmall(SysParam.AlmParam[Addr-1].INVALID_ALRAM_1);
    HostAlarm.INVALID_ALRAM_2 = FloatToSmall(SysParam.AlmParam[Addr-1].INVALID_ALRAM_2);
    
    SendPcCmd(Addr, CMD_READ_ALARM_PARA, (alt_u8 *)&HostAlarm, sizeof(HOST_ALRAM_PARA));

    return true;    
}


bool WriteAlarmParam(alt_u8 Addr)
{
    HOST_ALRAM_PARA HostAlarm;

    memcpy(&HostAlarm, &g_UartData[0].RecvBuff[DAT], sizeof(HOST_ALRAM_PARA));
    
    SysParam.AlmParam[Addr-1].DOSE_RATE_ALARM_1 = FloatToSmall(  HostAlarm.DOSE_RATE_ALARM_1);
    SysParam.AlmParam[Addr-1].DOSE_RATE_ALARM_2 = FloatToSmall(HostAlarm.DOSE_RATE_ALARM_2);
    SysParam.AlmParam[Addr-1].CUM_DOSE_RATE_ALARM_1 = FloatToSmall(HostAlarm.CUM_DOSE_RATE_ALARM_1);
    SysParam.AlmParam[Addr-1].CUM_DOSE_RATE_ALARM_2 = FloatToSmall(HostAlarm.CUM_DOSE_RATE_ALARM_2);
        

    SendPcCmd(Addr, CMD_WRITE_ALARM_PARA_B, NULL, 0);

    WriteParam();

    return true;
}


bool ReadSensorParam(alt_u8 Addr)
{
    HOST_SENSOR_PARAM HostParam;
    
    memset(&HostParam, 0, sizeof(HOST_SENSOR_PARAM));
    
    HostParam.LOW_REVISE_COE_A = FloatToSmall(SysParam.SenParam[Addr-1].LOW_REVISE_COE_A);
    HostParam.LOW_REVISE_COE_B = FloatToSmall(SysParam.SenParam[Addr-1].LOW_REVISE_COE_B);
    
    HostParam.DET_THR_1 = DwordToSmall(SysParam.SenParam[Addr-1].DET_THR_1);
    HostParam.DET_THR_2 = DwordToSmall(SysParam.SenParam[Addr-1].DET_THR_2);
    HostParam.DET_TIME  = DwordToSmall(SysParam.SenParam[Addr-1].DET_TIME);
    HostParam.HV_THR    = DwordToSmall(SysParam.SenParam[Addr-1].HV_THR);
    
    SendPcCmd(Addr, CMD_READ_DETER_PARA_R, (alt_u8 *)&HostParam, sizeof(HOST_SENSOR_PARAM));
    
    return true;    
}


bool WriteSensorParam(alt_u8 Addr)
{
    HOST_SENSOR_PARAM HostParam;
    memcpy(&HostParam, &g_UartData[0].RecvBuff[DAT], sizeof(HOST_SENSOR_PARAM));

    // 现在设备中只用了阈值1
    SysParam.SenParam[Addr-1].DET_THR_1 = DwordToSmall(HostParam.DET_THR_1);
    MCP4725_OutVol(ThresAddr[Addr-1], (WORD)SysParam.SenParam[Addr-1].DET_THR_1);
    
    SendPcCmd(Addr, CMD_WRITE_DETER_PARA_W, NULL, 0);
    WriteParam();
    
    return true;   
}



bool SoundCtl(alt_u8 Ctl)
{
    if (Ctl == 0xAA)
    {
        //LEDM_ON();      // 远程报警灯
        g_Output[ALARM_SOUND] = 2;
    }
    else
    {
        //LEDM_OFF();    
        g_Output[ALARM_SOUND] = 0;
    }
    SendPcCmd(0, CMD_SOUND, NULL, 0);
    return true;
}

bool LedCtl(alt_u8 *led)
{
    memcpy(g_Output, led, OUT_IO_COUNT-1);  // 报警音不在这里控制

    //SendPcCmd(0, CMD_LED, NULL, 0);
    return true;
}


bool DevVer(alt_u8 Addr)
{
    BYTE buf[7] = {0};

    // 
    memcpy(buf, VERSION, 6);
    
    SendPcCmd(Addr, CMD_VERSION, buf, 6);
    return true;
}



