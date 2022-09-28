#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bnu:bnu.mk	1.19.5.1"
#
# ***************************************************************
# *	Copyright (c) 1984 AT&T Technologies, Inc.		*
# *                 All Rights Reserved				*
# *	THIS IS UNPUBLISHED PROPRIETARY SOURCE			*
# *	CODE OF AT&T TECHNOLOGIES, INC.				*
# *	The copyright notice above does not			*
# *	evidence any actual or intended				*
# *	publication of such source code.			*
# ***************************************************************
#

#	/*  11/45, 11/70, and VAX version ('-i' has no effect on VAX)	*/
#	/* for 11/23, 11/34 (without separate I/D), IFLAG= */
#
#	CFLAGS:
#	-DSMALL can be used on small machines.
#	It reduces debugging statements in the object code.
#
#	${S5CFLAGS} is an environment variable set for the
#	SAFARI V build.  It MUST have a value of "-Ml", 
#	which specifies that the "large model" should be used
#	for compiling and linking.

# All protocols are compiled in based on the definitions in parms.h
# To activate a protocol, the appropriate definition must be
# made in parms.h. This list must contain at least gio.[co]
# because it is automatically included in the source.
# i.e. D_PROTOCOL, E_PROTOCOL, X_PROTOCOL
PROTOCOLS = dio.o eio.o gio.o xio.o
PROTOCOLSRC = dio.c eio.c gio.c xio.c

# For DATAKIT, define DATAKIT in parms.h (implies D_PROTOCOL) and
# use the following to make the BNU supplied library
LIBRARY = libdk.a
# use the following to indicate the location of the datakit library
# comment out the next line to use system default location
# or change . to your local directory name
DKDIR = .
# use the following to use a local datakit library
DKLIB = -L $(DKDIR) -ldk
# use the following to use a local datakit dk.h file
DKINC = -I $(DKDIR)

# For TLI(S), define TLI(S) in parms.h (implies E_PROTOCOL) and
# use the following two lines on systems without shared libraries
# COFFTLILIB = -lnsl
# ELFTLILIB = -lnsl
# use the following two lines on systems with shared libraries
COFFTLILIB = -lnsl_s
ELFTLILIB = -lnsl

# For UNET, define UNET in parms.h and use the following line
# UNETLIB = -lunet

# Some sites use BTL library for compatibility
# BTLLIB = -lbtl

# use default C compiler
CC = cc

# CFLAGS include Safari V environment flags, and loacl dk.h if defined.
# The second line will produce smaller a.outs by reducing degbu statements
CFLAGS = -O $(S5CFLAGS) $(DKINC)
# CFLAGS = -O $(S5CFLAGS) $(DKINC) -DSMALL

IFLAG =

LDFLAGS= -s $(IFLAG) $(S5LDFLAGS)

# Use the following line for systems with shared libraries
# LIBS = $(BTLLIB) $(UNETLIB) $(DKLIB)
# Use the following line for systems without shared libraries
LIBS = $(BTLLIB) $(UNETLIB) $(DKLIB)

# Use this to produce smaller a.outs by replacing .comment sections using mcs(1)
# MCS = mcs

# use this on systems that don't have strpbrk in libc
# STRPBRK = strpbrk.o
# STRPBRKSRC = strpbrk.c

# use this on systems that don't have getopt() in libc
# GETOPT = getopt.o
# GETOPTSRC = getopt.c

OWNER=uucp
GRP=uucp

INS=install
# If you system does not have "install" use the following line
# instead to use the one included with this package.
# INS=./Install

# save the last version in OLD<name> when installing new version
# OLD=-o

SYMLINK=:
# If you want to maintain the old logical uucp file/directory structure,
# define SYMLINK to be "ln -s". This way files will appear to be in their
# old locations. This is intended to ease the conversion culture shock.
# The BNU code will always access the (new) physical pathnames for
# performance reasons.
# At this time, symbolic links are not created for directories which
# already exist, or are linked elsewhere. The administrator can
# create the links after saving the data in those directories.

# if you change these directories, change them in uucp.h as well

USERBIN=$(ROOT)/usr/bin
UUCPBIN=$(ROOT)/usr/lib/uucp
UUCPDB=$(ROOT)/etc/uucp

