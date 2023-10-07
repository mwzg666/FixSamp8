#ifndef __LCD_H__
#define __LCD_H__

//#define CRC_ENABLE  

#define BIG_EDTION

#define LCD_HEAD   0x5AA5


#define LCD_CMD_WRITE  0x82
#define LCD_CMD_READ   0x83


// 串口屏寄存器
#define LCD_REG_RTC    0x0010
#define LCD_SET_RTC    0x009C
#define LCD_REG_PAGE   0x0084    // 页面切换
#define LCD_REG_LIGHT 0x0082     // 背光亮度


extern BYTE LcdFrameloc;
extern BYTE RecLength1;
extern BYTE xdata  SendBuf1[];
extern BYTE xdata RecvBuf1[];
extern BYTE Adc_Flag;

#pragma pack(1)

// 协议头结构
typedef struct
{
    WORD Head;       // 1 0x02 帧头
    BYTE Lenght;     // 2 帧长 整个帧的长度
    BYTE Cmd;        // 3 命令
    WORD Addr;
    BYTE Data[251];  // 数据最大249 + CRC
}LCD_FRAME;

#pragma pack()


WORD SwWord(WORD input);
DWORD SwDWord(DWORD input);

void ClearRevBuf();
BOOL LcdCmd(BYTE Cmd, WORD Addr, BYTE *Data, BYTE Length);
BOOL ValidLcdFrame();
void HndLcdFrame();
void HndLcdData();

bool CheckDataLen(BYTE Len);
WORD PopWord();

void EnterPage(BYTE Page);
void GetLcdTime();
void SetLcdTime();
void SetBkLight(bool s);
void SetTextColor(WORD mp, WORD color);
void HideModule(WORD mp);
void ShowModule(WORD mp, WORD module);

#endif
