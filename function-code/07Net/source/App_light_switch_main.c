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

#define RF_CHANNEL                17     

#define PAN_ID                32      //ͨ��PANID

#define A_ADDR           320     
#define B_ADDR           321
#define C_ADDR           322
#define D_ADDR           323    
#define E_ADDR           324
#define F_ADDR           325

#define APP_PAYLOAD_LENGTH        3       //�����
#define LIGHT_TOGGLE_CMD          0       //��������
#define LIGHTENING                1       //��������

// Ӧ��״̬
#define IDLE                      0
#define SEND_CMD                  1

//Ӧ�ý�ɫ
#define NONE                     7      
#define ANODE                    1
#define BNODE                    2
#define CNODE                    3
#define DNODE                    4
#define ENODE                    5
#define FNODE                    6
#define APP_MODES                7

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



#ifdef SECURITY_CCM                        //��ȫ��Կ
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
static uint8 appSelectMode(void);         //Ӧ�ù���ѡ����


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
    basicRfConfig.myAddr = node;       //���ý���ģ��ĵ�ַ
    uint8 me = node % 16;
    
    if(basicRfInit(&basicRfConfig)==FAILED)  //RF��ʼ��
    {
      ctrPCA9554FLASHLED(me+1);                 //RF��ʼ�����ɹ��������е�LED5��˸
    }
    
    ctrPCA9554LED(me,1);
    
    
    
   
    
    
    
    while(1){
    
         
      basicRfReceiveOn();                      //�򿪽��չ���
    
      uint8 key = halkeycmd();
    
      while(!basicRfPacketIsReady() && key != 4){      //׼����������
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
        
        
        basicRfReceiveOff();                        //�رս��չ���
       

        
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
        basicRfSendPacket(router_table[0][key-1], pTxData, APP_PAYLOAD_LENGTH);//��������

        halIntOff();                                               //�ر�ȫ���ж�

        halIntOn();                                               //���ж�
        
        
        
      }else{  //recieve
        uint16 addr;
        uint8 reci;
       
        if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //��������
        {
          addr = (pRxData[1] << 8) + pRxData[0];
          
          reci = addr - 320;
          
          
          if(reci == me && pRxData[2] == LIGHTENING){
             for(int i=0;i<=5;++i)ctrPCA9554LED(i,1);
           
         }else {//transmit
             basicRfReceiveOff();                        //�رս��չ���
      
          
            pTxData[0] = pRxData[0];
            pTxData[1] = pRxData[1];
            pTxData[2] = pRxData[2];
           ctrPCA9554FLASHLED(reci+1);   
          
            basicRfSendPacket(router_table[me][reci], pTxData, APP_PAYLOAD_LENGTH);//��������

           halIntOff();                                               //�ر�ȫ���ж�

           halIntOn();                                               //���ж�
        

          }
          
        }
    
      }//else
     
    }//while 1
    
}

/*

static void appSwitch()
{
 
  for(int i=0;i<=5;++i)ctrPCA9554LED(i,0);
   
  basicRfConfig.myAddr = A_ADDR;       //���ý���ģ��ĵ�ַ

    if(basicRfInit(&basicRfConfig)==FAILED)  //RF��ʼ��
    {
      ctrPCA9554FLASHLED(1);                 //RF��ʼ�����ɹ��������е�LED5��˸
    }
    
   ctrPCA9554LED(0,1);
    
    while(1){
    
      
      
      basicRfReceiveOn();                      //�򿪽��չ���
    
      uint8 key = halkeycmd();
    
      while(!basicRfPacketIsReady() && key != 4){      //׼����������
          key = halkeycmd();
          halMcuWaitMs(350);
      }
       
      for(int i=0;i<=5;++i)ctrPCA9554LED(i,0);
    
      if(key == 4){//send
        
        GUI_ClearScreen();                            
        GUI_PutString5_7(25,6,"SEND");          
        LCM_Refresh();       
     
        
        ctrPCA9554LED(0,1);
        
        basicRfReceiveOff();                        //�رս��չ���
    
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
        basicRfSendPacket(router_table[0][key-1], pTxData, APP_PAYLOAD_LENGTH);//��������

        halIntOff();                                               //�ر�ȫ���ж�

        halIntOn();                                               //���ж�
        
        
      }
      
    }//while 1
        
}

*/

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
    halMcuWaitMs(350);                       //��ʱ350MS

 }while(key < 1 || key > 6);                    //�ȴ�ģʽѡ��
 
 ctrPCA9554FLASHLED(5);
  ctrPCA9554FLASHLED(4);
  ctrPCA9554FLASHLED(3);
 
    if(key == HAL_BUTTON_1)                      //����ģʽ
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"A");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return ANODE;
    }
    if(key == HAL_BUTTON_2)                      //����ģʽ
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"B");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return BNODE;
    }
     if(key == HAL_BUTTON_3)                     //����ģʽ
    {
      
    GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"C");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return CNODE;
    }
    if(key == HAL_BUTTON_4)                      //����ģʽ
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"D");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return DNODE;
    }
    if(key == HAL_BUTTON_5)                      //����ģʽ
    {
      GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"E");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return ENODE;
    }
     if(key == HAL_BUTTON_6)                     //����ģʽ
    {
     GUI_ClearScreen();
      GUI_PutString5_7(25,6,"OURS-CC2530");      //��LCD����ʾ��Ӧ������              
      GUI_PutString5_7(10,22,"Device Mode: ");                          
      GUI_PutString5_7(10,35,"F");
      GUI_PutString5_7(10,48,"SW4 Send ");
      LCM_Refresh();
      
      return FNODE;
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
    
    GUI_PutString5_7(10,22,"Network");//��LCD����ʾ��Ӧ������
    GUI_PutString5_7(10,35,"SW1 -> Start");
    LCM_Refresh();
    

    while (halkeycmd() != HAL_BUTTON_1);     //�ȴ�����1���£�������һ���˵�
    halMcuWaitMs(350);                       //��ʱ350MS

    appMode = appSelectMode();               //����Ӧ��ְ�𣨽�ɫ�� ͬʱ��LCD����ʾ��Ӧ��������Ϣ

    if(appMode == ANODE)                    //����ģʽ
    {
       ctrPCA9554FLASHLED(6);            
        //ctrPCA9554LED(1,1);
           appLight(A_ADDR); 
    }
    else if(appMode == BNODE)                //����ģʽ
    {
             ctrPCA9554FLASHLED(6);            
        //ctrPCA9554LED(2,1);
        appLight(B_ADDR);                         //ִ�н���ģʽ����
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




