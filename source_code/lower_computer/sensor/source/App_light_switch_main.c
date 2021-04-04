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

#define SCL          P1_0     //SHT10ʱ��
#define SDA          P1_1     //SHT10������
#define SEND_DATA_LENGHT 5 // �������ݳ���
/*============================================================= temp add end ====================================================*/


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

static uint8 pTxData[SEND_DATA_LENGHT];  //������������ update for temp
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

static void appLight();                   //��Ӧ�ô�����
static void appSwitch();                  //����Ӧ�ô�����
static uint8 appSelectMode(void);         //Ӧ�ù���ѡ����

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
* �������ƣ�Wait
*
* ������������ʱ����������ȷ��ʱ��
*
* ��    ����ms -- ��ʱʱ��
*
* �� �� ֵ����
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
* �������ƣ�QWait
*
* ������������ʱ��������Լ1us����ʱ��
*
* ��    ������
*
* �� �� ֵ����
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
* �������ƣ�initIO
*
* ����������SHT10����ͨ��IO��ʼ��
*
* ��    ������
*
* �� �� ֵ����
**************************************************************************************************/
void initIO(void)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  P1INP |= 0x03;
  SDA = 1; SCL = 0;
}

/**************************************************************************************************
* �������ƣ�s_write_byte
*
* ������������SHT10дһ���ֽ�
*
* ��    ����value -- ��д����ֽ�ֵ
*
* �� �� ֵ��error -- �����Ƿ�ɹ�
**************************************************************************************************/
char s_write_byte(unsigned char value)
{ 
  unsigned char i,error=0;  
  IO_DIR_PORT_PIN(1, 0, IO_OUT);      //ʱ�Ӻ�����IO����Ϊ���
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  for (i=0x80;i>0;i/=2)               //��һ���ֽڵ�8λ��һ���        
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
  IO_DIR_PORT_PIN(1, 1, IO_IN);      //������������Ϊ���룬��׼������SHT10��ACK
  SCL = 1;  asm("NOP");                          
  error = SDA; 
  QWait();QWait();QWait();
  IO_DIR_PORT_PIN(1, 1, IO_OUT);     //�������߻ָ�Ϊ���״̬
  SDA = 1; 
  SCL = 0;        
  
  return error;                                   
}

/**************************************************************************************************
* �������ƣ�s_read_byte
*
* ������������SHT10��ȡһ���ֽ�
*
* ��    ����ack -- ��ȡ���ݺ���SHT10����ACK
*
* �� �� ֵ��val -- ��ȡ���ֽ�ֵ
**************************************************************************************************/
char s_read_byte(unsigned char ack)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);     //ʱ�Ӻ�����IO����Ϊ���
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  unsigned char i,val=0;
  SDA= 1;
  IO_DIR_PORT_PIN(1, 1, IO_IN);      //������������Ϊ���룬��׼������SHT10������
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
  IO_DIR_PORT_PIN(1, 1, IO_OUT);     //�������߻ָ�Ϊ���״̬
  SDA = !ack;
  SCL = 1;
  QWait();QWait();QWait();QWait();QWait();
  SCL = 0;
  SDA = 1;
  
  return val;                       //���ض�ȡ��ֵ
}

/**************************************************************************************************
* �������ƣ�s_transstart
*
* ��������������SHT10����ʼ��SHT10ͨ��
*
* ��    ������
*
* �� �� ֵ����
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
* �������ƣ�s_connectionreset
*
* ������������SHT10ͨ�Ÿ�λ
*
* ��    ������
*
* �� �� ֵ����
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
* �������ƣ�s_measure
*
* �������������������ȡSHT10�¶Ȼ�ʪ������
*
* ��    ����*p_checksum -- У���
*           mode -- ��ȡ�������ͣ�3Ϊ�¶ȣ�5Ϊʪ�ȣ�
*
* �� �� ֵ��er -- �������
**************************************************************************************************/
char s_measure( unsigned char *p_checksum, unsigned char mode)
{
  unsigned er=0;
  unsigned int i,j;
  s_transstart();                              //��������
  switch(mode)
  {
  case 3	:er+=s_write_byte(3);break;    //�����¶ȶ�ȡ����
  case 5	:er+=s_write_byte(5);break;    //����ʪ�ȶ�ȡ����
  default     :break;
  }
  IO_DIR_PORT_PIN(1, 1, IO_IN);                //������������Ϊ���룬��׼������SHT10��ACK
  for(i=0;i<65535;i++)
  {
    for(j=0;j<65535;j++)
    {if(SDA == 0)
    {break;}}
    if(SDA == 0)
    {break;}
  }
  
  if(SDA)                                     //SDAû�����ͣ�������Ϣ��1
    
  {er += 1;}
  d1 = s_read_byte(ACK);                     //���ݶ�ȡ
  d2 = s_read_byte(ACK);
  d3 = s_read_byte(noACK);
  return er;
}

