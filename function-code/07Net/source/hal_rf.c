/***********************************************************************************

  Filename:       hal_rf.c

  Description:    CC2530 radio interface.

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_board.h"
#include "hal_mcu.h"
#include "hal_int.h"
#include "hal_assert.h"
#include "hal_rf.h"
#include "util.h"


/***********************************************************************************
* CONSTANTS AND DEFINES
*/

// Chip revision
#define REV_A                      0x01
#define CHIPREVISION              REV_A

// CC2530 RSSI Offset
#define RSSI_OFFSET                               73
#define RSSI_OFFSET_LNA_HIGHGAIN                  79
#define RSSI_OFFSET_LNA_LOWGAIN                   67

// Various radio settings
#define AUTO_ACK                   0x20
#define AUTO_CRC                   0x40

// TXPOWER values
#if CC2530_PG1
#define CC2530_TXPOWER_MIN_3_DBM   0x88 
#define CC2530_TXPOWER_0_DBM       0x32
#define CC2530_TXPOWER_4_DBM       0xF7
#else
#define CC2530_TXPOWER_MIN_3_DBM   0xB5 
#define CC2530_TXPOWER_0_DBM       0xD5
#define CC2530_TXPOWER_4_DBM       0xF5
#endif

// RF interrupt flags
#define IRQ_TXDONE                 0x02
#define IRQ_RXPKTDONE              0x40

// Selected strobes
#define ISRXON()                st(RFST = 0xE3;)
#define ISTXON()                st(RFST = 0xE9;)
#define ISTXONCCA()             st(RFST = 0xEA;)
#define ISRFOFF()               st(RFST = 0xEF;)
#define ISFLUSHRX()             st(RFST = 0xEC;)
#define ISFLUSHTX()             st(RFST = 0xEE;)

// CC2590-CC2591 support
#if INCLUDE_PA==2591

// Support for PA/LNA
#define HAL_PA_LNA_INIT()

// Select CC2591 RX high gain mode 
#define HAL_PA_LNA_RX_HGM() st( uint8 i; P0_7 = 1; for (i=0; i<8; i++) asm("NOP"); )

// Select CC2591 RX low gain mode
#define HAL_PA_LNA_RX_LGM() st( uint8 i; P0_7 = 0; for (i=0; i<8; i++) asm("NOP"); )

// TX power lookup index
#define HAL_RF_TXPOWER_0_DBM          0
#define HAL_RF_TXPOWER_13_DBM         1
#define HAL_RF_TXPOWER_16_DBM         2
#define HAL_RF_TXPOWER_18_DBM         3
#define HAL_RF_TXPOWER_20_DBM         4

// TX power values
#define CC2530_91_TXPOWER_0_DBM       0x25
#define CC2530_91_TXPOWER_13_DBM      0x85
#define CC2530_91_TXPOWER_16_DBM      0xA5
#define CC2530_91_TXPOWER_18_DBM      0xC5
#define CC2530_91_TXPOWER_20_DBM      0xE5

#else // dummy macros when not using CC2591

#define HAL_PA_LNA_INIT()
#define HAL_PA_LNA_RX_LGM()
#define HAL_PA_LNA_RX_HGM()

#define HAL_RF_TXPOWER_MIN_3_DBM  0
#define HAL_RF_TXPOWER_0_DBM      1
#define HAL_RF_TXPOWER_4_DBM	  2

#endif

/***********************************************************************************
* GLOBAL DATA
*/
#if INCLUDE_PA==2591
static const menuItem_t pPowerSettings[] =
{
  "0dBm", HAL_RF_TXPOWER_0_DBM,
  "13dBm", HAL_RF_TXPOWER_13_DBM,
  "16dBm", HAL_RF_TXPOWER_16_DBM,
  "18dBm", HAL_RF_TXPOWER_18_DBM,
  "20dBm", HAL_RF_TXPOWER_20_DBM
};
#else
static const menuItem_t pPowerSettings[] =
{
  "-3dBm", HAL_RF_TXPOWER_MIN_3_DBM,
  "0dBm", HAL_RF_TXPOWER_0_DBM,
  "4dBm", HAL_RF_TXPOWER_4_DBM
};
#endif

const menu_t powerMenu =
{
  pPowerSettings,
  N_ITEMS(pPowerSettings)
};

