#!gmake
#
# Copyright (c) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

LLDIRT		= *.log *.dir *.pag so_locations

TOPDIR = ../..
include $(TOPDIR)/src/include/builddefs

SUBDIRS = src
LSRCFILES = pmdaproc.sh rc_pcp rc_pmcd rc_local pmcd.options \
	    rc-proc.sh rc-proc.sh.minimal

default : $(SUBDIRS)
	$(SUBDIRS_MAKERULE)

install :: $(SUBDIRS)
	$(SUBDIRS_MAKERULE)
ifeq ($(TARGET_OS),darwin)
	$(INSTALL) -m 755 -d $(PCP_RC_DIR)
endif
	$(INSTALL) -m 755 -d `dirname $(PCP_PMCDOPTIONS_PATH)`
	$(INSTALL) -m 644 pmcd.options $(PCP_PMCDOPTIONS_PATH)
	$(INSTALL) -m 755 -d `dirname $(PCP_PMCDRCLOCAL_PATH)`
	$(INSTALL) -m 755 rc_local $(PCP_PMCDRCLOCAL_PATH)
	$(INSTALL) -m 755 rc_pmcd $(PCP_RC_DIR)/pmcd
	$(INSTALL) -m 755 rc_pcp $(PCP_RC_DIR)/pcp
	$(INSTALL) -m 644 pmdaproc.sh $(PCP_SHARE_DIR)/lib/pmdaproc.sh
	$(INSTALL) -m 644 rc-proc.sh $(PCP_SHARE_DIR)/lib/rc-proc.sh
	$(INSTALL) -m 644 rc-proc.sh.minimal $(PCP_SHARE_DIR)/lib/rc-proc.sh.minimal
	$(INSTALL) -m 1777 -d $(PCP_LOG_DIR)/pmcd
ifneq ($(PACKAGE_DISTRIBUTION),debian)
	$(INSTALL) -m 755 -d $(PCP_SYSCONF_DIR)/pmcd
endif

include $(BUILDRULES)

default_pcp : default

install_pcp : install
