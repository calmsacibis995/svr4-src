#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:makefile	1.17.2.7"
#
# makefile for libc/port
#
#
# The macro PROF is null by default, causing profiled object to be maintained.
# If profiled object is not desired, the reassignment PROF=@# should appear in
# the make command line.
#

.SUFFIXES: .p .P .A
CC=cc
CFLAGS=-O
PROF=
NONPROF=
PIC=
ABI=
DEFLIST=
SDEFLIST=

all:	prof pic abi nonprof

nonprof:
	$(NONPROF)cd gen; make -e $(IGN) nonprof PROF=@# PIC=@# ABI=@#
	$(NONPROF)cd print; make -e $(IGN) nonprof PROF=@# PIC=@# ABI=@#
	$(NONPROF)cd stdio; make -e $(IGN) nonprof PROF=@# PIC=@# ABI=@#
	$(NONPROF)cd sys; make -e $(IGN) nonprof PROF=@# PIC=@# ABI=@#
prof:
	$(PROF)cd gen; make -e $(IGN) prof NONPROF=@# PIC=@# ABI=@#
	$(PROF)cd print; make -e $(IGN) prof NONPROF=@# PIC=@# ABI=@#
	$(PROF)cd stdio; make -e $(IGN) prof NONPROF=@# PIC=@# ABI=@#
	$(PROF)cd sys; make -e $(IGN) prof NONPROF=@# PIC=@# ABI=@#
pic:
	$(PIC)cd gen; make -e $(IGN) pic PROF=@# NONPROF=@# ABI=@#
	$(PIC)cd print; make -e $(IGN) pic PROF=@# NONPROF=@# ABI=@#
	$(PIC)cd stdio; make -e $(IGN) pic PROF=@# NONPROF=@# ABI=@#
	$(PIC)cd sys; make -e $(IGN) pic PROF=@# NONPROF=@# ABI=@#
abi:
	$(ABI)cd gen; make -e $(IGN) abi PROF=@# NONPROF=@# PIC=@#
	$(ABI)cd print; make -e $(IGN) abi PROF=@# NONPROF=@# PIC=@#
	$(ABI)cd stdio; make -e $(IGN) abi PROF=@# NONPROF=@# PIC=@#
	$(ABI)cd sys; make -e $(IGN) abi PROF=@# NONPROF=@# PIC=@#
	
clean:

clobber: clean
	-find . \( -name '*.o' -o -name '*.p' -o -name '*.P' -o -name '*.A' \) \
		-print | xargs rm -f
	-rm -f gen/new_list.c
	-rm -f gen/errlst.c
