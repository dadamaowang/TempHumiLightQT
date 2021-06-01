/**********************************************************************************************************************************************************
* 文 件 名：App_spectrum_analyzer_main.c
×
* 功    能：实验十三 信道信号强度测试实验
*
*            本实验主要是在学会了配置CC2530RF功能基础上。掌握分析2.4G频段信道11-26各个信道的信号强度。
*            
*         然后通过LCD显示测试结果，结果的显示分为两个部分，一部分是通过16矩形条的形式同时显示各个信道
*
*         中的信号强度。另一个是通过按键可以切换显示不同信道具体的RSSI值。
*
*            其中按键功能分配如下：
*                SW1 --- 开始测试
*                SW2 --- 显示RSSI值的信道加
*                SW3 --- 显示RSSI值的信道减
*
*
*               其中LED6为工作指示灯，当工作不正常时，LED5将为亮状态。
*
*
*           在\include\文件和\source\文件中包含了和RF相关的一些宏和函数，用户使用这些宏
*           和函数可以简化对CC2530的RF操作，提高代码的可读性，本实验中就使用了其中的一些宏和函数。
*
* 注    意：本实验所需硬件资源：
*           OURS-CC2530RF板 1块
*           带LCD的智能主板 1块
*           
*           
*
* 版    本：V1.0
* 作    者：wuxianhai
* 日    期：2011.2.18
* 奥尔斯科技主页：www.ourselec.com
**********************************************************************************************************************************************************/

#include "hal_board.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_rf.h"
#include "util.h"
#include "hal_rf_util.h"
#include "LCD.h"
#include "stdio.h"

#define SAMPLE_COUNT       8           //采样点数
#define CHANNELS           16          //信道数
#define CHANNEL_11         11          //信道初始值
#define SAMPLE_TIME        2000        // us 抽样时间

// 信号强度的最大值和最小值
#define MIN_RSSI_DBM       -120
#define MAX_RSSI_DBM       -10

//按键
#define HAL_BUTTON_1              1
#define HAL_BUTTON_2              2
#define HAL_BUTTON_3              3
#define HAL_BUTTON_4              4
#define HAL_BUTTON_5              5
#define HAL_BUTTON_6              6

static volatile uint8 txtChannel = 16;              //信道RSSI值显示选择
static int8 ppRssi[CHANNELS][SAMPLE_COUNT];        //保存RSSI值

static const char *channelText[CHANNELS] =          //显示文本
{
    "CH11 ", "CH12 ", "CH13 ", "CH14 ", "CH15 ", "CH16 ", "CH17 ", "CH18 ",
    "CH19 ", "CH20 ", "CH21 ", "CH22 ", "CH23 ", "CH24 ", "CH25 ", "CH26 "
};

extern void halboardinit(void);                      //硬件初始化函数
extern void ctrPCA9554FLASHLED(uint8 led);           //IIC灯控制函数
extern uint8 halkeycmd(void);                        //获取按键值函数
extern void ctrPCA9554LED(uint8 led,uint8 operation);//LED控制函数

/**************************************************************************************************
 * 函数名称：appSetShowText
 *
 * 功能描述：设置RSSI显示信道
 *                     
 *
 * 参    数：无
 *
 * 返 回 值：txtChannel -- 显示信道
 **************************************************************************************************/
uint8 appSetShowText(void)
{
  uint8 key = 0;
  key = halkeycmd();                     //读取按键值
    if (key == HAL_BUTTON_2)             //当SW2按下，显示信道值加一
    {
      txtChannel++;
      if(txtChannel>16)
      {
        txtChannel = 16;
      }
    }
   else if (key == HAL_BUTTON_3)         //当SW2按下，显示信道值减一
    {
      txtChannel--;
      if(txtChannel<1)
      {
        txtChannel = 1;
      }
    }
    return txtChannel;                   // 返回设置值
}

/**************************************************************************************************
 * 函数名称：utilDisplayBarGraph
 *
 * 功能描述：在指定的坐标画矩形
 *                     
 *
 * 参    数：line -- 矩形左上角的x坐标值(不同信道的x坐标)
 *           value -- 矩形左上角的y坐标值（不同信道的矩形高度）
 *
 * 返 回 值：无
 **************************************************************************************************/
