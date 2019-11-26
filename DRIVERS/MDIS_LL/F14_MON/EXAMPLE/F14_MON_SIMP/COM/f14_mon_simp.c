/****************************************************************************
 ************                                                    ************
 ************                   F14_MON_SIMP                     ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file f14_mon_simp.c
 *       \author dieter.pfeuffer@men.de
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
 /*
 *---------------------------------------------------------------------------
 * Copyright 2005-2019, MEN Mikro Elektronik GmbH
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


