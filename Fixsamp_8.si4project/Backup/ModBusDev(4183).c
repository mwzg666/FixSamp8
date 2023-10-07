#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "main.h"
#include "Lcd.h"
#include "LcdApp.h"
#include "ModBus.h"
#include "ModBusDev.h"
#include "uart.h"


MODBUS_PARAM xdata ModBusParam;
MODBUS_STATUS xdata ModBusStatus;
MODBUS_INFO xdata ModBusInfo;



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


DEVICE_READ_ACK xdata  DevReadAck;   
DEVICE_WRITE_ACK xdata DevWriteAck;

HOST_SEND_FRAME xdata RecvFrame;   


// 把浮点数转换为大端打包到发送数据区
void PackageFloatValue(WORD Offset, float val)
{
    BYTE temp[4] = {0};
    FloatToBytes(val,temp);
    memcpy(&DevReadAck.Data[Offset], temp, 4);  
}

void PackageDWordValue(WORD Offset, DWORD val)
{
    DWORD temp;
    temp = SwEndian(val);
    memcpy(&DevReadAck.Data[Offset], &temp, 4);  
}


void PackageWordValue(WORD Offset, WORD val)
{
    BYTE temp[2] = {0};
    temp[0] = (BYTE)(val >> 8);
    temp[1] = (BYTE)val;
    memcpy(&DevReadAck.Data[Offset], temp, 2);  
}

// 把寄存器值包装到发送缓存
bool PackageReg(WORD Reg, WORD Count)
{
    DWORD offset;
    BYTE *p;

    if (Count > 128)
    {
        return false;
    }

    if (Reg >= MODBUS_INFO_ADD)
    {
        offset = (Reg - MODBUS_INFO_ADD)*2;
        if (offset >= sizeof(MODBUS_INFO))
        {
            return false;
        }
        
        p = (BYTE *)&ModBusInfo;
        memcpy(DevReadAck.Data, &p[offset], Count*2);  
    }
    else if (Reg >= MODBUS_STATUS_ADD)
    {
        offset = (Reg - MODBUS_STATUS_ADD)*2;
        if (offset >= sizeof(MODBUS_STATUS))
        {
            return false;
        }
        
        p = (BYTE *)&ModBusStatus;
        memcpy(DevReadAck.Data, &p[offset], Count*2);  
    }
    else if (Reg >= MODBUS_PARAM_ADD)
    {
        offset = (Reg - MODBUS_PARAM_ADD)*2;
        if (offset >= sizeof(MODBUS_PARAM))
        {
            return false;
        }
        
        p = (BYTE *)&ModBusParam;
        memcpy(DevReadAck.Data, &p[offset], Count*2); 
    }
    else
    {
        return false;
    }

    return true;
}

BYTE ReadAck(BYTE Mode)
{
    WORD i = 0;
    WORD crc,SendLen;
    memset(&DevReadAck, 0, sizeof(DEVICE_READ_ACK));
    
    DevReadAck.Address = RecvFrame.Address; 
    DevReadAck.FunctionCode = RecvFrame.FunctionCode;
    
    SendLen = 2;

    DevReadAck.DataLen = RecvFrame.RegCount*2; 
    SendLen ++;
    
    PackageReg(RecvFrame.RegAddr, RecvFrame.RegCount);
    SendLen += DevReadAck.DataLen;
    
    // 计算CRC , 并添加到数据后面
    i = DevReadAck.DataLen;
    crc = CRC16Calc((BYTE *)&DevReadAck, SendLen);
    DevReadAck.Data[i]  = (BYTE)(crc);
    DevReadAck.Data[i+1] = (BYTE)(crc>>8);
    SendLen += 2; 
    

    if (Mode == RS485)
    {
        Uart4Send((BYTE *)&DevReadAck, (BYTE)SendLen);
    }
    return true;
}


// 发送写命令应答
void WriteAck(BYTE Mode)
{
    WORD crc;
    memset(&DevWriteAck, 0, sizeof(DEVICE_WRITE_ACK));
    
    DevWriteAck.Address = RecvFrame.Address;  //Param.DevAddr;
    DevWriteAck.FunctionCode = RecvFrame.FunctionCode;
    DevWriteAck.RegAddr = RegSw(RecvFrame.RegAddr);
    DevWriteAck.RegCount = RegSw(RecvFrame.RegCount);

    crc = CRC16Calc((BYTE *)&DevWriteAck, 6);
    DevWriteAck.Crc = crc;

    if (Mode == RS485)
    {
        Uart4Send((BYTE *)&DevWriteAck, sizeof(DEVICE_WRITE_ACK));
    }
}


void ModBusSaveParam()
{
    SysParam.AlarmThres = ModBusParam.AlamrThres;
    SysParam.SampVol = ModBusParam.SampFlow;
    SysParam.SampMode = ModBusParam.SampMode;
    SysParam.SampTime = ModBusParam.SampTime;
    SysParam.SampVol = ModBusParam.SampVol;
    WriteParam();
}

// 把接收到的数据加载到寄存器中
bool WriteRegValue(WORD Reg, WORD Count)
{
    BYTE *p;
    int len,offset;
    
    // 写设备地址
    if ((Reg == MODBUS_INFO_ADD) && (Count == 1))
    {
        SysParam.Address = RecvFrame.Data[2];
        WriteParam();
        return true;
    }

    // 远程控制
    if ((Reg == MODBUS_REM_CTL) && (Count == 1))
    {
        SysParam.SampMode = RecvFrame.Data[1];
        if (RecvFrame.Data[2] == 1)
        {
            StartSamp();
        }
        else
        {
            StopSamp(false);
        }
        return true;
    }

    
    if (Reg >= MODBUS_PARAM_ADD) 
    {
        len = sizeof(MODBUS_PARAM);
        offset = (Reg - MODBUS_PARAM_ADD) * 2;
        if ( (offset + Count * 2) > len )
        {
            return false;
        }
        p = (BYTE *)&ModBusParam;
        memcpy(&p[offset], &RecvFrame.Data[1], Count*2);
        ModBusSaveParam();
        return true;
    }
    
    return false;
}


// 设寄存器值
void SetRegValue(BYTE Mode)
{
    if (RecvFrame.Data[0] == 0)
    {
        return;
    }

    if (WriteRegValue(RecvFrame.RegAddr, RecvFrame.RegCount))
    {
        WriteAck(Mode);
    }
}


void HndModBusRecv(BYTE Mode, BYTE *buf, BYTE len)
{
    if (!ValidRtuFrame(buf, len))
    {
        return;
    }

    memset(&RecvFrame, 0, sizeof(HOST_SEND_FRAME));
    memcpy(&RecvFrame, buf, len);
    if (RecvFrame.Address != SysParam.Address)
    {
        return;
    }

    switch(RecvFrame.FunctionCode)
    {
        case CMD_READ_REG: ReadAck(Mode);  break;
        case CMD_WRITE_REG: SetRegValue(Mode);  break;
    }
}



