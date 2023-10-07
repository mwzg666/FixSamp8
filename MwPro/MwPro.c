#include "main.h"

extern u8  RX2_Buffer[]; //接收缓冲
extern u8  RX2_Cnt;





BYTE CheckSum(BYTE *buf, BYTE len)
{
    BYTE dwSum = 0;
    BYTE i;

    for (i = 0; i < len; i++)
    {
        dwSum += buf[i];
    }

    return dwSum;
}


BYTE BcdToHex(BYTE Bh, BYTE Bl)
{
    BYTE temp;
    if (Bh > 0x39)
    {
        temp = Bh - 0x37;
    }
    else
    {
        temp = Bh - 0x30;
    }

    temp <<= 4;
    temp &=0xF0;
    
    if (Bl > 0x39)
    {
        temp |= (Bl - 0x37)&0x0F;
    }
    else
    {
        temp |= (Bl - 0x30)&0x0F;
    }

    return temp;
}


#if 0
void SendCmd(BYTE Cmd, BYTE *buf, BYTE Length)
{
    FRAME_HEAD *head = (FRAME_HEAD*)SendBuf;
    BYTE Sum;
    BYTE CkLen = 0;
    BYTE Str[3] = {0};
    
    memset(SendBuf, 0, BUFF_LENGTH);
    
    head->Head   = HEAD;
    head->Len    = 8+Length;
    head->Type   = 0x93;    // 暂时未使用
    head->Addr   = SysParam.Address;    
    head->Cmd    = Cmd;

    // data
    if (Length > 0)
    {
        memcpy(&SendBuf[sizeof(FRAME_HEAD)], buf, Length);
    }

    // CS
    CkLen = Length+4;
    Sum = CheckSum(&SendBuf[1], CkLen);
    sprintf((char *)Str, "%02X", Sum);
    memcpy(&SendBuf[Length+5], Str, 2);

    SendBuf[Length+7] = TAIL;

    Rs485Send(SendBuf,head->Len);

    //CommIdleTime = 0;
}



BOOL ValidUartFrame()
{
    FRAME_HEAD *pFrameHead = (FRAME_HEAD *)RecvBuf;
    BYTE cs;
    BYTE rs;
    BYTE CkLen = 0;
    
    if ( (RecLength < (sizeof(FRAME_HEAD) + 3)) ||
      (RecLength >= BUFF_LENGTH) )   // 长度大于最大包长 或 小于 帧头长度
    {
        return FALSE;
    }
    
    if (pFrameHead->Head != HEAD)
    {
        return FALSE;
    }

    if (RecvBuf[RecLength-1] != TAIL)
    {
        return FALSE;
    }

    if (pFrameHead->Len != RecLength) // 协议数据长度 大于 实际长度
    {
        return FALSE;
    }

    CkLen = RecLength-4;
    cs = CheckSum(&RecvBuf[1], CkLen);
    rs = BcdToHex(RecvBuf[RecLength-3], RecvBuf[RecLength-2]);
    
    if (cs != rs)
    {
        #ifdef FOR_DEBUG
        // 把正确的校验码发回去，用于调试
        memset(RX1_Buffer,0,UART_BUF_LENGTH);
        sprintf(RX1_Buffer,"%02X",cs);
        Rs485Send(RX1_Buffer, (BYTE)strlen(RX1_Buffer));
        #endif
        
        return FALSE;
    }

    return TRUE;
}
#endif

