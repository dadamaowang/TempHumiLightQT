/***********************************************************************************

  Filename:     basic_rf_security.c

  Description:  Basic RF security library

***********************************************************************************/

#ifdef SECURITY_CCM

/***********************************************************************************
* INCLUDES
*/
#include "basic_rf_security.h"
#include "hal_rf_security.h"

/***********************************************************************************
* CONSTANTS AND DEFINES
*/
#define FLAG_FIELD                          0x09
#define NONCE_SIZE                          16

/***********************************************************************************
* LOCAL VARIABLES
*/

static uint8 nonceTx[NONCE_SIZE];
static uint8 nonceRx[NONCE_SIZE];



/***********************************************************************************
* GLOBAL FUNCTIONS
*/

/***********************************************************************************
* @fn          basicRfSecurityInit
*
* @brief       Initialise key and nonces and write to radio
*
* @param       pConfig - file scope variable holding configuration data for
*              basic RF
*
* @return      none
*/
void basicRfSecurityInit(basicRfCfg_t* pConfig)
{
    uint8 i;

    // Initialise nonce bytes to 0
    for(i=0;i<NONCE_SIZE;i++)
    {
        nonceRx[i] = 0;
        nonceTx[i] = 0;
    }

    // Set nonce flag field (Byte 0)
    nonceRx[0] = FLAG_FIELD;
    nonceTx[0] = FLAG_FIELD;

    // Set byte 7 and 8 of nonce to myAddr
    nonceTx[8] = (uint8)pConfig->myAddr;
    nonceTx[7] = (uint8)(pConfig->myAddr>>8);

    // Set Security mode field of nonces (Byte 13)
    nonceRx[13] = SECURITY_CONTROL;
    nonceTx[13] = SECURITY_CONTROL;

    halRfSecurityInit(pConfig->securityKey, nonceRx, nonceTx);
}

#endif
