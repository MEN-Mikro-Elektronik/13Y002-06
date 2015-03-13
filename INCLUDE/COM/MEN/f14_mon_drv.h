/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  f14_mon_drv.h
 *
 *      \author  dp
 *        $Date: 2009/08/31 12:18:00 $
 *    $Revision: 3.2 $
 *
 *       \brief  Header file for F14_MON driver containing
 *               F14_MON specific status codes and
 *               F14_MON function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: f14_mon_drv.h,v $
 * Revision 3.2  2009/08/31 12:18:00  MRoth
 * R: Porting to MIDS5, new MDIS_API and men_typs
 * M: for backward compatibility to MDIS4 optionally define new types
 *
 * Revision 3.1  2005/12/22 14:42:34  dpfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _F14_MON_DRV_H
#define _F14_MON_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name F14_MON specific Getstat/Setstat standard codes
 *  \anchor getstat_setstat_codes
 */
/**@{*/
#define F14_MON_STATUS_LED  M_DEV_OF+0x00   /**<  S: Set board status LED (1=on) */
#define F14_MON_PS_STATE	M_DEV_OF+0x01   /**<G,S: Get/reset CPCI power supply state */
/**@}*/

#define F14_MON_PS_FAIL     0x01   /** PS_FAIL occured (power fail) */
#define F14_MON_PS_DEG      0x02   /** PS_DEG occured (temperature rise) */

#ifndef  F14_MON_VARIANT
# define F14_MON_VARIANT F14_MON
#endif

# define _F14_MON_GLOBNAME(var,name) var##_##name
#ifndef _ONE_NAMESPACE_PER_DRIVER_
# define F14_MON_GLOBNAME(var,name) _F14_MON_GLOBNAME(var,name)
#else
# define F14_MON_GLOBNAME(var,name) _F14_MON_GLOBNAME(M16,name)
#endif

#define __F14_MON_GetEntry    F14_MON_GLOBNAME(F14_MON_VARIANT,GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __F14_MON_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
 /* we have an MDIS4 men_types.h and mdis_api.h included */
 /* only 32bit compatibility needed!                     */
 #define INT32_OR_64  int32
 #define U_INT32_OR_64 u_int32
 typedef INT32_OR_64  MDIS_PATH;
#endif /* U_INT32_OR_64 */


#ifdef __cplusplus
      }
#endif

#endif /* _F14_MON_DRV_H */
