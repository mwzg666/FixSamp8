#ifndef __MODBUS_H__
#define __MODBUS_H__


#define RJ45    0
#define RS485   1


#define DATA_BUF_LENGTH     256         // 一包最多256个字节


#define CMD_READ_REG                0x03     // 读取寄存器值
#define CMD_WRITE_REG               0x10     // 写寄存器值


#pragma pack(1)

// 主机读写命令：读数据时没有data字段
typedef struct 
{
    BYTE Address;         //地址域
    BYTE FunctionCode;    //功能代码
    WORD RegAddr;     // 地址为大端模式
    WORD RegCount;
    
    BYTE Data[DATA_BUF_LENGTH + 2];   // 有数据时，长度在第一个字节
}HOST_SEND_FRAME;

// 设备读应答
typedef struct 
{
    BYTE Address;        //地址域
    BYTE FunctionCode;   //功能代码

    // 读寄存器时没有下面两个字段
    BYTE DataLen;         // 后面数据的长度
    BYTE Data[DATA_BUF_LENGTH + 2];
}DEVICE_READ_ACK;

// 设备写应答
typedef struct 
{
    BYTE Address;         //地址域
    BYTE FunctionCode;   //功能代码
    WORD RegAddr;     // 地址为大端模式
    WORD RegCount;
    WORD Crc;
}DEVICE_WRITE_ACK;



extern HOST_SEND_FRAME xdata  HostSendFrame;   // 发送帧
extern DEVICE_READ_ACK xdata  ReadAckFrame;    // 读接收帧
extern DEVICE_WRITE_ACK xdata WriteAckFrame;  // 写接收帧



#pragma pack()

WORD CRC16Calc(BYTE *dataBuff, DWORD dataLen);
void FloatToBytes(float dat, BYTE *res);
float BytesToFloat(BYTE *dat);
DWORD BytesToDWord(BYTE *dat);
WORD BytesToWord(BYTE *dat);
DWORD SwEndian(DWORD input);
WORD SwEndianW(WORD input);
WORD RegSw(WORD dat);

BYTE ValidRtuFrame(BYTE *abyRdBuf,BYTE len);
BYTE ValidTcpFrame(BYTE *abyRdBuf,BYTE len);

#endif

