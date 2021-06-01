/*********************************************************************
	Filename:       hal_rf_util.c

	Description:    Support for anergy detection applications.

*********************************************************************/


/*********************************************************************
* INCLUDES
*/

#include "hal_board.h"
#include "hal_assert.h"
#include "hal_rf_util.h"
#include "hal_rf.h"


/*********************************************************************
* CONSTANTS
*/


/*********************************************************************
* MACROS
*/

/*********************************************************************
* TYPEDEFS
*/


/*********************************************************************
* GLOBAL VARIABLES
*/



/*********************************************************************
* FUNCTIONS
*/

/***********************************************************************************
* @fn          halSampleED
*
* @brief      Sample Energy Detect
*
* @param      uint8 channel - channel between 11 and 26
*             uint16 sampleTime - sample time in us
*            
* @return     int8 - sampled RSSI value      
*/
int8 halSampleED(uint8 channel, uint16 sampleTime)
{
  int8 rssi=0;
  
  // Set channel
  halRfSetChannel(channel);   //����ͨ��Ƶ��
  
  // Set RX on
  halRfReceiveOn();          //�򿪽���
  
  while (!RSSISTAT);        //�ȴ�RSSI״̬�Ĵ�����Ч
  
  // Enable energy scan mode, using peak signal strength
  FRMCTRL0 |= 0x10;         //RSSI+crc_okλ��7λSRCRESINDEX���ӵ�ÿ���յ�֡��ĩβ
  
  // Spend sampleTime us accumulating the peak RSSI value
  halMcuWaitUs(sampleTime);  //��ʱ2000us
  rssi = RSSI;               //��ȡRSSIֵ
  
  // Exit the current channel
  halRfReceiveOff();        //�رս���
  // Disable ED scan mode
  FRMCTRL0 &= ~0x10;        //�ر�ɨ��ģʽ
  
  return rssi;              //����RSSI��ֵ
}

/***********************************************************************************
* @fn          halSetRxScanMode
*
* @brief       Set chip in RX scanning mode
*
* @param       none 
*            
*
* @return     none
*/
void halSetRxScanMode(void)
{
  // Infinite RX mode (disables symbol search)
  FRMCTRL0 =  0x0C;   //����֡����Ĵ���  ��ֹ��������
}