VAR=$(ROOT)/var
SPOOL=$(VAR)/spool
LOCKS=$(SPOOL)/locks
UUCPSPL=$(SPOOL)/uucp
UUCPVAR=$(VAR)/uucp
UUCPPUB=$(SPOOL)/uucppublic
#
ADMIN=$(UUCPVAR)/.Admin
#		things are moved (linked) from UUCPSPL into XQTDIR and CORRUPT
CORRUPT=$(UUCPVAR)/.Corrupt
#		this is optional
XQTDIR=	$(UUCPVAR)/.Xqtdir
#		for logfiles
LOGDIR=$(UUCPVAR)/.Log
LOGCICO=$(LOGDIR)/uucico
LOGUUCP=$(LOGDIR)/uucp
LOGUUX=$(LOGDIR)/uux
LOGUUXQT=$(LOGDIR)/uuxqt
#		for saving old log files
OLDLOG=$(UUCPVAR)/.Old
#		for sequence number files
SEQDIR=$(UUCPVAR)/.Sequence
#		for STST files
STATDIR=$(UUCPVAR)/.Status
#
WORKSPACE=$(UUCPVAR)/.Workspace

CLEAN=
#	lint needs to find dk.h file
LINT=$(PFX)lint
LINTOP=-I $(INC) $(DKINC)
#
USERCMDS = ct cu uuglist uucp uuname uustat uux uudecode uuencode uugetty
UUCPCMDS = bnuconvert remote.unknown uucheck uucleanup uusched uucico uuxqt 
OFILES=utility.o cpmv.o expfile.o gename.o getpwinfo.o \
	ulockf.o xqt.o logent.o versys.o gnamef.o systat.o \
	sysfiles.o strsave.o $(GETOPT)
LFILES=utility.c cpmv.c expfile.c gename.c getpwinfo.c \
	ulockf.c xqt.c logent.c gnamef.c systat.c \
	sysfiles.c strsave.c $(GETOPTSRC)
OUUCP=uucpdefs.o uucp.o gwd.o permission.o getargs.o getprm.o uucpname.o\
	versys.o gtcfile.o grades.o $(STRPBRK) chremdir.o mailst.o
LUUCP=uucpdefs.c uucp.c gwd.c permission.c getargs.c getprm.c uucpname.c\
	versys.c gtcfile.c grades.c $(STRPBRKSRC) chremdir.c mailst.c
OUUX=uucpdefs.o uux.o mailst.o gwd.o permission.o getargs.o getprm.o\
	uucpname.o versys.o gtcfile.o grades.o chremdir.o $(STRPBRK)
LUUX=uucpdefs.c uux.c mailst.c gwd.c permission.c getargs.c getprm.c\
	uucpname.c versys.c gtcfile.c grades.c chremdir.c $(STRPBRKSRC)
OUUXQT=uucpdefs.o uuxqt.o mailst.o getprm.o uucpname.o \
	permission.o getargs.o gtcfile.o grades.o $(STRPBRK) \
	shio.o chremdir.o account.o perfstat.o statlog.o security.o \
	limits.o
LUUXQT=uucpdefs.c uuxqt.c mailst.c getprm.c uucpname.c \
	permission.c getargs.c gtcfile.c grades.c $(STRPBRKSRC) \
	shio.c chremdir.c account.c perfstat.c statlog.c security.c \
	limits.c
OUUCICO=uucpdefs.o uucico.o conn.o callers.o cntrl.o pk0.o pk1.o \
	anlwrk.o permission.o getargs.o \
	gnxseq.o pkdefs.o imsg.o gtcfile.o grades.o \
	mailst.o uucpname.o line.o chremdir.o \
	interface.o statlog.o strecpy.o stoa.o perfstat.o account.o\
	security.o limits.o $(STRPBRK) $(PROTOCOLS)
LUUCICO=uucpdefs.c uucico.c conn.c callers.c cntrl.c pk0.c pk1.c \
	anlwrk.c permission.c getargs.c \
	gnxseq.c pkdefs.c imsg.c gtcfile.c grades.c \
	mailst.c uucpname.c line.c chremdir.c \
	interface.c statlog.c strecpy.c stoa.c perfstat.c account.c\
	security.c limits.c $(STRPBRKSRC) $(PROTOCOLSRC)
OUUNAME=uuname.o uucpname.o uucpdefs.o getpwinfo.o sysfiles.o strsave.o
LUUNAME=uuname.c uucpname.c uucpdefs.c getpwinfo.c sysfiles.c strsave.c
OUUSTAT=uustat.o gnamef.o expfile.o uucpdefs.o getpwinfo.o ulockf.o getargs.o \
	utility.o uucpname.o versys.o strsave.o sysfiles.o cpmv.o \
	mailst.o permission.o $(STRPBRK) 
