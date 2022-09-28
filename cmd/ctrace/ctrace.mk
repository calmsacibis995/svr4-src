#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ctrace:ctrace.mk	1.17"
#	ctrace makefile
# installation directories:
ROOT=
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
USRBIN=$(ROOT)/usr/bin
USRLIB=$(ROOT)/usr/lib
INSDIR=$(CCSBIN)
CTLIB=$(CCSLIB)/ctrace
CRLIB=/usr/ccs/lib/ctrace

CC=cc
DEFLIST = -DRUNTIME=\"$(CRLIB)/runtime.c\"
CFLAGS = -O
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
CC_CMD = $(CC) -c $(CFLAGS) $(DEFLIST) -I$(INC)
CMDBASE=..
INS=$(CMDBASE)/install/install.sh
LIBELF=
LINK_MODE=

# Must derive y.tab.h from acgram.y
YACC=           yacc
# add -t for yacc debug (YYDEBUG)
# add -v for yacc y.output file
YFLAGS=
YYDEBUG=
YACC_CMD=       $(YACC) $(YFLAGS) -d

LEX= lex
LFLAGS=

LINT= lint
LINTFLAGS=-b

STRIP=strip

SOURCE	 = constants.h global.h aclex.h \
	   main.c parser.y scanner.l lookup.c trace.c \
	   runtime.c ctcr
CFILES =  main.c parser.c scanner.c lookup.c trace.c
OBJECTS =  main.$o parser.$o scanner.$o lookup.$o trace.$o

all build:	ctrace 

ctrace:		$(OBJECTS:$o=o)
		$(CC) $(CFLAGS) $(OBJECTS:$o=o) $(LINK_MODE) -o $@ 

main.o:		main.c
		if u3b2 || u3b5 || u3b15 ;\
		then \
			$(CC_CMD) '-DTRACEVERS="01.01"' -I../sgs/inc/m32 main.c;\
		elif i386 ;\
		then \
			$(CC_CMD) '-DTRACEVERS="01.01"' -I../sgs/inc/i386 main.c;\
		fi

parser.c:	parser.y
		$(YACC_CMD) parser.y
		mv y.tab.c parser.c
		if cmp -s y.tab.h parser.h; then rm y.tab.h; \
		else cp y.tab.h parser.h; fi

parser.h:	parser.c

scanner.c:	scanner.l
		$(LEX) $(LFLAGS) scanner.l
		mv lex.yy.c scanner.c

parser.o:	parser.c parser.h
		$(CC_CMD) $(YYDEBUG) parser.c

scanner.o:	scanner.c
		$(CC_CMD) scanner.c	

lookup.o:	lookup.c
		$(CC_CMD) lookup.c

trace.o:	trace.c
		$(CC_CMD) trace.c
	
install: 	all
		cp ctrace ctrace.bak
		cp ctcr ctcr.bak
		$(STRIP) ctrace
		/bin/sh $(INS) -f $(INSDIR) ctrace
		/bin/sh $(INS) -f $(INSDIR) ctcr
		ln $(INSDIR)/ctcr $(INSDIR)/ctc
		if [ ! -d $(CTLIB) ] ;\
		then \
			mkdir $(CTLIB);\
		fi
		/bin/sh $(INS) -f $(CTLIB) runtime.c
		mv ctrace.bak ctrace
		mv ctcr.bak ctcr

clean:
		rm -f *.o y.output
		rm -f lint.out
		rm -f *.ln

clobber: 	clean
		rm -f ctrace parser.[ch] scanner.c y.tab.h

lintit:		$(CFILES)
		if u3b2 || u3b5 || u3b15 ;\
		then \
			$(LINT) $(LINTFLAGS) -I../sgs/inc/m32 -I$(INC) $(CFILES) ;\
		elif i386 ;\
		then \
			$(LINT) $(LINTFLAGS) -I../sgs/inc/i386 -I$(INC) $(CFILES) ;\
		fi

ctrace.ln:	$(CFILES)
		rm -f $(OBJECTS:$o=ln)
		if u3b2 || u3b5 || u3b15 ;\
		then \
		$(LINT) $(LINTFLAGS) -c -I../sgs/inc/m32 -I$(INC) $(CFILES);\
		elif i386 ;\
		then \
		$(LINT) $(LINTFLAGS) -c -I../sgs/inc/i386 -I$(INC) $(CFILES);\
		fi
		cat $(OBJECTS:$o=ln) >ctrace.ln
