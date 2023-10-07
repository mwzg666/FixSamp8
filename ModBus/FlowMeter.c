#include "main.h"
#include "FlowMeter.h"
#include "LcdApp.h"
#include "ModBusHost.h"

FLOW_VALUE xdata RealFlow[FLOW_METER_CNT];


// Count :  寄存器个数
void SendReadFlow(BYTE ch)
{
    WORD RegCnt = 2;
    HostSendCmd(RS485, ch, CMD_READ_REG, FLOW_VALUE_REG, RegCnt, NULL);
}


void ReadFlow(BYTE ch)
{
    WORD r1,r2;
    float val;
    HostBufIndex = 0;
    
    r1 = PopReg();
    r2 = PopReg(); 
    val = (float)r1 * 65536 + r2;
    RealFlow[ch-1].val = (float)(val / 1000);

    // 计算体积
    RealFlow[ch-1].Totol += RealFlow[ch-1].val/30/1000;

    #if 0
    r1 = PopReg();
    r2 = PopReg(); 
    r3 = PopReg();
    val = ((float)r1 * 65536 + r2) * 1000 + r3; 
    //RealFlow[ch-1].Totol = (float)(val / 1000);
    
    
    r1 = PopReg();
    r2 = PopReg(); 
    val = (float)r1 * 65536 + r2;
    RealFlow[ch-1].Alarm = (float)(val / 1000);
    #endif

    ChannelError[ch-1] = 0;
}


void FlowMeterAck(BYTE *Buf, BYTE Len)
{
    BYTE ch;
    if (!ValidRtuFrame(Buf, Len))
    {
        //DebugMsg("Comm err\r\n",10);
        return;
    }

    memset(&ReadAckFrame, 0, sizeof(DEVICE_READ_ACK));
    memcpy(&ReadAckFrame, Buf, Len);
   
    ch = ReadAckFrame.Address; 
    switch(ReadAckFrame.FunctionCode)
    {      
        case CMD_READ_REG: ReadFlow(ch);   break;
    }
}