LUUSTAT=uustat.c gnamef.c expfile.c uucpdefs.c getpwinfo.c ulockf.c getargs.c \
	utility.c uucpname.c versys.c strsave.c sysfiles.c cpmv.c \
	mailst.c permission.c $(STRPBRKSRC) 
OUUSCHED=uusched.o gnamef.o expfile.o uucpdefs.o getpwinfo.o ulockf.o \
	systat.o getargs.o utility.o limits.o permission.o uucpname.o
LUUSCHED=uusched.c gnamef.c expfile.c uucpdefs.c getpwinfo.c ulockf.c \
	systat.c getargs.c utility.c limits.c permission.c uucpname.c
OUUCLEANUP=uucleanup.o gnamef.o expfile.o uucpdefs.o getpwinfo.o \
	uucpname.o ulockf.o getargs.o cpmv.o utility.o
LUUCLEANUP=uucleanup.c gnamef.c expfile.c uucpdefs.c getpwinfo.c \
	uucpname.c ulockf.c getargs.c cpmv.c utility.c
OUUGLIST=grades.o cpmv.o getargs.o getpwinfo.o strsave.o \
	uuglist.o uucpdefs.o expfile.o uucpname.o
LUUGLIST=grades.c cpmv.c getargs.c getpwinfo.c strsave.c \
	uuglist.c uucpdefs.c expfile.c uucpname.c
OBNUCONVERT=bnuconvert.o uucpdefs.o grades.o strsave.o \
	getpwinfo.o getargs.o cpmv.o chremdir.o expfile.o gename.o \
	gnamef.o gtcfile.o logent.o systat.o ulockf.o utility.o \
	uucpname.o
LBNUCONVERT=bnuconvert.c uucpdefs.c grades.c strsave.c \
	getpwinfo.c getargs.c cpmv.c chremdir.c expfile.c gename.c \
	gnamef.c gtcfile.c logent.c systat.c ulockf.c utility.c \
	uucpname.c
OUNKNOWN=unknown.o
LUNKNOWN=unknown.c
OCU =  cu.o callers.o getargs.o line.o uucpdefs.o ulockf.o\
	 conn.o interface.o strsave.o sysfiles.o strecpy.o stoa.o
LCU =  cu.c callers.c getargs.c line.c uucpdefs.c ulockf.c\
	 conn.c interface.c strsave.c sysfiles.c strecpy.c stoa.c
OCT =  ct.o callers.o getargs.o line.o uucpdefs.o ulockf.o\
	 conn.o interface.o sysfiles.o strsave.o strecpy.o stoa.o
LCT =  ct.c callers.c getargs.c line.c uucpdefs.c ulockf.c\
	 conn.c interface.c sysfiles.c strsave.c strecpy.c stoa.c
OUUDECODE=uudecode.o
LUUDECODE=uudecode.c
OUUENCODE=uuencode.o
LUUENCODE=uuencode.c
OUUGETTY=uugetty.o ulockf.o uucpdefs.o
LUUGETTY=uugetty.c ulockf.c uucpdefs.c

all:	init $(USERCMDS) $(UUCPCMDS)

libdk.a:	$$(@)(dkbreak.o) $$(@)(dkdial.o) \
	$$(@)(dkerr.o) $$(@)(dkminor.o) \
	$$(@)(dtnamer.o)
	@echo BNU supplied DATAKIT library is now up to date.
	
install:	copyright mkdirs all shells cp check

mcs:	
	$(MCS) -d -a "This space reserved for comments." $(USERCMDS) $(UUCPCMDS)

shells:
	#	SetUp appropriate BNU database files
	sh SetUp  "$(SYMLINK)"
	$(INS) $(OLD) -f $(UUCPBIN) -m 0555 -u $(OWNER) -g $(GRP) SetUp
	$(INS) $(OLD) -f $(UUCPBIN) -m 0555 -u $(OWNER) -g $(GRP) Teardown
	$(INS) $(OLD) -f $(UUCPBIN) -m 0555 -u $(OWNER) -g $(GRP) Uutry
	$(INS) $(OLD) -f $(UUCPBIN) -m 0555 -u $(OWNER) -g $(GRP) uudemon.admin
	$(INS) $(OLD) -f $(UUCPBIN) -m 0555 -u $(OWNER) -g $(GRP) uudemon.cleanup
	$(INS) $(OLD) -f $(UUCPBIN) -m 0555 -u $(OWNER) -g $(GRP) uudemon.hour
	$(INS) $(OLD) -f $(UUCPBIN) -m 0555 -u $(OWNER) -g $(GRP) uudemon.poll
	$(INS) $(OLD) -f $(USERBIN) -m 0555 -u $(OWNER) -g $(GRP) uulog
	$(INS) $(OLD) -f $(USERBIN) -m 0555 -u $(OWNER) -g $(GRP) uupick
	$(INS) $(OLD) -f $(USERBIN) -m 0555 -u $(OWNER) -g $(GRP) uuto

