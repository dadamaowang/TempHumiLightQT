/**********************************************************************************************************************************************************
* �� �� ����App_light_switch_main.c
��
* ��    �ܣ�ʵ��ʮһ ��RFʵ��
*
*            ��ʵ����Ҫ��ѧϰ��ô����CC2530RF���ܡ���ʵ����Ҫ��Ϊ3�󲿷֣���һ����Ϊ��ʼ����RF��ص���Ϣ��
*            
*          �ڶ�����Ϊ�������ݺͽ������ݣ����Ϊѡ��ģ�鹦�ܺ���������ģ�鹦�ܵ�ѡ����ͨ���������ϵİ�����
*          
*          ѡ��ģ����а������ܷ������£�
*                SW1 --- ��ʼ���ԣ����빦��ѡ��˵���
*                SW2 --- ����ģ��Ϊ���չ��ܣ�Light��
*                SW3 --- ����ģ��Ϊ���͹��ܣ�Switch��
*                SW4 --- ����ģ�鷢�������
*
*               ������ģ�鰴��SW4ʱ��������һ�������������ģ���ڽ��յ�������󣬽�����LDE1����������
*
*               ����LED6Ϊ����ָʾ�ƣ�������������ʱ��LED5��Ϊ��״̬��
*
*
*           ��\include\�ļ���\source\�ļ��а����˺�RF��ص�һЩ��ͺ������û�ʹ����Щ��
*           �ͺ������Լ򻯶�CC2530��RF��������ߴ���Ŀɶ��ԣ���ʵ���о�ʹ�������е�һЩ��ͺ�����
*
* ע    �⣺��ʵ������Ӳ����Դ��
*           OURS-CC2530RF�� 2��
*           ��LCD���������� 2��
*           
*           
*
* ��    ����V1.0
* ��    �ߣ�wuxianhai
* ��    �ڣ�2011.2.15
* �¶�˹�Ƽ���ҳ��www.ourselec.com
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

#define RF_CHANNEL                25      // 2.4 GHz RF ʹ���ŵ�25

#define PAN_ID                0x2011      //ͨ��PANID
#define SWITCH_ADDR           0x2530      //����ģ���ַ
#define LIGHT_ADDR            0xBEEF      //��ģ���ַ
#define APP_PAYLOAD_LENGTH        1       //�����
#define LIGHT_TOGGLE_CMD          0       //��������

// Ӧ��״̬
#define IDLE                      0
#define SEND_CMD                  1

//Ӧ�ý�ɫ
#define NONE                      0      
#define SWITCH                    1
#define LIGHT                     2
#define APP_MODES                 2

//����
#define HAL_BUTTON_1              1
#define HAL_BUTTON_2              2
#define HAL_BUTTON_3              3
#define HAL_BUTTON_4              4
#define HAL_BUTTON_5              5
#define HAL_BUTTON_6              6

/* uart add start */
/* ����HAL_ISR_FUNCTION(f,v)��������CC2530��ISR������f ΪISR�����ƣ�vΪ�ж�����*/
#define _PRAGMA(x) _Pragma(#x)
#define HAL_ISR_FUNC_DECLARATION(f,v)   _PRAGMA(vector=v) __near_func __interrupt void f(void)
#define HAL_ISR_FUNC_PROTOTYPE(f,v)     _PRAGMA(vector=v) __near_func __interrupt void f(void)
#define HAL_ISR_FUNCTION(f,v)           HAL_ISR_FUNC_PROTOTYPE(f,v); HAL_ISR_FUNC_DECLARATION(f,v)
#define RECEIVE_DATA_LENGTH 5
UINT8 *str = 0;    //���ڽ�������ָ��
UINT16 count = 0;   //���ڽ������ݼ���

char str_receive[16];
char str_temp[16];

/* uart add end */

