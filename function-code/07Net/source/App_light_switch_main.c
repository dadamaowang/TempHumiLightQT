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

#define RF_CHANNEL                17     

#define PAN_ID                32      //通信PANID

#define A_ADDR           320     
#define B_ADDR           321
#define C_ADDR           322
#define D_ADDR           323    
#define E_ADDR           324
#define F_ADDR           325

#define APP_PAYLOAD_LENGTH        3       //命令长度
#define LIGHT_TOGGLE_CMD          0       //命令数据
#define LIGHTENING                1       //命令数据

// 应用状态
#define IDLE                      0
#define SEND_CMD                  1

//应用角色
#define NONE                     7      
#define ANODE                    1
#define BNODE                    2
#define CNODE                    3
#define DNODE                    4
#define ENODE                    5
#define FNODE                    6
#define APP_MODES                7

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



#ifdef SECURITY_CCM                        //安全密钥
static uint8 key[]= {
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
};
#endif

uint16 router_table[6][6] = {
  {A_ADDR,B_ADDR,B_ADDR,B_ADDR,B_ADDR,B_ADDR},
  {A_ADDR,B_ADDR,C_ADDR,D_ADDR,C_ADDR,C_ADDR},
  {B_ADDR,B_ADDR,C_ADDR,B_ADDR,E_ADDR,F_ADDR},
  {B_ADDR,B_ADDR,B_ADDR,B_ADDR,B_ADDR,B_ADDR},
  {C_ADDR,C_ADDR,C_ADDR,C_ADDR,C_ADDR,C_ADDR},
  {C_ADDR,C_ADDR,C_ADDR,C_ADDR,C_ADDR,C_ADDR}  
};

static void appLight(uint16 node);                   //BCDEF
static void appSwitch();                  //A
static void prepare(uint8 dest);
static uint8 appSelectMode(void);         //应用功能选择函数


static void prepare(uint8 dest)
{
  uint16 addr;
  
  addr = 320 + dest -1;
  pTxData[0] =addr & 0x00ff;
  pTxData[1] = (addr &0xff00 )>> 8;
  pTxData[2] = LIGHTENING;
  
  return;
  
}



static void appLight(uint16 node)
{
  for(int i=0;i<=5;++i)ctrPCA9554LED(i,0);
    basicRfConfig.myAddr = node;       //设置接收模块的地址
    uint8 me = node % 16;
    
    if(basicRfInit(&basicRfConfig)==FAILED)  //RF初始化
    {
      ctrPCA9554FLASHLED(me+1);                 //RF初始化不成功，则所有的LED5闪烁
    }
    
    ctrPCA9554LED(me,1);
    
    
    
   
    
    
    
    while(1){
    
         
      basicRfReceiveOn();                      //打开接收功能
    
      uint8 key = halkeycmd();
    
      while(!basicRfPacketIsReady() && key != 4){      //准备接收数据
          key = halkeycmd();
          halMcuWaitMs(350);
      }
      
      
      for(int i=0;i<=5;++i)ctrPCA9554LED(i,0);
      
    
      if(key == 4){//send
        
       // for(int i=0;i<=5;++i)ctrPCA9554LED(i,0);
          GUI_ClearScreen();                            
        GUI_PutString5_7(25,6,"SEND");          
        LCM_Refresh(); 
        ctrPCA9554LED(me,1);
        
        
        basicRfReceiveOff();                        //关闭接收功能
       

        
        key = halkeycmd();
        while( key < 1 || key > 6){
          key = halkeycmd();
          halMcuWaitMs(350);
        }
        
        ctrPCA9554FLASHLED(key); 
        
        char s[2];
        sprintf(s, (char*)"%d",((INT16)((int)key)));
       
        GUI_ClearScreen();                            
        GUI_PutString5_7(25,6,(char*)s);          
        LCM_Refresh();     
        
        prepare(key);
        basicRfSendPacket(router_table[0][key-1], pTxData, APP_PAYLOAD_LENGTH);//发送数据

        halIntOff();                                               //关闭全局中断

        halIntOn();                                               //打开中断
        
        
        
      }else{  //recieve
        uint16 addr;
        uint8 reci;
       
        if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //接收数据
        {
          addr = (pRxData[1] << 8) + pRxData[0];
          
          reci = addr - 320;
          
          
          if(reci == me && pRxData[2] == LIGHTENING){
             for(int i=0;i<=5;++i)ctrPCA9554LED(i,1);
           
         }else {//transmit
             basicRfReceiveOff();                        //关闭接收功能
      
          
            pTxData[0] = pRxData[0];
            pTxData[1] = pRxData[1];
            pTxData[2] = pRxData[2];
           ctrPCA9554FLASHLED(reci+1);   
          
            basicRfSendPacket(router_table[me][reci], pTxData, APP_PAYLOAD_LENGTH);//发送数据

           halIntOff();                                               //关闭全局中断

           halIntOn();                                               //打开中断
        

          }
          
        }
    
      }//else
     
    }//while 1
    
}

