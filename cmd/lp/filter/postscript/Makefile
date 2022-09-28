#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)lp:filter/postscript/Makefile	1.9.3.1"
#
# Version 3.1 makefile. A few things have been added and changed so take a look
# at the comments before you compile or install anything. ENCODING is the most
# important addition to this file. If you're willing to take a small chance for a
# potential 20% reduction in print time for dpost output files set ENCODING to 2.
# If you're running on a UTS system make sure the programs are compiled with the
# native compiler. Finally, if you do a partial install include the new version
# of postreverse. It's backwards compatible, but old versions of postreverse
# will not work with the new translators.
#
# There's not much you'll need to change, but you may not agree with some of my
# choices, so before you do anything else, check the following definitions:
#
#	ENCODING - An integer (0, 1, 2 or 3) that sets the default text encoding
#		  scheme used by dpost. Increasing ENCODING (up to 3) decreases
#		  print time and the size of output files produced by dpost. 0 is
#		  slow, but the most stable choice and produces output essentially
#		  equivalent to previous versions of dpost. 2 and 3 are encoding
#		  schemes based on widthshow. Both are fast, perhaps 20% faster
#		  than the 0 level scheme, but neither is well tested. Setting
#		  encoding to 3 is not recommended, and will result in ragged right
#		  margins. The encoding scheme can also be set at run time using
#		  the -e option. Setting ENCODING to 2 may be worth a try.
#
#	ROOT	- A string that's prepended to all the installation directories
#		  (eg. BINDIR). Only used when things are installed, and probably
#		  won't be of much use to anyone.
#
#	BINDIR	- Where programs, like dpost and postprint, are installed. You may
#		  want to change this definition. Things are set up so the programs
#		  get put in a directory that's probably not in anyone's PATH.
#
#	FONTDIR - Where the binary font files go. Should be set to troff's font
#		  directory on your system. If you change it do the same thing to
#		  the definition of FONTDIR in ./common/path.h.
#
#	LIBDIR	- All the files from directory ./postscript that end in .ps (plus
#		  a few others) get put here. Mostly prologues for translators. If
#		  you change it fix the corresponding paths in ./common/path.h
#		  before doing a compile.
#
#	MANDIR	- Where the manual pages are installed. This one is undoubtedly
#		  wrong!
#
#	MACRODIR - Special macro packages get installed in this directory. The ones
#		  I've included handle picture inclusion and color selection.
#
#	OWNER	- Owner of any files that are installed.
#
#	GROUP	- Group that's assigned to all installed files.
#
#	SYSTEM	- The version of Unix you're running. Recognized choices are,
#
#			SYSV    - System V
#			V9	- Ninth Edition
#			BSD4_2	- Berkeley
#
#		  Primarily for conditional compilation in postio.
#
#	CFLAGS	- Some of the programs use floating point arithmetic, so if you're
#		  running on a system without floating point hardware add the -f
#		  option before compiling the programs.
#
#	LIST	- The command that's run to produce a source listing. Not terribly
#		  important, but the default will only be right on MHCC systems.
#
#	TARGETS	- The default group of things (ie. source directories) that make
#		  operates on when you select targets like install or clobber.
#
#	DKHOSTDIR - If your system has DKHOST software this is where it should be.
#		  Used for conditional compilation in postio, and only if SYSTEM
#		  is set to SYSV. Needed so dk.h and libdk.a are picked up. To
#		  disable the DKHOST stuff just remove this definition.
#
#
# That's about all you should have to do. After you've made suitable definitions
# type,
#
#		make install
#
# to install everything listed under TARGETS, although you'll probably have to be
# root before it will work. If you're just interested in part of the package (eg.
# installing dpost and the font files) type,
#
#		make TARGETS="dpost font" install
#
# There are makefiles in most of the subdirectories, but they're not designed to
# be used on their own, especially not if you're installing things. Each needs
# many of the definitions listed above before they're guaranteed to work. As long
# as you run make in this directory using Makefile you won't have to worry about
# any other changes or additions. The appropriate definitions will be exported
# before any of the other makefiles are used.
#

OWNER=lp
GROUP=lp

ENCODING=2

BINDIR=$(ROOT)/usr/lib/lp/postscript
FONTDIR=$(ROOT)/usr/lib/font
LIBDIR=$(ROOT)/usr/lib/lp/postscript
#MANDIR=$(ROOT)/usr/man/mhcc
MACRODIR=$(ROOT)/usr/lib/macros
DOCDIR=$(ROOT)/usr/docs/postscript

DKHOSTDIR=/usr/add-on/dkit

SYSTEM=SYSV
CFLAGS=-O -D$(SYSTEM) -s

LIST=i10send -lpr -von -f2

#
# That's the end of things you may have to change. Everything that follows can
# probably remain as is.
#

#MAKE=/bin/make
MAKEFILE=Makefile
COMMONDIR=common
ARGS=all

#
# TARGETS lists most of the source directories that you'll need. Each one is
# expected to have an appropriately named makefile that's called from this file,
# and that understands a few common targets.
#

TARGETS=postreverse\
        dpost\
        postprint\
        posttek\
        postdmd\
        postdaisy\
        postmd\
	postplot\
        postscript\
        postio\
	picpack\
	download\
	macros\
        font\
	filtdesc

all : $(TARGETS)

clean clobber :
	@$(MAKE) -e -f $(MAKEFILE) MAKE=$(MAKE) ARGS=$@ $(TARGETS)

list :
	pr -n README $(MAKEFILE) | $(LIST)
	@echo
	@LIST='$(LIST)'; export LIST; \
	$(MAKE) -e -f $(MAKEFILE) MAKE=$(MAKE) ARGS=$@ $(TARGETS)

install :
	@BINDIR='$(BINDIR)'; export BINDIR; \
	FONTDIR='$(FONTDIR)'; export FONTDIR; \
	LIBDIR='$(LIBDIR)'; export LIBDIR; \
	MANDIR='$(MANDIR)'; export MANDIR; \
	MACRODIR='$(MACRODIR)'; export MACRODIR; \
	DOCDIR='$(DOCDIR)'; export DOCDIR; \
	OWNER='$(OWNER)'; export OWNER; \
	GROUP='$(GROUP)'; export GROUP; \
	$(MAKE) -e -f $(MAKEFILE) MAKE=$(MAKE) ARGS=$@ $(TARGETS)

$(TARGETS) ::
	@if [ -d $@ -a -f $@/$@.mk ]; then  \
	    cd $@; \
	    COMMONDIR="../$(COMMONDIR)"; export COMMONDIR; \
	    DKHOSTDIR='$(DKHOSTDIR)'; export DKHOSTDIR; \
	    SYSTEM="$(SYSTEM)"; export SYSTEM; \
	    CFLAGS="$(CFLAGS) -I$$COMMONDIR"; \
	    if [ $@ = postio -a -d "$(DKHOSTDIR)" -a "$(SYSTEM)" = SYSV ]; then \
		CFLAGS="$(CFLAGS) -DDKHOST -I$$COMMONDIR -I$(DKHOSTDIR)/usr/include"; \
	    fi; \
	    if [ $@ = dpost ]; then \
		CFLAGS="$(CFLAGS) -DDFLTENCODING=$(ENCODING) -I$$COMMONDIR"; \
	    fi; \
	    export CFLAGS; \
	    echo "---- Making target $(ARGS) in directory $@ ----"; \
	    $(MAKE) -e -f $@.mk MAKE=$(MAKE) $(ARGS); \
	    echo; \
	fi

