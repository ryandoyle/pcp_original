#
# Copyright (c) 2000,2003,2004 Silicon Graphics, Inc.  All Rights Reserved.
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

TOPDIR = ../../../..
include $(TOPDIR)/src/include/builddefs

TARGETS	= trace$(EXECSUFFIX)
CFILES	= trace.c client.c comms.c data.c pmda.c
HFILES	= data.h client.h comms.h

LLDFLAGS= -L$(TOPDIR)/src/libpcp_trace/src
ifeq "$(TARGET_OS)" "darwin"
# __htonll() is in libpcp and libpcp_trace ... ld on Mac OS X has a
# fit over this, sigh ... thanks to Nick Blievers for finding the
# magic spell needed to workaround this.
#LLDFLAGS += -Wl,-m -Wl,-bind_at_load
# Update: -Wl,-m is apparently now obsolete
LLDFLAGS += -Wl,-bind_at_load
endif
LLDLIBS	= -lpcp_trace $(PCP_PMDALIB)

IAM	= trace
DOMAIN	= TRACE
PMDADIR	= $(PCP_PMDAS_DIR)/$(IAM)

LDIRT		= *.log *.dir *.pag domain.h $(TARGETS)

default:	build-me

include $(BUILDRULES)

ifneq "$(TARGET_OS)" "mingw"
build-me:	$(TARGETS)

install:	build-me
	$(INSTALL) -m 755 -d $(PMDADIR)
	$(INSTALL) -m 755 $(IAM) $(PMDADIR)/pmda$(IAM)
	$(INSTALL) -m 644 domain.h $(PMDADIR)/domain.h
else
build-me:
install:
endif

$(IAM)$(EXECSUFFIX):	$(OBJECTS)

comms.o trace.o pmda.o:	domain.h

domain.h:	../../../pmns/stdpmid
	$(DOMAIN_MAKERULE)

default_pcp:	default

install_pcp:	install
