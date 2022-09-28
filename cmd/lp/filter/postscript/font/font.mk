#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp:filter/postscript/font/font.mk	1.4.3.1"

#
# makefile for PostScript font files.
#

MAKEFILE=font.mk
ARGS=all

#
# Common header and source files have been moved to $(COMMONDIR).
#

COMMONDIR=../common

#
# makedev is the only program that's compiled. NEEDS $(COMMONDIR) for dev.h.
#

CFLAGS=-O -I$(COMMONDIR)

#
# If you want the %%DocumentFonts: comment from dpost set DOCUMENTFONTS to ON.
# It controls whether the *.name files are left in $(FONTDIR)/devpost.
#

DOCUMENTFONTS=ON

#
# ASCII includes the DESC file and all the ASCII font files. Only the binary
# files, built by makedev, are left in $(FONTDIR)/devpost after we install things.
#

ASCII=DESC ? ??
EXTRA=LINKFILE *.big *.small


#all : devpost
all :

#####
#
# Problem: The font width tables are machine-dependent, but we
# will typically be building and installing in a cross-environment.
# Thus we can't ``compile'' the tables during the regular install step.
#
# Solution: Not a perfect one, but it works:
#
# The "install" target assumes you are installing in a cross environment,
# and simply copies the source font-width tables to the devpost directory.
# When the package (built from the installed tree) gets installed on the
# destination machine, it will compile the tables. So guess what? This
# means we have to deliver the makedev program!
#
# The "realinstall" target assumes you are installing on the destination
# machine. It runs makedev directly and installs only the compiled tables.
#####

realinstall : devpost bothinstall
	cd devpost; \
	ls *.out \
	| xargs -i install -m 444 -u $(OWNER) -g $(GROUP) -f $(FONTDIR)/devpost {}
#	find devpost -print | cpio -pdu $(FONTDIR)
#	cd $(FONTDIR)/devpost; find . -depth -print | xargs chmod ugo+r
#	cd $(FONTDIR)/devpost; find . -depth -print | xargs chgrp $(GROUP)
#	cd $(FONTDIR)/devpost; find . -depth -print | xargs chown $(OWNER)
	cd $(FONTDIR)/devpost; rm -f $(ASCII) $(EXTRA)
#	if [ "$(DOCUMENTFONTS)" != "ON" ]; then \
#	    cd $(FONTDIR)/devpost; rm -f ?.name ??.name; \
#	fi

install : makedev bothinstall
	install -m 555 -u $(OWNER) -g $(GROUP) -f $(FONTDIR) makedev
	install -m 555 -u $(OWNER) -g $(GROUP) -f $(FONTDIR)/devpost devpost/LINKFILE
	cd devpost; \
	ls $(ASCII) \
	| xargs -i install -m 444 -u $(OWNER) -g $(GROUP) -f $(FONTDIR)/devpost {}

bothinstall :
	@if [ ! -d $(FONTDIR) ]; then \
	    mkdir $(FONTDIR); \
	    $(CH)chmod 775 $(FONTDIR); \
	    $(CH)chgrp $(GROUP) $(FONTDIR); \
	    $(CH)chown $(OWNER) $(FONTDIR); \
	fi
	@if [ ! -d $(FONTDIR)/devpost ]; then \
	    mkdir $(FONTDIR)/devpost; \
	    $(CH)chmod 775 $(FONTDIR)/devpost; \
	    $(CH)chgrp $(GROUP) $(FONTDIR)/devpost; \
	    $(CH)chown $(OWNER) $(FONTDIR)/devpost; \
	fi
	@if [ ! -d $(FONTDIR)/devpost/charlib ]; then \
	    mkdir $(FONTDIR)/devpost/charlib; \
	    $(CH)chmod 775 $(FONTDIR)/devpost/charlib; \
	    $(CH)chgrp $(GROUP) $(FONTDIR)/devpost/charlib; \
	    $(CH)chown $(OWNER) $(FONTDIR)/devpost/charlib; \
	fi
	if [ "$(DOCUMENTFONTS)" = "ON" ]; then \
	    find devpost/*.name -print \
	    | xargs -i install -m 444 -u $(OWNER) -g $(GROUP) -f $(FONTDIR)/devpost {}; \
	fi
	find devpost/charlib/* -print \
	| xargs -i install -m 444 -u $(OWNER) -g $(GROUP) -f $(FONTDIR)/devpost/charlib {}

devpost ::
	@$(MAKE) -f $(MAKEFILE) makedev
	cd devpost; ../makedev $(ASCII); sh LINKFILE

makedev : $(COMMONDIR)/dev.h makedev.c
	$(CC) $(CFLAGS) -o makedev makedev.c


clean :
	rm -f *.o
	cd devpost; rm -f *.out

clobber : clean
	rm -f makedev

list :
	cd devpost; pr -n ../README ../$(MAKEFILE) $(ASCII) | $(LIST)

