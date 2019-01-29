/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  f14_mon_drv.c
 *
 *      \author  alfred.jendrusch@men.de, dieter.pfeuffer@men.de
 *        $Date: 2009/08/31 12:17:53 $
 *    $Revision: 1.5 $
 *
 *      \brief   Low-level driver for F14/F15/F17/F18/D601 monitoring features
 *
 *     Required: OSS, DESC, DBG, libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_, WDOG, WDOG_ONLY
 */
 /*
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/
/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _NO_LL_HANDLE		/* ll_defs.h: don't define LL_HANDLE struct */

#ifdef F14_MON_SW				/* swapped variant */
#	define MAC_MEM_MAPPED
#	define ID_SW
#endif

#ifdef WDOG_ONLY
#	define WDOG
#endif

#include <MEN/men_typs.h>   /* system dependent definitions   */
#include <MEN/maccess.h>    /* hw access macros and types     */
#include <MEN/dbg.h>        /* debug functions                */
#include <MEN/oss.h>        /* oss functions                  */
#include <MEN/desc.h>       /* descriptor functions           */
#include <MEN/mdis_api.h>   /* MDIS global defs               */
#include <MEN/mdis_com.h>   /* MDIS common defs               */
#include <MEN/mdis_err.h>   /* MDIS error codes               */
#include <MEN/ll_defs.h>    /* low-level driver definitions   */
#include <MEN/wdog.h>		/* watchdog specific definitions  */

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* general defines */
#define CH_NUMBER			1			/**< Number of device channels */
#define USE_IRQ				FALSE		/**< Interrupt required  */
#define ADDRSPACE_COUNT		1			/**< Number of required address spaces */
#define ADDRSPACE_SIZE		1			/**< Size of address space */

/* debug defines */
#define DBG_MYLEVEL			llHdl->dbgLevel   /**< Debug level */
#define DBH					llHdl->dbgHdl     /**< Debug handle */

/* register offsets within PCI-Config-Space of ICH */
#define PMBASE			0x40	/**< PM base within i/o space (32-bit) */
#define PMSIZE			128		/**< PM space size */
#define GPIOBASE		0x48	/**< GPIO base within i/o space (32-bit) */
#define GPIOSIZE		64		/**< GPIO space size */
#if 0 /* note: info only, we will not change the routing */
#define GPI_ROUT			0xb8		/**< GPI routing control (32-bit) */
#define   GPI_ROUT_MASK_12	0x03000000
#define   GPI_ROUT_MASK_13	0x0c000000
#endif

/* GPI register offsets (from PMBASE) and bit definitions */
#define GPE0_STS		0x28			/**< GP Event 0 Status (32-bit) */
#define   GPI_12		  0x10000000	  /**< 0=low, 1=high, write 1 to clear */
#define   GPI_13		  0x20000000	  /**< 0=low, 1=high, write 1 to clear */

/* GPI register usage */
#define PS_REG			GPE0_STS		/**< CPCI power supply status */
#define   PS_DEG		  GPI_12		  /**< 1=temperature rise */
#define   PS_FAIL		  GPI_13		  /**< 1=power fail */

/* GPIO register offsets (from GPIOBASE) and bit definitions */
#define GP_IO_SEL		0x04			/**< GPIO Input/Output Select (32-bit) */
#define GP_LVL			0x0c			/**< GPIO Level (32-bit) */
#define   GPIO_20         0x00100000	  /**< 0=out/low, 1=in/high */
#define   GPIO_27 		  0x08000000	  /**< 0=out/low, 1=in/high */
#define GP_IO_SEL2		0x34			/**< GPIO Input/Output Select 2 (32-bit) */
#define GP_LVL2			0x38			/**< GPIO Level 2 (32-bit) */
#define   GPIO_33		  0x00000002	  /**< 0=out/low, 1=in/high */
#define   GPIO_34		  0x00000004	  /**< 0=out/low, 1=in/high */

