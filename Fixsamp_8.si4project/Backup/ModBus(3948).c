#include "main.h"
#include "ModBus.h"

//校验和计算
WORD CRC16Calc(BYTE *dataBuff, DWORD dataLen)
{
    DWORD CRCResult = 0xFFFF;
    DWORD i,j;
    
    for (i = 0; i < dataLen; i++)
    {   
        CRCResult = CRCResult ^ dataBuff[i];
        for (j = 0; j < 8; j++)
        {
            if ((CRCResult & 1) == 1)
            {
                CRCResult = (CRCResult >> 1) ^ 0xA001;
            }
            else
            {
                CRCResult >>= 1;
            }
        }
    }
    
    return (WORD)CRCResult;
}


/*转化字节序*/
void ByteConvert(BYTE *InData, BYTE *OutData)
{  
    OutData[3] = InData[0];
    OutData[2] = InData[1];
    OutData[1] = InData[2];
    OutData[0] = InData[3];  
}

// 把浮点数转化为大端字节数组
void FloatToBytes(float dat, BYTE *res)
{
    BYTE temp[4] = {0};
    memcpy(temp, &dat, 4);
    ByteConvert(temp, res);
}

// 把大端字节数组转化为浮点数
float BytesToFloat(BYTE *dat)
{
    float ret;
    BYTE temp[4] = {0};
    ByteConvert(dat, temp);
    memcpy(&ret, temp, 4);
    return ret;
}

// 把大端字节数组转化为32bit整数
DWORD BytesToDWord(BYTE *dat)
{
    DWORD ret,temp;
    memcpy(&temp, dat, 4);
    ret = SwEndian(temp);
    return ret;
}

// 把大端字节数组转化为16bit整数
WORD BytesToWord(BYTE *dat)
{
    WORD ret,temp;
    memcpy(&temp, dat, 2);
    ret = SwEndianW(temp);
    return ret;
}


/*********************************
函数名称:DWORD SwEndian(DWORD input)
函数功能:大小端整数互换
作者:姚明钢
修改记录:2018.5.4
**********************************/
DWORD SwEndian(DWORD input)
{ 
    DWORD temp=0;
    temp = ((input&0x000000FF)<<24) |
           ((input&0x0000FF00)<<8) |
           ((input&0x00FF0000)>>8) |
           ((input&0xFF000000)>>24);
    return temp;
}

WORD SwEndianW(WORD input)
{ 
    WORD temp=0;
    temp = ((input&0x00FF)<<8) |
           ((input&0xFF00)>>8);
    return temp;
}


WORD RegSw(WORD dat)
{
    //WORD h,l;
    //h = (dat >> 8) &0x00FF;
    //l = dat & 0x00FF;
    //return (l<<8) | h;
    return dat;
}


// 判断帧的合法性
BYTE ValidRtuFrame(BYTE *abyRdBuf,BYTE len)
{
    WORD crc;


    //memset(&ReadAckFrame, 0, sizeof(DEVICE_READ_ACK));
    //memcpy(&ReadAckFrame, abyRdBuf, len);
    // 判断CRC
    crc = CRC16Calc(abyRdBuf,len);
    if (crc != 0)
    {
        return false;
    }
    
    return true;
}



// 判断帧的合法性
BYTE ValidTcpFrame(BYTE *abyRdBuf,BYTE len)
{
    BYTE rlen = 0;

    //memcpy(MbId, abyRdBuf, 2);
    //memset(&RecvBuf, 0, sizeof(DEVICE_READ_ACK));
    //memcpy(&RecvBuf, &abyRdBuf[6], len);

    rlen = abyRdBuf[5];
    if (rlen != len - 6)
    {
        return false;
    }

   
    return true;
}



