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

#define RF_CHANNEL                11      // 2.4 GHz RF 使用信道11

#define PAN_ID                0x0020      //通信PANID
#define SWITCH_ADDR           0x0140      //开关模块地址
#define LIGHT_ADDR            0x0141      //灯模块地址
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

static uint8 pTxData[APP_PAYLOAD_LENGTH];  //发送数据数组
static uint8 pRxData[APP_PAYLOAD_LENGTH];  //接收数据数组
static basicRfCfg_t basicRfConfig;         //RF初始化结构体

extern void halboardinit(void);            //硬件初始化函数
extern void ctrPCA9554FLASHLED(uint8 led); //IIC灯控制函数
extern void ctrPCA9554LED(uint8 led,uint8 operation);
extern uint8 halkeycmd(void);              //获取按键值函数

extern uint8 getseq;

#ifdef SECURITY_CCM                        //安全密钥
static uint8 key[]= {
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
};
#endif

static void appLight();                   //灯应用处理函数
static void appSwitch();                  //开关应用处理函数
static uint8 appSelectMode(void);         //应用功能选择函数

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
  
    
    int count=0;
    char countb[16];
    char seq[16];
 
    
    // Main loop
    while (TRUE) 
    {
        while(!basicRfPacketIsReady());      //准备接收数据

        if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //接收数据
        {
         
            
                count++;
                ctrPCA9554FLASHLED(1);        //关闭或打开LED1
                
                
                
                if(pRxData[0]==0){
                
                sprintf(countb, (char*)"%d%d",  ((INT16)((int)count / 10)), ((INT16)((int)count% 10)));  
                
                GUI_PutString5_7(10,48,"KEY4-COUNT:");
                GUI_PutString5_7(75,48,(char*)countb);
                
                   LCM_Refresh();
                }else{
                
  
                 sprintf(seq, (char*)"%d%d",  ((INT16)((int)getseq / 10)), ((INT16)((int)getseq% 10)));  
                 
                  GUI_PutString5_7(10,48,"KEY5---SEQ:");
                GUI_PutString5_7(75,48,(char*)seq);
                
                   LCM_Refresh();
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
        int key = halkeycmd();
        if(key == HAL_BUTTON_4 )         //判断是否按下SW4
        {
            pTxData[0] =0 ;
                 
            ctrPCA9554FLASHLED(2);        //关闭或打开LED1
            
                  
            basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//发送数据

            halIntOff();                                               //关闭全局中断

            halIntOn(); }
         else if(key == HAL_BUTTON_5){
            
       
             pTxData[0] =1; 
                 
              ctrPCA9554FLASHLED(2);        //关闭或打开LED1
         
              
            basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//发送数据

            halIntOff();                                               //关闭全局中断

            halIntOn();  
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
      GUI_PutString5_7(10,48,"PUSH SW4/SW5");
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

    basicRfConfig.panId = PAN_ID;             //配置PANID  
    basicRfConfig.channel = RF_CHANNEL;       //设置信道   
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
    
    GUI_PutString5_7(10,22,"Simple RF test");//在LCD上显示相应的文字
    GUI_PutString5_7(10,35,"SW1 -> Start");
    LCM_Refresh();

    while (halkeycmd() != HAL_BUTTON_1);     //等待按键1按下，进入下一级菜单
    halMcuWaitMs(350);                       //延时350MS

    appMode = appSelectMode();               //设置应用职责（角色） 同时在LCD上显示相应的设置信息

    if(appMode == SWITCH)                    //发送模式
    {
        ctrPCA9554LED(2,1);
        appSwitch();                         //执行发送模式功能
    }
    else if(appMode == LIGHT)                //接收模式
    {
        ctrPCA9554LED(3,1);
        appLight();                         //执行接收模式功能
    }
}