/* GPIO register usage */
#define LED_DIR_REG		GP_IO_SEL		/**< LED direction */
#define LED_REG			GP_LVL			/**< LED state */

#define WD_DIR_REG		GP_IO_SEL2		/**< wdog direction */
#define   WD_DIS_IN		  GPIO_33		  /**< 1=in */
#define   WD_TRIG_IN	  GPIO_34		  /**< 1=in */
#define WD_STATE_REG	GP_LVL2			/**< wdog state */
#define   WD_DIS		  GPIO_33		  /**< 1=disable */
#define   WD_TRIG		  GPIO_34		  /**< 1=trigger */

/* Global */
#define VAR_PCI_DEV_ID_ICH8  0x00002811	/* pci LPC device-id - ICH8M(F18) */

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** low-level handle */
typedef struct {
	/* general */
    int32           memAlloc;		/**< Size allocated for the handle */
    OSS_HANDLE      *osHdl;         /**< OSS handle */
    DESC_HANDLE     *descHdl;       /**< DESC handle */
    MACCESS         maGpio;         /**< HW access handle for GPIO space */
#ifndef WDOG_ONLY
    MACCESS         maPm;           /**< HW access handle for PM space */
    int32			ledOnIn;		/**< 1=in */
    int32			ledOff;	    /**< 1=off */
#endif
	MDIS_IDENT_FUNCT_TBL idFuncTbl;	/**< ID function table */
	/* debug */
    u_int32         dbgLevel;		/**< Debug level */
	DBG_HANDLE      *dbgHdl;        /**< Debug handle */
	/* misc */
#ifdef WDOG
	u_int8			wdStatus;		/**< Watchdog status */
#endif
    u_int32         pciBus;         /**< PCI bus */
    u_int32         pciDev;         /**< PCI dev */
    u_int32         pciFunc;        /**< PCI func */
    int32			DevID;
} LL_HANDLE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>   /* low-level driver jump table  */
#include <MEN/f14_mon_drv.h>   /* F14_MON driver header file */

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 F14_MON_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
					   MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
					   OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 F14_MON_Exit(LL_HANDLE **llHdlP );
static int32 F14_MON_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 F14_MON_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 F14_MON_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code,
								INT32_OR_64 value32_or_64 );
static int32 F14_MON_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code,
								INT32_OR_64 *value32_or_64P );
static int32 F14_MON_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							int32 *nbrRdBytesP);
static int32 F14_MON_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							 int32 *nbrWrBytesP);
static int32 F14_MON_Irq(LL_HANDLE *llHdl );
static int32 F14_MON_Info(int32 infoType, ... );

static char* Ident( void );
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);


/****************************** F14_MON_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT Pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
    extern void LL_GetEntry( LL_ENTRY* drvP )
#else
    extern void __F14_MON_GetEntry( LL_ENTRY* drvP )
#endif /* _ONE_NAMESPACE_PER_DRIVER_ */
{
    drvP->init        = F14_MON_Init;
    drvP->exit        = F14_MON_Exit;
    drvP->read        = F14_MON_Read;
    drvP->write       = F14_MON_Write;
    drvP->blockRead   = F14_MON_BlockRead;
    drvP->blockWrite  = F14_MON_BlockWrite;
    drvP->setStat     = F14_MON_SetStat;
    drvP->getStat     = F14_MON_GetStat;
    drvP->irq         = F14_MON_Irq;
    drvP->info        = F14_MON_Info;
}

/******************************** F14_MON_Init **********************************/
/** Allocate and return low-level handle, initialize hardware
 *
 * The function initializes the HW with the definitions made
 * in the descriptor.
 *
 * The function decodes \ref descriptor_entries "these descriptor entries"
 * in addition to the general descriptor keys.
 *
 *  \param descP      \IN  Pointer to descriptor data
 *  \param osHdl      \IN  OSS handle
 *  \param ma         \IN  HW access handle
 *  \param devSemHdl  \IN  Device semaphore handle
 *  \param irqHdl     \IN  IRQ handle
 *  \param llHdlP     \OUT Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 */
