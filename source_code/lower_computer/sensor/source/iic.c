/**********************************************************************************************************
* �� �� ����iic.C
* ��    �ܣ�
*            ��ʵ�����CC2530��I/O�ڣ�P1.0��P1.1��ģ��IIC���ߵ�SCL��SDA��Ȼ��ͨ��IIC������ʽ����GPIO��չоƬ
*            PCA9554�����ͨ����չ��IO������LED������
*
* Ӳ�����ӣ���OURS��CC2530RFģ����뵽��ͨ��ذ�����ܵ�ذ��ϡ�
*
*           P1.0 ------ SCL
*           P1.1 ------ SDA
*           
* ��    ����V1.0
* ��    �ߣ�WUXIANHAI
* ��    �ڣ�2011.1.18
* �¶�˹������ҳ��www.ourselec.com
**************************************************************************************************************/

#include "ioCC2530.h" 
#include "hal_mcu.h"

#define SCL          P1_0       //IICʱ����
#define SDA          P1_1       //IIC������

#define ON           0x01       //LED״̬
#define OFF          0x00


//����IO������ƺ���
#define IO_DIR_PORT_PIN(port, pin, dir)  \
   do {                                  \
      if (dir == IO_OUT)                 \
         P##port##DIR |= (0x01<<(pin));  \
      else                               \
         P##port##DIR &= ~(0x01<<(pin)); \
   }while(0)

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

#define IO_IN   0           //����
#define IO_OUT  1           //���

uint8 ack;	            //Ӧ���־λ
uint8 PCA9554ledstate = 0;  //����LED��ǰ״̬

// temp remove
///******************************************************************************
// * �������ƣ�QWait
// *
// * ����������1us����ʱ
// *
// * ��    ������
// *
// * �� �� ֵ����
// *****************************************************************************/ 
//void QWait()     
//{
//    asm("NOP");asm("NOP");
//    asm("NOP");asm("NOP");
//    asm("NOP");asm("NOP");
//    asm("NOP");asm("NOP");
//    asm("NOP");asm("NOP");
//    asm("NOP");
//}
// 
///******************************************************************************
// * �������ƣ�Wait
// *
// * ����������ms����ʱ
// *
// * ��    ����ms - ��ʱʱ��
// *
// * �� �� ֵ����
// *****************************************************************************/ 
//void Wait(unsigned int ms)
//{                    
//   unsigned char g,k;
//   while(ms)
//   {
//      
//	  for(g=0;g<=167;g++)
//	   {
//	     for(k=0;k<=48;k++);
//	   }
//      ms--;                            
//   }
//}
/******************************************************************************
 * �������ƣ�Start_I2c
 *
 * ��������������I2C����,������I2C��ʼ����.
 *
 * ��    ������
 *
 * �� �� ֵ����
 *****************************************************************************/ 
void Start_I2c()
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //����P1.0Ϊ���
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //����P1.1Ϊ���
  
  SDA=1;                   /*������ʼ�����������ź�*/
  asm("NOP");
  SCL=1;
  QWait();                /*��ʼ��������ʱ�����4.7us,��ʱ*/
  QWait();
  QWait();
  QWait();
  QWait();    
  SDA=0;                   /*������ʼ�ź�*/
  QWait();                 /* ��ʼ��������ʱ�����4��s*/
  QWait();
  QWait();
  QWait();
  QWait();       
  SCL=0;                   /*ǯסI2C���ߣ�׼�����ͻ�������� */
  asm("NOP");
  asm("NOP");
}

/******************************************************************************
 * �������ƣ�Stop_I2c
 *
 * ��������������I2C����,������I2C��������.
 *
 * ��    ������
 *
 * �� �� ֵ����
 *****************************************************************************/ 
void Stop_I2c()
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //����P1.0Ϊ���
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //����P1.1Ϊ���
  SDA=0;                            /*���ͽ��������������ź�*/
  asm("NOP");                       /*���ͽ���������ʱ���ź�*/
  SCL=1;                            /*������������ʱ�����4��s*/
  QWait();
  QWait();
  QWait();
  QWait();
  QWait();
  SDA=1;                           /*����I2C���߽����ź�*/
  QWait();
  QWait();
  QWait();
  QWait();
}

