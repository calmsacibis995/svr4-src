#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)iconv:iconv.mk	1.4.4.1"

ROOT=
INC=$(ROOT)/usr/include
CFLAGS=-O -I$(INC) -I.
LDFLAGS=-s
YACC=yacc

BIN=$(ROOT)/usr/bin
ICONV=$(ROOT)/usr/lib/iconv
INS=install

IOBJS=	iconv.o gettab.o process.o
KOBJS=	main.o gram.o lexan.o output.o reach.o sort.o sym.o tree.o
ALL=iconv kbdcomp 
CODESETS=\
codesets/646da.8859.p codesets/646de.8859.p codesets/646en.8859.p \
codesets/646es.8859.p codesets/646fr.8859.p codesets/646it.8859.p \
codesets/646sv.8859.p codesets/8859.646.p codesets/8859.646da.p \
codesets/8859.646de.p codesets/8859.646en.p codesets/8859.646es.p \
codesets/8859.646fr.p codesets/8859.646it.p codesets/8859.646sv.p


all:	$(ALL)

iconv:	$(IOBJS)
	$(CC) -o iconv $(CFLAGS) $(IOBJS) $(SHLIBS) $(LDFLAGS)

kbdcomp: $(KOBJS)
	$(CC) -o kbdcomp $(CFLAGS) $(KOBJS) $(SHLIBS) $(LDFLAGS)

install : $(ALL)
	$(INS) -f $(BIN) -m 555 -u bin -g bin iconv
	$(INS) -f $(BIN) -m 555 -u bin -g bin kbdcomp
	if [ ! -d $(ICONV) ] ; \
	then \
	mkdir $(ICONV); \
	$(CH)chmod 755 $(ICONV); \
	$(CH)chown bin $(ICONV); \
	$(CH)chgrp bin $(ICONV); \
	fi
	if [ ! -d $(ICONV)/codesets ] ; \
	then \
	mkdir $(ICONV)/codesets; \
	$(CH)chmod 755 $(ICONV)/codesets; \
	$(CH)chown bin $(ICONV)/codesets; \
	$(CH)chgrp bin $(ICONV)/codesets; \
	fi
	for i in  $(CODESETS) ;\
		do \
		$(CH)$(BIN)/kbdcomp -o `basename $$i .p` $$i ; \
		$(CH)$(INS) -f $(ICONV) -m 444 -u bin -g bin `basename $$i .p` ; \
		$(INS) -f $(ICONV)/codesets -m 444 -u bin -g bin $$i; \
		done
	$(INS) -f  $(ICONV) -m 444 -u bin -g bin codesets/iconv_data

	
$(IOBJS): ./iconv.h ./symtab.h ./kbd.h

$(KOBJS):	./symtab.h ./kbd.h

.c.o :
	$(CC) -c $(CFLAGS) $<

.PRECIOUS:	gram.y

gram.c:	gram.y ./symtab.h ./kbd.h
	$(YACC) -vd gram.y
	mv y.tab.c gram.c

clean:
	rm -f *.o *.t y.tab.h y.output gram.c
	
clobber: clean
	rm -f *.o *.t iconv kbdcomp
	$(CH)for i in  $(CODESETS) ;\
		$(CH)do \
		$(CH) rm -f `basename $$i .p` ; \
		$(CH)done