static int32 F14_MON_Init(
    DESC_SPEC       *descP,
    OSS_HANDLE      *osHdl,
    MACCESS         *ma,
    OSS_SEM_HANDLE  *devSemHdl,
    OSS_IRQ_HANDLE  *irqHdl,
    LL_HANDLE       **llHdlP
)
{
    LL_HANDLE *llHdl = NULL;
    u_int32 gotsize;
    int32	error;
    u_int32	value;
	int32	physBase;

    /*------------------------------+
    |  prepare the handle           |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
    if ((llHdl = (LL_HANDLE*)OSS_MemGet(
    				osHdl, sizeof(LL_HANDLE), &gotsize)) == NULL)
       return(ERR_OSS_MEM_ALLOC);

	/* clear */
    OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
    llHdl->memAlloc   = gotsize;
    llHdl->osHdl      = osHdl;

	llHdl->pciBus	  = MAC_MAHDL2PCI_BUS(*ma);
	llHdl->pciDev	  = MAC_MAHDL2PCI_DEV(*ma);
	llHdl->pciFunc	= MAC_MAHDL2PCI_FUNC(*ma);

    /*------------------------------+
    |  init id function table       |
    +------------------------------*/
	/* driver's ident function */
	llHdl->idFuncTbl.idCall[0].identCall = Ident;
	/* library's ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;

    /*------------------------------+
    |  prepare debugging            |
    +------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;	/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

    /*------------------------------+
    |  scan descriptor              |
    +------------------------------*/
	/* prepare access */
    if ((error = DESC_Init(descP, osHdl, &llHdl->descHdl)))
		return( Cleanup(llHdl,error) );

    /* DEBUG_LEVEL_DESC */
    if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&value, "DEBUG_LEVEL_DESC")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

    /* DEBUG_LEVEL */
    if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&llHdl->dbgLevel, "DEBUG_LEVEL")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    DBGWRT_1((DBH, "LL - F14_MON_Init\n"));

	/* get Device ID */
	if( (error = OSS_PciGetConfig(
		osHdl, llHdl->pciBus, llHdl->pciDev, llHdl->pciFunc,
		OSS_PCI_DEVICE_ID, &(llHdl->DevID) ))
	) return error;

    /*------------------------------+
    |  get and map address spaces   |
    +------------------------------*/
#ifndef WDOG_ONLY
	/* PMBASE */
	if ( (error = OSS_PciGetConfig(
		osHdl, llHdl->pciBus, llHdl->pciDev, llHdl->pciFunc,
		OSS_PCI_ACCESS_32 | PMBASE, &physBase))
	) return error;

	/* not io mapped? */
	if( !(physBase & 0x01) ){
		DBGWRT_ERR((DBH," *** LL - F14_MON_Init: PMBASE=0x%x not valid\n",
			physBase));
		return ERR_LL_ILL_PARAM;
	}

	/* mask out bit 1..0 and map*/
	if ( (error = OSS_MapPhysToVirtAddr( osHdl, (void*)((INT32_OR_64)(physBase & ~0x3)), PMSIZE,
		OSS_ADDRSPACE_IO, OSS_BUSTYPE_PCI,
		llHdl->pciBus, (void**)&llHdl->maPm ))
	) return error;

	/* GPIO LED STATUS */
	if( llHdl->DevID == VAR_PCI_DEV_ID_ICH8 )
		{
			llHdl->ledOnIn = GPIO_20;
			llHdl->ledOff = GPIO_20;
		}
	else
		{
			llHdl->ledOnIn = GPIO_27;
			llHdl->ledOff = GPIO_27;
		}
