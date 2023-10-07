//#include "pch.h"

#include "main.h"
#include "Lcd.h"
#include "LcdApp.h"
#include "Temper.h"

//#include "SerialPort.h"
//#include "SampDemo.h"
//#include "SampDemoDlg.h"


#define UART_BUFF_LENGTH   MAX_LENGTH

//#define Uart1Send ((CSampDemoDlg *)theApp.m_pMainWnd)->m_pSerialPort->WriteToPort


BYTE LcdFrameloc = 0;
BYTE RecLength1;
BYTE xdata SendBuf1[UART_BUFF_LENGTH] = {0};
BYTE xdata RecvBuf1[UART_BUFF_LENGTH] = {0};
BYTE DataIndex = 0;


WORD SwWord(WORD input)
{ 
    //WORD temp=0;
    //temp = ((input&0x00FF)<<8) |
           //((input&0xFF00)>>8);
    //return temp;

    #ifdef BIG_EDTION
    return input;
    #else
    WORD temp=0;
    temp = ((input&0x00FF)<<8) |
           ((input&0xFF00)>>8);
    return temp;
    #endif
}


DWORD SwDWord(DWORD input)
{ 
    //DWORD temp=0;
    //temp = ((input&0x000000FF)<<24) |
           //((input&0x0000FF00)<<8) |
           //((input&0x00FF0000)>>8) |
           //((input&0xFF000000)>>24);
    //return temp;

    #ifdef BIG_EDTION
    return input;
    #else
    DWORD temp=0;
    temp = ((input&0x000000FF)<<24) |
           ((input&0x0000FF00)<<8) |
           ((input&0x00FF0000)>>8) |
           ((input&0xFF000000)>>24);
    return temp;
    #endif
}



//校验和计算
/*
WORD CRC16Calc(BYTE *dataBuff, WORD dataLen)
{
    DWORD CRCResult = 0xFFFF;
    WORD i,j;
    
    for (i = 0; i < dataLen; i++)
    {
        CRCResult = CRCResult ^ dataBuff[i];
        for (j = 0; j < 8; j++)
        {
            if ((CRCResult & 1) == 1)
            {
                CRCResult = (CRCResult >> 1) ^ 0xA001;   // 0x8005 
            }
            else
            {
                CRCResult >>= 1;
            }
        }
    }
    
    return (WORD)CRCResult;
}
*/


BOOL LcdCmd(BYTE Cmd, WORD Addr, BYTE *Data, BYTE Length)
{
    LCD_FRAME *head;
    //WORD Sum;
    
    memset(SendBuf1, 0, UART_BUFF_LENGTH);
    head = (LCD_FRAME*)SendBuf1;

    head->Head   = SwWord(LCD_HEAD);
    head->Lenght = 3+Length; // + 命令1 + 校验2  （字节）
    head->Cmd    = Cmd;
    head->Addr   = SwWord(Addr);
    
    // data
    if (Length > 0)
    {
        memcpy(&SendBuf1[6], Data, Length);
    }

    #ifdef CRC_ENABLE
    // CRC ： 命令和数据
    Sum = SwWord(CRC16Calc(&SendBuf1[3], Length+5));
    memcpy(&SendBuf1[Length+6], (BYTE *)&Sum, 2);
    #endif

    DebugMsg(".");
    Uart2Send(SendBuf1,(u8)(Length+6));
    
    //Delay(200);
    return TRUE;
}



BOOL ValidLcdFrame()
{
    // 有可能两个包黏在一起了
    WORD rs = 0;
    //WORD cs;
    LCD_FRAME *pFrame;

    if ( (RecLength1 < 6) ||
          (RecLength1 >= MAX_LENGTH) )   // 长度大于最大包长 或 小于 帧头长度
    {
        return FALSE;
    }

    if (LcdFrameloc >= RecLength1)
    {
        LcdFrameloc = 0;
        return FALSE;
    }

    pFrame = (LCD_FRAME *)&RecvBuf1[LcdFrameloc];
    if (pFrame->Head != SwWord(LCD_HEAD))
    {
        LcdFrameloc = 0;
        return FALSE;
    }

    #ifdef CRC_ENABLE
    cs = CRC16Calc(&RecvBuf1[3], pFrame->Lenght-2);
    memcpy((BYTE *)&rs, &RecvBuf1[RecLength1-2], 2);
    
    if (cs != rs)
    {
        return FALSE;
    }
    #endif

    return TRUE;
}


