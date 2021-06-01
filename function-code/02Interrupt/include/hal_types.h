/**************************************************************************************************
  Filename:       hal_types.h
  Revised:        $Date: 2008-03-20 17:17:05 -0700 (Thu, 20 Mar 2008) $
  Revision:       $Revision: 16618 $

  Description:    Describe the purpose and contents of the file.

**************************************************************************************************/

#ifndef _HAL_TYPES_H
#define _HAL_TYPES_H

/* Texas Instruments CC2530 */

/* ------------------------------------------------------------------------------------------------
 *                                               Types
 * ------------------------------------------------------------------------------------------------
 */
typedef signed   char   int8;
typedef unsigned char   uint8;

typedef signed   short  int16;
typedef unsigned short  uint16;

typedef signed   long   int32;
typedef unsigned long   uint32;

typedef unsigned char   bool;

typedef uint8           halDataAlign_t;


/* ------------------------------------------------------------------------------------------------
 *                                       Memory Attributes
 * ------------------------------------------------------------------------------------------------
 */

/* ----------- IAR Compiler ----------- */
#ifdef __IAR_SYSTEMS_ICC__
#define  CODE   __code
#define  XDATA  __xdata

/* ----------- GNU Compiler ----------- */
#elif defined __KEIL__
#define  CODE   code
#define  XDATA  xdata

/* ----------- Unrecognized Compiler ----------- */
#else
#error "ERROR: Unknown compiler."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                        Standard Defines
 * ------------------------------------------------------------------------------------------------
 */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif


/**************************************************************************************************
 */
#endif