#endif

	/* GPIOBASE */
	if ( (error = OSS_PciGetConfig(
		osHdl, llHdl->pciBus, llHdl->pciDev, llHdl->pciFunc,
		OSS_PCI_ACCESS_32 | GPIOBASE, &physBase))
	) return error;

	/* not io mapped? */
	if( !(physBase & 0x01) ){
		DBGWRT_ERR((DBH," *** LL - F14_MON_Init: PMBASE=0x%x not valid\n",
			physBase));
		return ERR_LL_ILL_PARAM;
	}

	/* mask out bit 1..0 and map*/
	if ( (error = OSS_MapPhysToVirtAddr( osHdl, (void*)((INT32_OR_64)(physBase & ~0x3)), GPIOSIZE,
		OSS_ADDRSPACE_IO, OSS_BUSTYPE_PCI,
		llHdl->pciBus, (void**)&llHdl->maGpio ))
	) return error;

    /*------------------------------+
    |  init hardware                |
    +------------------------------*/
#ifndef WDOG_ONLY
	/* configure GPIO for LED to out */
	MCLRMASK_D32( llHdl->maGpio, LED_DIR_REG, llHdl->ledOnIn );
#endif

#ifdef WDOG
	/* configure GPIO for WDOG to out */
	MCLRMASK_D32( llHdl->maGpio, WD_DIR_REG, WD_DIS_IN | WD_TRIG_IN );

	/* get WDOG state */
	value = MREAD_D32( llHdl->maGpio, WD_STATE_REG );
	if( value & WD_DIS )
		llHdl->wdStatus = 0;
	else
		llHdl->wdStatus = 1;
#endif

	/* note: GPI routing for CPCI power supply status will not be touched */

	*llHdlP = llHdl;	/* set low-level driver handle */

	return(ERR_SUCCESS);
}

/****************************** F14_MON_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes the HW, the watchdog will be disabled.
 *
 *  \param llHdlP      \IN  Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 */
