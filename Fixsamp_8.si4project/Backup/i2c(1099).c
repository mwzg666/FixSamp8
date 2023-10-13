#include "main.h"
#include "i2c.h"

BYTE P2DIR = 0;

void delay_us(BYTE us)
{
    while(us--)
    {
        ;
    }
}

#if 0
/*******************************************************************************
函数功能:初始化IIC的IO口，设置方向为输出
输入:无
输出:无
*******************************************************************************/
void I2C_Init(void)
{   
    I2C_DIR |= (I2C_SDA_MASK+I2C_SCL_MASK);
    I2C_OUT &= ~(I2C_SDA_MASK+I2C_SCL_MASK);
}


/*******************************************************************************
函数功能:产生IIC的开始信号
输入:无
输出:无
*******************************************************************************/
void I2C_Start()
{
	IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);
	IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);
	delay_us(I2C_DELAY);
	IOWrite(I2C_OUT,I2C_SDA_MASK,0);
	delay_us(I2C_DELAY);
	IOWrite(I2C_OUT,I2C_SCL_MASK,0);
	delay_us(I2C_DELAY);
}

/*******************************************************************************
函数功能:产生IIC的结束信号
输入:无
输出:无
*******************************************************************************/
void I2C_Stop()
{
	//IOWrite(I2C_OUT,I2C_SCL_MASK,0);
	IOWrite(I2C_OUT,I2C_SDA_MASK,0);	
	delay_us(I2C_DELAY);
	IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);
	delay_us(I2C_DELAY);
	IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);
	delay_us(I2C_DELAY);
}

void I2C_Ack()
{
    IOWrite(I2C_OUT,I2C_SDA_MASK,0);	
    delay_us(I2C_DELAY);
	IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);
    delay_us(I2C_DELAY);
	IOWrite(I2C_OUT,I2C_SCL_MASK,0);
    IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);
}

void I2C_NAck()
{
    IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);	
	IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);
    delay_us(I2C_DELAY);
	IOWrite(I2C_OUT,I2C_SCL_MASK,0);
    IOWrite(I2C_OUT,I2C_SDA_MASK,0);
}

BOOL I2C_ReadAck()
{
    WORD i = 0;
    BYTE d = 0;
    BOOL r = FALSE;

    IODireIn(I2C_DIR, I2C_SDA_MASK);
	IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);
    
    delay_us(I2C_DELAY);
    while(++i<10000)
    {
        d = IORead(I2C_IN);
        if ((d & I2C_SDA_MASK) == 0)
        {
            r = TRUE;
			break;
        }

        delay_us(2);
    }
	IOWrite(I2C_OUT,I2C_SCL_MASK,0);
    IODireOut(I2C_DIR, I2C_SDA_MASK);
    delay_us(I2C_DELAY);
    return r;
}

/*******************************************************************************
函数功能:IIC发送单字节
输入:待发送的数据
输出:成功返回1
*******************************************************************************/
unsigned char I2C_Write_Byte(char out_data)
{
	unsigned char cnt;
	for(cnt=0;cnt<8;cnt++)
	{
		IOWrite(I2C_OUT,I2C_SCL_MASK,0);
		delay_us(I2C_DELAY);
		if(out_data&(0x80>>cnt))
			IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);
		else
			IOWrite(I2C_OUT,I2C_SDA_MASK,0);
		delay_us(I2C_DELAY);
		IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);
  	  	delay_us(I2C_DELAY);
	}

	IOWrite(I2C_OUT, I2C_SCL_MASK,0);//this code is necesary
    IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);
    delay_us(I2C_DELAY);
    
 	return 1;
}


/*******************************************************************************
函数功能:IIC接收单字节
输入:给出的ACK或NACK信号
输出:返回读出的数据
*******************************************************************************/
unsigned char I2C_Read_Byte(char ack)
{
	unsigned char rd_data=0,cnt=0;
	unsigned char io_data=0,dat;
	
        
    /*
    release SDA bus
    */
    IOWrite(I2C_OUT,I2C_SCL_MASK,0);
    delay_us(I2C_DELAY);
    IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);
	delay_us(I2C_DELAY);
    IODireIn(I2C_DIR, I2C_SDA_MASK);	
    
	for(cnt=0;cnt<8;cnt++)
    {
		IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);
		delay_us(I2C_DELAY);
        
		io_data=IORead(I2C_IN);
		rd_data<<=1;        
		if(io_data&(I2C_SDA_MASK))
	   	  rd_data|=1;

        IOWrite(I2C_OUT,I2C_SCL_MASK,0);
		delay_us(I2C_DELAY);
    }

    //IOWrite(I2C_OUT,I2C_SCL_MASK,0);
    IODireOut(I2C_DIR, I2C_SDA_MASK);

    if (ack == I2C_ACK)
    {
        I2C_Ack();
    }
    else
    {
        I2C_NAck();
    }

    #if 0
    //ACK  mcp4725
	IOWrite(I2C_OUT,I2C_SCL_MASK,0);	
	delay_us(5);
	IODireOut(I2C_DIR, I2C_SDA_MASK);
	IOWrite(I2C_OUT,I2C_SDA_MASK,ack);
	delay_us(5);        
	IOWrite(I2C_OUT,I2C_SCL_MASK,I2C_SCL_MASK);	
	delay_us(5);
	IOWrite(I2C_OUT,I2C_SCL_MASK,0);
	delay_us(5);
        
    //ReleaseSDA();
    #endif
    
    
	dat=rd_data;
	return dat;
}

void ReleaseSDA(void)
{
    /*
    release SDA bus
    */
    IOWrite(I2C_OUT,I2C_SDA_MASK,I2C_SDA_MASK);
    IODireIn(I2C_DIR, I2C_SDA_MASK);	
    delay_us(I2C_DELAY);
}
#endif

