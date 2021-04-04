/***********************************************************************************
  Filename:     hal_int.c

  Description:  HAL interrupt control

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_int.h"
#include "hal_board.h"

/***********************************************************************************
* GLOBAL FUNCTIONS
*/

/***********************************************************************************
* @fn      halIntOn
*
* @brief   Enable global interrupts.
*
* @param   none
*
* @return  none
*/
void halIntOn(void)
{
    HAL_INT_ON();
}


/***********************************************************************************
* @fn      halIntOff
*
* @brief   Turns global interrupts off.
*
* @param   none
*
* @return  none
*/
void halIntOff(void)
{
    HAL_INT_OFF();
}


/***********************************************************************************
* @fn      halIntLock
*
* @brief   Turns global interrupts off and returns current interrupt state.
*          Should always be used together with halIntUnlock().
*
* @param   none
*
* @return  uint16 - current interrupt state
*/
uint16 halIntLock(void)
{
    istate_t key;
    HAL_INT_LOCK(key);
    return(key);
}


/***********************************************************************************
* @fn      halIntUnlock
*
* @brief   Set interrupt state back to the state it had before calling halIntLock().
*          Should always be used together with halIntLock().
*
* @param   key
*
* @return  none
*/
void halIntUnlock(uint16 key)
{
    HAL_INT_UNLOCK(key);
}




