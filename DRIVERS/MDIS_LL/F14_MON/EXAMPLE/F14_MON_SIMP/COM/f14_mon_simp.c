/****************************************************************************
 ************                                                    ************
 ************                   F14_MON_SIMP                     ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file f14_mon_simp.c
 *       \author dieter.pfeuffer@men.de
 *        $Date: 2009/08/31 12:17:55 $
 *    $Revision: 1.3 $
 *
 *       \brief  Simple example program for the F14_MON driver
 *
 *               - loop until keypress:
 *                 - switch status LED on/off
 *                 - get/reset power supply states
 *
 *     Required: libraries: mdis_api
 *     \switches (none)
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: f14_mon_simp.c,v $
 * Revision 1.3  2009/08/31 12:17:55  MRoth
 * R: Porting to MIDS5
 * M: added support for 64bit (MDIS_PATH)
 *
 * Revision 1.2  2006/11/02 13:26:44  DPfeuffer
 * watchdog functionality removed
 *
 * Revision 1.1  2005/12/22 14:42:31  dpfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2005 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: f14_mon_simp.c,v 1.3 2009/08/31 12:17:55 MRoth Exp $";

#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/f14_mon_drv.h>

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);

/********************************* main ************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	MDIS_PATH path;
	int32	value, ledState=0;
	char	*device;

	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: f14_mon_simp <device> <chan>\n");
		printf("Function: F14_MON example program for the F14_MON driver\n");
		printf("Options:\n");
		printf("    device       device name\n");
		printf("\n");
		return(1);
	}

	device = argv[1];

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return(1);
	}

	/*----------------------+
    |  loop until keypress  |
    +----------------------*/
	do {
		/* LED */
		ledState = !ledState;
		printf("- switch status LED %s\n", (ledState ? "on" : "off"));
		if ((M_setstat(path, F14_MON_STATUS_LED, ledState)) < 0) {
			PrintError("setstat F14_MON_STATUS_LED");
			goto abort;
		}

		/* Power Supply State */
		printf("- power supply state:");
		if ((M_getstat(path, F14_MON_PS_STATE, &value)) < 0) {
			PrintError("getstat F14_MON_PS_STATE");
			goto abort;
		}
		if(value & F14_MON_PS_FAIL)
			printf(" *** PS_FAIL occured ***");
		if(value & F14_MON_PS_DEG)
			printf(" *** PS_DEG occured ***");

		if( value ){
			printf(" - reset PS_XXX\n");
			if ((M_setstat(path, F14_MON_PS_STATE, 0)) < 0) {
				PrintError("setstat F14_MON_PS_STATE");
				goto abort;
			}
		}
		else
			printf(" ok\n");

		printf("Press any key to abort\n\n");
		UOS_Delay(1000);
	} while(UOS_KeyPressed() == -1);

	/*--------------------+
    |  cleanup            |
    +--------------------*/
	abort:
	if (M_close(path) < 0)
		PrintError("close");

	return(0);
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
*/
static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}


