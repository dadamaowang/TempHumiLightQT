#include "ioCC2530.h" 
#include "hal_mcu.h"

#define SCL          P1_0       //IIC时钟线
#define SDA          P1_1       //IIC数据线

#define ON           0x01       //LED状态
#define OFF          0x00

//定义IO方向控制函数
#define IO_DIR_PORT_PIN(port, pin, dir)  \
   do {                                  \
      if (dir == IO_OUT)                 \
         P##port##DIR |= (0x01<<(pin));  \
      else                               \
         P##port##DIR &= ~(0x01<<(pin)); \
   }while(0)


#define OSC_32KHZ  0x00                //使用外部32K晶体振荡器

//时钟设置函数
#define HAL_BOARD_INIT()                                         \
{                                                                \
  uint16 i;                                                      \
                                                                 \
  SLEEPCMD &= ~OSC_PD;                       /* 开启 16MHz RC 和32MHz XOSC */         \
  while (!(SLEEPSTA & XOSC_STB));            /* 等待 32MHz XOSC 稳定 */               \
  asm("NOP");                                                                         \
  for (i=0; i<504; i++) asm("NOP");          /* 延时63us*/                            \
  CLKCONCMD = (CLKCONCMD_32MHZ | OSC_32KHZ); /* 设置 32MHz XOSC 和 32K 时钟 */        \
  while (CLKCONSTA != (CLKCONCMD_32MHZ | OSC_32KHZ)); /* 等待时钟生效*/               \
  SLEEPCMD |= OSC_PD;                        /* 关闭 16MHz RC */                      \
}

#define IO_IN   0           //输入
#define IO_OUT  1           //输出

uint8 ack;	            //应答标志位
uint8 PCA9554ledstate = 0;  //所有LED当前状态

/******************************************************************************
 * 函数名称：QWait
 *
 * 功能描述：1us的延时
 *
 * 参    数：无
 *
 * 返 回 值：无
 *****************************************************************************/ 
void QWait()     
{
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");
}

/******************************************************************************
 * 函数名称：Wait
 *
 * 功能描述：ms的延时
 *
 * 参    数：ms - 延时时间
 *
 * 返 回 值：无
 *****************************************************************************/ 
void Wait(unsigned int ms)
{                    
   unsigned char g,k;
   while(ms)
   {
      
	  for(g=0;g<=167;g++)
	   {
	     for(k=0;k<=48;k++);
	   }
      ms--;                            
   }
} 

/******************************************************************************
 * 函数名称：Start_I2c
 *
 * 功能描述：启动I2C总线,即发送I2C起始条件.
 *
 * 参    数：无
 *
 * 返 回 值：无
 *****************************************************************************/ 
void Start_I2c()
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //设置P1.0为输出
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //设置P1.1为输出
  
  SDA=1;                   /*发送起始条件的数据信号*/
  asm("NOP");
  SCL=1;
  QWait();                /*起始条件建立时间大于4.7us,延时*/
  QWait();
  QWait();
  QWait();
  QWait();    
  SDA=0;                   /*发送起始信号*/
  QWait();                 /* 起始条件锁定时间大于4μs*/
  QWait();
  QWait();
  QWait();
  QWait();       
  SCL=0;                   /*钳住I2C总线，准备发送或接收数据 */
  asm("NOP");
  asm("NOP");
}

/******************************************************************************
 * 函数名称：Stop_I2c
 *
 * 功能描述：结束I2C总线,即发送I2C结束条件.
 *
 * 参    数：无
 *
 * 返 回 值：无
 *****************************************************************************/ 
void Stop_I2c()
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //设置P1.0为输出
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //设置P1.1为输出
  SDA=0;                            /*发送结束条件的数据信号*/
  asm("NOP");                       /*发送结束条件的时钟信号*/
  SCL=1;                            /*结束条件建立时间大于4μs*/
  QWait();
  QWait();
  QWait();
  QWait();
  QWait();
  SDA=1;                           /*发送I2C总线结束信号*/
  QWait();
  QWait();
  QWait();
  QWait();
}

