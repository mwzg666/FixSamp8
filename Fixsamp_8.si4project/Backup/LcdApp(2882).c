//#include "pch.h"

#include "main.h"
#include "Lcd.h"
#include "LcdApp.h"
#include "Temper.h"


//#include "SerialPort.h"
//#include "SampDemo.h"
//#include "SampDemoDlg.h"

#define Log //((CSampDemoDlg *)theApp.m_pMainWnd)->AddLog

#define TEMP_BUF_LEN  128
BYTE xdata TempBuf[TEMP_BUF_LEN] = {0};

SHOW_PARAM xdata ShowParam;
RUN_INFO xdata RunInfo;
DEV_INFO xdata DevInfo;

bool NeedGetRet = false;

char xdata ChannelStatus[CHANNLE_NUM][16] = {0};   // 状态文字
BYTE ChannelAlarm[CHANNLE_NUM] = {1,1,1,1,1,1,1,1};   // 默认状态正常
BYTE HisAlarm[CHANNLE_NUM] = {1,1,1,1,1,1,1,1};

extern BYTE code VERSION;

WORD code StatusPoint[CHANNLE_NUM] = 
{
    0xA000,0xA100,0xA200,0xA300,0xA400,0xA500,0xA600,0xA700
};

WORD code TxtColor[5] = 
{
    //禁用    正常    流量低     流量高     异常
    //白色    黄色     红色     红色      红色
    0xFFFF,0xFFE0,0xF800,0xF800,0xF800
};

char code ModeText[4][16]=
{   
    "",
    "定时取样",
    "定量取样",
    "手动取样"
};





void ShowDevInfo()
{
    BYTE i,Cnt = 0;;
    for (i=0;i<8;i++)
    {
        if (SysParam.Enable & (1<<i))
        {
            Cnt ++;
        }
    }
    memset(&DevInfo, 0, sizeof(DEV_INFO));
    sprintf(DevInfo.DevType,"MNNA-1/%d",Cnt);

    sprintf(DevInfo.SoftVer,"V%d.%d.%d", VERSION/100, VERSION%100/10, VERSION%10 );
    LcdCmd(LCD_CMD_WRITE, REG_DEV_TYPE, (BYTE *)&DevInfo, 32);
}

// 启动界面开始按钮
void StartButton()
{
    WORD dat;
    if (!CheckDataLen(1))
    {
        return;
    }

    dat = PopWord();
    if (dat == BUTTON_OK)
    {
        
        StartSamp();
        NeedGetRet = true;
    }    
}


// 修改主界面的启停按钮 1：开始  0：停止
void SetStartBtn(BYTE s)
{
    TempBuf[0] = 0;
    TempBuf[1] = s;
    LcdCmd(LCD_CMD_WRITE, REG_DEV_CTL, TempBuf, 2);
    
}

// 登录密码
void VerifyPassWord()
{
    WORD PassWord;
    if (!CheckDataLen(1))
    {
        return;
    }

    PassWord = PopWord();
    if (PassWord == LOGIN_PASSWORD)
    {
        EnterPage(PAGE_SET);
        //Log(_T("登录成功\r\n"));
    }    
}

void ReturnStart()
{
    WORD dat;
    if (!CheckDataLen(1))
    {
        return;
    }

    dat = PopWord();
    if (dat == 0x31)    // 返回确认
    {
        //TemSendFlag = false;
        NeedGetRet = false;
        
        StopSamp(false);
        Delay(200);
        // 要把变量重新写为0
        memset(TempBuf,0,TEMP_BUF_LEN);
        LcdCmd(LCD_CMD_WRITE, REG_RETRUN, TempBuf, 2);
        Delay(200);
        EnterPage(PAGE_START);
        Delay(200);
        
    }   
    else if (dat == 0x0D0A)
    {
        if (RunStatus.Running)
        {
            StopSamp(false);
        }
        else
        {
            StartSamp();
        }

        // 要把变量重新写为0
        Delay(200);
        memset(TempBuf,0,TEMP_BUF_LEN);
        LcdCmd(LCD_CMD_WRITE, REG_RETRUN, TempBuf, 2);
        
    }
     
}

void GetRetCode()
{
    BYTE len = 1;

    if (NeedGetRet)
    {
        LcdCmd(LCD_CMD_READ, REG_RETRUN, &len, 1);
    }
}


void SampCtl()
{
    WORD dat;
    if (!CheckDataLen(1))
    {
        return;
    }

    dat = PopWord();
    if (dat == 0)
    {
        StartSamp();
       
    }  
    else
    {
        StopSamp(false);
        
    }

    HideModule(MP_HINT_END);
   
}


// 主界面取样模式提示
void ModeHint()
{
    memset(TempBuf,0,TEMP_BUF_LEN);
    switch (SysParam.SampMode)
    {
        case MODE_TIME: sprintf((char *)TempBuf, "定时取样:%d分钟" ,    SysParam.SampTime); break;
        case MODE_VOL : sprintf((char *)TempBuf, "定量取样:%.1f立方米" , SysParam.SampVol);  break;
        case MODE_MAN : sprintf((char *)TempBuf, "手动取样"); break;
    }
    
    LcdCmd(LCD_CMD_WRITE, REG_MODEHINT, TempBuf, 32);
    //Delay(200);
    
}


