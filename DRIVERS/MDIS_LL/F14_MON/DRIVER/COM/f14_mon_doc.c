/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  f14_mon_doc.c
 *
 *      \author  dp
 *        $Date: 2007/11/20 14:56:28 $
 *    $Revision: 1.4 $
 *
 *      \brief   User documentation for F14_MON driver
 *
 *     Required: -
 *
 *     \switches -
 */
 /*
 *---------------------------------------------------------------------------
 * (c) Copyright 2005 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
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

/*! \mainpage
    This is the documentation of the F14_MON MDIS low-level driver that supports
    the F14/F15/F17/F18/D601 monitoring features.

    The driver supports the following HW features:\n
	- System watchdog
    - CPCI power supply status (power fail, temperature rise)
    - Board status LED

    \n
    \section Variants Variants
    In order to support different CPU HW, the F14_MON driver can be built in
	the following variants at compilation time:

    \code
    Driver              Variant Description
    --------            --------------------------------
    Standard            F14_MON driver for F14/F15/F17/F18/D601 CPU boards
    _wdog               F14_MON_WDOG driver for F14/F15/D601 CPU boards (HW rev. 00.xx)
    _sc14               F14_MON_SC14 driver for SC14 CPU board
    \endcode

    Notes:\n
	- The standard driver supports no watchdog functionality.
	- The F14_MON_WDOG driver variant supports system watchdog for 02F014/02F015/
	  CPU boards with HW rev. 00.xx.
    - The F14_MON_SC14 driver variant supports only the system watchdog of SC14 CPU boards.
		
    \n \section FuncDesc Functional Description

    \n \subsection General General
	The driver supports the watchdog functionality (start, stop, trigger and state),
	the CPCI power supply status lines PS_FAIL (power fail) and PS_DEG
	(temperature rise) and the functionality to switch the board status LED
    on/off with the M_setstat() / M_getstat() functions.

	When the first path is opened to the F14_MON device, the HW and the driver
	are being initialized with default values.

    \n \subsection channels Logical channels
    The driver provides only one logical channel\n

    \n \section api_functions Supported API Functions

    <table border="0">
    <tr>
        <td><b>API function</b></td>
        <td><b>Functionality</b></td>
        <td><b>Corresponding low level function</b></td></tr>

    <tr><td>M_open()</td><td>Open device</td>
	<td>F14_MON_Init()</td></tr>
    <tr><td>M_close()     </td><td>Close device             </td>
    <td>F14_MON_Exit())</td></tr>
    <tr><td>M_setstat()   </td><td>Set device parameter     </td>
    <td>F14_MON_SetStat()</td></tr>
    <tr><td>M_getstat()   </td><td>Get device parameter     </td>
    <td>F14_MON_GetStat()</td></tr>
    <tr><td>M_errstringTs() </td><td>Generate error message </td>
    <td>-</td></tr>
    </table>

    \n \section descriptor_entries Descriptor Entries
    
    The low-level driver initialization routine decodes only the general
	descriptor entries ("keys").

    \n \subsection f14_mon_min   Minimum descriptor
    f14_mon_min.dsc, f14_mon_sc14_min.dsc (see Examples section)\n
    Demonstrates the minimum set of options necessary for using the driver.

    \n \subsection f14_mon_max   Maximum descriptor
    f14_mon_max.dsc, f14_mon_sc14_max.dsc (see Examples section)\n
    Shows all possible configuration options for this driver.

    \n \section codes F14_MON specific Getstat/Setstat codes
	
	For the watchdog functionality, the driver supports a subset of the WDOG
	Getstat/Setstat codes (defined in wdog.h): 
	- #WDOG_START
	- #WDOG_STOP
	- #WDOG_TRIG
	- #WDOG_STATUS\n

	The other driver functionality is supported by the F14_MON Getstat/Setstat
	codes.
	See \ref getstat_setstat_codes "section about F14_MON Getstat/Setstat codes".

    \n \section programs Overview of provided programs

    \subsection f14_mon_simp  Simple example for using the driver
    f14_mon_simp.c (see example section)

    \subsection wdog_test  Tool to service and test the watchdog
    wdog_test.c (see example section)

*/

/** \example f14_mon_simp.c */
/** \example wdog_test.c */
/** \example f14_mon_min.dsc */
/** \example f14_mon_max.dsc */
/** \example f14_mon_wdog_min.dsc */
/** \example f14_mon_wdog_max.dsc */
/** \example f14_mon_sc14_min.dsc */
/** \example f14_mon_sc14_max.dsc */

/*! \page dummy
  \menimages
*/

