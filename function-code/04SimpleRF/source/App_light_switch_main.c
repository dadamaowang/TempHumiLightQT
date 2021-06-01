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
#include "hal_board.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "LCD.h"

#define RF_CHANNEL                11      // 2.4 GHz RF ʹ���ŵ�11

#define PAN_ID                0x0020      //ͨ��PANID
#define SWITCH_ADDR           0x0140      //����ģ���ַ
#define LIGHT_ADDR            0x0141      //��ģ���ַ
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

static uint8 pTxData[APP_PAYLOAD_LENGTH];  //������������
static uint8 pRxData[APP_PAYLOAD_LENGTH];  //������������
static basicRfCfg_t basicRfConfig;         //RF��ʼ���ṹ��

extern void halboardinit(void);            //Ӳ����ʼ������
extern void ctrPCA9554FLASHLED(uint8 led); //IIC�ƿ��ƺ���
extern void ctrPCA9554LED(uint8 led,uint8 operation);
extern uint8 halkeycmd(void);              //��ȡ����ֵ����

extern uint8 getseq;

#ifdef SECURITY_CCM                        //��ȫ��Կ
static uint8 key[]= {
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
};
#endif

static void appLight();                   //��Ӧ�ô�����
static void appSwitch();                  //����Ӧ�ô�����
static uint8 appSelectMode(void);         //Ӧ�ù���ѡ����

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
    basicRfConfig.myAddr = LIGHT_ADDR;       //���ý���ģ��ĵ�ַ
    if(basicRfInit(&basicRfConfig)==FAILED)  //RF��ʼ��
    {
      ctrPCA9554FLASHLED(5);                 //RF��ʼ�����ɹ��������е�LED5��˸
    }
    basicRfReceiveOn();                      //�򿪽��չ���
  
    
    int count=0;
    char countb[16];
    char seq[16];
 
    
    // Main loop
    while (TRUE) 
    {
        while(!basicRfPacketIsReady());      //׼����������

        if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //��������
        {
         
            
                count++;
                ctrPCA9554FLASHLED(1);        //�رջ��LED1
                
                
                
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
        int key = halkeycmd();
        if(key == HAL_BUTTON_4 )         //�ж��Ƿ���SW4
        {
            pTxData[0] =0 ;
                 
            ctrPCA9554FLASHLED(2);        //�رջ��LED1
            
                  
            basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//��������

            halIntOff();                                               //�ر�ȫ���ж�

            halIntOn(); }
         else if(key == HAL_BUTTON_5){
            
       
             pTxData[0] =1; 
                 
              ctrPCA9554FLASHLED(2);        //�رջ��LED1
         
              
            basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//��������

            halIntOff();                                               //�ر�ȫ���ж�

            halIntOn();  
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
      GUI_PutString5_7(10,48,"PUSH SW4/SW5");
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

    basicRfConfig.panId = PAN_ID;             //����PANID  
    basicRfConfig.channel = RF_CHANNEL;       //�����ŵ�   
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
    
    GUI_PutString5_7(10,22,"Simple RF test");//��LCD����ʾ��Ӧ������
    GUI_PutString5_7(10,35,"SW1 -> Start");
    LCM_Refresh();

    while (halkeycmd() != HAL_BUTTON_1);     //�ȴ�����1���£�������һ���˵�
    halMcuWaitMs(350);                       //��ʱ350MS

    appMode = appSelectMode();               //����Ӧ��ְ�𣨽�ɫ�� ͬʱ��LCD����ʾ��Ӧ��������Ϣ

    if(appMode == SWITCH)                    //����ģʽ
    {
        ctrPCA9554LED(2,1);
        appSwitch();                         //ִ�з���ģʽ����
    }
    else if(appMode == LIGHT)                //����ģʽ
    {
        ctrPCA9554LED(3,1);
        appLight();                         //ִ�н���ģʽ����
    }
}




