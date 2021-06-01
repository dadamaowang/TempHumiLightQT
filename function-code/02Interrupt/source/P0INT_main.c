/**********************************************************************************************************************************************************
* �� �� ����P0INT_main.c
��
* ��    �ܣ�ʵ���� P0�������ж�
*           ͨ��PCA9554����չIO�İ�������仯����Ӧ��PCA9554�����һ���͵�ƽ�жϣ����жϽ���CC2530��
*           P0.7�˿ڣ���������P0�жϡ����жϷ�������ж�ȡ��Ӧ�İ���ֵ��Ȼ����ͨ��IIC������һ��PCA9554��
*           ��LED������
*
*
* ע    �⣺��ʵ������Ӳ����Դ��
*           OURS-CC2530RFģ��
*           OURS-CC2530���ܵ�Դ�����ͨ��ذ塣
*
* ��    ����V1.0
* ��    �ߣ�WUXIANHAI
* ��    �ڣ�2011.1.19
* �¶�˹�Ƽ���ҳ��www.ourselec.com
**********************************************************************************************************************************************************/
#include "ioCC2530.h"
#include "hal_mcu.h"

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

extern void ctrPCA9554LED(uint8 led,uint8 operation); 
extern void PCA9554ledInit();
extern void ctrPCA9554FLASHLED(uint8 led);   
extern uint8 GetKeyInput();

/**************************************************************************************************
 * �������ƣ�Init_IO
 *
 * ����������P0.7�жϳ�ʼ������
 *
 * ��    ������
 *
 * �� �� ֵ����
 **************************************************************************************************/
void Init_IO(void)
{
    P0SEL |=0x80;            //��P0.7 ����Ϊ���蹦��
    P0DIR &=~0x80;           //��P0.7����Ϊ����
    P0INP &=~0x80;           //�˿�����ģʽ���ã�P0.7��������������
    P0IEN |=0x80;            //P0.7�ж�ʹ��
    PICTL |=0x01;            //P0.7Ϊ�½��ش����ж�       
    EA = 1;                  //ʹ��ȫ���ж�
    IEN1 |=0x20;             //P0���ж�ʹ��
    P0IFG &= ~0x80;          //P0.7�жϱ�־��0   
};


/**************************************************************************************************
 * �������ƣ�P0_IRQ
 *
 * ����������P0�������жϵ��жϷ������
 *
 * ��    ������
 *
 * �� �� ֵ����
 **************************************************************************************************/
#pragma vector=P0INT_VECTOR
__interrupt void P0_IRQ(void)
{ 
     uint8 key = 0;  
     P0IFG &= ~0x80;               //P0.7�жϱ�־��0  
    key = GetKeyInput();          //��ȡ����ֵ
    if(key)
    {
     ctrPCA9554FLASHLED(key);     //������Ӧ��LED����
    }    
}

/**************************************************************************************************
 * �������ƣ�main
 *
 * ������������ʼ��ʱ�Ӻ��жϿڡ�P0.7���½���(����6�������е�����һ��)����P0�������жϡ�
 *                     
 *
 * ��    ������
 *
 * �� �� ֵ����
 **************************************************************************************************/
void main(void)
{ 
  HAL_BOARD_INIT();             //��ʼ��ʱ��
  Init_IO();                    //�жϳ�ʼ��
  PCA9554ledInit();             //LED��ʼ��

  while(1);                     // ��ѭ�����ȴ�P0���½����ж�(��ʵ������P0.7����)
}



