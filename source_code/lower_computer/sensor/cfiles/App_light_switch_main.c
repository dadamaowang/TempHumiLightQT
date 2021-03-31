/**********************************************************************************************************************************************************
* 文 件 名：App_light_switch_main.c
×
* 功    能：实验十一 简单RF实验
*
*            本实验主要是学习怎么配置CC2530RF功能。本实验主要分为3大部分，第一部分为初始化与RF相关的信息；
*            
*          第二部分为发送数据和接收数据；最后为选择模块功能函数。其中模块功能的选择是通过开发板上的按键来
*          
*          选择的，其中按键功能分配如下：
*                SW1 --- 开始测试（进入功能选择菜单）
*                SW2 --- 设置模块为接收功能（Light）
*                SW3 --- 设置模块为发送功能（Switch）
*                SW4 --- 发送模块发送命令按键
*
*               当发送模块按下SW4时，将发射一个控制命令，接收模块在接收到该命令后，将控制LDE1的亮或者灭。
*
*               其中LED6为工作指示灯，当工作不正常时，LED5将为亮状态。
*
*
*           在\include\文件和\source\文件中包含了和RF相关的一些宏和函数，用户使用这些宏
*           和函数可以简化对CC2530的RF操作，提高代码的可读性，本实验中就使用了其中的一些宏和函数。
*
* 注    意：本实验所需硬件资源：
*           OURS-CC2530RF板 2块
*           带LCD的智能主板 2块
*           
*           
*
* 版    本：V1.0
* 作    者：wuxianhai
* 日    期：2011.2.15
* 奥尔斯科技主页：www.ourselec.com
**********************************************************************************************************************************************************/
#include "hal_board.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "LCD.h"

/*============================================================= temp add start ====================================================*/
#include "hal.h"
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
#define SEND_DATA_LENGHT 5 // 发送数据长度
/*============================================================= temp add end ====================================================*/


#define RF_CHANNEL                25      // 2.4 GHz RF 使用信道25

#define PAN_ID                0x2011      //通信PANID
#define SWITCH_ADDR           0x2530      //开关模块地址
#define LIGHT_ADDR            0xBEEF      //灯模块地址
#define APP_PAYLOAD_LENGTH        1       //命令长度
#define LIGHT_TOGGLE_CMD          0       //命令数据

// 应用状态
#define IDLE                      0
#define SEND_CMD                  1

//应用角色
#define NONE                      0      
#define SWITCH                    1
#define LIGHT                     2
#define APP_MODES                 2

//按键
#define HAL_BUTTON_1              1
#define HAL_BUTTON_2              2
#define HAL_BUTTON_3              3
#define HAL_BUTTON_4              4
#define HAL_BUTTON_5              5
#define HAL_BUTTON_6              6

static uint8 pTxData[SEND_DATA_LENGHT];  //发送数据数组 update for temp
static uint8 pRxData[APP_PAYLOAD_LENGTH];  //接收数据数组
static basicRfCfg_t basicRfConfig;         //RF初始化结构体

extern void halboardinit(void);            //硬件初始化函数
extern void ctrPCA9554FLASHLED(uint8 led); //IIC灯控制函数
extern void ctrPCA9554LED(uint8 led,uint8 operation);
extern uint8 halkeycmd(void);              //获取按键值函数



#ifdef SECURITY_CCM                        //安全密钥
static uint8 key[]= {
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
  0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
};
#endif

static void appLight();                   //灯应用处理函数
static void appSwitch();                  //开关应用处理函数
static uint8 appSelectMode(void);         //应用功能选择函数

/*============================================================= temp add start ====================================================*/
unsigned char d1,d2,d3,d4,d5,d6,d7;

void Wait(unsigned int ms);
void QWait(void);
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
/*============================================================= temp add end ====================================================*/

/**************************************************************************************************
* 函数名称：appLight
*
* 功能描述：接收模式应用函数，初始化RF一些参数，接收另一个模块发送的控制命令，然后控制相应的LED灯
*                     
*
* 参    数：无
*
* 返 回 值：无
**************************************************************************************************/
static void appLight()
{
  basicRfConfig.myAddr = LIGHT_ADDR;       //设置接收模块的地址
  if(basicRfInit(&basicRfConfig)==FAILED)  //RF初始化
  {
    ctrPCA9554FLASHLED(5);                 //RF初始化不成功，则所有的LED5闪烁
  }
  basicRfReceiveOn();                      //打开接收功能
  // Main loop
  while (TRUE) 
  {
    while(!basicRfPacketIsReady());      //准备接收数据
    
    if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //接收数据
    {
      if(pRxData[0] == LIGHT_TOGGLE_CMD) //判断命令是否正确
      {
        ctrPCA9554FLASHLED(1);        //关闭或打开LED1
      }
    }
  }
}

