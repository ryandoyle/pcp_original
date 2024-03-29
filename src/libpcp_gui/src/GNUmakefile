#
# Copyright (c) 2006 Aconex.  All Rights Reserved.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#

TOPDIR = ../../..
include	$(TOPDIR)/src/include/builddefs

CFILES = timeclient.c timestate.c record.c
LLDLIBS = -lpcp

STATICLIBTARGET = libpcp_gui.a

DSOVERSION = 2
LIBTARGET = libpcp_gui.$(DSOSUFFIX).$(DSOVERSION)
SYMTARGET = libpcp_gui.$(DSOSUFFIX) libpcp_gui.$(DSOSUFFIX).1

ifeq "$(PACKAGE_DISTRIBUTION)" "debian"
SYMTARGET = libpcp_gui.$(DSOSUFFIX)
endif
ifeq "$(TARGET_OS)" "darwin"
LIBTARGET = libpcp_gui.$(DSOVERSION).$(DSOSUFFIX)
endif
ifeq "$(TARGET_OS)" "mingw"
LIBTARGET = libpcp_gui.$(DSOSUFFIX)
STATICLIBTARGET =
SYMTARGET =
endif
ifeq "$(ENABLE_SHARED)" "no"
LIBTARGET =
SYMTARGET =
endif

LDIRT = $(LIBTARGET) $(SYMTARGET) $(STATICLIBTARGET) links.done

default: $(LIBTARGET) $(SYMTARGET) $(STATICLIBTARGET) links.done

ifneq "$(SYMTARGET)" ""
$(SYMTARGET):
	$(LN_S) -f $(LIBTARGET) $@
endif

include $(BUILDRULES)

install: default
ifneq "$(LIBTARGET)" ""
	$(INSTALL) -m 755 $(LIBTARGET) $(PCP_LIB_DIR)/$(LIBTARGET)
endif
ifneq "$(SYMTARGET)" ""
	for tt in $(SYMTARGET); do \
	    $(INSTALL) -S $(LIBTARGET) $(PCP_LIB_DIR)/$$tt || exit 1; \
	done
endif
ifneq "$(STATICLIBTARGET)" ""
	$(INSTALL) -m 755 $(STATICLIBTARGET) $(PCP_LIB_DIR)/$(STATICLIBTARGET)
endif

default_pcp:	default

install_pcp:	install

# Populate a common lib directory for simplifying the builds of some other
# components that need to find multiple PCP libraries easily.  Also makes
# use of LD_LIBRARY_PATH for debugging exercises a little simpler.
# 
links.done: $(STATICLIBTARGET) $(LIBTARGET)
ifneq "$(LIBTARGET)" ""
	$(LN_S) -f $(shell pwd)/libpcp_gui.$(DSOSUFFIX) $(TOPDIR)/src/lib
endif
ifneq "$(STATICLIBTARGET)" ""
	$(LN_S) -f $(shell pwd)/$(STATICLIBTARGET) $(TOPDIR)/src/lib
endif
	touch links.done
