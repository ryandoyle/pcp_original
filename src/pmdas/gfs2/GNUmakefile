#
# Copyright (c) 2013 Red Hat.
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

TOPDIR = ../../..
include	$(TOPDIR)/src/include/builddefs

CFILES		= glocks.c sbstats.c dynamic.c pmda.c
HFILES		= glocks.h sbstats.h dynamic.h pmdagfs2.h
CMDTARGET	= pmdagfs2
LLDLIBS		= $(PCP_PMDALIB)
LSRCFILES	= Install Remove pmns root help

IAM		= gfs2
DOMAIN		= GFS2
PMDADIR		= $(PCP_PMDAS_DIR)/$(IAM)

LDIRT		= domain.h $(IAM).log

default:	build-me

include $(BUILDRULES)

ifeq "$(TARGET_OS)" "linux"
build-me: domain.h $(CMDTARGET)

install: default
	$(INSTALL) -m 755 -d $(PMDADIR)
	$(INSTALL) -m 755 Install Remove $(CMDTARGET) $(PMDADIR)
	$(INSTALL) -m 644 root pmns domain.h help $(PMDADIR)
else
build-me:
install:
endif

default_pcp : default

install_pcp : install

domain.h: ../../pmns/stdpmid
	$(DOMAIN_MAKERULE)

pmda.o glocks.o:	glocks.h
pmda.o sbstats.o:	sbstats.h
pmda.o dynamic.o sbstats.o:	dynamic.h
pmda.o dynamic.o:	pmdagfs2.h
