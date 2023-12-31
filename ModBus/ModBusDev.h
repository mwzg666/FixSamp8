#ifndef __MODBUS_DEV_H__
#define __MODBUS_DEV_H__

#define MODBUS_PARAM_ADD  0xA800
#define MODBUS_STATUS_ADD 0xA900
#define MODBUS_INFO_ADD  0xFFFD
#define MODBUS_REM_CTL  0xFFF0

#pragma pack(1)
typedef struct
{
    WORD Addr;
    //WORD SampTime;
    //WORD SampVol;
    //WORD SampFlow;
    //WORD SampMode;
    WORD ChEnable;
    //WORD AlamrThres;
    //WORD ChFlow[8];
    //WORD ChValve[8];
    WORD RemCtlFlag;
}MODBUS_PARAM;

typedef struct
{
      WORD Addr;
//    DWORD TotleTime; 
//    float TotleVol;
//    float SampleFlow;  
//    DWORD RemTime;
//    float SampleVol;
//    DWORD RunTime;
    WORD  RunStatus;
    WORD  Alarm[8];
//    float ChFlow[8];
//    float ChVol[8];
}MODBUS_STATUS;

typedef struct
{
    WORD  Addr;
    WORD  Address;
    WORD  Version;
}MODBUS_INFO;


#pragma pack()


extern MODBUS_PARAM xdata ModBusParam;
extern MODBUS_STATUS xdata ModBusStatus;
extern MODBUS_INFO xdata ModBusInfo;

BYTE ReadAck();
bool WriteRegValue(WORD Reg, WORD Count);

void HndModBusRecv(BYTE Mode, BYTE *buf, BYTE len);

#endif /*PROTOCOL_H_*/