/***********************************************************************************
* LOCAL DATA
*/
#ifndef MRFI
static ISR_FUNC_PTR pfISR= NULL;
#endif
static uint8 rssiOffset = RSSI_OFFSET;

/***********************************************************************************
* LOCAL FUNCTIONS
*/
static void halPaLnaInit(void);

/***********************************************************************************
* GLOBAL FUNCTIONS
*/

/***********************************************************************************
* @fn      halRfInit
*
* @brief   Power up, sets default tuning settings, enables autoack, enables random
*          generator.
*
* @param   none
*
* @return  SUCCESS always (for interface compatibility)
*/
uint8 halRfInit(void)
{
    // Enable auto ack and auto crc ����֡����Ĵ���
    FRMCTRL0 |= (AUTO_ACK | AUTO_CRC);  //ʹ��autoack��autocrc����

    // Recommended RX settings
    TXFILTCFG = 0x09;  //TX���������ã�����TX������������Ի�ú��ʵĴ���
    AGCCTRL1 = 0x15;   //����AGCĿ��ֵ ��AGC �Զ�������ƣ�
    FSCAL1 = 0x00;     //����Ƶ��У׼����Ĭ�����ñȽϣ�����VCOй¶3dB���Ƽ�Ĭ�������Ի�����EVM

    // Enable random generator -> Not implemented yet

    // Enable CC2591 with High Gain Mode
    halPaLnaInit(); //û�иñ���ѡ��

    // Enable RX interrupt
    halRfEnableRxInterrupt();  //ʹ��RXһЩ�ж�

    return SUCCESS;
}



/***********************************************************************************
* @fn      halRfGetChipId
*
* @brief   Get chip id
*
* @param   none
*
* @return  uint8 - result
*/
uint8 halRfGetChipId(void)
{
    return CHIPID;
}


/***********************************************************************************
* @fn      halRfGetChipVer
*
* @brief   Get chip version
*
* @param   none
*
* @return  uint8 - result
*/
uint8 halRfGetChipVer(void)
{
    // return major revision (4 upper bits)
    return CHVER>>4;
}

/***********************************************************************************
* @fn      halRfGetRandomByte
*
* @brief   Return random byte
*
* @param   none
*
* @return  uint8 - random byte
*/
uint8 halRfGetRandomByte(void)
{
    // Not yet implemented
    //HAL_ASSERT(FALSE);
    return 0;
}


/***********************************************************************************
* @fn      halRfGetRssiOffset
*
* @brief   Return RSSI Offset
*
* @param   none
*
* @return  uint8 - RSSI offset
*/
uint8 halRfGetRssiOffset(void)
{
    return rssiOffset;
}

/***********************************************************************************
* @fn      halRfSetChannel
*
* @brief   Set RF channel in the 2.4GHz band. The Channel must be in the range 11-26,
*          11= 2005 MHz, channel spacing 5 MHz.
*
* @param   channel - logical channel number
*
* @return  none
*/
void halRfSetChannel(uint8 channel)
{
    FREQCTRL = (MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);  //����RFƵ��
}


/***********************************************************************************
* @fn      halRfSetShortAddr
*
* @brief   Write short address to chip
*
* @param   none
*
* @return  none
*/
void halRfSetShortAddr(uint16 shortAddr)
{
    SHORT_ADDR0 = LO_UINT16(shortAddr);
    SHORT_ADDR1 = HI_UINT16(shortAddr);
}


/***********************************************************************************
* @fn      halRfSetPanId
*
* @brief   Write PAN Id to chip
*
* @param   none
*
* @return  none
*/
void halRfSetPanId(uint16 panId)
{
    PAN_ID0 = LO_UINT16(panId);
    PAN_ID1 = HI_UINT16(panId);
}