void utilDisplayBarGraph(UINT8 line, UINT8 value)
{
  if(value <= 2)
  {
    GUI_RectangleFill((line)*8,62,6+(line)*8,64,1);
  }
  else
  {   
    GUI_RectangleFill((line)*8,64-value,6+(line)*8,64,1);
  }
}

/**************************************************************************************************
 * 函数名称：utilDisplayBarGraphN
 *
 * 功能描述：擦除指定坐标的矩形
 *                     
 *
 * 参    数：line -- 矩形左上角的x坐标值(不同信道的x坐标)
 *           value -- 矩形左上角的y坐标值（不同信道的矩形高度）
 *
 * 返 回 值：无
 **************************************************************************************************/
void utilDisplayBarGraphN(UINT8 line, UINT8 value)
{
  if(value <= 2)
  {
    GUI_RectangleFill((line)*8,62,6+(line)*8,64,0);
  }
  else
  {   
    GUI_RectangleFill((line)*8,64-value,6+(line)*8,64,0);
  }
}

/**************************************************************************************************
 * 函数名称：utilLcdDisplayValue
 *
 * 功能描述：显示各个信道具体的RSSI值
 *                     
 *
 * 参    数：channel -- 所显示的信道
 *           value -- RSSI值
 *
 * 返 回 值：无
 **************************************************************************************************/
void utilLcdDisplayValue(UINT8 channel, INT32 value)
{
  char  s[16];
  value = -value;
  if (value<100)
  {
   sprintf(s, (char*)"-%d%d dBm",  ((INT16)((int)value/10)), ((INT16)((int)value%10))); //将RSSI值转换为字符形式
   GUI_PutString5_7(30,3,(char *)s); //显示RSSI值
   LCM_Refresh();
  }
  else
  {
    sprintf(s, (char*)"-%d%d%ddBm",  ((INT16)((int)value/100)), ((INT16)((int)value%100/10)),((INT16)((int)value%10))); //将RSSI值转换为字符形式
    GUI_PutString5_7(30,3,(char *)s); //显示RSSI值
    LCM_Refresh();
  }
  GUI_PutString5_7(0, 3, (char*)channelText[channel-1]);
 // GUI_PutString5_7(55, 3, "dBm");
}


void utilLcdDisplayMValue(UINT8 channel, INT32 value)
{
  char  s[16];
  value = -value;
  if (value<100)
  {
   sprintf(s, (char*)"-%d%d dBm",  ((INT16)((int)value/10)), ((INT16)((int)value%10))); //将RSSI值转换为字符形式
   GUI_PutString5_7(30,13,(char *)s); //显示RSSI值
   LCM_Refresh();
  }
  else
  {
    sprintf(s, (char*)"-%d%d%ddBm",  ((INT16)((int)value/100)), ((INT16)((int)value%100/10)),((INT16)((int)value%10))); //将RSSI值转换为字符形式
    GUI_PutString5_7(30,13,(char *)s); //显示RSSI值
    LCM_Refresh();
  }
  //GUI_PutString5_7(0, 13, "max:");
  GUI_PutString5_7(0, 13, (char*)channelText[channel-1]);
  LCM_Refresh();
 // GUI_PutString5_7(55, 13, "dBm");
 // GUI_PutString5_7(105, 13, "max");
  
}