/******************************************************************************
 * 函数名称：SendByte
 *
 * 功能描述：将数据c发送出去,可以是地址,也可以是数据,发完后等待应答,并对
 *           此状态位进行操作.(不应答或非应答都使ack=0 假)     
 *           发送数据正常，ack=1; ack=0表示被控器无应答或损坏。
 *
 * 参    数：c - 需发送的数据
 *
 * 返 回 值：无
 *****************************************************************************/ 
void  SendByte(uint8 c)
{
 uint8 BitCnt;
 IO_DIR_PORT_PIN(1, 0, IO_OUT);    //设置P1.0为输出
 IO_DIR_PORT_PIN(1, 1, IO_OUT);    //设置P1.1为输出
 for(BitCnt=0;BitCnt<8;BitCnt++)  /*要传送的数据长度为8位*/
    {
     if((c<<BitCnt)&0x80)SDA=1;   /*判断发送位*/
       else  SDA=0;                
      asm("NOP");
     SCL=1;                       /*置时钟线为高，通知被控器开始接收数据位*/
      QWait(); 
      QWait();                    /*保证时钟高电平周期大于4μs*/
      QWait();
      QWait();
      QWait();         
     SCL=0; 
    }    
    QWait();
    QWait();
    QWait();
    SDA=1;                        /*8位发送完后释放数据线，准备接收应答位*/
    asm("NOP");
    IO_DIR_PORT_PIN(1, 1, IO_IN);  
    SCL=1;
    QWait();
    QWait();
    QWait();
    QWait();
    if(SDA==1)ack=0;     
    else ack=1;                   /*判断是否接收到应答信号*/
    SCL=0;   
    QWait();
    QWait();
    IO_DIR_PORT_PIN(1, 1, IO_OUT);
}

/******************************************************************************
 * 函数名称：RcvByte
 *
 * 功能描述：用来接收从器件传来的数据,并判断总线错误(不发应答信号)，
 *           发完后请用应答函数。
 *
 * 参    数：无
 *
 * 返 回 值：retc - 从器件传来的数据
 *****************************************************************************/ 
uint8  RcvByte()
{
  uint8 retc;
  uint8 BitCnt;
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //设置P1.0为输出
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //设置P1.1为输出
  retc=0; 
  SDA=1;                            /*置数据线为输入方式*/
  IO_DIR_PORT_PIN(1, 1, IO_IN);
  for(BitCnt=0;BitCnt<8;BitCnt++)
      {
        asm("NOP");          
        SCL=0;                     /*置时钟线为低，准备接收数据位*/
        QWait();
        QWait();                   /*时钟低电平周期大于4.7μs*/
        QWait();
        QWait();
        QWait();
        SCL=1;                    /*置时钟线为高使数据线上数据有效*/
        QWait();
        QWait();
        retc=retc<<1;
        if(SDA==1)retc=retc+1;   /*读数据位,接收的数据位放入retc中 */
        QWait();
        QWait(); 
      }
  SCL=0;    
  QWait();
  QWait();
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  return(retc);
}

/******************************************************************************
 * 函数名称：Ack_I2c
 *
 * 功能描述：主控器进行应答信号,(可以是应答或非应答信号)
 *           
 *
 * 参    数：无
 *
 * 返 回 值：无
 *****************************************************************************/ 
void Ack_I2c(uint8 a)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //设置P1.0为输出
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //设置P1.1为输出
  if(a==0)SDA=0;                   /*在此发出应答或非应答信号 */
  else SDA=1;
  QWait();
  //QWait();
  //QWait();      
  SCL=1;
  QWait();
  QWait();                         /*时钟低电平周期大于4μs*/
  QWait();
  QWait();
  QWait();  
  SCL=0;                           /*清时钟线，钳住I2C总线以便继续接收*/
  QWait();
  //QWait();    
}