/*

static void appSwitch()
{
 
  for(int i=0;i<=5;++i)ctrPCA9554LED(i,0);
   
  basicRfConfig.myAddr = A_ADDR;       //设置接收模块的地址

    if(basicRfInit(&basicRfConfig)==FAILED)  //RF初始化
    {
      ctrPCA9554FLASHLED(1);                 //RF初始化不成功，则所有的LED5闪烁
    }
    
   ctrPCA9554LED(0,1);
    
    while(1){
    
      
      
      basicRfReceiveOn();                      //打开接收功能
    
      uint8 key = halkeycmd();
    
      while(!basicRfPacketIsReady() && key != 4){      //准备接收数据
          key = halkeycmd();
          halMcuWaitMs(350);
      }
       
      for(int i=0;i<=5;++i)ctrPCA9554LED(i,0);
    
      if(key == 4){//send
        
        GUI_ClearScreen();                            
        GUI_PutString5_7(25,6,"SEND");          
        LCM_Refresh();       
     
        
        ctrPCA9554LED(0,1);
        
        basicRfReceiveOff();                        //关闭接收功能
    
        key = halkeycmd();
        while( key <2 || key > 6){
          key = halkeycmd();
          halMcuWaitMs(350);
        }
          
         ctrPCA9554FLASHLED(key); 
        
        char s[2];
        sprintf(s, (char*)"%d",((INT16)((int)key)));
       
        GUI_ClearScreen();                            
        GUI_PutString5_7(25,6,(char*)s);          
        LCM_Refresh();     
        
        prepare(key);
        basicRfSendPacket(router_table[0][key-1], pTxData, APP_PAYLOAD_LENGTH);//发送数据

        halIntOff();                                               //关闭全局中断

        halIntOn();                                               //打开中断
        
        
      }
      
    }//while 1
        
}

*/

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
  //GUI_PutString5_7(10,22,"Device Mode: ");                          
  GUI_PutString5_7(10,22,"SW1:A SW2:B");
  GUI_PutString5_7(10,35,"SW3:C SW4:D");
  GUI_PutString5_7(10,48,"SW5:E SW6:F");
  LCM_Refresh();
  
  ctrPCA9554FLASHLED(5);
  ctrPCA9554FLASHLED(4);
  ctrPCA9554FLASHLED(3);
    
 do 
 {
   key = halkeycmd();
    halMcuWaitMs(350);                       //延时350MS

 }while(key < 1 || key > 6);                    //等待模式选择
 
 ctrPCA9554FLASHLED(5);
  ctrPCA9554FLASHLED(4);
  ctrPCA9554FLASHLED(3);
 
    if(key == HAL_BUTTON_1)                      //接收模式
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"A");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return ANODE;
    }
    if(key == HAL_BUTTON_2)                      //接收模式
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"B");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return BNODE;
    }
     if(key == HAL_BUTTON_3)                     //发送模式
    {
      
    GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"C");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return CNODE;
    }
    if(key == HAL_BUTTON_4)                      //接收模式
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"D");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return DNODE;
    }
    if(key == HAL_BUTTON_5)                      //接收模式
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"E");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return ENODE;
    }
     if(key == HAL_BUTTON_6)                     //发送模式
    {
     GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //在LCD上显示相应的文字              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"F");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return FNODE;
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
    
    GUI_PutString5_7(10,22,"Network");//在LCD上显示相应的文字
    GUI_PutString5_7(10,35,"SW1 -> Start");
    LCM_Refresh();
    

    while (halkeycmd() != HAL_BUTTON_1);     //等待按键1按下，进入下一级菜单
    halMcuWaitMs(350);                       //延时350MS

    appMode = appSelectMode();               //设置应用职责（角色） 同时在LCD上显示相应的设置信息

    if(appMode == ANODE)                    //发送模式
    {
       ctrPCA9554FLASHLED(6);            
        //ctrPCA9554LED(1,1);
           appLight(A_ADDR); 
    }
    else if(appMode == BNODE)                //接收模式
    {
             ctrPCA9554FLASHLED(6);            
        //ctrPCA9554LED(2,1);
        appLight(B_ADDR);                         //执行接收模式功能
    }
    else if(appMode == CNODE){
          ctrPCA9554FLASHLED(6);            
     // ctrPCA9554LED(3,1);
      appLight(C_ADDR);            
    }
    else if(appMode == DNODE){
             ctrPCA9554FLASHLED(6);            
    //  ctrPCA9554LED(4,1);
      appLight(D_ADDR);            
    }
    else if(appMode == ENODE){
            ctrPCA9554FLASHLED(6);            
     // ctrPCA9554LED(5,1);
      appLight(E_ADDR);            
    }
    else {
             ctrPCA9554FLASHLED(6);            
     // ctrPCA9554LED(6,1);
      appLight(F_ADDR);            
    }
}