/***********************************************************************************
* @fn      halRfSetPower
*
* @brief   Set TX output power
*
* @param   uint8 power - power level: TXPOWER_MIN_4_DBM, TXPOWER_0_DBM,
*                        TXPOWER_4_DBM
*
* @return  uint8 - SUCCESS or FAILED
*/
uint8 halRfSetTxPower(uint8 power)
{
    uint8 n;

    switch(power)   //�����书��ѡ��ֵת��ΪTXPOWER�Ĵ�����ֵ
    {
#if INCLUDE_PA==2591
    case HAL_RF_TXPOWER_0_DBM: n = CC2530_91_TXPOWER_0_DBM; break;
    case HAL_RF_TXPOWER_13_DBM: n = CC2530_91_TXPOWER_13_DBM; break;
    case HAL_RF_TXPOWER_16_DBM: n = CC2530_91_TXPOWER_16_DBM; break;
    case HAL_RF_TXPOWER_18_DBM: n = CC2530_91_TXPOWER_18_DBM; break;
    case HAL_RF_TXPOWER_20_DBM: n = CC2530_91_TXPOWER_20_DBM; break;
#else
    case HAL_RF_TXPOWER_MIN_3_DBM: n = CC2530_TXPOWER_MIN_3_DBM; break;
    case HAL_RF_TXPOWER_0_DBM: n = CC2530_TXPOWER_0_DBM; break;
    case HAL_RF_TXPOWER_4_DBM: n = CC2530_TXPOWER_4_DBM; break;
#endif
    default:
        return FAILED;
    }

    // Set TX power
    TXPOWER = n;                      //�����书������ֵд��TXPOWER�Ĵ�����

    return SUCCESS;
}


/***********************************************************************************
* @fn      halRfSetGain
*
* @brief   Set gain mode - only applicable for units with CC2590/91.
*
* @param   uint8 - gain mode
*
* @return  none
*/
void halRfSetGain(uint8 gainMode)  //��������ģʽ
{
    if (gainMode==HAL_RF_GAIN_LOW) 
    {
        HAL_PA_LNA_RX_LGM();
        rssiOffset = RSSI_OFFSET_LNA_LOWGAIN;
    }
    else 
    {
        HAL_PA_LNA_RX_HGM();                   //����Ϊ������ģʽ
        rssiOffset = RSSI_OFFSET_LNA_HIGHGAIN;  //RSSI�������� 79
    }
}

/***********************************************************************************
* @fn      halRfWriteTxBuf
*
* @brief   Write to TX buffer
*
* @param   uint8* pData - buffer to write
*          uint8 length - number of bytes
*
* @return  none
*/
void halRfWriteTxBuf(uint8* pData, uint8 length)
{
    uint8 i;

    ISFLUSHTX();          // Making sure that the TX FIFO is empty. ȷ��TX FIFO�Ĵ���Ϊ��

    RFIRQF1 = ~IRQ_TXDONE;   // Clear TX done interrupt

    // Insert data
    for(i=0;i<length;i++)   //д������
    {
        RFD = pData[i];
    }
}


/***********************************************************************************
* @fn      halRfAppendTxBuf
*
* @brief   Write to TX buffer
*
* @param   uint8* pData - buffer to write
*          uint8 length - number of bytes
*
* @return  none
*/
void halRfAppendTxBuf(uint8* pData, uint8 length)
{
    uint8 i;

    // Insert data
    for(i=0;i<length;i++)
    {
        RFD = pData[i];
    }
}


/***********************************************************************************
* @fn      halRfReadRxBuf
*
* @brief   Read RX buffer
*
* @param   uint8* pData - data buffer. This must be allocated by caller.
*          uint8 length - number of bytes
*
* @return  none
*/
void halRfReadRxBuf(uint8* pData, uint8 length)
{
    // Read data
    while (length>0) 
    {
        *pData++= RFD;
        length--;
    }
}


/***********************************************************************************
* @fn      halRfReadMemory
*
* @brief   Read RF device memory
*
* @param   uint16 addr - memory address
*          uint8* pData - data buffer. This must be allocated by caller.
*          uint8 length - number of bytes
*
* @return  Number of bytes read
*/
uint8 halRfReadMemory(uint16 addr, uint8* pData, uint8 length)
{
    return 0;
}


/***********************************************************************************
* @fn      halRfWriteMemory
*
* @brief   Write RF device memory
*
* @param   uint16 addr - memory address
*          uint8* pData - data buffer. This must be allocated by caller.
*          uint8 length - number of bytes
*
* @return  Number of bytes written
*/
uint8 halRfWriteMemory(uint16 addr, uint8* pData, uint8 length)
{
    return 0;
}

