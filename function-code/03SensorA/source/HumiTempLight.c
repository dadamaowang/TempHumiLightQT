/**********************************************************************************************************************************************************
* 文 件 名：HumiTempLight.c
×
* 功    能：实验十 SHT10温湿度读取、光照强度读取
*
*              SHT10 是一款高度集成的温湿度传感器芯片， 提供全标定的数字输出。它采用专利的CMOSens技术，确保产品具有极高的可靠性与卓
*          越的长期稳定性。传感器包括一个电容性聚合体测湿敏感元件、一个用能隙材料制成的测温元件，并在同一芯片上，与14 位的A/D 转
*          换器以及串行接口电路实现无缝连接。
*
*           SHT10引脚特性如下：
*             1. VDD，GND SHT10 的供电电压为2.4~5.5V。传感器上电后，要等待11ms 以越过“休眠”状态。在此期间无需发送任何指令。
*               电源引脚（VDD，GND）之间可增加一个100nF 的电容，用以去耦滤波。
*             2. SCK 用于微处理器与SHT10 之间的通讯同步。由于接口包含了完全静态逻辑，因而不存在最小SCK频率。
*             3. DATA 三态门用于数据的读取。DATA 在SCK 时钟下降沿之后改变状态，并仅在SCK 时钟上升沿有效。数据传输期间，
*                在SCK 时钟高电平时，DATA必须保持稳定。为避免信号冲突，微处理器应驱动DATA 在低电平。需要一个外部的上拉电阻
*               （例如：10kΩ）将信号提拉至高电平。上拉电阻通常已包含在微处理器的I/O 电路中。
*
*           向SHT10发送命令：  　　　　　　　
*               用一组“ 启动传输”时序，来表示数据传输的初始化。它包括：当SCK 时钟高电平时DATA 翻转为低电平，紧接着SCK变为低电平，
*           随后是在SCK 时钟高电平时DATA 翻转为高电平。后续命令包含三个地址位（目前只支持“000”），和五个命令位。SHT10会以下
*           述方式表示已正确地接收到指令：在第8 个SCK 时钟的下降沿之后，将DATA 拉为低电平（ACK 位）。在第9 个SCK 时钟的下降沿
*           之后，释放DATA（恢复高电平）。
*
*           测量时序(RH 和 T)：
*               发布一组测量命令（‘00000101’表示相对湿度RH，‘00000011’表示温度T）后，控制器要等待测量结束。这个过程需要大约
*            11/55/210ms，分别对应8/12/14bit 测量。确切的时间随内部晶振速度，最多有±15%变化。SHTxx 通过下拉DATA 至低电平并
*            进入空闲模式，表示测量的结束。控制器在再次触发SCK 时钟前，必须等待这个“数据备妥”信号来读出数据。检测数据可以
*           先被存储，这样控制器可以继续执行其它任务在需要时再读出数据。
*                接着传输2 个字节的测量数据和1 个字节的CRC 奇偶校验。uC 需要通过下拉DATA 为低电平，以确认每个字节。所有的数据从
*           MSB 开始，右值有效（例如：对于12bit 数据，从第5 个SCK 时钟起算作MSB； 而对于 8bit 数据， 首字节则无意义）。用 
*            CRC 数据的确认位，表明通讯结束。如果不使用CRC-8 校验，控制器可以在测量值LSB 后，通过保持确认位ack 高电平， 来中
*           止通讯。在测量和通讯结束后，SHTxx 自动转入休眠模式。
*
*           通讯复位时序：
*               如果与 SHTxx 通讯中断，下列信号时序可以复位串口：当DATA 保持高电平时，触发SCK 时钟9 次或更多。在下一次指令前，发送
*            一个“传输启动”时序。这些时序只复位串口，状态寄存器内容仍然保留。
*            
*            更多SHT10信息，请参考相应文档。             
*           
*            光照强度采集：
*                光照采集主要是通过用CC2530内部的ADC来得到OURS-CC2530开发板上的光照传感器输出电压。传感器输出电压(连接到CC2530的AIN0)
*
*           本实验将使用CC2530读取温湿度传感器SHT10的温度和湿度数据，并通过ADC得到光照传感器的数据。最后将采样到的数据转换然后在LCD上显示。
*
*           在\include\hal.h文件中包含了和ADC相关的一些宏，用户使用这些宏
*           可以简化对ADC的操作，提高代码的可读性，本实验中就使用了其中的一些宏。
*
* 注    意：本实验所需硬件资源：
*           OURS-CC2530RF板
*           带LCD的智能主板
*           温湿度+光照传感器板
*           
*
* 版    本：V1.0
* 作    者：wuxianhai
* 日    期：2011.2.14
* 奥尔斯科技主页：www.ourselec.com
**********************************************************************************************************************************************************/
#include "hal.h"
#include "LCD.h"
#include "stdio.h"

#define noACK 0
#define ACK   1

#define STATUS_REG_W 0x06
#define STATUS_REG_R 0x07
#define MEASURE_TEMP 0x03
#define MEASURE_HUMI 0x05
#define RESET        0x1e

