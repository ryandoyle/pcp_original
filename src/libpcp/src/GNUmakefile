#
# Copyright (c) 2012-2013 Red Hat.
# Copyright (c) 2008 Aconex.  All Rights Reserved.
# Copyright (c) 2000,2003,2004 Silicon Graphics, Inc.  All Rights Reserved.
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
include $(TOPDIR)/src/include/builddefs
-include ./GNUlocaldefs

CFILES = connect.c context.c desc.c err.c fetch.c freeresult.c \
	help.c instance.c p_desc.c p_error.c p_fetch.c p_instance.c \
	p_profile.c p_result.c p_text.c p_pmns.c p_creds.c p_userauth.c \
        pdu.c pdubuf.c pmns.c profile.c store.c units.c util.c ipc.c \
	sortinst.c logmeta.c logportmap.c logutil.c tz.c interp.c \
	checksum.c rtime.c tv.c spec.c fetchlocal.c optfetch.c AF.c \
	stuffvalue.c endian.c config.c access.c auxconnect.c auxserver.c \
	p_lcontrol.c p_lrequest.c p_lstatus.c logconnect.c logcontrol.c \
	connectlocal.c derive.c derive_fetch.c events.c lock.c hash.c \
	fault.c

HFILES = derive.h internal.h

LSRCFILES = check-statics

ifeq "$(ENABLE_SECURE)" "true"
LLDLIBS += $(LIB_FOR_SSL) $(LIB_FOR_NSS) $(LIB_FOR_NSPR) $(LIB_FOR_SASL)
LCFLAGS += $(NSSCFLAGS) $(NSPRCFLAGS) $(SASLCFLAGS)
CFILES += secureserver.c secureconnect.c
else
LSRCFILES += secureserver.c secureconnect.c
endif

ifneq "$(TARGET_OS)" "mingw"
LSRCFILES += win32.c
else
CFILES += win32.c
LLDLIBS	+= -lpsapi
endif

ifeq "$(LIB_FOR_BASENAME)" "-lpcp"
# don't need to be linked to myself in this case!
LIB_FOR_BASENAME =
endif

LLDLIBS	+= $(LIB_FOR_MATH) $(LIB_FOR_PTHREADS)

LCFLAGS += -DLIBPCP_INTERNAL '-DEXEC_SUFFIX="$(EXECSUFFIX)"' \
	'-DDSO_SUFFIX="$(DSOSUFFIX)"'

DSOVERSION = 3
STATICLIBTARGET = libpcp.a
LIBTARGET = libpcp.$(DSOSUFFIX).$(DSOVERSION)
SYMTARGET = libpcp.$(DSOSUFFIX) libpcp.$(DSOSUFFIX).2

ifeq "$(PACKAGE_DISTRIBUTION)" "debian"
SYMTARGET = libpcp.$(DSOSUFFIX)
endif
ifeq "$(TARGET_OS)" "darwin"
LIBTARGET = libpcp.$(DSOVERSION).$(DSOSUFFIX)
SYMTARGET = libpcp.$(DSOSUFFIX)
endif
ifeq "$(TARGET_OS)" "mingw"
STATICLIBTARGET = 
LIBTARGET = libpcp.$(DSOSUFFIX)
SYMTARGET =
endif
ifeq "$(ENABLE_SHARED)" "no"
LIBTARGET = 
SYMTARGET =
endif

LDIRT += $(SYMTARGET) check.done links.done
LDIRT += nss_server.o nss_connect.o

base default : $(LIBTARGET) check.done links.done $(SYMTARGET) $(STATICLIBTARGET)

ifneq "$(SYMTARGET)" ""
$(SYMTARGET): 
	$(LN_S) -f $(LIBTARGET) $@
endif

include $(BUILDRULES)

derive.o derive_fetch.o: derive.h

util.o:	$(TOPDIR)/src/include/pcp/pmdbg.h

auxserver.o auxconnect.o: internal.h
secureserver.o secureconnect.o: internal.h
context.o derive.o fetch.o fetch.o lock.o: internal.h

$(OBJECTS):	$(TOPDIR)/src/include/pcp/pmapi.h \
		$(TOPDIR)/src/include/pcp/impl.h \
		$(TOPDIR)/src/include/pcp/platform_defs.h

ifeq "$(TARGET_OS)" "mingw"
kernel_pmda_dso	= windows
else
kernel_pmda_dso	= $(TARGET_OS)
endif

install : default
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

default_pcp : default

install_pcp : install

$(TOPDIR)/src/pmns/stdpmid:
	cd $(@D); $(MAKE) $(@F)

# The library is thread-safe ... check-statics will force a build failure
# if there has been any change to the static variables and their disposition
# ... refer to check-statics to add exceptions and annotations for new
# cases
#
check.done:	$(OBJECTS)
	./check-statics
	@touch check.done

# Populate a common lib directory for simplifying the builds of some other
# components that need to find multiple PCP libraries easily.  Also makes
# use of LD_LIBRARY_PATH for debugging exercises a little simpler.
# 
links.done: $(STATICLIBTARGET) $(LIBTARGET)
ifneq "$(LIBTARGET)" ""
	$(LN_S) -f $(shell pwd)/libpcp.$(DSOSUFFIX) $(TOPDIR)/src/lib
endif
ifneq "$(STATICLIBTARGET)" ""
	$(LN_S) -f $(shell pwd)/$(STATICLIBTARGET) $(TOPDIR)/src/lib
endif
	touch links.done