// 调整状态文字的颜色
void StatusColor(bool force)
{
    BYTE i;
    for (i=0;i<CHANNLE_NUM;i++)
    {
        if ((ChannelAlarm[i] != HisAlarm[i]) || force)
        {
            Delay(200);
            SetTextColor(StatusPoint[i], SwWord(TxtColor[ChannelAlarm[i]]));
        }
    }
}

void SetRunTime(DWORD tm)
{
    WORD h,m,s;
    h = (WORD)(tm/3600);
    m = (tm%3600)/60;
    s = tm%60;
    memset(RunInfo.RunTime,0,16);
    sprintf((char *)RunInfo.RunTime, "%04d:%02d:%02d",h,m,s);
    //WriteParam();
}

void ShowFlow()
{
    SetRunTime(RunStatus.RunTime);  // 运行时长
    LcdCmd(LCD_CMD_WRITE, REG_CH_FLOW, (BYTE *)&RunInfo, sizeof(RUN_INFO));
}

void ShowStatus()
{
    BYTE i;
    memset(ChannelStatus,0,sizeof(ChannelStatus));
    for (i=0;i<CHANNLE_NUM;i++)
    {
        switch (ChannelAlarm[i])
        {
            case ALM_CH_DISABLE:  sprintf(&ChannelStatus[i][0],"禁用");               break;
            case ALM_FLOW_NOR:    sprintf(&ChannelStatus[i][0],"正常");               break;
            case ALM_FLOW_LOW:    sprintf(&ChannelStatus[i][0],"流量偏小");             break;
            case ALM_FLOW_HIGH:   sprintf(&ChannelStatus[i][0],"流量偏大");             break;
            case ALM_FLOW_ABNOR:  sprintf(&ChannelStatus[i][0],"通信异常");             break;
        }
    }

    LcdCmd(LCD_CMD_WRITE, REG_CH_STATUS, (BYTE *)ChannelStatus, 128);
}

void ShowTemp(u16 TEMPER)
{
    WORD i = TEMPER;
    LcdCmd(LCD_CMD_WRITE, REG_IO_TEMP, (BYTE *)&i,2);
}

void ShowEmStop(bool on)
{
    WORD i = on;
    LcdCmd(LCD_CMD_WRITE, REG_IO_EMSTOP, (BYTE *)&i,2);
}



void SendParam()
{
    BYTE i;
    for (i=0;i<CHANNLE_NUM;i++)
    {
        ShowParam.Flow[i] = SwWord((WORD)(SysParam.SampFlow[i]*10));
    }
    ShowParam.Address = SwWord((WORD)SysParam.Address);
    ShowParam.SampTime = SwWord((WORD)SysParam.SampTime);
    ShowParam.SampVol = SwWord((WORD)(SysParam.SampVol*10));
    ShowParam.BkLight = SwWord((WORD)SysParam.BkLight);
    ShowParam.AlarmThres = SwWord((WORD)SysParam.AlarmThres);
    ShowParam.SampMode = SwWord((WORD)SysParam.SampMode);
    ShowParam.ChOnOff = SwWord((WORD)SysParam.Enable);

    memset(ShowParam.ModeTxt,0,16);
    sprintf(ShowParam.ModeTxt, ModeText[SysParam.SampMode]);
    
    LcdCmd(LCD_CMD_WRITE, REG_SYS_PARAM, (BYTE *)&ShowParam, sizeof(SHOW_PARAM));
}


// 设置界面显示取样模式
void SetSampMode()
{
    SysParam.SampMode = (BYTE)PopWord();
    memset((char *)TempBuf,0,TEMP_BUF_LEN);
    sprintf((char *)TempBuf, ModeText[SysParam.SampMode]);
    LcdCmd(LCD_CMD_WRITE, REG_MODE_TXT, TempBuf, 16);
}


// 获取修改后的时间
void GetInputTime()
{
    BYTE len = 7;
    NeedGetRet = true;
    //Delay(300);
    LcdCmd(LCD_CMD_READ, REG_DATE_TIME, &len, 1);
}

// 在时间设置界面写入当前时间
void SetCurTime()
{
    LCD_FRAME *pFrame;
    LCD_DATE_TIME  ldt;
    
    BYTE dt[8] = {0};
    pFrame = (LCD_FRAME *)&RecvBuf1[LcdFrameloc];
    if (!CheckDataLen(4))
    {
        return;
    }
    
    memcpy(dt, &pFrame->Data[1], 8);
    
    ldt.YearH = SwWord((WORD)(dt[0]/10));
    ldt.YearL = SwWord((WORD)(dt[0]%10));
    ldt.Month = SwWord((WORD)dt[1]);
    ldt.Day   = SwWord((WORD)dt[2]);

    ldt.Hour   = SwWord((WORD)dt[4]);
    ldt.Minute = SwWord((WORD)dt[5]);
    ldt.Secend = SwWord((WORD)dt[6]);

    LcdCmd(LCD_CMD_WRITE, REG_DATE_TIME, (BYTE *)&ldt, sizeof(LCD_DATE_TIME));
    
}