#define SCL          P1_0     //SHT10时钟
#define SDA          P1_1     //SHT10数据线

unsigned char d1,d2,d3,d4,d5,d6,d7;

void Wait(unsigned int ms);
void QWait(void)  ;
char s_write_byte(unsigned char value);
char s_read_byte(unsigned char ack);
void s_transstart(void);
void s_connectionreset(void);
char s_measure( unsigned char *p_checksum, unsigned char mode);
void initIO(void);

/**************************************************************************************************
 * 函数名称：Wait
 *
 * 功能描述：延时函数（不精确延时）
 *
 * 参    数：ms -- 延时时间
 *
 * 返 回 值：无
 **************************************************************************************************/
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

/**************************************************************************************************
 * 函数名称：QWait
 *
 * 功能描述：延时函数（大约1us的延时）
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void QWait()     
{
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");

}

/**************************************************************************************************
 * 函数名称：initIO
 *
 * 功能描述：SHT10串行通信IO初始化
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void initIO(void)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  P1INP |= 0x03;
  SDA = 1; SCL = 0;
}

/**************************************************************************************************
 * 函数名称：s_write_byte
 *
 * 功能描述：从SHT10写一个字节
 *
 * 参    数：value -- 需写入的字节值
 *
 * 返 回 值：error -- 操作是否成功
 **************************************************************************************************/
char s_write_byte(unsigned char value)
{ 
  unsigned char i,error=0;  
  IO_DIR_PORT_PIN(1, 0, IO_OUT);      //时钟和数据IO设置为输出
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  for (i=0x80;i>0;i/=2)               //将一个字节的8位逐一输出        
  {
     if (i & value)
  	 SDA=1;          
     else
    	 SDA=0;                        
    SCL = 1;                        
    QWait();QWait();QWait();QWait();QWait();
    SCL = 0;
    asm("NOP"); asm("NOP");
  }
  SDA = 1; 
  IO_DIR_PORT_PIN(1, 1, IO_IN);      //将数据线设置为输入，以准备接收SHT10的ACK
  SCL = 1;  asm("NOP");                          
  error = SDA; 
  QWait();QWait();QWait();
  IO_DIR_PORT_PIN(1, 1, IO_OUT);     //将数据线恢复为输出状态
  SDA = 1; 
  SCL = 0;        
  
  return error;                                   
}

/**************************************************************************************************
 * 函数名称：s_read_byte
 *
 * 功能描述：从SHT10读取一个字节
 *
 * 参    数：ack -- 读取数据后，向SHT10发送ACK
 *
 * 返 回 值：val -- 读取的字节值
 **************************************************************************************************/
char s_read_byte(unsigned char ack)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);     //时钟和数据IO设置为输出
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  unsigned char i,val=0;
  SDA= 1;
  IO_DIR_PORT_PIN(1, 1, IO_IN);      //将数据线设置为输入，以准备接收SHT10的数据
  for (i=0x80;i>0;i/=2) 
  {
    SCL = 1;
    if (SDA)
     val = (val | i);
    else
      val = (val | 0x00);
    SCL = 0;
    QWait();QWait();QWait();QWait();QWait();
  }
  IO_DIR_PORT_PIN(1, 1, IO_OUT);     //将数据线恢复为输出状态
  SDA = !ack;
  SCL = 1;
  QWait();QWait();QWait();QWait();QWait();
  SCL = 0;
  SDA = 1;
  
  return val;                       //返回读取的值
}

/**************************************************************************************************
 * 函数名称：s_transstart
 *
 * 功能描述：启动SHT10，开始与SHT10通信
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void s_transstart(void)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
   SDA = 1; SCL = 0;
   QWait();QWait();
   SCL = 1;QWait();QWait();
   SDA = 0;QWait();QWait(); 
   SCL = 0;QWait();QWait();QWait();QWait();QWait();
   SCL = 1;QWait();QWait();
   SDA = 1;QWait();QWait();
   SCL = 0;QWait();QWait();
}

/**************************************************************************************************
 * 函数名称：s_connectionreset
 *
 * 功能描述：与SHT10通信复位
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void s_connectionreset(void)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  unsigned char i;
  SDA = 1; SCL= 0;
  for(i=0;i<9;i++)
  {
    SCL = 1;QWait();QWait();
    SCL = 0;QWait();QWait();
  }
  s_transstart();
}

/**************************************************************************************************
 * 函数名称：s_measure
 *
 * 功能描述：发送命令、读取SHT10温度或湿度数据
 *
 * 参    数：*p_checksum -- 校验和
 *           mode -- 读取数据类型（3为温度，5为湿度）
 *
 * 返 回 值：er -- 操作结果
 **************************************************************************************************/