/******************************************************************************
 * �������ƣ�SendByte
 *
 * ����������������c���ͳ�ȥ,�����ǵ�ַ,Ҳ����������,�����ȴ�Ӧ��,����
 *           ��״̬λ���в���.(��Ӧ����Ӧ��ʹack=0 ��)     
 *           ��������������ack=1; ack=0��ʾ��������Ӧ����𻵡�
 *
 * ��    ����c - �跢�͵�����
 *
 * �� �� ֵ����
 *****************************************************************************/ 
void  SendByte(uint8 c)
{
 uint8 BitCnt;
 IO_DIR_PORT_PIN(1, 0, IO_OUT);    //����P1.0Ϊ���
 IO_DIR_PORT_PIN(1, 1, IO_OUT);    //����P1.1Ϊ���
 for(BitCnt=0;BitCnt<8;BitCnt++)  /*Ҫ���͵����ݳ���Ϊ8λ*/
    {
     if((c<<BitCnt)&0x80)SDA=1;   /*�жϷ���λ*/
       else  SDA=0;                
      asm("NOP");
     SCL=1;                       /*��ʱ����Ϊ�ߣ�֪ͨ��������ʼ��������λ*/
      QWait(); 
      QWait();                    /*��֤ʱ�Ӹߵ�ƽ���ڴ���4��s*/
      QWait();
      QWait();
      QWait();         
     SCL=0; 
    }    
    QWait();
    QWait();
    QWait();
    SDA=1;                        /*8λ��������ͷ������ߣ�׼������Ӧ��λ*/
    asm("NOP");
    IO_DIR_PORT_PIN(1, 1, IO_IN);  
    SCL=1;
    QWait();
    QWait();
    QWait();
    QWait();
    if(SDA==1)ack=0;     
    else ack=1;                   /*�ж��Ƿ���յ�Ӧ���ź�*/
    SCL=0;   
    QWait();
    QWait();
    IO_DIR_PORT_PIN(1, 1, IO_OUT);
}

/******************************************************************************
 * �������ƣ�RcvByte
 *
 * �����������������մ���������������,���ж����ߴ���(����Ӧ���ź�)��
 *           ���������Ӧ������
 *
 * ��    ������
 *
 * �� �� ֵ��retc - ����������������
 *****************************************************************************/ 
uint8  RcvByte()
{
  uint8 retc;
  uint8 BitCnt;
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //����P1.0Ϊ���
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //����P1.1Ϊ���
  retc=0; 
  SDA=1;                            /*��������Ϊ���뷽ʽ*/
  IO_DIR_PORT_PIN(1, 1, IO_IN);
  for(BitCnt=0;BitCnt<8;BitCnt++)
      {
        asm("NOP");          
        SCL=0;                     /*��ʱ����Ϊ�ͣ�׼����������λ*/
        QWait();
        QWait();                   /*ʱ�ӵ͵�ƽ���ڴ���4.7��s*/
        QWait();
        QWait();
        QWait();
        SCL=1;                    /*��ʱ����Ϊ��ʹ��������������Ч*/
        QWait();
        QWait();
        retc=retc<<1;
        if(SDA==1)retc=retc+1;   /*������λ,���յ�����λ����retc�� */
        QWait();
        QWait(); 
      }
  SCL=0;    
  QWait();
  QWait();
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  return(retc);
}

/******************************************************************************
 * �������ƣ�Ack_I2c
 *
 * ��������������������Ӧ���ź�,(������Ӧ����Ӧ���ź�)
 *           
 *
 * ��    ������
 *
 * �� �� ֵ����
 *****************************************************************************/ 
