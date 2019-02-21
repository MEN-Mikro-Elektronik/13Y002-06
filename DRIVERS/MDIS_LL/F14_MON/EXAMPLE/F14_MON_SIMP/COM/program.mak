#***************************  M a k e f i l e  *******************************
#
#         Author: dp
#          $Date: 2005/12/22 14:42:32 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the F14_MON example program
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2005-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=f14_mon_simp

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX) \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)

MAK_INCL=$(MEN_INC_DIR)/f14_mon_drv.h \
         $(MEN_INC_DIR)/wdog.h \
         $(MEN_INC_DIR)/men_typs.h \
         $(MEN_INC_DIR)/mdis_api.h \
         $(MEN_INC_DIR)/mdis_err.h \
         $(MEN_INC_DIR)/usr_oss.h \

MAK_INP1=f14_mon_simp$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
