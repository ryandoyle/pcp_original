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

TOPDIR = ../../..
include $(TOPDIR)/src/include/builddefs

IAM	= solaris
DOMAIN	= SOLARIS
PMDADIR	= $(PCP_PMDAS_DIR)/$(IAM)
CMDTARGET = pmdasolaris
LIBTARGET = pmda_solaris.so
CONF_LINE = "solaris	75	dso	solaris_init	$(PMDADIR)/$(LIBTARGET)"

CFILES	= solaris.c data.c sysinfo.c disk.c zpool.c zfs.c \
	 zpool_perdisk.c netmib2.c netlink.c kvm.c arcstats.c vnops.c

BARE_NS = disk kernel mem network hinv zpool zfs zpool_perdisk
PMNS	= $(BARE_NS:%=pmns.%)
LSRCFILES = $(PMNS) help root common.h clusters.h netmib2.h
LDIRT	= domain.h *.log *.dir *.pag root_solaris

LLDLIBS = $(PCP_PMDALIB) -lkstat -lzfs -lnvpair -lkvm -ldevinfo

default: build-me

include $(BUILDRULES)

ifeq "$(TARGET_OS)" "solaris"
build-me: root_solaris domain.h $(LIBTARGET) $(CMDTARGET) help.dir help.pag
	@if [ `grep -c $(CONF_LINE) ../pmcd.conf` -eq 0 ]; then \
		echo $(CONF_LINE) >> ../pmcd.conf ; \
	fi

install: build-me
	$(INSTALL) -m 755 -d $(PMDADIR)
	$(INSTALL) -m 644 domain.h help.dir help.pag $(PMDADIR)
	$(INSTALL) -m 755 $(LIBTARGET) $(CMDTARGET) $(PMDADIR)
	$(INSTALL) -m 644 root_solaris $(PCP_VAR_DIR)/pmns/root_solaris
else
build-me:
install:
endif

default_pcp : default

install_pcp : install

$(OBJECTS):	common.h

help.dir help.pag: help root_solaris
	$(RUN_IN_BUILD_ENV) $(TOPDIR)/src/newhelp/newhelp -n root_solaris -v 2 -o help < help

root_solaris: ../../pmns/stdpmid $(PMNS) root
	rm -f root_solaris
	sed -e 's;<stdpmid>;"../../pmns/stdpmid";' <root \
	| $(RUN_IN_BUILD_ENV) $(TOPDIR)/src/pmcpp/pmcpp \
	| sed -e '/^#/d' -e '/^$$/d' >root_solaris

domain.h: ../../pmns/stdpmid
	$(DOMAIN_MAKERULE)
