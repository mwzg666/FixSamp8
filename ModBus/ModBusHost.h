#ifndef __MODBUS_HOST_H__
#define __MODBUS_HOST_H__

extern WORD HostBufIndex;

BYTE HostSendCmd(BYTE Mode, BYTE Addr, BYTE Cmd, WORD Reg, WORD Count, BYTE * Data);
void SendBuffer(BYTE Mode, BYTE * Data, WORD Length);

WORD PopReg();


#endif /*PROTOCOL_H_*/
