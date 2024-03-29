#
# Copyright (c) 2001,2009 Silicon Graphics, Inc.  All Rights Reserved.
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

CFILES	= import.c stuff.c archive.c
HFILES	= private.h

IAM = libpcp_import
STATICLIBTARGET	= libpcp_import.a
DSOVERSION = 1
LIBTARGET = libpcp_import.$(DSOSUFFIX).$(DSOVERSION)
SYMTARGET = libpcp_import.$(DSOSUFFIX) 
ifeq "$(TARGET_OS)" "darwin"
LIBTARGET = libpcp_import.$(DSOVERSION).$(DSOSUFFIX)
endif
ifeq "$(TARGET_OS)" "mingw"
LIBTARGET = libpcp_import.$(DSOSUFFIX) 
SYMTARGET =
STATICLIBTARGET	=
endif
ifeq "$(ENABLE_SHARED)" "no"
LIBTARGET =
SYMTARGET =
endif

LCFLAGS =
LLDLIBS = -lpcp
LDIRT = $(SYMTARGET) domain.h links.done

DOMAIN = PMI_DOMAIN

default: domain.h $(LIBTARGET) $(SYMTARGET) $(STATICLIBTARGET) links.done

$(OBJECTS):	$(HFILES)

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

default_pcp: default

install_pcp: install

ifneq "$(SYMTARGET)" ""
$(SYMTARGET):
	$(LN_S) -f $(LIBTARGET) $@
endif

domain.h: $(TOPDIR)/src/pmns/stdpmid
	rm -f domain.h
	@echo "/*" >domain.h
	@echo " * built from $<" >>domain.h
	@echo " */" >>domain.h
	@$(AWK) <$< '\
$$1=="#define" && $$2 == "$(DOMAIN)" { print "#define $(DOMAIN) " $$3 >>"$@"; found++ }\
END	{ if (found == 0) { print "Botch: no #define for domain $(DOMAIN) in $<";  exit(1) }\
          if (found > 1) { print "Botch: multiple #defines for domain $(DOMAIN) in $<"; print "... see $@ for details"; exit(1) }\
	  exit(0)\
	}'
#
# Populate a common lib directory for simplifying the builds of some other
# components that need to find multiple PCP libraries easily.  Also makes
# use of LD_LIBRARY_PATH for debugging exercises a little simpler.
# 
links.done: $(STATICLIBTARGET) $(LIBTARGET)
ifneq "$(LIBTARGET)" ""
	$(LN_S) -f $(shell pwd)/libpcp_import.$(DSOSUFFIX) $(TOPDIR)/src/lib
endif
ifneq "$(STATICLIBTARGET)" ""
	$(LN_S) -f $(shell pwd)/$(STATICLIBTARGET) $(TOPDIR)/src/lib
endif
	touch links.done