static int32 F14_MON_Exit(
   LL_HANDLE    **llHdlP
)
{
    LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;

    DBGWRT_1((DBH, "LL - F14_MON_Exit\n"));

    /*------------------------------+
    |  de-init hardware             |
    +------------------------------*/
#ifdef WDOG
	/* stop WDOG */
	MSETMASK_D32( llHdl->maGpio, WD_STATE_REG, WD_DIS );
#endif

    /*------------------------------+
    |  clean up memory              |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl,error);

	return(error);
}

/****************************** F14_MON_Read ************************************/
/** Read a value from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param valueP     \OUT Read value
 *
 *  \return           \c ERR_LL_ILL_FUNC
 */
static int32 F14_MON_Read(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 *valueP
)
{
    DBGWRT_1((DBH, "LL - F14_MON_Read: ch=%d\n",ch));

	return(ERR_LL_ILL_FUNC);
}

/****************************** F14_MON_Write ***********************************/
/** Description:  Write a value to the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param value      \IN  Read value
 *
 *  \return           \c ERR_LL_ILL_FUNC
 */
static int32 F14_MON_Write(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 value
)
{
    DBGWRT_1((DBH, "LL - F14_MON_Write: ch=%d\n",ch));

	return(ERR_LL_ILL_FUNC);
}

/****************************** F14_MON_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl  	  \IN  Low-level handle
 *  \param code       \IN  \ref getstat_setstat_codes "status code"
 *  \param ch         \IN  Current channel
 *  \param value32_or_64  \IN  Data or pointer to block data structure (M_SG_BLOCK)
 *                             for block status codes
 *  \return           \c 0 On success or error code
 */
static int32 F14_MON_SetStat(
    LL_HANDLE *llHdl,
    int32  code,
    int32  ch,
    INT32_OR_64 value32_or_64
)
{
	int32 error = ERR_SUCCESS;

	int32		value  	= (int32)value32_or_64;	/* 32bit value */
	INT32_OR_64	valueP 	= value32_or_64;		/* stores 32/64bit pointer */
	(void) valueP;
    DBGWRT_1((DBH, "LL - F14_MON_SetStat: ch=%d code=0x%04x value=0x%x\n",
			  ch,code,value));

    switch(code) {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            llHdl->dbgLevel = value;
            break;
#ifdef WDOG
        /*--------------------------+
        |  start WDOG               |
        +--------------------------*/
        case WDOG_START:
			MCLRMASK_D32( llHdl->maGpio, WD_STATE_REG, WD_DIS );
			llHdl->wdStatus = 1;
            break;
        /*--------------------------+
        |  stop WDOG                |
        +--------------------------*/
        case WDOG_STOP:
			MSETMASK_D32( llHdl->maGpio, WD_STATE_REG, WD_DIS );
			llHdl->wdStatus = 0;
            break;
        /*--------------------------+
        |  trigger WDOG             |
        +--------------------------*/
        case WDOG_TRIG:
			/* watchdog not enabled ? */
			if ( llHdl->wdStatus == 0 ) {
				error = ERR_LL_DEV_NOTRDY;
				break;
			}
			/* trigger (H-->L) */
			MSETMASK_D32( llHdl->maGpio, WD_STATE_REG, WD_TRIG );
			OSS_Delay( llHdl->osHdl, 1 );
			MCLRMASK_D32( llHdl->maGpio, WD_STATE_REG, WD_TRIG );
            break;
#endif
#ifndef WDOG_ONLY
        /*--------------------------+
        |  reset power supply state |
        +--------------------------*/
        case F14_MON_PS_STATE:
			MSETMASK_D32( llHdl->maPm, PS_REG, value & (PS_FAIL | PS_DEG) );
            break;
        /*--------------------------+
        |  switch LED               |
        +--------------------------*/
        case F14_MON_STATUS_LED:
			/* on */
			if( value ){
				MCLRMASK_D32( llHdl->maGpio, LED_REG, llHdl->ledOff );
			}
			/* off */
			else{
				MSETMASK_D32( llHdl->maGpio, LED_REG, llHdl->ledOff );
			}
           	 break;
#endif
        /*--------------------------+
        |  unknown                  |
        +--------------------------*/
        default:
            error = ERR_LL_UNK_CODE;
    }

	return(error);
}

/****************************** F14_MON_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param code       \IN  \ref getstat_setstat_codes "status code"
 *  \param ch         \IN  Current channel
 *  \param value32_or_64P  \IN  Pointer to block data structure (M_SG_BLOCK) for
 *                         block status codes
 *  \param value32_or_64P  \OUT Data pointer or pointer to block data structure
 *                         (M_SG_BLOCK) for block status codes
 *
 *  \return           \c 0 On success or error code
 */
static int32 F14_MON_GetStat(
    LL_HANDLE *llHdl,
    int32  code,
    int32  ch,
    INT32_OR_64 *value32_or_64P
)
{
	int32 error = ERR_SUCCESS;

	int32		*valueP		= (int32*)value32_or_64P;	/* pointer to 32bit value  */
	INT32_OR_64	*value64P	= value32_or_64P;		 	/* stores 32/64bit pointer */

    DBGWRT_1((DBH, "LL - F14_MON_GetStat: ch=%d code=0x%04x\n",
			  ch,code));

    switch(code)
    {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            *valueP = llHdl->dbgLevel;
            break;
        /*--------------------------+
        |  number of channels       |
        +--------------------------*/
        case M_LL_CH_NUMBER:
            *valueP = CH_NUMBER;
            break;
        /*--------------------------+
        |  channel type info        |
        +--------------------------*/
        case M_LL_CH_TYP:
#ifdef WDOG
            *valueP = M_CH_PROFILE_WDOG;
#else
            *valueP = M_CH_UNKNOWN;
#endif
            break;
#ifdef WDOG
		/*--------------------------+
        |  watchdog state           |
        +--------------------------*/
        case WDOG_STATUS:
			*valueP = llHdl->wdStatus;
            break;
#endif
#ifndef WDOG_ONLY
        /*--------------------------+
        |  power supply state       |
        +--------------------------*/
        case F14_MON_PS_STATE:
		{
			int32 val = MREAD_D32( llHdl->maPm, PS_REG );
			*valueP = 0;
			if( val & PS_FAIL )
				*valueP |= F14_MON_PS_FAIL;
			if( val & PS_DEG )
				*valueP |= F14_MON_PS_DEG;
            break;
		}
#endif
		/*--------------------------+
        |   ident table pointer     |
        |   (treat as non-block!)   |
        +--------------------------*/
        case M_MK_BLK_REV_ID:
           *value64P = (INT32_OR_64)&llHdl->idFuncTbl;
           break;
        /*--------------------------+
        |  unknown                  |
        +--------------------------*/
        default:
            error = ERR_LL_UNK_CODE;
    }

	return(error);
}

/******************************* F14_MON_BlockRead ******************************/
/** Read a data block from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl       \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrRdBytesP \OUT Number of read bytes
 *
 *  \return            \c ERR_LL_ILL_FUNC
 */
static int32 F14_MON_BlockRead(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrRdBytesP
)
{
    DBGWRT_1((DBH, "LL - F14_MON_BlockRead: ch=%d, size=%d\n",ch,size));

	/* return number of read bytes */
	*nbrRdBytesP = 0;

	return(ERR_LL_ILL_FUNC);
}

/****************************** F14_MON_BlockWrite ******************************/
/** Write a data block from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrWrBytesP \OUT Number of written bytes
 *
 *  \return            \c ERR_LL_ILL_FUNC
 */
static int32 F14_MON_BlockWrite(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrWrBytesP
)
{
    DBGWRT_1((DBH, "LL - F14_MON_BlockWrite: ch=%d, size=%d\n",ch,size));

	/* return number of written bytes */
	*nbrWrBytesP = 0;

	return(ERR_LL_ILL_FUNC);
}


/****************************** F14_MON_Irq ************************************/
/** Interrupt service routine
 *
 *  The driver supports no interrupts and always returns LL_IRQ_DEVICE.
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \return LL_IRQ_DEVICE	IRQ caused by device
 *          LL_IRQ_DEV_NOT  IRQ not caused by device
 *          LL_IRQ_UNKNOWN  Unknown
 */
static int32 F14_MON_Irq(
   LL_HANDLE *llHdl
)
{
    IDBGWRT_1((DBH, ">>> F14_MON_Irq:\n"));

	return(LL_IRQ_DEVICE);
}

/****************************** F14_MON_Info ***********************************/
/** Get information about hardware and driver requirements
 *
 *  The following info codes are supported:
 *
 * \code
 *  Code                      Description
 *  ------------------------  -----------------------------
 *  LL_INFO_HW_CHARACTER      Hardware characteristics
 *  LL_INFO_ADDRSPACE_COUNT   Number of required address spaces
 *  LL_INFO_ADDRSPACE         Address space information
 *  LL_INFO_IRQ               Interrupt required
 *  LL_INFO_LOCKMODE          Process lock mode required
 * \endcode
 *
 *  The LL_INFO_HW_CHARACTER code returns all address and
 *  data modes (ORed) which are supported by the hardware
 *  (MDIS_MAxx, MDIS_MDxx).
 *
 *  The LL_INFO_ADDRSPACE_COUNT code returns the number
 *  of address spaces used by the driver.
 *
 *  The LL_INFO_ADDRSPACE code returns information about one
 *  specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *  data mode represents the widest hardware access used by
 *  the driver.
 *
 *  The LL_INFO_IRQ code returns whether the driver supports an
 *  interrupt routine (TRUE or FALSE).
 *
 *  The LL_INFO_LOCKMODE code returns which process locking
 *  mode the driver needs (LL_LOCK_xxx).
 *
 *  \param infoType	   \IN  Info code
 *  \param ...         \IN  Argument(s)
 *
 *  \return            \c 0 On success or error code
 */
static int32 F14_MON_Info(
   int32  infoType,
   ...
)
{
    int32   error = ERR_SUCCESS;
    va_list argptr;

    va_start(argptr, infoType );

    switch(infoType) {
		/*-------------------------------+
        |  hardware characteristics      |
        |  (all addr/data modes ORed)   |
        +-------------------------------*/
        case LL_INFO_HW_CHARACTER:
		{
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);

			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD08 | MDIS_MD16;
			break;
	    }
		/*-------------------------------+
        |  nr of required address spaces |
        |  (total spaces used)           |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE_COUNT:
		{
			u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

			*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
			break;
	    }
		/*-------------------------------+
        |  address space type            |
        |  (widest used data mode)       |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE:
		{
			u_int32 addrSpaceIndex = va_arg(argptr, u_int32);
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);
			u_int32 *addrSizeP = va_arg(argptr, u_int32*);

			if (addrSpaceIndex >= ADDRSPACE_COUNT)
				error = ERR_LL_ILL_PARAM;
			else {
				*addrModeP = MDIS_MA_PCICFG;
				*dataModeP = MDIS_MD_NONE;
				*addrSizeP = ADDRSPACE_SIZE;
			}

			break;
	    }
		/*-------------------------------+
        |   interrupt required           |
        +-------------------------------*/
        case LL_INFO_IRQ:
		{
			u_int32 *useIrqP = va_arg(argptr, u_int32*);

			*useIrqP = USE_IRQ;
			break;
	    }
		/*-------------------------------+
        |   process lock mode            |
        +-------------------------------*/
        case LL_INFO_LOCKMODE:
		{
			u_int32 *lockModeP = va_arg(argptr, u_int32*);

			*lockModeP = LL_LOCK_CALL;
			break;
	    }
		/*-------------------------------+
        |   (unknown)                    |
        +-------------------------------*/
        default:
          error = ERR_LL_ILL_PARAM;
    }

    va_end(argptr);
    return(error);
}