/**************************************************************************************************
 * 函数名称：main
 *
 * 功能描述：初始化出差CC2530无线通信的一些基本参数，通过按键选择模块的应用类型（发送或接收）。
 *                               
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void main (void)
{
    int8 minRssi, maxRssi, rssiOffset;
    int16 barValue;
    int16 txtValue;
    uint16 mValue;
    uint8  n;
    int16 barValuesave[16];
    uint8 txtChannelsave;

    halboardinit();                           //初始化板的外围设备(包括LED LCD 和按键等)

    if(halRfInit()==FAILED)                  //初始化RF的一些硬件配置
    {
      ctrPCA9554FLASHLED(5);                 //RF初始化不成功，则所有的LED5闪烁
    }
  
    ctrPCA9554FLASHLED(6);                   //点亮LED6，以指示设备正常运行  

    GUI_PutString5_7(10,22,"Spectrum Anl");  //在LCD上显示相应的文字
    GUI_PutString5_7(10,35,"SW1 -> Start");
    LCM_Refresh();
    while (halkeycmd() != HAL_BUTTON_1);     //等待按键1按下，进入下一级菜单
    halMcuWaitMs(500);                       //延时

    halRfSetGain(HAL_RF_GAIN_HIGH);          //设置RSSI的增益模式和RSSI补偿值

    rssiOffset = halRfGetRssiOffset();       //获得RSSI补偿值 73
    
    minRssi=    MIN_RSSI_DBM + rssiOffset;  //最大值和最小值加上补偿值
    maxRssi=    MAX_RSSI_DBM + rssiOffset;

    halSetRxScanMode();                     //配置RX扫描模式

    GUI_ClearScreen();                     // 清除显示缓冲区中的数据
    LCM_Refresh();
    while(1) 
    {
    	uint8 sample;
                 
        uint16 mbvalue = 500;
        uint8 mbchannel;
        

    	
        for (sample = 0; sample < SAMPLE_COUNT; sample++)                             //每个信道抽取8个值
        {
        	uint8 channel;
            for(channel = 0; channel < CHANNELS; channel++ )                          //扫描16个信道，间隔2000us
            {
                ppRssi[channel][sample] = halSampleED(channel+CHANNEL_11, SAMPLE_TIME);
            }
            

            for(channel = 0; channel < CHANNELS; channel++ )                           //得到barValue显示值，并显示
            {
			
                barValue = -128;
                for (n = 0; n < SAMPLE_COUNT; n++)                                     //取每个频道中8个值的最大值
                {
                    barValue = MAX((int8) barValue, ppRssi[channel][n]);               //获得最大值
                }
                barValue -= minRssi;                                                   //减去最小值

                if (barValue < 0)                                                      //调整最大和最小值
                    barValue = 0;
                if (barValue > ((int16) maxRssi - (int16) minRssi))
                    barValue = (int16) maxRssi - (int16) minRssi;
                
                barValue = barValue*50;                                                //计算显示矩形的高度
                barValue /= maxRssi - minRssi;
                if(barValuesave[channel] != barValue)                                  //判断与上次测试结果是否相同
                {
                  utilDisplayBarGraphN(channel, barValuesave[channel]);                //不相同，更新显示
                  // utilDisplayBarGraphN用来擦除矩形
                  LCM_Refresh();
                }
               /* 按指定坐标画矩形 */
                utilDisplayBarGraph(channel, barValue);//重画矩形
                LCM_Refresh();                                                          // 将显示缓冲区中的数据刷新到SO12864FPD-13ASBE(3S)上显示
                
                barValuesave[channel] = barValue;                                       //保存本次测试值
                
                txtChannelsave = appSetShowText();     //获取需具体显示信道RSSI值
                
                if(barValue < mbvalue){
                  mbvalue = barValue;
                  mbchannel = channel;
                }
          
            }
            
          
            
        } 
           txtValue = -128;
           for (n = 0; n < SAMPLE_COUNT; n++)                                            //得到选定信道的RSSI值
           {
               txtValue = MAX((int8) txtValue, ppRssi[txtChannelsave-1][n]);
            }
            txtValue -= rssiOffset;          
           mValue = -128;
           for (n = 0; n < SAMPLE_COUNT; n++)                                            //得到选定信道的RSSI值
           {
               mValue = MAX((int8) mValue, ppRssi[mbchannel][n]);
            }
            mValue -= rssiOffset;  
           
           utilLcdDisplayValue(txtChannelsave, txtValue);                                //显示选定信道具体的RSSI值 
          // LCM_Refresh();
           utilLcdDisplayMValue(mbchannel,mValue); 
          // LCM_Refresh();
          
    }
}





