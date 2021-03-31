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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ioCC2530.h" 
#include "hal_board.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "LCD.h"

// uart add start
#include "hal.h"
// uart add end

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

/* uart add start */
/* 定义HAL_ISR_FUNCTION(f,v)宏来声明CC2530的ISR，其中f 为ISR的名称，v为中断向量*/
#define _PRAGMA(x) _Pragma(#x)
#define HAL_ISR_FUNC_DECLARATION(f,v)   _PRAGMA(vector=v) __near_func __interrupt void f(void)
#define HAL_ISR_FUNC_PROTOTYPE(f,v)     _PRAGMA(vector=v) __near_func __interrupt void f(void)
#define HAL_ISR_FUNCTION(f,v)           HAL_ISR_FUNC_PROTOTYPE(f,v); HAL_ISR_FUNC_DECLARATION(f,v)
#define RECEIVE_DATA_LENGTH 5
UINT8 *str = 0;    //串口接收数据指针
UINT16 count = 0;   //串口接收数据计数

char str_receive[16];
char str_temp[16];

/* uart add end */

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

static uint8 pTxData[APP_PAYLOAD_LENGTH];  //发送数据数组

//static uint8 pRxData[APP_PAYLOAD_LENGTH];  //接收数据数组 uart update
static uint8 pRxData[RECEIVE_DATA_LENGTH];  //接收数据数组

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

/* ------------------------------------uart add start-------------------------------------------- */
/**************************************************************************************************
* 函数名称：halWait
*
* 功能描述：延时函数（不精确延时）
*
* 参    数：wait -- 延时时间
*
* 返 回 值：无
**************************************************************************************************/
void halWait(BYTE wait)
{
  UINT32 largeWait;
  
  if(wait == 0)
  {return;}
  largeWait = ((UINT16) (wait << 7));
  largeWait += 114*wait;
  
  
  largeWait = (largeWait >> CLKSPD);
  while(largeWait--);
  
  return;
}

/**************************************************************************************************
* 函数名称：initUART
*
* 功能描述：CC2530串口初始化
*
* 参    数：无
*
* 返 回 值：无
**************************************************************************************************/
void initUART(void)
{
  
  PERCFG = 0x00;				//位置1 P0口
  P0SEL = 0x3c;				//P0用作串口
  
  U0CSR |= 0x80;				//UART方式
  U0GCR |= 11;				//baud_e = 11;
  U0BAUD |= 216;				//波特率设为115200
  UTX0IF = 1;
  
  U0CSR |= 0X40;				//允许接收
  IEN0 |= 0x84;				//uart0接收中断
}

/**************************************************************************************************
* 函数名称：UartTX_Send_String
*
* 功能描述：串口发送数据函数
*
* 参    数：*Data --- 发送数据指针
*            len  --- 发送的数据长度
*
* 返 回 值：无
**************************************************************************************************/
void UartTX_Send_String(UINT8 *Data,int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0); 
    UTX0IF = 0;
  }
}

