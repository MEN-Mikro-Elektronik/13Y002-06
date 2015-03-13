#***************************  M a k e f i l e  *******************************
#
#         Author: dp
#          $Date: 2006/11/02 14:04:19 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the F14_MON_WDOG driver
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: driver_wdog.mak,v $
#   Revision 1.1  2006/11/02 14:04:19  DPfeuffer
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2006 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=f14_mon_wdog

MAK_SWITCH=$(SW_PREFIX)MAC_IO_MAPPED \
           $(SW_PREFIX)F14_MON_VARIANT=F14_MON_SC14 \
           $(SW_PREFIX)WDOG

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/desc$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/dbg$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/f14_mon_drv.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/oss.h		\
         $(MEN_INC_DIR)/mdis_err.h	\
         $(MEN_INC_DIR)/maccess.h	\
         $(MEN_INC_DIR)/desc.h		\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/mdis_com.h	\
         $(MEN_INC_DIR)/ll_defs.h	\
         $(MEN_INC_DIR)/ll_entry.h	\
         $(MEN_INC_DIR)/dbg.h		\
         $(MEN_INC_DIR)/wdog.h		\

MAK_INP1=f14_mon_drv$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
