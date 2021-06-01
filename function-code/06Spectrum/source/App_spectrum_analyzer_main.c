/**********************************************************************************************************************************************************
* �� �� ����App_spectrum_analyzer_main.c
��
* ��    �ܣ�ʵ��ʮ�� �ŵ��ź�ǿ�Ȳ���ʵ��
*
*            ��ʵ����Ҫ����ѧ��������CC2530RF���ܻ����ϡ����շ���2.4GƵ���ŵ�11-26�����ŵ����ź�ǿ�ȡ�
*            
*         Ȼ��ͨ��LCD��ʾ���Խ�����������ʾ��Ϊ�������֣�һ������ͨ��16����������ʽͬʱ��ʾ�����ŵ�
*
*         �е��ź�ǿ�ȡ���һ����ͨ�����������л���ʾ��ͬ�ŵ������RSSIֵ��
*
*            ���а������ܷ������£�
*                SW1 --- ��ʼ����
*                SW2 --- ��ʾRSSIֵ���ŵ���
*                SW3 --- ��ʾRSSIֵ���ŵ���
*
*
*               ����LED6Ϊ����ָʾ�ƣ�������������ʱ��LED5��Ϊ��״̬��
*
*
*           ��\include\�ļ���\source\�ļ��а����˺�RF��ص�һЩ��ͺ������û�ʹ����Щ��
*           �ͺ������Լ򻯶�CC2530��RF��������ߴ���Ŀɶ��ԣ���ʵ���о�ʹ�������е�һЩ��ͺ�����
*
* ע    �⣺��ʵ������Ӳ����Դ��
*           OURS-CC2530RF�� 1��
*           ��LCD���������� 1��
*           
*           
*
* ��    ����V1.0
* ��    �ߣ�wuxianhai
* ��    �ڣ�2011.2.18
* �¶�˹�Ƽ���ҳ��www.ourselec.com
**********************************************************************************************************************************************************/

#include "hal_board.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_rf.h"
#include "util.h"
#include "hal_rf_util.h"
#include "LCD.h"
#include "stdio.h"

#define SAMPLE_COUNT       8           //��������
#define CHANNELS           16          //�ŵ���
#define CHANNEL_11         11          //�ŵ���ʼֵ
#define SAMPLE_TIME        2000        // us ����ʱ��

// �ź�ǿ�ȵ����ֵ����Сֵ
#define MIN_RSSI_DBM       -120
#define MAX_RSSI_DBM       -10

//����
#define HAL_BUTTON_1              1
#define HAL_BUTTON_2              2
#define HAL_BUTTON_3              3
#define HAL_BUTTON_4              4
#define HAL_BUTTON_5              5
#define HAL_BUTTON_6              6

static volatile uint8 txtChannel = 16;              //�ŵ�RSSIֵ��ʾѡ��
static int8 ppRssi[CHANNELS][SAMPLE_COUNT];        //����RSSIֵ

static const char *channelText[CHANNELS] =          //��ʾ�ı�
{
    "CH11 ", "CH12 ", "CH13 ", "CH14 ", "CH15 ", "CH16 ", "CH17 ", "CH18 ",
    "CH19 ", "CH20 ", "CH21 ", "CH22 ", "CH23 ", "CH24 ", "CH25 ", "CH26 "
};

extern void halboardinit(void);                      //Ӳ����ʼ������
extern void ctrPCA9554FLASHLED(uint8 led);           //IIC�ƿ��ƺ���
extern uint8 halkeycmd(void);                        //��ȡ����ֵ����
extern void ctrPCA9554LED(uint8 led,uint8 operation);//LED���ƺ���

/**************************************************************************************************
 * �������ƣ�appSetShowText
 *
 * ��������������RSSI��ʾ�ŵ�
 *                     
 *
 * ��    ������
 *
 * �� �� ֵ��txtChannel -- ��ʾ�ŵ�
 **************************************************************************************************/
