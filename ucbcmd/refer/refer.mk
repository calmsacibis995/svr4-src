#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#     Portions Copyright(c) 1988, Sun Microsystems, Inc.
#     All Rights Reserved

#ident	"@(#)ucbrefer:refer.mk	1.4.3.1"

#     Makefile for refer


ROOT=
INSDIR=$(ROOT)/usr/ucb
INSLIB=$(ROOT)/usr/ucblib
INS=install

CMDS = mkey inv hunt refer addbib lookbib sortbib roffbib indxbib

CFLAGS= -O
LDFLAGS= -s $(SHLIBS)
DEFS=
DIR=$(INSLIB)/reftools
DIR1=$(INSLIB)/doctools

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $<

all:	$(CMDS)

$(DIR):
	-mkdir $@
	$(CH)-chmod 755 $@
	$(CH)-chgrp bin $@
	$(CH)-chown bin $@

$(DIR1):
	-mkdir $(INSLIB)/doctools
	$(CH)-chmod 755 $(INSLIB)/doctools
	$(CH)-chgrp bin $(INSLIB)/doctools
	$(CH)-chown bin $(INSLIB)/doctools
	-mkdir $(INSLIB)/doctools/tmac
	$(CH)-chmod 755 $(INSLIB)/doctools/tmac
	$(CH)-chgrp bin $(INSLIB)/doctools/tmac
	$(CH)-chown bin $(INSLIB)/doctools/tmac

install: all $(DIR) $(DIR1)
	install -f $(INSLIB)/reftools -u bin -g bin -m 00555 mkey
	install -f $(INSLIB)/reftools -u bin -g bin -m 00555 inv
	install -f $(INSLIB)/reftools -u bin -g bin -m 00555 hunt
	install -f $(INSDIR) -u bin -g bin -m 00555 refer
	install -f $(INSDIR) -u bin -g bin -m 00555 addbib
	install -f $(INSDIR) -u bin -g bin -m 00555 sortbib
	install -f $(INSDIR) -u bin -g bin -m 00555 roffbib
	install -f $(INSDIR) -u bin -g bin -m 00555 indxbib
	install -f $(INSDIR) -u bin -g bin -m 00555 lookbib
	install -f $(INSLIB)/doctools/tmac -u bin -g bin -m 00644 tmac.bib
	cd papers; make -f papers.mk install

mkey:	mkey1.o mkey2.o mkey3.o deliv2.o
	$(CC) $(CFLAGS) mkey?.o deliv2.o -o mkey $(LDFLAGS) 

inv:	inv1.o inv2.o inv3.o inv5.o inv6.o deliv2.o
	$(CC) $(CFLAGS) inv?.o deliv2.o -o inv $(LDFLAGS)

hunt:	hunt1.o hunt2.o hunt3.o hunt5.o hunt6.o hunt7.o glue5.o refer3.o hunt9.o shell.o deliv2.o hunt8.o glue4.o tick.o
	$(CC) $(CFLAGS) hunt?.o refer3.o glue5.o glue4.o shell.o deliv2.o tick.o -o hunt $(LDFLAGS)


refer: glue1.o refer1.o refer2.o refer4.o refer5.o refer6.o mkey3.o refer7.o refer8.o hunt2.o hunt3.o deliv2.o hunt5.o hunt6.o hunt8.o glue3.o hunt7.o hunt9.o glue2.o glue4.o glue5.o refer0.o shell.o
	$(CC) $(CFLAGS) glue?.o refer[01245678].o hunt[2356789].o mkey3.o shell.o deliv2.o -o refer $(LDFLAGS)

addbib: addbib.o
	$(CC) $(CFLAGS) addbib.o -o addbib $(LDFLAGS)

lookbib: lookbib.o
	$(CC) $(CFLAGS) lookbib.o -o lookbib $(LDFLAGS)

sortbib: sortbib.o
	$(CC) $(CFLAGS) sortbib.o -o sortbib $(LDFLAGS)

indxbib: indxbib.sh
	cp indxbib.sh indxbib

roffbib: roffbib.sh
	cp roffbib.sh roffbib

clean:
	rm -f *.o 

clobber:	clean
	rm -f $(CMDS)
