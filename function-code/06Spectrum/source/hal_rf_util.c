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
  halRfSetChannel(channel);   //设置通信频道
  
  // Set RX on
  halRfReceiveOn();          //打开接收
  
  while (!RSSISTAT);        //等待RSSI状态寄存器有效
  
  // Enable energy scan mode, using peak signal strength
  FRMCTRL0 |= 0x10;         //RSSI+crc_ok位和7位SRCRESINDEX附加到每个收到帧的末尾
  
  // Spend sampleTime us accumulating the peak RSSI value
  halMcuWaitUs(sampleTime);  //延时2000us
  rssi = RSSI;               //读取RSSI值
  
  // Exit the current channel
  halRfReceiveOff();        //关闭接收
  // Disable ED scan mode
  FRMCTRL0 &= ~0x10;        //关闭扫描模式
  
  return rssi;              //返回RSSI的值
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
  FRMCTRL0 =  0x0C;   //配置帧处理寄存器  禁止符号搜索
}