char s_measure( unsigned char *p_checksum, unsigned char mode)
{
  unsigned er=0;
  unsigned int i,j;
  s_transstart();                              //启动传输
  switch(mode)
  {
    case 3	:er+=s_write_byte(3);break;    //发送温度读取命令
    case 5	:er+=s_write_byte(5);break;    //发送湿度读取命令
    default     :break;
  }
  IO_DIR_PORT_PIN(1, 1, IO_IN);                //将数据线设置为输入，以准备接收SHT10的ACK
  for(i=0;i<65535;i++)
  {
    for(j=0;j<65535;j++)
    {if(SDA == 0)
    {break;}}
    if(SDA == 0)
    {break;}
  }
  
  if(SDA)                                     //SDA没有拉低，错误信息加1
    
  {er += 1;}
  d1 = s_read_byte(ACK);                     //数据读取
  d2 = s_read_byte(ACK);
  d3 = s_read_byte(noACK);
  return er;
}

/**************************************************************************************************
 * 函数名称：th_read
 *
 * 功能描述：调用相应函数，读取温度和数据数据并校验和计算
 *
 * 参    数：*t -- 温度值
 *           *h -- 湿度值
 *
 * 返 回 值：无
 **************************************************************************************************/
void th_read(int *t,int *h )
{
  unsigned char error,checksum;
  float humi,temp;
  int tmp;
  initIO();
  
  s_connectionreset();                  //启动传输
    error=0;
   error+=s_measure(&checksum,5);       //读取湿度数据并校验
    humi = d1*256+d2;
    
    error+=s_measure(&checksum,3);      //读取温度数据并校验
    temp = d1*256+d2;
    if(error!=0) s_connectionreset();   //读取失败，通信复位
    else                                //读取成功，计算数据
    {      
       temp = temp*0.01  -  44.0 ;
       humi = (temp - 25) * (0.01 + 0.00008 * humi) -0.0000028 * humi * humi + 0.0405 * humi-4;
       if(humi>100)
       {humi = 100;}
       if(humi<0.1)
       {humi = 0.1;}
    }
    
    tmp=(int)(temp*10)%10;
    
    if(tmp>4)
    {
     temp=temp+1; 
    }
    else
    {
       temp=temp;
    }
    
  *t=(int)temp;
  
   tmp=(int)(humi*10)%10;
    
    if(humi>4)
    {
     humi=humi+1; 
    }
    else
    {
       humi=humi;
    }
    
  *h=(int)humi;
  
}

/**************************************************************************************************
 * 函数名称：main
 *
 * 功能描述：读取温度、湿度和光照强度数据，并同过LCD显示
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void main()
{
  int tempera;
  int humidity;
  char  s[16];
  UINT8 adc0_value[2];
  float num = 0;
  SET_MAIN_CLOCK_SOURCE(CRYSTAL);                          // 设置系统时钟源为32MHz晶体振荡器
  
  GUI_Init();                                              // GUI初始化
  GUI_SetColor(1,0);                                       // 显示色为亮点，背景色为暗点
  GUI_PutString5_7(25,6,"OURS-CC2530");                    //显示 OURS-CC2530
  GUI_PutString5_7(10,22,"Temp:");                       
  GUI_PutString5_7(10,35,"Humi:");
  GUI_PutString5_7(10,48,"Light:");
  LCM_Refresh();
  
  while(1)
  {
   th_read(&tempera,&humidity);                            //读取温度和湿度
   sprintf(s, (char*)"%d%d C",  ((INT16)((int)tempera / 10)), ((INT16)((int)tempera % 10)));  //将温度结果转换为字符串
   GUI_PutString5_7(48,22,(char *)s);                      //显示结果
   LCM_Refresh();
   sprintf(s, (char*)"%d%d %%",  ((INT16)((int)humidity / 10)), ((INT16)((int)humidity % 10)));//将湿度结果转换为字符串
   GUI_PutString5_7(48,35,(char *)s);                     //显示结果
   LCM_Refresh();
   
       /* AIN0通道采样 */
    ADC_ENABLE_CHANNEL(ADC_AIN0);                          // 使能AIN0为ADC输入通道

    /* 配置ADCCON3寄存器以便在ADCCON1.STSEL = 11(复位默认值)且ADCCON1.ST = 1时进行单一转换 */
    /* 参考电压：AVDD_SOC引脚上的电压 */
    /* 抽取率：512                     */
    /* ADC输入通道：AIN0              */
    ADC_SINGLE_CONVERSION(ADC_REF_AVDD | ADC_14_BIT | ADC_AIN0);

    ADC_SAMPLE_SINGLE();                                   // 启动一个单一转换

    while(!ADC_SAMPLE_READY());                            // 等待转换完成

    ADC_ENABLE_CHANNEL(ADC_AIN0);                          // 禁止AIN0

    adc0_value[0] = ADCL;                                      // 读取ADC值
    adc0_value[1] = ADCH;                                      // 读取ADC值
    adc0_value[0] = adc0_value[0]>>2;
    
     num = (adc0_value[1]*256+adc0_value[0])*3.3/8192;  //有一位符号位,取2^13;
     num /= 4;          
     num=num*913;                                        //转换为Lx
     sprintf(s, (char*)"%d%d%d%d lx",  ((INT16)((int)num/1000)), ((INT16)((int)num%1000/100)),((INT16)((int)num%100/10)),((INT16)((int)num%10))); //将光照结果转换为字符串
     GUI_PutString5_7(48,48,(char *)s);                    //显示结果
     LCM_Refresh();
  }
}