void SetChCount()
{
    BYTE dat = (BYTE)PopWord();
    BYTE i;

    SysParam.Enable = 0;
    for (i=0;i<dat;i++)
    {
        SysParam.Enable |= (1<<i);
    }

    ShowDevInfo();
}



void ReadReg()
{
    LCD_FRAME *pFrame = (LCD_FRAME *)&RecvBuf1[LcdFrameloc];
    WORD addr = SwWord(pFrame->Addr);
    
    DebugInt(addr);
    
    switch (addr)
    {
        // 启动界面
        case REG_STRART:   StartButton();        break;  // 点击开始
        case REG_PASSWORD: VerifyPassWord();     break;    // 密码

        // 主界面
        case REG_DEV_CTL:  SampCtl();         break;  // 启停控制  
        case REG_RETRUN:   ReturnStart();     break;  // 返回启动页面
        case REG_TIME_BTN: GetLcdTime();      break;  // 点击时间
        case REG_HINT_ENTER: HideModule(MP_HINT_END);    break;

        // 参数设置界面
        case REG_SP_FLOW1: SysParam.SampFlow[0] = ((float)PopWord())/10;   break; //0x4000
        case REG_SP_FLOW2: SysParam.SampFlow[1] = ((float)PopWord())/10;break; //   0x4001
        case REG_SP_FLOW3: SysParam.SampFlow[2] = ((float)PopWord())/10;break; //   0x4002
        case REG_SP_FLOW4: SysParam.SampFlow[3] = ((float)PopWord())/10;break; //   0x4003
        case REG_SP_FLOW5: SysParam.SampFlow[4] = ((float)PopWord())/10;break; //   0x4004
        case REG_SP_FLOW6: SysParam.SampFlow[5] = ((float)PopWord())/10;break; //   0x4005
        case REG_SP_FLOW7: SysParam.SampFlow[6] = ((float)PopWord())/10;break; //   0x4006
        case REG_SP_FLOW8: SysParam.SampFlow[7] = ((float)PopWord())/10;break; //   0x4007
        case REG_SP_ADDR:  SysParam.Address = (BYTE)PopWord();             break; //   0x4008
        case REG_SP_TIME:  SysParam.SampTime = PopWord();                  break; //   0x4009
        case REG_SP_VOL:   SysParam.SampVol = ((float)PopWord())/10;       break; //   0x400A
        case REG_SP_LIGHT: SetBkLight(true);                               break; //   0x400B
        case REG_SP_THRES: SysParam.AlarmThres = (BYTE)PopWord();          break; //   0x400C
        case REG_SP_MODE:   SetSampMode();                                 break; //   0x400D
        case REG_CH_ONOFF: SysParam.Enable = (BYTE)PopWord();              break;
        case REG_SP_RET:    ModeHint(); UpdataUI(); SaveParam();           break;   // 点击参数界面返回按钮

        // 修改时间
        case REG_ADJ_TIME:   GetInputTime();                               break;    // 时间修改完成，点击了返回按钮
        case REG_DATE_TIME:  SetLcdTime();                                 break;      // 修改屏幕RTC      
        case LCD_REG_RTC:    SetCurTime();                                 break;      // 获取屏幕时间返回

        // IO 测试
        case REG_IO_BUMP:   g_Output[GAS_BUMP]  = (BYTE)PopWord();   ShowTemp(TEMPER_Val);      break; // 0x5000
        case REG_IO_BLUE:   g_Output[LIGHT_BLUE]  = (BYTE)PopWord(); ShowTemp(TEMPER_Val);      break; // 0x5001
        case REG_IO_YELLOW: g_Output[LIGHT_YELLOW]  = (BYTE)PopWord();ShowTemp(TEMPER_Val);     break; // 0x5002
        case REG_IO_SOUND:  g_Output[ALARM_SOUND]  = (BYTE)PopWord(); ShowTemp(TEMPER_Val);     break; // 0x5004
        case REG_IO_FAN:    ((BYTE)PopWord())?FANS_M(1):FANS_M(0);    ShowTemp(TEMPER_Val);     break; // 0x5005
        // 关于界面
        // case REG_CH_COUNT:  SetChCount();  break;  // 取消了，改到参数设置界面了
        
    }

}


void WriteReg()
{
    #if 0
    LCD_FRAME *pFrame = (LCD_FRAME *)&RecvBuf1[LcdFrameloc];
    CString str;
    str.Format(_T("Write:0x%04X\r\n"), SwWord(pFrame->Addr));
    Log(str);
    #endif
}