check:
	if [ \( -z "$(ROOT)" -o "$(ROOT)" = "/" \) -a "$(CH)" != "#" ]; then ./uucheck; fi

cp:	all 
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u root -g $(GRP) ct
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u $(OWNER) -g $(GRP) cu
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u $(OWNER) -g $(GRP) uucp
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u $(OWNER) -g $(GRP) uugetty
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u $(OWNER) -g $(GRP) uuglist
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u $(OWNER) -g $(GRP) uuname
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u $(OWNER) -g $(GRP) uustat
	$(INS) $(OLD) -f $(USERBIN) -m 4111 -u $(OWNER) -g $(GRP) uux
	$(INS) $(OLD) -f $(USERBIN) -m 0555 -u $(OWNER) -g $(GRP) uudecode
	$(INS) $(OLD) -f $(USERBIN) -m 0555 -u $(OWNER) -g $(GRP) uuencode

	$(INS) $(OLD) -f $(UUCPBIN) -m 4111 -u $(OWNER) -g $(GRP) remote.unknown
	$(INS) $(OLD) -f $(UUCPBIN) -m 4111 -u $(OWNER) -g $(GRP) uucico
	$(INS) $(OLD) -f $(UUCPBIN) -m 4111 -u $(OWNER) -g $(GRP) uusched
	$(INS) $(OLD) -f $(UUCPBIN) -m 4111 -u $(OWNER) -g $(GRP) uuxqt

# uucheck should only be run by root or uucp administrator
# uucleanup should only be run by root or uucp administrator
# bnuconvert should only be run by root or uucp administrator
	$(INS) $(OLD) -f $(UUCPBIN) -m 0110 -u $(OWNER) -g $(GRP) bnuconvert
	$(INS) $(OLD) -f $(UUCPBIN) -m 0110 -u $(OWNER) -g $(GRP) uucheck
	$(INS) $(OLD) -f $(UUCPBIN) -m 0110 -u $(OWNER) -g $(GRP) uucleanup

