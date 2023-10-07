#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "main.h"
#include "Lcd.h"
#include "LcdApp.h"
#include "ModBus.h"
#include "ModBusHost.h"
#include "uart.h"


/*
ModBus 帧格式
1. 发送帧
地址           命令     寄存器    寄存器数量    数据                                             CRC   
0A(固定值)     Cmd(1)   RX(2)     n(2)          无数据表示读取、有数据表示写对应的寄存器 

数据定义:  长度 + 数据
           n*2    dat(n*2)

2. 应答帧 -- 返回数据
地址           命令   数据长度    数据      CRC   
0A(固定值)     Cmd    n(1)        dat(n)

3. 应答帧 -- 返回状态
地址           命令   寄存器   寄存器数量     CRC   
0A(固定值)     Cmd    Rx(2)    n(2)                       
*/

HOST_SEND_FRAME xdata  HostSendFrame;   // 发送帧
DEVICE_READ_ACK xdata  ReadAckFrame;    // 读接收帧
DEVICE_WRITE_ACK xdata WriteAckFrame;  // 写接收帧


BYTE MbId[2] = {0};


WORD HostBufIndex = 0;


WORD PopReg()
{
    WORD reg = 0;
    memcpy(&reg, &ReadAckFrame.Data[HostBufIndex], 2);
    HostBufIndex +=2;
    reg =  RegSw(reg);
    return reg;
}


BYTE HostSendCmd(BYTE Mode, BYTE Addr, BYTE Cmd, WORD Reg, WORD Count, BYTE * Data)
{
    WORD i = 0;
    WORD crc, SendLen;
    //Error();
    memset(&HostSendFrame, 0, sizeof(HOST_SEND_FRAME));
    
    HostSendFrame.Address = Addr; //Param.DevAddr;
    HostSendFrame.FunctionCode = Cmd;
    HostSendFrame.RegAddr = RegSw(Reg);
    HostSendFrame.RegCount =  RegSw(Count);

    SendLen = 6;

    if (((Count*2) > 0) && (Data != NULL))
    {
        HostSendFrame.Data[0] = Count*2;  // 数据长度
        SendLen ++;
        i++;
        memcpy(&HostSendFrame.Data[1], Data, Count*2);
        SendLen += Count*2;
        i+=2;
    }
    
    // 计算CRC , 并添加到数据后面
    crc = CRC16Calc((BYTE *)&HostSendFrame, SendLen);
    HostSendFrame.Data[i]  = (BYTE)(crc);
    HostSendFrame.Data[i+1] = (BYTE)(crc>>8);
    
    SendLen += 2; 
    SendBuffer(Mode, (BYTE *)&HostSendFrame, SendLen);

    return true;
}



// 由于原来是安装MODBUS_RTU写的，发送MODBUS_TCP时特殊处理一下
void SendTcpFrame(BYTE *Data, WORD Length)
{
    BYTE Head[6] = {0};
    WORD sLen = Length - 2;  // MODBUS_TCP 没有CRC
    memcpy(Head,MbId, 2);
    Head[5] = (BYTE)sLen;
    
    //UnusedVar(Data);
    //Uart3Send(Head,6);
    //Uart3Send(Data, (BYTE)sLen);
}

void SendBuffer(BYTE Mode, BYTE * Data, WORD Length)
{
    switch(Mode)
    {
        case RJ45  : SendTcpFrame(Data, Length);        break;
        case RS485 : Uart3Send(Data, (BYTE)Length);     break;
    }
}

