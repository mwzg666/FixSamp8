#ifndef __FLOWMETER_H__
#define __FLOWMETER_H__


#define FLOW_METER_ADDR   0x01       // 流量计默认地址是255
#define FLOW_VALUE_REG    0x003A




typedef struct
{
    float val;    // L/min
    float Totol;   // 立方米
    float Alarm;
}FLOW_VALUE; 


extern FLOW_VALUE xdata RealFlow[FLOW_METER_CNT];


void SendReadFlow(BYTE ch);
void ReadFlow(BYTE ch);
void FlowMeterAck(BYTE *Buf, BYTE Len);

#endif