/*******************************  Ident  ***********************************/
/** Return ident string
 *
 *  \return            Pointer to ident string
 */
static char* Ident( void )
{
    return( "F14_MON - F14_MON low-level driver: $Id: f14_mon_drv.c,v 1.5 2009/08/31 12:17:53 MRoth Exp $" );
}

/********************************* Cleanup *********************************/
/** Close all handles, free memory and return error code
 *
 *	\warning The low-level handle is invalid after this function is called.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param retCode    \IN  Return value
 *
 *  \return           \IN   retCode
 */
static int32 Cleanup(
   LL_HANDLE    *llHdl,
   int32        retCode
)
{
    /*------------------------------+
    |  free resources               |
    +------------------------------*/
#ifndef WDOG_ONLY
	if( llHdl->maPm )
		OSS_UnMapVirtAddr( llHdl->osHdl, (void**)&llHdl->maPm,
			PMSIZE, OSS_ADDRSPACE_IO );
#endif
	if( llHdl->maGpio )
		OSS_UnMapVirtAddr( llHdl->osHdl, (void**)&llHdl->maGpio,
			GPIOSIZE, OSS_ADDRSPACE_IO );

    /*------------------------------+
    |  close handles                |
    +------------------------------*/
	/* clean up desc */
	if (llHdl->descHdl)
		DESC_Exit(&llHdl->descHdl);

	/* clean up debug */
	DBGEXIT((&DBH));

    /*------------------------------+
    |  free memory                  |
    +------------------------------*/
    /* free my handle */
    OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

    /*------------------------------+
    |  return error code            |
    +------------------------------*/
	return(retCode);
}