restore:
	$(CH)-chmod u+w $(USERBIN)/ct
	$(CH)-mv $(USERBIN)/OLDct $(USERBIN)/ct
	$(CH)-chown root $(USERBIN)/ct
	$(CH)-chgrp $(GRP) $(USERBIN)/ct
	$(CH)-chmod 4111 $(USERBIN)/ct
	$(CH)-chmod u+w $(USERBIN)/cu
	$(CH)-mv $(USERBIN)/OLDcu $(USERBIN)/cu
	$(CH)-chown $(OWNER) $(USERBIN)/cu
	$(CH)-chgrp $(GRP) $(USERBIN)/cu
	$(CH)-chmod 4111 $(USERBIN)/cu
	$(CH)-chmod u+w $(USERBIN)/uucp
	$(CH)-mv $(USERBIN)/OLDuucp $(USERBIN)/uucp
	$(CH)-chown $(OWNER) $(USERBIN)/uucp
	$(CH)-chgrp $(GRP) $(USERBIN)/uucp
	$(CH)-chmod 4111 $(USERBIN)/uucp
	$(CH)-chmod u+w $(USERBIN)/uuglist
	$(CH)-mv $(USERBIN)/OLDuuglist $(USERBIN)/uuglist
	$(CH)-chown $(OWNER) $(USERBIN)/uuglist
	$(CH)-chgrp $(GRP) $(USERBIN)/uuglist
	$(CH)-chmod 4111 $(USERBIN)/uuglist
	$(CH)-chmod u+w $(USERBIN)/uulog
	$(CH)-mv $(USERBIN)/OLDuulog $(USERBIN)/uulog
	$(CH)-chown $(OWNER) $(USERBIN)/uulog
	$(CH)-chgrp $(GRP) $(USERBIN)/uulog
	$(CH)-chmod 555 $(USERBIN)/uulog
	$(CH)-chmod u+w $(USERBIN)/uuname
	$(CH)-mv $(USERBIN)/OLDuuname $(USERBIN)/uuname
	$(CH)-chown $(OWNER) $(USERBIN)/uuname
	$(CH)-chgrp $(GRP) $(USERBIN)/uuname
	$(CH)-chmod 4111 $(USERBIN)/uuname
	$(CH)-chmod u+w $(USERBIN)/uustat
	$(CH)-mv $(USERBIN)/OLDuustat $(USERBIN)/uustat
	$(CH)-chown $(OWNER) $(USERBIN)/uustat
	$(CH)-chgrp $(GRP) $(USERBIN)/uustat
	$(CH)-chmod 4111 $(USERBIN)/uustat
	$(CH)-chmod u+w $(USERBIN)/uux
	$(CH)-mv $(USERBIN)/OLDuux $(USERBIN)/uux
	$(CH)-chown $(OWNER) $(USERBIN)/uux
	$(CH)-chgrp $(GRP) $(USERBIN)/uux
	$(CH)-chmod 4111 $(USERBIN)/uux
	$(CH)-chmod u+w $(USERBIN)/uudecode
	$(CH)-mv $(USERBIN)/OLDuudecode $(USERBIN)/uudecode
	$(CH)-chown $(OWNER) $(USERBIN)/uudecode
	$(CH)-chgrp $(GRP) $(USERBIN)/uudecode
	$(CH)-chmod 0555 $(USERBIN)/uudecode
	$(CH)-chmod u+w $(USERBIN)/uuencode
	$(CH)-mv $(USERBIN)/OLDuuencode $(USERBIN)/uuencode
	$(CH)-chown $(OWNER) $(USERBIN)/uuencode
	$(CH)-chgrp $(GRP) $(USERBIN)/uuencode
	$(CH)-chmod 0555 $(USERBIN)/uuencode

	$(CH)-chmod u+w $(UUCPBIN)/bnuconvert
	$(CH)-mv $(UUCPBIN)/OLDbnuconvert $(UUCPBIN)/bnuconvert
	$(CH)-chown $(OWNER) $(UUCPBIN)/bnuconvert
	$(CH)-chgrp $(GRP) $(UUCPBIN)/bnuconvert
	$(CH)-chmod 0110 $(UUCPBIN)/bnuconvert
	$(CH)-chmod u+w $(UUCPBIN)/remote.unknown
	$(CH)-mv $(UUCPBIN)/OLDremote.unknown $(UUCPBIN)/remote.unknown
	$(CH)-chown $(OWNER) $(UUCPBIN)/remote.unknown
	$(CH)-chgrp $(GRP) $(UUCPBIN)/remote.unknown
	$(CH)-chmod 4111 $(UUCPBIN)/remote.unknown
	$(CH)-chmod u+w $(UUCPBIN)/uucico
	$(CH)-mv $(UUCPBIN)/OLDuucico $(UUCPBIN)/uucico
	$(CH)-chown $(OWNER) $(UUCPBIN)/uucico
	$(CH)-chgrp $(GRP) $(UUCPBIN)/uucico
	$(CH)-chmod 4111 $(UUCPBIN)/uucico
	$(CH)-chmod u+w $(UUCPBIN)/uucheck
	$(CH)-mv $(UUCPBIN)/OLDuucheck $(UUCPBIN)/uucheck
	$(CH)-chown $(OWNER) $(UUCPBIN)/uucheck
	$(CH)-chgrp $(GRP) $(UUCPBIN)/uucheck
	$(CH)-chmod 0110 $(UUCPBIN)/uucheck
	$(CH)-chmod u+w $(UUCPBIN)/uucleanup
	$(CH)-mv $(UUCPBIN)/OLDuucleanup $(UUCPBIN)/uucleanup
	$(CH)-chown $(OWNER) $(UUCPBIN)/uucleanup
	$(CH)-chgrp $(GRP) $(UUCPBIN)/uucleanup
	$(CH)-chmod 0110 $(UUCPBIN)/uucleanup
	$(CH)-chmod u+w $(UUCPBIN)/uusched
	$(CH)-mv $(UUCPBIN)/OLDuusched $(UUCPBIN)/uusched
	$(CH)-chown $(OWNER) $(UUCPBIN)/uusched
	$(CH)-chgrp $(GRP) $(UUCPBIN)/uusched
	$(CH)-chmod 4111 $(UUCPBIN)/uusched
	$(CH)-chmod u+w $(UUCPBIN)/uuxqt
	$(CH)-mv $(UUCPBIN)/OLDuuxqt $(UUCPBIN)/uuxqt
	$(CH)-chown $(OWNER) $(UUCPBIN)/uuxqt
	$(CH)-chgrp $(GRP) $(UUCPBIN)/uuxqt
	$(CH)-chmod 4111 $(UUCPBIN)/uuxqt

