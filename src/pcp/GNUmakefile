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

TOPDIR = ../..
include $(TOPDIR)/src/include/builddefs

LSRCFILES = pcp.sh pmlogger.config

default:

include $(BUILDRULES)

CONFIGDIR = $(PCP_SYSCONF_DIR)/pmlogger

install: default
	$(INSTALL) -m 755 pcp.sh $(PCP_BIN_DIR)/pcp$(SHELLSUFFIX)
	$(INSTALL) -m 644 pmlogger.config $(CONFIGDIR)/config.pcp

default_pcp : default

install_pcp : install