/**************************************************************************************************
* �������ƣ�th_read
*
* ����������������Ӧ��������ȡ�¶Ⱥ��������ݲ�У��ͼ���
*
* ��    ����*t -- �¶�ֵ
*           *h -- ʪ��ֵ
*
* �� �� ֵ����
**************************************************************************************************/
void th_read(int *t,int *h )
{
  unsigned char error,checksum;
  float humi,temp;
  int tmp;
  initIO();
  
  s_connectionreset();                  //��������
  error=0;
  error+=s_measure(&checksum,5);       //��ȡʪ�����ݲ�У��
  humi = d1*256+d2;
  
  error+=s_measure(&checksum,3);      //��ȡ�¶����ݲ�У��
  temp = d1*256+d2;
  if(error!=0) s_connectionreset();   //��ȡʧ�ܣ�ͨ�Ÿ�λ
  else                                //��ȡ�ɹ�����������
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
  // Main loop
  while (TRUE) 
  {
    while(!basicRfPacketIsReady());      //׼����������
    
    if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0)   //��������
    {
      if(pRxData[0] == LIGHT_TOGGLE_CMD) //�ж������Ƿ���ȷ
      {
        ctrPCA9554FLASHLED(1);        //�رջ��LED1
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
  // temp add start 
  int tempera;
  int humidity;
  char  s[16];
  UINT8 adc0_value[2];
  float num = 0;
  SET_MAIN_CLOCK_SOURCE(CRYSTAL);                          // ����ϵͳʱ��ԴΪ32MHz��������
  
  GUI_Init();                                              // GUI��ʼ��
  GUI_SetColor(1,0);                                       // ��ʾɫΪ���㣬����ɫΪ����
  GUI_PutString5_7(25,6,"OURS-CC2530");                    //��ʾ OURS-CC2530
  GUI_PutString5_7(10,22,"Temp:");                       
  GUI_PutString5_7(10,35,"Humi:");
  GUI_PutString5_7(10,48,"Light:");
  LCM_Refresh();
  // temp add end 
  
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
    // temp update
    //    if(halkeycmd() == HAL_BUTTON_4)         //�ж��Ƿ���SW4
    //    {
    //      basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);//��������
    //      
    //      halIntOff();                                               //�ر�ȫ���ж�
    //      
    //      halIntOn();                                               //���ж�
    //      
    //    }
    th_read(&tempera,&humidity);                            //��ȡ�¶Ⱥ�ʪ��
    sprintf(s, (char*)"%d%d C",  ((INT16)((int)tempera / 10)), ((INT16)((int)tempera % 10)));  //���¶Ƚ��ת��Ϊ�ַ���
    GUI_PutString5_7(48,22,(char *)s);                      //��ʾ���
    LCM_Refresh();
    sprintf(s, (char*)"%d%d %%",  ((INT16)((int)humidity / 10)), ((INT16)((int)humidity % 10)));//��ʪ�Ƚ��ת��Ϊ�ַ���
    GUI_PutString5_7(48,35,(char *)s);                     //��ʾ���
    LCM_Refresh();
    
    /* AIN0ͨ������ */
    ADC_ENABLE_CHANNEL(ADC_AIN0);                          // ʹ��AIN0ΪADC����ͨ��
    
    /* ����ADCCON3�Ĵ����Ա���ADCCON1.STSEL = 11(��λĬ��ֵ)��ADCCON1.ST = 1ʱ���е�һת�� */
    /* �ο���ѹ��AVDD_SOC�����ϵĵ�ѹ */
    /* ��ȡ�ʣ�512                     */
    /* ADC����ͨ����AIN0              */
    ADC_SINGLE_CONVERSION(ADC_REF_AVDD | ADC_14_BIT | ADC_AIN0);
    
    ADC_SAMPLE_SINGLE();                                   // ����һ����һת��
    
    while(!ADC_SAMPLE_READY());                            // �ȴ�ת�����
    
    ADC_ENABLE_CHANNEL(ADC_AIN0);                          // ��ֹAIN0
    
    adc0_value[0] = ADCL;                                      // ��ȡADCֵ
    adc0_value[1] = ADCH;                                      // ��ȡADCֵ
    adc0_value[0] = adc0_value[0]>>2;
    
    num = (adc0_value[1]*256+adc0_value[0])*3.3/8192;  //��һλ����λ,ȡ2^13;
    num /= 4;          
    num=num*913;                                        //ת��ΪLx
    sprintf(s, (char*)"%d%d%d%d lx",  ((INT16)((int)num/1000)), ((INT16)((int)num%1000/100)),((INT16)((int)num%100/10)),((INT16)((int)num%10))); //�����ս��ת��Ϊ�ַ���
    GUI_PutString5_7(48,48,(char *)s);                    //��ʾ���
    LCM_Refresh();
    
    pTxData[1] = (uint8)tempera;
    pTxData[2] = (uint8)humidity;
    pTxData[3] = (uint8)((int)num/1000 + (int)num%1000/100);
    pTxData[4] = (uint8)((int)num%100/10 + (int)num%10);
    basicRfSendPacket(LIGHT_ADDR, pTxData, SEND_DATA_LENGHT);//��������
    halIntOff();                                               //�ر�ȫ���ж�
    halIntOn();                                               //���ж�
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
 

  appSwitch();                         //ִ�з���ģʽ����
}