/***********************************************************************************
* @fn      halRfTransmit
*
* @brief   Transmit frame with Clear Channel Assessment.
*
* @param   none
*
* @return  uint8 - SUCCESS or FAILED
*/
uint8 halRfTransmit(void)
{
    uint8 status;

    ISTXON(); // Sending   ��������

    // Waiting for transmission to finish  �ȴ��������
    while(!(RFIRQF1 & IRQ_TXDONE) );

    RFIRQF1 = ~IRQ_TXDONE;
    status= SUCCESS;

    return status;
}



/***********************************************************************************
* @fn      halRfReceiveOn
*
* @brief   Turn receiver on
*
* @param   none
*
* @return  none
*/
void halRfReceiveOn(void)
{
    ISFLUSHRX();     // Making sure that the TX FIFO is empty.
    ISRXON();
}

/***********************************************************************************
* @fn      halRfReceiveOff
*
* @brief   Turn receiver off
*
* @param   none
*
* @return  none
*/
void halRfReceiveOff(void)
{
    ISRFOFF();
    ISFLUSHRX();    // Making sure that the TX FIFO is empty.
}


#ifndef MRFI
/***********************************************************************************
* @fn      halRfDisableRxInterrupt
*
* @brief   Clear and disable RX interrupt.
*
* @param   none
*
* @return  none
*/
void halRfDisableRxInterrupt(void)
{
    // disable RXPKTDONE interrupt
    RFIRQM0 &= ~BV(6);         //RFIRQM0 RF�ж����μĴ���
    // disable general RF interrupts
    IEN2 &= ~BV(0);           //�ж�ʹ��2
} 


/***********************************************************************************
* @fn      halRfEnableRxInterrupt
*
* @brief   Enable RX interrupt.
*
* @param   none
*
* @return  none
*/
void halRfEnableRxInterrupt(void)
{
    // enable RXPKTDONE interrupt
    RFIRQM0 |= BV(6);  //ʹ��RXPKTDONE ���յ�һ��������֡���ж�δ��
    // enable general RF interrupts
    IEN2 |= BV(0);    //RFһ���ж�ʹ��
}


/***********************************************************************************
* @fn      halRfRxInterruptConfig
*
* @brief   Configure RX interrupt.
*
* @param   none
*
* @return  none
*/
void halRfRxInterruptConfig(ISR_FUNC_PTR pf)
{
    uint8 x;
    HAL_INT_LOCK(x);
    pfISR= pf;
    HAL_INT_UNLOCK(x);
}

#endif

/***********************************************************************************
* @fn      halRfWaitTransceiverReady
*
* @brief   Wait until the transciever is ready (SFD inactive).
*
* @param   none
*
* @return  none
*/
void halRfWaitTransceiverReady(void)  // FSMSTAT1 ���ߵ�״̬�Ĵ���
{
    // Wait for SFD not active and TX_Active not active
    while (FSMSTAT1 & (BV(1) | BV(5) ));   //�ȴ�����״̬
}

#ifndef MRFI
/************************************************************************************
* @fn          rfIsr
*
* @brief       Interrupt service routine that handles RFPKTDONE interrupt.
*
* @param       none
*
* @return      none
*/
HAL_ISR_FUNCTION( rfIsr, RF_VECTOR )
{
    uint8 x;

    HAL_INT_LOCK(x);

    if( RFIRQF0 & IRQ_RXPKTDONE )
    {
        if(pfISR){
            (*pfISR)();                 // Execute the custom ISR
        }
        S1CON= 0;                   // Clear general RF interrupt flag
        RFIRQF0&= ~IRQ_RXPKTDONE;   // Clear RXPKTDONE interrupt
    }
    HAL_INT_UNLOCK(x);
}
#endif

/***********************************************************************************
* LOCAL FUNCTIONS
*/
/***********************************************************************************
* LOCAL FUNCTIONS
*/
static void halPaLnaInit(void)
{
#if INCLUDE_PA==2591
    // Initialize CC2591 to RX high gain mode
    static uint8 fFirst= TRUE;

    if(fFirst) {
        AGCCTRL1  = 0x15;
        FSCAL1 = 0x0; 
        RFC_OBS_CTRL0 = 0x68;
        RFC_OBS_CTRL1 = 0x6A;
        OBSSEL1 = 0xFB;
        OBSSEL4 = 0xFC;
        P0DIR |= 0x80;
        halRfSetGain(HAL_RF_GAIN_HIGH);
    }

#else // do nothing
#endif
}



