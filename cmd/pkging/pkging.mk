#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pkging:pkging.mk	1.3"

ARCH = AT386
BUS = AT386
ROOTLIBS = -dn

CFLAGS = -O -D$(ARCH) -D$(BUS) 

INS=install
FILES = installpkg message removepkg displaypkg Install.sh removepkg.r \
	installpkg.r flop_disk Install.tape get_sel flop_num

all: $(FILES)

install: all
	$(INS) -f $(ROOT)/usr/bin installpkg
	$(INS) -f $(ROOT)/usr/bin message
	$(INS) -f $(ROOT)/usr/bin removepkg
	$(INS) -f $(ROOT)/usr/bin displaypkg
	$(INS) -f $(ROOT)/usr/lbin Install.sh
	$(INS) -f $(ROOT)/usr/lbin installpkg.r
	$(INS) -f $(ROOT)/usr/lbin removepkg.r
	$(INS) -f $(ROOT)/usr/lbin Install.tape
	$(INS) -f $(ROOT)/usr/lbin get_sel
	$(INS) -f $(ROOT)/sbin flop_disk
	$(INS) -f $(ROOT)/sbin flop_num
	if [ -d $(ROOT)/xenv ]; then cc $(CFLAGS) -s -o $(ROOT)/xenv/flop_num flop_num.c; fi

clean clobber:
	rm -f message.o message flop_disk.o flop_disk get_sel.o get_sel flop_num flop_num.o

flop_num: $$@.o
	$(CC) -s -o $@ $@.o $(LDLIBS) $(ROOTLIBS)

message flop_disk get_sel: $$@.o
	$(CC) -s -o $@ $@.o $(LDLIBS) $(SHLIBS)