#define OSC_32KHZ  0x00                //ʹ���ⲿ32K��������
//ʱ�����ú���
#define HAL_BOARD_INIT()                                         \
{                                                                \
  uint16 i;                                                      \
    \
      SLEEPCMD &= ~OSC_PD;                       /* ���� 16MHz RC ��32MHz XOSC */         \
        while (!(SLEEPSTA & XOSC_STB));            /* �ȴ� 32MHz XOSC �ȶ� */               \
          asm("NOP");                                                                         \
            for (i=0; i<504; i++) asm("NOP");          /* ��ʱ63us*/                            \
              CLKCONCMD = (CLKCONCMD_32MHZ | OSC_32KHZ); /* ���� 32MHz XOSC �� 32K ʱ�� */        \
                while (CLKCONSTA != (CLKCONCMD_32MHZ | OSC_32KHZ)); /* �ȴ�ʱ����Ч*/               \
                  SLEEPCMD |= OSC_PD;                        /* �ر� 16MHz RC */                      \
}

static uint8 pTxData[APP_PAYLOAD_LENGTH];  //������������

//static uint8 pRxData[APP_PAYLOAD_LENGTH];  //������������ uart update
static uint8 pRxData[RECEIVE_DATA_LENGTH];  //������������

static basicRfCfg_t basicRfConfig;         //RF��ʼ���ṹ��

extern void halboardinit(void);            //Ӳ����ʼ������
extern void ctrPCA9554FLASHLED(uint8 led); //IIC�ƿ��ƺ���
extern void ctrPCA9554LED(uint8 led,uint8 operation);
extern uint8 halkeycmd(void);              //��ȡ����ֵ����



#ifdef SECURITY_CCM                        //��ȫ��Կ
static uint8 key[]= {
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
  0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
};
#endif

static void appLight();                   //��Ӧ�ô�����
static void appSwitch();                  //����Ӧ�ô�����
static uint8 appSelectMode(void);         //Ӧ�ù���ѡ����

/* ------------------------------------uart add start-------------------------------------------- */
/**************************************************************************************************
* �������ƣ�halWait
*
* ������������ʱ����������ȷ��ʱ��
*
* ��    ����wait -- ��ʱʱ��
*
* �� �� ֵ����
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
* �������ƣ�initUART
*
* ����������CC2530���ڳ�ʼ��
*
* ��    ������
*
* �� �� ֵ����
**************************************************************************************************/
void initUART(void)
{
  
  PERCFG = 0x00;				//λ��1 P0��
  P0SEL = 0x3c;				//P0��������
  
  U0CSR |= 0x80;				//UART��ʽ
  U0GCR |= 11;				//baud_e = 11;
  U0BAUD |= 216;				//��������Ϊ115200
  UTX0IF = 1;
  
  U0CSR |= 0X40;				//�������
  IEN0 |= 0x84;				//uart0�����ж�
}

