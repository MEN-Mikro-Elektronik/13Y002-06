#************************** MDIS4 device descriptor *************************
#
#        Author: dp
#         $Date: 2006/11/02 13:26:41 $
#     $Revision: 1.2 $
#
#   Description: Metadescriptor for F14_MON_SC14
#
#****************************************************************************

F14_MON_SC14_1  {
	#------------------------------------------------------------------------
	#	general parameters (don't modify)
	#------------------------------------------------------------------------
    DESC_TYPE        = U_INT32  1           # descriptor type (1=device)
    HW_TYPE          = STRING   F14_MON     # hardware name of device

	#------------------------------------------------------------------------
	#	reference to base board
	#------------------------------------------------------------------------
    BOARD_NAME       = STRING   PCI_1       # device name of baseboard
    DEVICE_SLOT      = U_INT32  0           # used slot on baseboard (0..n)

	#------------------------------------------------------------------------
	#	device parameters
	#------------------------------------------------------------------------
}