void Ack_I2c(uint8 a)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);    //����P1.0Ϊ���
  IO_DIR_PORT_PIN(1, 1, IO_OUT);    //����P1.1Ϊ���
  if(a==0)SDA=0;                   /*�ڴ˷���Ӧ����Ӧ���ź� */
  else SDA=1;
  QWait();
  //QWait();
  //QWait();      
  SCL=1;
  QWait();
  QWait();                         /*ʱ�ӵ͵�ƽ���ڴ���4��s*/
  QWait();
  QWait();
  QWait();  
  SCL=0;                           /*��ʱ���ߣ�ǯסI2C�����Ա��������*/
  QWait();
  //QWait();    
}

/******************************************************************************
 * �������ƣ�ISendByte
 *
 * �������������������ߵ����͵�ַ�����ݣ��������ߵ�ȫ����,��������ַsla.
 *           �������1��ʾ�����ɹ��������������
 *           
 *
 * ��    ����sla - ��������ַ
 *           c - �跢�͵�����
 *
 * �� �� ֵ��0 -- ʧ��
 *           1 -- �ɹ�
 *****************************************************************************/ 
uint8 ISendByte(uint8 sla,uint8 c)
{
   Start_I2c();               /*��������*/
   SendByte(sla);             /*����������ַ*/
     if(ack==0)return(0);
   SendByte(c);               /*��������*/
     if(ack==0)return(0);
  Stop_I2c();                 /*��������*/ 
  return(1);
}

/******************************************************************************
 * �������ƣ�ISendStr
 *
 * �������������������ߵ����͵�ַ���ӵ�ַ,���ݣ��������ߵ�ȫ����,������
 *            ��ַsla���ӵ�ַsuba������������sָ������ݣ�����no���ֽڡ�
 *           �������1��ʾ�����ɹ��������������
 *           
 *
 * ��    ����sla - ��������ַ
 *           suba - �������ӵ�ַ
 *           *s - ����
 *           no - �����ֽ���Ŀ
 *
 * �� �� ֵ��0 -- ʧ��
 *           1 -- �ɹ�
 *
 * ע    �⣺ʹ��ǰ�����ѽ������ߡ�
 *****************************************************************************/ 
uint8 ISendStr(uint8 sla,uint8 suba,uint8 *s,uint8 no)
{
   uint8 i;

   Start_I2c();               /*��������*/
   SendByte(sla);             /*����������ַ*/
     if(ack==0)return(0);
   SendByte(suba);            /*���������ӵ�ַ*/
     if(ack==0)return(0);
   for(i=0;i<no;i++)
    {   
     SendByte(*s);            /*��������*/
       if(ack==0)return(0);
     s++;
    } 
 Stop_I2c();                  /*��������*/ 
  return(1);
}

/******************************************************************************
 * �������ƣ�IRcvByte
 *
 * �������������������ߵ����͵�ַ�������ݣ��������ߵ�ȫ����,��������
 *          ַsla������ֵ��c. �������1��ʾ�����ɹ��������������
 *           
 *
 * ��    ����sla - ��������ַ
 *           *c - �跢�͵�����
 *
 * �� �� ֵ��0 -- ʧ��
 *           1 -- �ɹ�
 *
 *ע    �⣺ʹ��ǰ�����ѽ������ߡ�
 *****************************************************************************/ 
uint8 IRcvByte(uint8 sla,uint8 *c)
{
   Start_I2c();                /*��������*/
   SendByte(sla+1);            /*����������ַ*/
   //SendByte(sla); 
   if(ack==0)return(0);
   *c=RcvByte();               /*��ȡ����*/
   Ack_I2c(1);                 /*���ͷǾʹ�λ*/
   Stop_I2c();                 /*��������*/ 
   return(1);
}

/******************************************************************************
 * �������ƣ�IRcvStr
 *
 * �������������������ߵ����͵�ַ���ӵ�ַ,�����ݣ��������ߵ�ȫ����,������
 *          ��ַsla���ӵ�ַsuba�����������ݷ���sָ��Ĵ洢������no���ֽڡ�
 *         �������1��ʾ�����ɹ��������������
 *           
 *
 * ��    ����sla - ��������ַ
 *           suba - �������ӵ�ַ
 *           *s - ����
 *           no - �����ֽ���Ŀ
 *
 * �� �� ֵ��0 -- ʧ��
 *           1 -- �ɹ�
 *
 * ע    �⣺ʹ��ǰ�����ѽ������ߡ�
 *****************************************************************************/ 