uint8 appSetShowText(void)
{
  uint8 key = 0;
  key = halkeycmd();                     //��ȡ����ֵ
    if (key == HAL_BUTTON_2)             //��SW2���£���ʾ�ŵ�ֵ��һ
    {
      txtChannel++;
      if(txtChannel>16)
      {
        txtChannel = 16;
      }
    }
   else if (key == HAL_BUTTON_3)         //��SW2���£���ʾ�ŵ�ֵ��һ
    {
      txtChannel--;
      if(txtChannel<1)
      {
        txtChannel = 1;
      }
    }
    return txtChannel;                   // ��������ֵ
}

/**************************************************************************************************
 * �������ƣ�utilDisplayBarGraph
 *
 * ������������ָ�������껭����
 *                     
 *
 * ��    ����line -- �������Ͻǵ�x����ֵ(��ͬ�ŵ���x����)
 *           value -- �������Ͻǵ�y����ֵ����ͬ�ŵ��ľ��θ߶ȣ�
 *
 * �� �� ֵ����
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
 * �������ƣ�utilDisplayBarGraphN
 *
 * ��������������ָ������ľ���
 *                     
 *
 * ��    ����line -- �������Ͻǵ�x����ֵ(��ͬ�ŵ���x����)
 *           value -- �������Ͻǵ�y����ֵ����ͬ�ŵ��ľ��θ߶ȣ�
 *
 * �� �� ֵ����
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
 * �������ƣ�utilLcdDisplayValue
 *
 * ������������ʾ�����ŵ������RSSIֵ
 *                     
 *
 * ��    ����channel -- ����ʾ���ŵ�
 *           value -- RSSIֵ
 *
 * �� �� ֵ����
 **************************************************************************************************/
void utilLcdDisplayValue(UINT8 channel, INT32 value)
{
  char  s[16];
  value = -value;
  if (value<100)
  {
   sprintf(s, (char*)"-%d%d dBm",  ((INT16)((int)value/10)), ((INT16)((int)value%10))); //��RSSIֵת��Ϊ�ַ���ʽ
   GUI_PutString5_7(30,3,(char *)s); //��ʾRSSIֵ
   LCM_Refresh();
  }
  else
  {
    sprintf(s, (char*)"-%d%d%ddBm",  ((INT16)((int)value/100)), ((INT16)((int)value%100/10)),((INT16)((int)value%10))); //��RSSIֵת��Ϊ�ַ���ʽ
    GUI_PutString5_7(30,3,(char *)s); //��ʾRSSIֵ
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
   sprintf(s, (char*)"-%d%d dBm",  ((INT16)((int)value/10)), ((INT16)((int)value%10))); //��RSSIֵת��Ϊ�ַ���ʽ
   GUI_PutString5_7(30,13,(char *)s); //��ʾRSSIֵ
   LCM_Refresh();
  }
  else
  {
    sprintf(s, (char*)"-%d%d%ddBm",  ((INT16)((int)value/100)), ((INT16)((int)value%100/10)),((INT16)((int)value%10))); //��RSSIֵת��Ϊ�ַ���ʽ
    GUI_PutString5_7(30,13,(char *)s); //��ʾRSSIֵ
    LCM_Refresh();
  }
  //GUI_PutString5_7(0, 13, "max:");
  GUI_PutString5_7(0, 13, (char*)channelText[channel-1]);
  LCM_Refresh();
 // GUI_PutString5_7(55, 13, "dBm");
 // GUI_PutString5_7(105, 13, "max");
  
}