clean:
	-rm -f *.o
	-rm -f libdk.a

clobber:	clean
	-rm -f $(USERCMDS) $(UUCPCMDS)

burn:
	-rm -f $(USERBIN)/OLDct
	-rm -f $(USERBIN)/OLDcu
	-rm -f $(USERBIN)/OLDuucp
	-rm -f $(USERBIN)/OLDuugetty
	-rm -f $(USERBIN)/OLDuuglist
	-rm -f $(USERBIN)/OLDuuname
	-rm -f $(USERBIN)/OLDuustat
	-rm -f $(USERBIN)/OLDuux
	-rm -f $(USERBIN)/OLDuudecode
	-rm -f $(USERBIN)/OLDuuencode
	-rm -f $(UUCPBIN)/OLDuucleanup
	-rm -f $(UUCPBIN)/OLDuucheck
	-rm -f $(UUCPBIN)/OLDuucico
	-rm -f $(UUCPBIN)/OLDuusched
	-rm -f $(UUCPBIN)/OLDuuxqt

cmp:	all
	cmp ct $(USERBIN)/ct
	rm ct
	cmp cu $(USERBIN)/cu
	rm cu
	cmp uucp $(USERBIN)/uucp
	rm uucp
	cmp uugetty $(USERBIN)/uugetty
	rm uugetty
	cmp uuglist $(USERBIN)/uuglist
	rm uuglist
	cmp uuname $(USERBIN)/uuname
	rm uuname
	cmp uustat $(USERBIN)/uustat
	rm uustat
	cmp uux $(USERBIN)/uux
	rm uux
	cmp uudecode $(USERBIN)/uudecode
	rm uudecode
	cmp uuencode $(USERBIN)/uuencode
	rm uuencode
	cmp uucleanup $(UUCPBIN)/uucleanup
	rm uucleanup
	cmp uucheck $(UUCPBIN)/uucheck
	rm uucheck
	cmp uucico $(UUCPBIN)/uucico
	rm uucico
	cmp uusched $(UUCPBIN)/uusched
	rm uusched
	cmp uuxqt $(UUCPBIN)/uuxqt
	rm uuxqt
	rm *.o

copyright:
	@echo "\n\n**********************************************"
	@echo "* Copyright (c) 1984, 1986, 1987, 1988       *"
	@echo "*               1989 AT&T                    *"
	@echo "*           All Rights Reserved              *"
	@echo "* THIS IS UNPUBLISHED PROPRIETARY SOURCE     *"
	@echo "* CODE OF AT&T TECHNOLOGIES, INC.            *"
	@echo "* The copyright notice above does not        *"
	@echo "* evidence any actual or intended            *"
	@echo "* publication of such source code.           *"
	@echo "**********************************************\n\n"


init:	copyright anlwrk.o permission.o cpmv.o expfile.o gename.o \
	getargs.o getprm.o getpwinfo.o gnamef.o \
	gnxseq.o gwd.o imsg.o logent.o \
	mailst.o shio.o \
	systat.o ulockf.o uucpname.o versys.o xqt.o

bnuconvert:	$(OBNUCONVERT) $(GETOPT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBNUCONVERT) $(GETOPT) \
		-o bnuconvert $(SHLIBS)
 
ct:	$(OCT) $(LIBRARY)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) $(CFLAGS) $(OCT) $(LIBS) $(COFFTLILIB) $(LDFLAGS) -o ct $(SHLIBS) ; \
	else \
		$(CC) $(CFLAGS) $(OCT) $(LIBS) $(ELFTLILIB) $(LDFLAGS) -o ct $(SHLIBS) ; \
	fi

cu:	$(OCU) $(LIBRARY)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) $(CFLAGS) $(OCU) $(LIBS) $(COFFTLILIB) $(LDFLAGS) -o cu $(SHLIBS) ; \
	else \
		$(CC) $(CFLAGS) $(OCU) $(LIBS) $(ELFTLILIB) $(LDFLAGS) -o cu $(SHLIBS) ; \
	fi

dial:	
	$(MAKE) -f dial.mk dial.o

