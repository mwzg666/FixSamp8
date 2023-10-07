#ifndef __IRDA_H__
#define __IRDA_H__

#ifdef IRDA_FUN

void DelayUs(WORD t);

void IrDAConfig();
void IrDASend(u8 *buf, u8 len);
void ClearIrDABuf();
void IrDASendByte(BYTE dat);
void IrDARec();
void IrDA_Init();
void StopTimer3();

#endif

#endif
