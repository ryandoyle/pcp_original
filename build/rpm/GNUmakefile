#
# Copyright (c) 2012-2013 Red Hat.
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

TOPDIR = ../..
include $(TOPDIR)/src/include/builddefs
-include ./GNUlocaldefs

SPEC = pcp.spec
LDIRT += *-[0-9]*.*.rpm $(SPEC) rpmmacros

PERLFILES = pcp-pmda-perl.desc pcp-logsummary-perl.desc pcp-mmv-perl.desc
LSRCFILES = macros.template fedora.spec pcp.spec.in $(PERLFILES)

default install install_pcp default_pcp :

# Blow the spec away after rpm has finished its work - it IS
# going to make sure it'll be rebuild next time we come here
pack_pcp : rpmmacros $(SPEC)
	DEFS=`grep '^--define' rpmmacros`; \
	eval $(RPMPROG) -ba $$DEFS \
		--target $(TARGET_CPU)-$(TARGET_VENDOR)-$(TARGET_OS) \
		--clean $(SPEC)
	rm -f $(SPEC) $(TOPDIR)/install.manifest $(TOPDIR)/files.rpm \
		rpmmacros 

rpmmacros : macros.template
	@TREEROOT=`cd ${TOPDIR}; pwd`; \
	for d in RPM SRPM; do \
	    eval D=\$$$${d}DIR; \
	    if [ -z "$$D" -o ! -d "$$D" ] ; then \
		eval $${d}DIR=$$TREEROOT/build/rpm; \
	    fi; \
	done; \
	sed -e "s|%topdir%|$${TREEROOT}|g" \
	    -e "s|@rpmsdir@|$${RPMDIR}|g" \
	    -e "s|@srpmsdir@|$${SRPMDIR}|"g \
	    -e '/^%/s/[ \t]*$$/'\''/' \
	    -e '/^%/s/^%/--define '\''/' <$< >$@

pcp.spec: pcp.spec.in
	@echo Generating $@ from $?; \
	sed -e's|@package_sgirelease@|$(SGIRELEASENUMBER)|g' \
	    -e's|@sgi_chroot_build@|$(SGI_CHROOT_BUILD)|g' \
	    -e's|@sgi_issp_build@|$(SGI_ISSP_BUILD)|g' \
	    -e's|@package_name@|$(PACKAGE_NAME)|g' \
	    -e's|@package_version@|$(PACKAGE_VERSION)|g' \
	    -e's|@package_release@|$(PACKAGE_BUILD)|g' \
	    -e's|@package_configure@|$(PACKAGE_CONFIGURE)|g' \
	    -e's|@package_distribution@|$(PACKAGE_DISTRIBUTION)|g' \
	    -e's|@package_builder@|$(PACKAGE_BUILDER)|g' \
	    -e"s|@build_root@|$${DIST_ROOT}|g" \
	    -e's|@pcp_sysconf_dir@|$(PCP_SYSCONF_DIR)|g' \
	    -e's|@pcp_log_dir@|$(PCP_LOG_DIR)|g' \
	    -e's|@pcp_var_dir@|$(PCP_VAR_DIR)|g' \
	    -e's|@make@|$(PCP_MAKE_PROG)|g' <$? >$@

include $(BUILDRULES)