uint8 IRcvStr(uint8 sla,uint8 suba,uint8 *s,uint8 no)
{
   Start_I2c();               /*��������*/
   SendByte(sla);             /*����������ַ*/
   if(ack==0)return(0);
   SendByte(suba);            /*���������ӵ�ַ*/
  // if(ack==0)return(0);
  // SendByte(sla+1);
   if(ack==0)return(0);
   while(no > 0) 
   {
    *s++ = RcvByte();
     if(no > 1)  Ack_I2c(0);   /*���;ʹ�λ*/ 
     else Ack_I2c(1);          /*���ͷ�Ӧλ*/
     no--;
   }
   Stop_I2c();                 /*��������*/ 
   return(1);
}

/******************************************************************************
 * �������ƣ�ctrPCA9554LED
 *
 * ����������ͨ��IIC���߿���PCA9554�����������������Ӧ��LED��
 *                    
 *
 * ��    ����LED - �����Ƶ�LED
 *           operation - ����ز���
 *
 * �� �� ֵ����
 *           
 *
 * ע    �⣺PCA9554�ĵ�ַΪ��0x40
 *****************************************************************************/ 
void ctrPCA9554LED(uint8 led,uint8 operation)
{
  uint8 output = 0x00;
  uint8 *data = 0;
  if(ISendStr(0x40,0x03,&output,1))  //����PCA9554�Ĵ���
  {
    switch(led)
    {
      case 0:                        //LED0����
        if (operation)
        {
          output = PCA9554ledstate & 0xfe;
        }
        else
        {
          output = PCA9554ledstate | 0x01;
        }
      break;
       case 1:                      //LED1����
        if (operation)
        {
          output = PCA9554ledstate & 0xfd;
        }
        else
        {
          output = PCA9554ledstate | 0x02;
        }
      break;
       case 2:                     //LED2����
        if (operation)
        {
          output = PCA9554ledstate & 0xf7;
        }
        else
        {
          output = PCA9554ledstate | 0x08;
        }
      break;
       case 3:                     //LED3����
        if (operation)
        {
          output = PCA9554ledstate & 0xfb;
        }
        else
        {
          output = PCA9554ledstate | 0x04;
        }
      break;
       case 4:                    //LED4����
        if (operation)
        {
          output = PCA9554ledstate & 0xdf;
        }
        else
        {
          output = PCA9554ledstate | 0x20;
        }
      break;
       case 5:                   //LED5����
        if (operation)
        {
          output = PCA9554ledstate & 0xef;
        }
        else
        {
          output = PCA9554ledstate | 0x10;
        }
      break;

     default:break;
    }
    if(ISendStr(0x40,0x01,&output,1)) //дPCA9554����Ĵ���
    {
      if(IRcvByte(0x40,data))         //��PCA9554����Ĵ���
      {
        PCA9554ledstate = *data;
      }
    }
  }
}

/******************************************************************************
 * �������ƣ�ctrPCA9554FLASHLED
 *
 * ����������ͨ��IIC���߿���PCA9554����������������ǰ�����ж�������PCA9554���
 *           �Ĵ�����ֵ��Ȼ��������Ƶ�λ����ȡ�����ơ�
 *                    
 *
 * ��    ����led - ��ӦPCA9554������˿�
 *           
 *
 * �� �� ֵ����
 *           
 *
 * ע    �⣺PCA9554�ĵ�ַΪ��0x40
 *****************************************************************************/ 