remote.unknown: $(OUNKNOWN)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUNKNOWN) \
		-o remote.unknown $(SHLIBS)
		 
uucheck:	uucheck.o uucpname.o $(GETOPT)
	$(CC) $(CFLAGS)  $(LDFLAGS) uucheck.o uucpname.o $(GETOPT) \
		-o uucheck $(SHLIBS)
 
uucico:	$(OUUCICO) $(OFILES) $(LIBRARY)
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUCICO) $(OFILES) $(LIBS) $(COFFTLILIB) -o uucico $(SHLIBS) ; \
	else \
		$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUCICO) $(OFILES) $(LIBS) $(ELFTLILIB) -o uucico $(SHLIBS) ; \
	fi

uucleanup:	$(OUUCLEANUP) $(GETOPT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUUCLEANUP) $(GETOPT) \
		-o uucleanup $(SHLIBS)
 
uucp:	$(OUUCP) $(OFILES)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUCP) $(OFILES) \
		-o uucp $(SHLIBS)

uugetty: $(OUUGETTY)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUGETTY) \
		-o uugetty $(SHLIBS)

uuglist: $(OUUGLIST) $(GETOPT)
	 $(CC) $(CFLAGS) $(LDFLAGS) $(OUUGLIST) \
		-o uuglist $(SHLIBS)

uuname:	$(OUUNAME)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUNAME) \
		-o uuname $(SHLIBS)

uusched:	$(OUUSCHED) $(GETOPT)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUSCHED) $(GETOPT) \
		-o uusched $(SHLIBS)
 
uustat:	$(OUUSTAT) $(GETOPT)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUSTAT) $(GETOPT) \
		-o uustat $(SHLIBS)
 
uux:	$(OUUX) $(OFILES)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUX) $(OFILES) \
		-o uux $(PERFLIBS)

uuxqt:	$(OUUXQT) $(OFILES)
	$(CC) $(CFLAGS)  $(LDFLAGS) $(OUUXQT) $(OFILES) \
		-o uuxqt $(PERFLIBS)

uudecode:	$(OUUDECODE)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUUDECODE) -o uudecode $(SHLIBS)

uuencode:	$(OUUENCODE)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUUENCODE) -o uuencode $(SHLIBS)

uucheck.o:	permission.c sysfiles.h

sysfiles.o:	sysfiles.h

limits.o:	uucp.h

perfstat.o account.o security.o: log.h

gio.o pk0.o pk1.o pkdefs.o:	pk.h

utility.o permission.o uucico.o conn.o callers.o cpmv.o\
	anlwrk.o grades.o cntrl.o expfile.o gename.o\
	getpwinfo.o gio.o xio.o gnamef.o gnxseq.o gwd.o imsg.o ioctl.o\
	logent.o mailst.o sdmail.o line.o shio.o\
	systat.o ulockf.o uucpdefs.o uucpname.o uuname.o\
	uucleanup.o uucheck.o uusched.o  uucp.o uustat.o dio.o \
	bnuconvert.o uux.o uuxqt.o versys.o xqt.o interface.o statlog.o \
	uuglist.o strecpy.o:	uucp.h parms.h log.h