/**************************************************************************************************
* �������ƣ�UartTX_Send_String
*
* �������������ڷ������ݺ���
*
* ��    ����*Data --- ��������ָ��
*            len  --- ���͵����ݳ���
*
* �� �� ֵ����
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
* �������ƣ�HAL_ISR_FUNCTION
*
* �������������ڽ��������жϺ���
*
* ��    ����halUart0RxIsr --- �ж�����
*           URX0_VECTOR  --- �ж�����
*
* �� �� ֵ����
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
* �������ƣ�appLight
*
* ��������������ģʽӦ�ú�������ʼ��RFһЩ������������һ��ģ�鷢�͵Ŀ������Ȼ�������Ӧ��LED��
*                     
*
* ��    ������
*
* �� �� ֵ����
**************************************************************************************************/
static void appLight()
{
 //uart add start
    char temperature[2] = "20";
    char humidity[2] = "21";
    char light[4] = "0319";

  SET_MAIN_CLOCK_SOURCE(CRYSTAL);          //������ʱ��Ϊ32M����
  //initUART();                              //��ʼ������
  
 
  //uart add end
  
  basicRfConfig.myAddr = LIGHT_ADDR;       //���ý���ģ��ĵ�ַ
  if(basicRfInit(&basicRfConfig)==FAILED)  //RF��ʼ��
  {
    ctrPCA9554FLASHLED(5);                 //RF��ʼ�����ɹ��������е�LED5��˸
  }
  basicRfReceiveOn();                      //�򿪽��չ���
  
  initUART();
  
  // Main loop
  while (TRUE) 
  {
    while(!basicRfPacketIsReady());      //׼����������
    // uart update start
    //    if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //��������
    //    {
    //      if(pRxData[0] == LIGHT_TOGGLE_CMD) //�ж������Ƿ���ȷ
    //      {
    //        ctrPCA9554FLASHLED(1);        //�رջ��LED1
    //      }
    //    }
    if(basicRfReceive(pRxData, RECEIVE_DATA_LENGTH, NULL)>0)   //��������
    {
      if(pRxData[0] == LIGHT_TOGGLE_CMD) //�ж������Ƿ���ȷ
      {
        int temp = pRxData[1];
        int humi = pRxData[2];
        int light_h = pRxData[3];
        int light_l = pRxData[4];
        
        // �¶�����ת�ַ�
        int ones = temp % 10;
        int tens = temp / 10;
        temperature[0] = tens + '0';
        temperature[1] = ones + '0';
        
        // ʪ������ת�ַ�
        ones = humi % 10;
        tens = humi / 10;
        humidity[0] = tens + '0';
        humidity[1] = ones + '0';
        
        // ��������ת�ַ�
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
        UartTX_Send_String(uartch1, sizeof(temperature));         //USB����
        UartTX_Send_String(uartch2, sizeof(humidity));
        UartTX_Send_String(uartch3, sizeof(light));

        ctrPCA9554FLASHLED(1);        //�رջ��LED1
      }
    }//if��������

    
    //below are LED control
    //SET_MAIN_CLOCK_SOURCE(CRYSTAL);                          // ����ϵͳʱ��ԴΪ32MHz��������
    halWait(200);
    halWait(200);
    UINT8 temp;
    
    memset(str_receive, 0, 16);
   
    memset(str_temp, 0, 16);
    if (count)                              //�жϴ����Ƿ���յ�����
    {
        temp = count;    //�ݴ���ܵ������ݳ���
        halWait(50); //�ȴ����ݽ������
        if (temp == count)   //���յ������ݳ��Ȳ��ٱ仯����ɽ�������
        {
         // if(str[0] == 30){   // �յ���λ��������Ϣ
          memset(str_receive, 0, 16);
          strncpy(str_receive, str + 2, str[1]);
          memset(str_temp, 0, 16);
          sprintf(str_temp, (char *)"%-12s", str_receive);
          GUI_PutString5_7(48,45,(char *)str_temp);                    //��ʾ������Ϣ
          LCM_Refresh();
        //}
            
        for (int k = 0; k < 6; ++k) {//�յ�������ȫ��
                ctrPCA9554LED(k, 1);
            }

          // UartTX_Send_String(str, count); //�ش����յ�������

             //��λstr��count
            str = 0;
            count = 0;
        } //if_temp_count    

    }//if_count
    Wait(400);
    PCA9554ledInit();  //��ʼ��LED
    for (int k = 0; k < 6; ++k) {
        ctrPCA9554LED(k, 0);
    }
    Wait(200);
    
    
    
    
    
  }//while
}

/**************************************************************************************************
* �������ƣ�appSwitch
*
* ��������������ģʽӦ�ú�������ʼ������ģʽRF��ͨ������SW4����һ��ģ�鷢�Ϳ������
*                     
*
* ��    ������
*
* �� �� ֵ����
**************************************************************************************************/
static void appSwitch()
{
  pTxData[0] = LIGHT_TOGGLE_CMD;              //����������д������
  
  basicRfConfig.myAddr = SWITCH_ADDR;         //���÷���ģ��ĵ�ַ
  
  if(basicRfInit(&basicRfConfig)==FAILED)     //RF��ʼ��
  {
    ctrPCA9554FLASHLED(5);                    //RF��ʼ�����ɹ��������е�LED5��˸
  }
  
  basicRfReceiveOff();                        //�رս��չ���
  // Main loop
  while (TRUE) 
  {
    if(halkeycmd() == HAL_BUTTON_4)         //�ж��Ƿ���SW4
    {
      basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//��������
      
      halIntOff();                                               //�ر�ȫ���ж�
      
      halIntOn();                                               //���ж�
      
    }
  }
}