/**************************************************************************************************
* 函数名称：appSwitch
*
* 功能描述：发送模式应用函数，初始化发送模式RF，通过按下SW4向另一个模块发送控制命令。
*                     
*
* 参    数：无
*
* 返 回 值：无
**************************************************************************************************/
static void appSwitch()
{
  // temp add start 
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
  // temp add end 
  
  pTxData[0] = LIGHT_TOGGLE_CMD;              //向发送数据中写入命令
  
  basicRfConfig.myAddr = SWITCH_ADDR;         //设置发送模块的地址
  
  if(basicRfInit(&basicRfConfig)==FAILED)     //RF初始化
  {
    ctrPCA9554FLASHLED(5);                    //RF初始化不成功，则所有的LED5闪烁
  }
  
  basicRfReceiveOff();                        //关闭接收功能
  // Main loop
  while (TRUE) 
  {
    // temp update
    //    if(halkeycmd() == HAL_BUTTON_4)         //判断是否按下SW4
    //    {
    //      basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//发送数据
    //      
    //      halIntOff();                                               //关闭全局中断
    //      
    //      halIntOn();                                               //打开中断
    //      
    //    }
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
    
    pTxData[1] = (uint8)tempera;
    pTxData[2] = (uint8)humidity;
    pTxData[3] = (uint8)((int)num/1000 + (int)num%1000/100);
    pTxData[4] = (uint8)((int)num%100/10 + (int)num%10);
    basicRfSendPacket(LIGHT_ADDR, pTxData, SEND_DATA_LENGHT);//发送数据
    halIntOff();                                               //关闭全局中断
    halIntOn();                                               //打开中断
  }
}

/**************************************************************************************************
* 函数名称：appSelectMode
*
* 功能描述：通过SW2或SW3选择模块的应用模式。
*                     
*
* 参    数：无
*
* 返 回 值：LIGHT -- 接收模式
*           SWITCH -- 发送模式
*           NONE -- 不正确模式
*
**************************************************************************************************/
static uint8 appSelectMode(void)        
{
  uint8 key;
  GUI_ClearScreen();                             //LCD清屏
  GUI_PutString5_7(25,6,"OURS-CC2530");          //在LCD上显示相应的文字             
  GUI_PutString5_7(10,22,"Device Mode: ");                          
  GUI_PutString5_7(10,35,"SW2 -> Light");
  GUI_PutString5_7(10,48,"SW3 -> Switch");
  LCM_Refresh();
  do 
  {
    key = halkeycmd();
  }while(key == HAL_BUTTON_1);                    //等待模式选择
  if(key == HAL_BUTTON_2)                      //接收模式
  {
    GUI_ClearScreen();
    GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字              
    GUI_PutString5_7(10,22,"Device Mode: ");                          
    GUI_PutString5_7(10,35,"Light");
    LCM_Refresh();
    
    return LIGHT;
  }
  if(key == HAL_BUTTON_3)                     //发送模式
  {
    GUI_ClearScreen();
    GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字          
    GUI_PutString5_7(10,22,"Device Mode: ");                            
    GUI_PutString5_7(10,35,"Switch");
    GUI_PutString5_7(10,48,"SW4 Send Command");
    LCM_Refresh();
    
    return SWITCH;
  }
  return NONE;        
}

/**************************************************************************************************
* 函数名称：main
*
* 功能描述：通过不同的按键，设置模块的应用角色（接收模式或发送模式）。通过SW4发送控制命令
*                     
*
* 参    数：无
*
* 返 回 值：无
**************************************************************************************************/
void main(void)
{
  uint8 appMode = NONE;                     //应用职责（角色）初始化
  
  basicRfConfig.panId = PAN_ID;             //配置PANID  2011
  basicRfConfig.channel = RF_CHANNEL;       //设置信道   25
  basicRfConfig.ackRequest = TRUE;          //需要ACK请求
  
#ifdef SECURITY_CCM                           //编译选项（未选）
  basicRfConfig.securityKey = key;          // 安全密钥
#endif
  
  halboardinit();                           //初始化板的外围设备(包括LED LCD 和按键等)
  
  if(halRfInit()==FAILED)                  //初始化RF
  {                    
    ctrPCA9554FLASHLED(5);                 //RF初始化不成功，则所有的LED5闪烁
  }
  
  ctrPCA9554FLASHLED(6);                   //点亮LED6，以指示设备正常运行       
  // temp update
  //  GUI_PutString5_7(10,22,"Simple RF test");//在LCD上显示相应的文字
  //  GUI_PutString5_7(10,35,"SW1 -> Start");
  //  LCM_Refresh();
  //  
  //  while (halkeycmd() != HAL_BUTTON_1);     //等待按键1按下，进入下一级菜单
  //  halMcuWaitMs(350);                       //延时350MS
  //  
  //  appMode = appSelectMode();               //设置应用职责（角色） 同时在LCD上显示相应的设置信息
  //  
  //  if(appMode == SWITCH)                    //发送模式
  //  {
  //    ctrPCA9554LED(2,1);
  //    appSwitch();                         //执行发送模式功能
  //  }
  //  else if(appMode == LIGHT)                //接收模式
  //  {
  //    ctrPCA9554LED(3,1);
  //    appLight();                         //执行接收模式功能
  //  }
  ctrPCA9554LED(2,1);
  appSwitch();                         //执行发送模式功能
}