/******************************************************************************
 * 函数名称：ISendByte
 *
 * 功能描述：从启动总线到发送地址，数据，结束总线的全过程,从器件地址sla.
 *           如果返回1表示操作成功，否则操作有误。
 *           
 *
 * 参    数：sla - 从器件地址
 *           c - 需发送的数据
 *
 * 返 回 值：0 -- 失败
 *           1 -- 成功
 *****************************************************************************/ 
uint8 ISendByte(uint8 sla,uint8 c)
{
   Start_I2c();               /*启动总线*/
   SendByte(sla);             /*发送器件地址*/
     if(ack==0)return(0);
   SendByte(c);               /*发送数据*/
     if(ack==0)return(0);
  Stop_I2c();                 /*结束总线*/ 
  return(1);
}

/******************************************************************************
 * 函数名称：ISendStr
 *
 * 功能描述：从启动总线到发送地址，子地址,数据，结束总线的全过程,从器件
 *            地址sla，子地址suba，发送内容是s指向的内容，发送no个字节。
 *           如果返回1表示操作成功，否则操作有误。
 *           
 *
 * 参    数：sla - 从器件地址
 *           suba - 从器件子地址
 *           *s - 数据
 *           no - 数据字节数目
 *
 * 返 回 值：0 -- 失败
 *           1 -- 成功
 *
 * 注    意：使用前必须已结束总线。
 *****************************************************************************/ 
uint8 ISendStr(uint8 sla,uint8 suba,uint8 *s,uint8 no)
{
   uint8 i;

   Start_I2c();               /*启动总线*/
   SendByte(sla);             /*发送器件地址*/
     if(ack==0)return(0);
   SendByte(suba);            /*发送器件子地址*/
     if(ack==0)return(0);
   for(i=0;i<no;i++)
    {   
     SendByte(*s);            /*发送数据*/
       if(ack==0)return(0);
     s++;
    } 
 Stop_I2c();                  /*结束总线*/ 
  return(1);
}

/******************************************************************************
 * 函数名称：IRcvByte
 *
 * 功能描述：从启动总线到发送地址，读数据，结束总线的全过程,从器件地
 *          址sla，返回值在c. 如果返回1表示操作成功，否则操作有误。
 *           
 *
 * 参    数：sla - 从器件地址
 *           *c - 需发送的数据
 *
 * 返 回 值：0 -- 失败
 *           1 -- 成功
 *
 *注    意：使用前必须已结束总线。
 *****************************************************************************/ 
uint8 IRcvByte(uint8 sla,uint8 *c)
{
   Start_I2c();                /*启动总线*/
   SendByte(sla+1);            /*发送器件地址*/
   //SendByte(sla); 
   if(ack==0)return(0);
   *c=RcvByte();               /*读取数据*/
   Ack_I2c(1);                 /*发送非就答位*/
   Stop_I2c();                 /*结束总线*/ 
   return(1);
}

/******************************************************************************
 * 函数名称：IRcvStr
 *
 * 功能描述：从启动总线到发送地址，子地址,读数据，结束总线的全过程,从器件
 *          地址sla，子地址suba，读出的内容放入s指向的存储区，读no个字节。
 *         如果返回1表示操作成功，否则操作有误。
 *           
 *
 * 参    数：sla - 从器件地址
 *           suba - 从器件子地址
 *           *s - 数据
 *           no - 数据字节数目
 *
 * 返 回 值：0 -- 失败
 *           1 -- 成功
 *
 * 注    意：使用前必须已结束总线。
 *****************************************************************************/ 
uint8 IRcvStr(uint8 sla,uint8 suba,uint8 *s,uint8 no)
{
   Start_I2c();               /*启动总线*/
   SendByte(sla);             /*发送器件地址*/
   if(ack==0)return(0);
   SendByte(suba);            /*发送器件子地址*/
  // if(ack==0)return(0);
  // SendByte(sla+1);
   if(ack==0)return(0);
   while(no > 0) 
   {
    *s++ = RcvByte();
     if(no > 1)  Ack_I2c(0);   /*发送就答位*/ 
     else Ack_I2c(1);          /*发送非应位*/
     no--;
   }
   Stop_I2c();                 /*结束总线*/ 
   return(1);
}