/**************************************************************************************************
* 函数名称：HAL_ISR_FUNCTION
*
* 功能描述：串口接收数据中断函数
*
* 参    数：halUart0RxIsr --- 中断名称
*           URX0_VECTOR  --- 中断向量
*
* 返 回 值：无
**************************************************************************************************/
HAL_ISR_FUNCTION( halUart0RxIsr, URX0_VECTOR )
{
  UINT8 temp;  
  URX0IF = 0;	   
  temp = U0DBUF; 
  *(str + count) = temp;
  count++;   
}
/* ------------------------------------uart add end-------------------------------------------- */


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
 //uart add start
    char temperature[2] = "20";
    char humidity[2] = "21";
    char light[4] = "0319";

  SET_MAIN_CLOCK_SOURCE(CRYSTAL);          //设置主时钟为32M晶振
  //initUART();                              //初始化串口
  
 
  //uart add end
  
  basicRfConfig.myAddr = LIGHT_ADDR;       //设置接收模块的地址
  if(basicRfInit(&basicRfConfig)==FAILED)  //RF初始化
  {
    ctrPCA9554FLASHLED(5);                 //RF初始化不成功，则所有的LED5闪烁
  }
  basicRfReceiveOn();                      //打开接收功能
  
  initUART();
  
  // Main loop
  while (TRUE) 
  {
    while(!basicRfPacketIsReady());      //准备接收数据
    // uart update start
    //    if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //接收数据
    //    {
    //      if(pRxData[0] == LIGHT_TOGGLE_CMD) //判断命令是否正确
    //      {
    //        ctrPCA9554FLASHLED(1);        //关闭或打开LED1
    //      }
    //    }
    if(basicRfReceive(pRxData, RECEIVE_DATA_LENGTH, NULL)>0)   //接收数据
    {
      if(pRxData[0] == LIGHT_TOGGLE_CMD) //判断命令是否正确
      {
        int temp = pRxData[1];
        int humi = pRxData[2];
        int light_h = pRxData[3];
        int light_l = pRxData[4];
        
        // 温度数字转字符
        int ones = temp % 10;
        int tens = temp / 10;
        temperature[0] = tens + '0';
        temperature[1] = ones + '0';
        
        // 湿度数字转字符
        ones = humi % 10;
        tens = humi / 10;
        humidity[0] = tens + '0';
        humidity[1] = ones + '0';
        
        // 光照数字转字符
        ones = light_h % 10;
        tens = light_h / 10;
        light[0] = tens + '0';
        light[1] = ones + '0';
        ones = light_l % 10;
        tens = light_l / 10;
        light[2] = tens + '0';
        light[3] = ones + '0';

        UINT8 *uartch1 = (UINT8 *)temperature;
        UINT8* uartch2 = (UINT8*)humidity;
        UINT8* uartch3 = (UINT8*)light;
        UartTX_Send_String(uartch1, sizeof(temperature));         //USB发送
        UartTX_Send_String(uartch2, sizeof(humidity));
        UartTX_Send_String(uartch3, sizeof(light));

        ctrPCA9554FLASHLED(1);        //关闭或打开LED1
      }
    }//if接收数据

    
    //below are LED control
    //SET_MAIN_CLOCK_SOURCE(CRYSTAL);                          // 设置系统时钟源为32MHz晶体振荡器
    halWait(200);
    halWait(200);
    UINT8 temp;
    
    memset(str_receive, 0, 16);
   
    memset(str_temp, 0, 16);
    if (count)                              //判断串口是否接收到数据
    {
        temp = count;    //暂存接受到的数据长度
        halWait(50); //等待数据接收完成
        if (temp == count)   //接收到的数据长度不再变化，完成接收数据
        {
         // if(str[0] == 30){   // 收到上位机文字信息
          memset(str_receive, 0, 16);
          strncpy(str_receive, str + 2, str[1]);
          memset(str_temp, 0, 16);
          sprintf(str_temp, (char *)"%-12s", str_receive);
          GUI_PutString5_7(48,45,(char *)str_temp);                    //显示文字信息
          LCM_Refresh();
        //}
            
        for (int k = 0; k < 6; ++k) {//收到警报就全亮
                ctrPCA9554LED(k, 1);
            }

          // UartTX_Send_String(str, count); //回传接收到的数据

             //复位str和count
            str = 0;
            count = 0;
        } //if_temp_count    

    }//if_count
    Wait(400);
    PCA9554ledInit();  //初始化LED
    for (int k = 0; k < 6; ++k) {
        ctrPCA9554LED(k, 0);
    }
    Wait(200);
    
    
    
    
    
  }//while
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
    if(halkeycmd() == HAL_BUTTON_4)         //判断是否按下SW4
    {
      basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//发送数据
      
      halIntOff();                                               //关闭全局中断
      
      halIntOn();                                               //打开中断
      
    }
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
  
  // uart update start
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
  GUI_Init();   
  // GUI初始化
  GUI_SetColor(1,0);                                       // 显示色为亮点，背景色为暗点
  GUI_PutString5_7(10,6,"Coordinator");//在LCD上显示相应的文字
  GUI_PutString5_7(10,19,"Receive Mode");
  GUI_PutString5_7(10,32,"Send data to upper computer by COM.");
  GUI_PutString5_7(10,45,"Msg:");
  LCM_Refresh();
  ctrPCA9554LED(3,1); // 三号LED灯亮

  
  PCA9554ledInit();  //初始化LED
  for (UINT8 i = 0; i < 6; i++)     // 关灯
  {
      ctrPCA9554LED(i, 0);
  }

  appLight();                         //执行接收模式功能
  // uart update end
}