void ctrPCA9554FLASHLED(uint8 led)   
{
  uint8 output = 0x00;
  uint8 *data = 0;
  if(ISendStr(0x40,0x03,&output,1))    //д����
  {
    switch(led)
    {
     case 1:     
     output = PCA9554ledstate & 0x01;  //�ж�������λ�ĵ�ǰ״̬ 
      if (output)                      //�����ǰ��λ״̬Ϊ1����ı����Ϊ0
     {
       output = PCA9554ledstate & 0xfe;
     }
      else                             //�����ǰ״̬Ϊ0����ı����Ϊ1
     {
       output = PCA9554ledstate | 0x01;
     }
     break;
     case 2:     
     output = PCA9554ledstate & 0x02;
      if (output)
     {
       output = PCA9554ledstate & 0xfd;
     }
      else
     {
       output = PCA9554ledstate | 0x02;
     }
     break;
     case 3:     
     output = PCA9554ledstate & 0x08;
      if (output)
     {
       output = PCA9554ledstate & 0xf7;
     }
      else
     {
       output = PCA9554ledstate | 0x08;
     }
     break;
     case 4:     
     output = PCA9554ledstate & 0x04;
      if (output)
     {
       output = PCA9554ledstate & 0xfb;
     }
      else
     {
       output = PCA9554ledstate | 0x04;
     }
     break;
     case 5:     
     output = PCA9554ledstate & 0x20;
      if (output)
     {
       output = PCA9554ledstate & 0xdf;
     }
      else
     {
       output = PCA9554ledstate | 0x20;
     }
     break;
     case 6:     
     output = PCA9554ledstate & 0x10;
      if (output)
     {
       output = PCA9554ledstate & 0xef;
     }
      else
     {
       output = PCA9554ledstate | 0x10;
     }
     break;
     default: break;
    }
     if(ISendStr(0x40,0x01,&output,1))    //д����Ĵ���
    {
      if(IRcvByte(0x40,data))             //������Ĵ���
      {
        PCA9554ledstate = *data;          //��������Ĵ�����ǰ״̬��
      }
    }
  }
}

/******************************************************************************
 * �������ƣ�PCA9554ledInit
 *
 * ������������ʼ��6��LED�����ر����е�LED
 *                    
 * ��    ������          
 *
 * �� �� ֵ����
 *   
 *****************************************************************************/ 
void PCA9554ledInit()
{
  uint8 output = 0x00;
  uint8 *data = 0;
  if(ISendStr(0x40,0x03,&output,1))  //����PCA9554�Ĵ���
  {
    output = 0xbf;
    if(ISendStr(0x40,0x01,&output,1)) //д����Ĵ���
    {
      if(IRcvByte(0x40,data))         //������Ĵ���
      {
        PCA9554ledstate = *data;
      }
    }
  }
}

/******************************************************************************
 * �������ƣ�ctrPCA9554Key
 *
 * ������������ȡ��ӦIO��չоƬPCA9554�İ���ֵ
 *                    
 * ��    ������          
 *
 * �� �� ֵ����
 *  
 * ע    �⣺PCA9554�ĵ�ַΪ��0x42
 *****************************************************************************/ 
uint8 ctrPCA9554Key()
{
  uint8 input = 0xff;
  uint8 *data = 0;
  if(ISendStr(0x42,0x03,&input,1))  //����PCA9554�Ĵ���
  {   
    if(ISendByte(0x42,0x00))        //д����
    {
     if(IRcvByte(0x42,data))        //������Ĵ���
     {
       return *data;
      }
    }
  }
  return 0;
}

/******************************************************************************
 * �������ƣ�GetKeyInput
 *
 * �������������ð��������ȡ�����ĵ�����ֵ��Ȼ�������ֵ���б���
 *                    
 * ��    ������          
 *
 * �� �� ֵ��ksave0 - ��������ֵ
 *  
 *****************************************************************************/ 
uint8 GetKeyInput()
{
  uint8 key;
  uint8 ksave0 = 0;
    key = ctrPCA9554Key();     //��ȡ����ֵ
    if (key == 0xfe)           //������ֵ���б���
    {
      ksave0 = 1;
    }
    else if (key == 0xfd)
    {
      ksave0 = 2;
    }
    else if (key == 0xfb)
    {
      ksave0 = 3;
    }
    else if (key == 0xf7)
    {
      ksave0 = 4;
    }
    else if (key == 0xdf)
    {
      ksave0 = 5;
    }
     else if (key == 0xef)
    {
      ksave0 = 6;
    }
    // Wait(50);                  //��ʱȥ��
  return ksave0;
}