/**************************************************************************************************
 * �������ƣ�main
 *
 * ������������ʼ������CC2530����ͨ�ŵ�һЩ����������ͨ������ѡ��ģ���Ӧ�����ͣ����ͻ���գ���
 *                               
 *
 * ��    ������
 *
 * �� �� ֵ����
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

    halboardinit();                           //��ʼ�������Χ�豸(����LED LCD �Ͱ�����)

    if(halRfInit()==FAILED)                  //��ʼ��RF��һЩӲ������
    {
      ctrPCA9554FLASHLED(5);                 //RF��ʼ�����ɹ��������е�LED5��˸
    }
  
    ctrPCA9554FLASHLED(6);                   //����LED6����ָʾ�豸��������  

    GUI_PutString5_7(10,22,"Spectrum Anl");  //��LCD����ʾ��Ӧ������
    GUI_PutString5_7(10,35,"SW1 -> Start");
    LCM_Refresh();
    while (halkeycmd() != HAL_BUTTON_1);     //�ȴ�����1���£�������һ���˵�
    halMcuWaitMs(500);                       //��ʱ

    halRfSetGain(HAL_RF_GAIN_HIGH);          //����RSSI������ģʽ��RSSI����ֵ

    rssiOffset = halRfGetRssiOffset();       //���RSSI����ֵ 73
    
    minRssi=    MIN_RSSI_DBM + rssiOffset;  //���ֵ����Сֵ���ϲ���ֵ
    maxRssi=    MAX_RSSI_DBM + rssiOffset;

    halSetRxScanMode();                     //����RXɨ��ģʽ

    GUI_ClearScreen();                     // �����ʾ�������е�����
    LCM_Refresh();
    while(1) 
    {
    	uint8 sample;
                 
        uint16 mbvalue = 500;
        uint8 mbchannel;
        

    	
        for (sample = 0; sample < SAMPLE_COUNT; sample++)                             //ÿ���ŵ���ȡ8��ֵ
        {
        	uint8 channel;
            for(channel = 0; channel < CHANNELS; channel++ )                          //ɨ��16���ŵ������2000us
            {
                ppRssi[channel][sample] = halSampleED(channel+CHANNEL_11, SAMPLE_TIME);
            }
            

            for(channel = 0; channel < CHANNELS; channel++ )                           //�õ�barValue��ʾֵ������ʾ
            {
			
                barValue = -128;
                for (n = 0; n < SAMPLE_COUNT; n++)                                     //ȡÿ��Ƶ����8��ֵ�����ֵ
                {
                    barValue = MAX((int8) barValue, ppRssi[channel][n]);               //������ֵ
                }
                barValue -= minRssi;                                                   //��ȥ��Сֵ

                if (barValue < 0)                                                      //����������Сֵ
                    barValue = 0;
                if (barValue > ((int16) maxRssi - (int16) minRssi))
                    barValue = (int16) maxRssi - (int16) minRssi;
                
                barValue = barValue*50;                                                //������ʾ���εĸ߶�
                barValue /= maxRssi - minRssi;
                if(barValuesave[channel] != barValue)                                  //�ж����ϴβ��Խ���Ƿ���ͬ
                {
                  utilDisplayBarGraphN(channel, barValuesave[channel]);                //����ͬ��������ʾ
                  // utilDisplayBarGraphN������������
                  LCM_Refresh();
                }
               /* ��ָ�����껭���� */
                utilDisplayBarGraph(channel, barValue);//�ػ�����
                LCM_Refresh();                                                          // ����ʾ�������е�����ˢ�µ�SO12864FPD-13ASBE(3S)����ʾ
                
                barValuesave[channel] = barValue;                                       //���汾�β���ֵ
                
                txtChannelsave = appSetShowText();     //��ȡ�������ʾ�ŵ�RSSIֵ
                
                if(barValue < mbvalue){
                  mbvalue = barValue;
                  mbchannel = channel;
                }
          
            }
            
          
            
        } 
           txtValue = -128;
           for (n = 0; n < SAMPLE_COUNT; n++)                                            //�õ�ѡ���ŵ���RSSIֵ
           {
               txtValue = MAX((int8) txtValue, ppRssi[txtChannelsave-1][n]);
            }
            txtValue -= rssiOffset;          
           mValue = -128;
           for (n = 0; n < SAMPLE_COUNT; n++)                                            //�õ�ѡ���ŵ���RSSIֵ
           {
               mValue = MAX((int8) mValue, ppRssi[mbchannel][n]);
            }
            mValue -= rssiOffset;  
           
           utilLcdDisplayValue(txtChannelsave, txtValue);                                //��ʾѡ���ŵ������RSSIֵ 
          // LCM_Refresh();
           utilLcdDisplayMValue(mbchannel,mValue); 
          // LCM_Refresh();
          
    }
}