/******************************************************************************
 * 函数名称：ctrPCA9554LED
 *
 * 功能描述：通过IIC总线控制PCA9554的输出，进而控制相应的LED。
 *                    
 *
 * 参    数：LED - 所控制的LED
 *           operation - 开或关操作
 *
 * 返 回 值：无
 *           
 *
 * 注    意：PCA9554的地址为：0x40
 *****************************************************************************/ 
void ctrPCA9554LED(uint8 led,uint8 operation)
{
  uint8 output = 0x00;
  uint8 *data = 0;
  if(ISendStr(0x40,0x03,&output,1))  //配置PCA9554寄存器
  {
    switch(led)
    {
      case 0:                        //LED0控制
        if (operation)
        {
          output = PCA9554ledstate & 0xfe;
        }
        else
        {
          output = PCA9554ledstate | 0x01;
        }
      break;
       case 1:                      //LED1控制
        if (operation)
        {
          output = PCA9554ledstate & 0xfd;
        }
        else
        {
          output = PCA9554ledstate | 0x02;
        }
      break;
       case 2:                     //LED2控制
        if (operation)
        {
          output = PCA9554ledstate & 0xf7;
        }
        else
        {
          output = PCA9554ledstate | 0x08;
        }
      break;
       case 3:                     //LED3控制
        if (operation)
        {
          output = PCA9554ledstate & 0xfb;
        }
        else
        {
          output = PCA9554ledstate | 0x04;
        }
      break;
       case 4:                    //LED4控制
        if (operation)
        {
          output = PCA9554ledstate & 0xdf;
        }
        else
        {
          output = PCA9554ledstate | 0x20;
        }
      break;
       case 5:                   //LED5控制
        if (operation)
        {
          output = PCA9554ledstate & 0xef;
        }
        else
        {
          output = PCA9554ledstate | 0x10;
        }
      break;

     default:break;
    }
    if(ISendStr(0x40,0x01,&output,1)) //写PCA9554输出寄存器
    {
      if(IRcvByte(0x40,data))         //读PCA9554输出寄存器
      {
        PCA9554ledstate = *data;
      }
    }
  }
}

/******************************************************************************
 * 函数名称：PCA9554ledInit
 *
 * 功能描述：初始化6个LED，即关闭所有的LED
 *                    
 * 参    数：无          
 *
 * 返 回 值：无
 *           
 *****************************************************************************/ 
void PCA9554ledInit()
{
  uint8 output = 0x00;
  uint8 *data = 0;
  if(ISendStr(0x40,0x03,&output,1))  //配置PCA9554寄存器
  {
    output = 0xbf;
    if(ISendStr(0x40,0x01,&output,1)) //写输出寄存器
    {
      if(IRcvByte(0x40,data))         //读输出寄存器
      {
        PCA9554ledstate = *data;
      }
    }
  }
}

/******************************************************************************
 * 函数名称：main
 *
 * 功能描述：6个LED轮流开启和关闭
 *                    
 * 参    数：无          
 *
 * 返 回 值：无
 *           
 *****************************************************************************/ 
void main()
{  
  uint8 i;
    HAL_BOARD_INIT();    //时钟设置
    PCA9554ledInit();
    int  flag=0;
    while(1)             //流水灯
    {
      
      if(flag==0){
        for(i=0;i<6;++i)
          ctrPCA9554LED(i,ON);
          Wait(100);
      }
      
      if(flag==1){
        for(i=0;i<6;++i){
          ctrPCA9554LED(i,OFF);
          Wait(100);
        }
      }
      
      if(flag==0)flag=1;
      else flag=0;
     
    }
}