void ClearRevBuf()
{
    memset(RecvBuf1, 0, UART_BUFF_LENGTH);
    LcdFrameloc = 0;
    RecLength1 = 0;
}


void HndLcdFrame()
{
    LCD_FRAME *pFrame;
    pFrame = (LCD_FRAME *)&RecvBuf1[LcdFrameloc];
    switch(pFrame->Cmd)
    {
        case LCD_CMD_READ:  ReadReg();    break;
        case LCD_CMD_WRITE: WriteReg();  break;
    }

    LcdFrameloc += (pFrame->Lenght + 3);
}

void HndLcdData()
{
    while (ValidLcdFrame())
    {
        DataIndex = 1;
        HndLcdFrame();
    }

    ClearRevBuf();
}


// 检查数据长度
bool CheckDataLen(BYTE Len)
{
    LCD_FRAME *pFrame;
    pFrame = (LCD_FRAME *)&RecvBuf1[LcdFrameloc];
    if (Len != pFrame->Data[0])
    {
        return false;
    }

    return true;
}


// 从数据中取一个Word
WORD PopWord()
{
    WORD ret = 0;
    LCD_FRAME *pFrame;
    pFrame = (LCD_FRAME *)&RecvBuf1[LcdFrameloc];
    
    memcpy(&ret, &pFrame->Data[DataIndex], 2);
    ret = SwWord(ret);
    DataIndex += 2;
    return ret;
}


// 切换页面
void EnterPage(BYTE Page)
{
    BYTE dt[4] = {0};
    memcpy(dt, "\x5A\x01\x00\x00" , 4);
    dt[3] = Page;
    LcdCmd(LCD_CMD_WRITE, LCD_REG_PAGE, dt, 4);
}



void GetLcdTime()
{
    BYTE len = 4;
    NeedGetRet  = false;

    DebugMsg("GetTime\r\n");
    LcdCmd(LCD_CMD_READ, LCD_REG_RTC, &len, 1);
    
    //Delay(200);
    //HideModule(MP_HINT_END);
    //Delay(200); 
}


void SetLcdTime()
{
    LCD_DATE_TIME  ldt;
    BYTE dt[8] = {0};
    
    if (!CheckDataLen(7))
    {
        return;
    }

    ldt.YearH  = PopWord();
    ldt.YearL  = PopWord();
    ldt.Month  = PopWord();
    ldt.Day    = PopWord();
    ldt.Hour   = PopWord();
    ldt.Minute = PopWord();
    ldt.Secend = PopWord();

    dt[0] = 0x5A;
    dt[1] = 0xA5;
    dt[2] = (BYTE)(ldt.YearH*10+ldt.YearL);
    dt[3] = (BYTE)ldt.Month;
    dt[4] = (BYTE)ldt.Day;

    dt[5] = (BYTE)ldt.Hour;
    dt[6] = (BYTE)ldt.Minute;
    dt[7] = (BYTE)ldt.Secend;

    LcdCmd(LCD_CMD_WRITE, LCD_SET_RTC, dt, 8);
    NeedGetRet = true;
}

void SetBkLight(bool s)
{
    BYTE dat[2] = {0};

    if (s)  //通过参数设置界面修改背光
    {
        SysParam.BkLight = (BYTE)PopWord();
    }
    dat[0] = SysParam.BkLight;
    dat[1] = SysParam.BkLight / 2 ;  // 如果开启自动亮度：超时后亮度减半
    LcdCmd(LCD_CMD_WRITE, LCD_REG_LIGHT, (BYTE *)dat, 2);
}


// 修改文本颜色
void SetTextColor(WORD mp, WORD color)
{
    LcdCmd(LCD_CMD_WRITE, mp+3, (BYTE *)&color, 2);
}


// 隐藏模组
void HideModule(WORD mp)
{
    BYTE dat[2] = {0xFF, 00};
   
    LcdCmd(LCD_CMD_WRITE, mp, (BYTE *)dat, 2);
}

// 显示模组
void ShowModule(WORD mp, WORD module)
{
    WORD dat = SwWord(module);
    LcdCmd(LCD_CMD_WRITE, mp, (BYTE *)&dat, 2);
}