mkdirs:
	-mkdir $(UUCPBIN)
	$(CH)-chmod 755 $(UUCPBIN)
	$(CH)-chown $(OWNER) $(UUCPBIN)
	$(CH)-chgrp $(GRP) $(UUCPBIN)
	-mkdir $(UUCPDB)
	$(CH)-chmod 755 $(UUCPDB)
	$(CH)-chown $(OWNER) $(UUCPDB)
	$(CH)-chgrp $(GRP) $(UUCPDB)
	-mkdir $(LOCKS)
	$(CH)-chmod 755 $(LOCKS)
	$(CH)-chown $(OWNER) $(LOCKS)
	$(CH)-chgrp $(GRP) $(LOCKS)
	-mkdir $(UUCPSPL)
	$(CH)-chmod 755 $(UUCPSPL)
	$(CH)-chown $(OWNER) $(UUCPSPL)
	$(CH)-chgrp $(GRP) $(UUCPSPL)
	-mkdir $(UUCPPUB)
	$(CH)-chmod 777 $(UUCPPUB)
	$(CH)-chown $(OWNER) $(UUCPPUB)
	$(CH)-chgrp $(GRP) $(UUCPPUB)
	-mkdir $(UUCPVAR)
	$(CH)-chmod 755 $(UUCPVAR)
	$(CH)-chown $(OWNER) $(UUCPVAR)
	$(CH)-chgrp $(GRP) $(UUCPVAR)
	-mkdir $(ADMIN)
	$(CH)-chmod 755 $(ADMIN)
	$(CH)-chown $(OWNER) $(ADMIN)
	$(CH)-chgrp $(GRP) $(ADMIN)
	-$(SYMLINK) $(ADMIN) $(UUCPSPL)
	-mkdir $(CORRUPT)
	$(CH)-chmod 755 $(CORRUPT)
	$(CH)-chown $(OWNER) $(CORRUPT)
	$(CH)-chgrp $(GRP) $(CORRUPT)
	-$(SYMLINK) $(CORRUPT) $(UUCPSPL)
	-mkdir $(LOGDIR)
	$(CH)-chmod 755 $(LOGDIR)
	$(CH)-chown $(OWNER) $(LOGDIR)
	$(CH)-chgrp $(GRP) $(LOGDIR)
	-$(SYMLINK) $(LOGDIR) $(UUCPSPL)
	-mkdir $(LOGCICO)
	$(CH)-chmod 755 $(LOGCICO)
	$(CH)-chown $(OWNER) $(LOGCICO)
	$(CH)-chgrp $(GRP) $(LOGCICO)
	-mkdir $(LOGUUCP)
	$(CH)-chmod 755 $(LOGUUCP)
	$(CH)-chown $(OWNER) $(LOGUUCP)
	$(CH)-chgrp $(GRP) $(LOGUUCP)
	-mkdir $(LOGUUX)
	$(CH)-chmod 755 $(LOGUUX)
	$(CH)-chown $(OWNER) $(LOGUUX)
	$(CH)-chgrp $(GRP) $(LOGUUX)
	-mkdir $(LOGUUXQT)
	$(CH)-chmod 755 $(LOGUUXQT)
	$(CH)-chown $(OWNER) $(LOGUUXQT)
	$(CH)-chgrp $(GRP) $(LOGUUXQT)
	-mkdir $(OLDLOG)
	$(CH)-chmod 755 $(OLDLOG)
	$(CH)-chown $(OWNER) $(OLDLOG)
	$(CH)-chgrp $(GRP) $(OLDLOG)
	-$(SYMLINK) $(OLDLOG) $(UUCPSPL)
	-mkdir $(SEQDIR)
	$(CH)-chmod 755 $(SEQDIR)
	$(CH)-chown $(OWNER) $(SEQDIR)
	$(CH)-chgrp $(GRP) $(SEQDIR)
	-$(SYMLINK) $(SEQDIR) $(UUCPSPL)
	-mkdir $(STATDIR)
	$(CH)-chmod 755 $(STATDIR)
	$(CH)-chown $(OWNER) $(STATDIR)
	$(CH)-chgrp $(GRP) $(STATDIR)
	-$(SYMLINK) $(STATDIR) $(UUCPSPL)
	-mkdir $(WORKSPACE)
	$(CH)-chmod 755 $(WORKSPACE)
	$(CH)-chown $(OWNER) $(WORKSPACE)
	$(CH)-chgrp $(GRP) $(WORKSPACE)
	-$(SYMLINK) $(WORKSPACE) $(UUCPSPL)
	-mkdir $(XQTDIR)
	$(CH)-chmod 755 $(XQTDIR)
	$(CH)-chown $(OWNER) $(XQTDIR)
	$(CH)-chgrp $(GRP) $(XQTDIR)
	-$(SYMLINK) $(XQTDIR) $(UUCPSPL)

#  lint procedures

lint:	lintuucp lintuucico lintuux lintuuxqt \
	lintuuname lintct lintcu lintuudecode \
	lintuuencode

lintuucp:
	$(LINT) $(LINTOP) $(LUUCP) $(LFILES)

lintuucico:
	$(LINT) $(LINTOP) $(LUUCICO) $(LFILES)

lintuux:
	$(LINT) $(LINTOP) $(LUUX) $(LFILES)

lintuuxqt:
	$(LINT) $(LINTOP) $(LUUXQT) $(LFILES)

lintuuname:
	$(LINT) $(LINTOP) $(LUUNAME)

lintct:
	$(LINT) $(LINTOP) $(LCT)

lintcu:
	$(LINT) $(LINTOP) $(LCU)

lintuudecode:
	$(LINT) $(LINTOP) $(LUUDECODE)

lintuuencode:
	$(LINT) $(LINTOP) $(LUUENCODE)