/**************************************************************************************************
* �������ƣ�appSelectMode
*
* ����������ͨ��SW2��SW3ѡ��ģ���Ӧ��ģʽ��
*                     
*
* ��    ������
*
* �� �� ֵ��LIGHT -- ����ģʽ
*           SWITCH -- ����ģʽ
*           NONE -- ����ȷģʽ
*
**************************************************************************************************/
static uint8 appSelectMode(void)        
{
  uint8 key;
  GUI_ClearScreen();                             //LCD����
  GUI_PutString5_7(25,6,"OURS-CC2530");          //��LCD����ʾ��Ӧ������             
  GUI_PutString5_7(10,22,"Device Mode: ");                          
  GUI_PutString5_7(10,35,"SW2 -> Light");
  GUI_PutString5_7(10,48,"SW3 -> Switch");
  LCM_Refresh();
  do 
  {
    key = halkeycmd();
  }while(key == HAL_BUTTON_1);                    //�ȴ�ģʽѡ��
  if(key == HAL_BUTTON_2)                      //����ģʽ
  {
    GUI_ClearScreen();
    GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������              
    GUI_PutString5_7(10,22,"Device Mode: ");                          
    GUI_PutString5_7(10,35,"Light");
    LCM_Refresh();
    
    return LIGHT;
  }
  if(key == HAL_BUTTON_3)                     //����ģʽ
  {
    GUI_ClearScreen();
    GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������          
    GUI_PutString5_7(10,22,"Device Mode: ");                            
    GUI_PutString5_7(10,35,"Switch");
    GUI_PutString5_7(10,48,"SW4 Send Command");
    LCM_Refresh();
    
    return SWITCH;
  }
  return NONE;        
}

/**************************************************************************************************
* �������ƣ�main
*
* ����������ͨ����ͬ�İ���������ģ���Ӧ�ý�ɫ������ģʽ����ģʽ����ͨ��SW4���Ϳ�������
*                     
*
* ��    ������
*
* �� �� ֵ����
**************************************************************************************************/
void main(void)
{
  uint8 appMode = NONE;                     //Ӧ��ְ�𣨽�ɫ����ʼ��
  
  basicRfConfig.panId = PAN_ID;             //����PANID  2011
  basicRfConfig.channel = RF_CHANNEL;       //�����ŵ�   25
  basicRfConfig.ackRequest = TRUE;          //��ҪACK����
  
#ifdef SECURITY_CCM                           //����ѡ�δѡ��
  basicRfConfig.securityKey = key;          // ��ȫ��Կ
#endif
  
  halboardinit();                           //��ʼ�������Χ�豸(����LED LCD �Ͱ�����)
  
  if(halRfInit()==FAILED)                  //��ʼ��RF
  {                    
    ctrPCA9554FLASHLED(5);                 //RF��ʼ�����ɹ��������е�LED5��˸
  }
  
  ctrPCA9554FLASHLED(6);                   //����LED6����ָʾ�豸��������       
  
  // uart update start
  //  GUI_PutString5_7(10,22,"Simple RF test");//��LCD����ʾ��Ӧ������
  //  GUI_PutString5_7(10,35,"SW1 -> Start");
  //  LCM_Refresh();
  //  
  //  while (halkeycmd() != HAL_BUTTON_1);     //�ȴ�����1���£�������һ���˵�
  //  halMcuWaitMs(350);                       //��ʱ350MS
  //  
  //  appMode = appSelectMode();               //����Ӧ��ְ�𣨽�ɫ�� ͬʱ��LCD����ʾ��Ӧ��������Ϣ
  //  
  //  if(appMode == SWITCH)                    //����ģʽ
  //  {
  //    ctrPCA9554LED(2,1);
  //    appSwitch();                         //ִ�з���ģʽ����
  //  }
  //  else if(appMode == LIGHT)                //����ģʽ
  //  {
  //    ctrPCA9554LED(3,1);
  //    appLight();                         //ִ�н���ģʽ����
  //  }
  GUI_Init();   
  // GUI��ʼ��
  GUI_SetColor(1,0);                                       // ��ʾɫΪ���㣬����ɫΪ����
  GUI_PutString5_7(10,6,"Coordinator");//��LCD����ʾ��Ӧ������
  GUI_PutString5_7(10,19,"Receive Mode");
  GUI_PutString5_7(10,32,"Send data to upper computer by COM.");
  GUI_PutString5_7(10,45,"Msg:");
  LCM_Refresh();
  ctrPCA9554LED(3,1); // ����LED����

  
  PCA9554ledInit();  //��ʼ��LED
  for (UINT8 i = 0; i < 6; i++)     // �ص�
  {
      ctrPCA9554LED(i, 0);
  }

  appLight();                         //ִ�н���ģʽ����
  // uart update end
}




