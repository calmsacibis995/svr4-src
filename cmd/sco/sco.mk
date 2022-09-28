#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sco:sco.mk	1.3.1.1"

#	Makefile for sco 

ROOT =

DEODX = $(ROOT)/usr/src/cmd/layers/deodx

LEX = lex

DIR = $(ROOT)/bin

INC = $(ROOT)/usr/include

INS = install

INSDIR = $(ROOT)/usr/bin

INSDIR2 = $(ROOT)/etc/default

INSDIR3 = $(ROOT)/usr/lib/console

INSDIR4 = $(ROOT)/usr/lib/keyboard

INSDIR5 = $(ROOT)/usr/lib/vidi

CFLAGS = -O -I$(INC)

LDFLAGS = -s $(SHLIBS)

MAKEFILE = sco.mk

MAINS = scompat setcolor mapkey mapstr mapscrn vidi mapchan trchan

CCHANOBJ = convert.o display.o lex.yy.o oops.o deflt.o

OBJECTS =  scompat.o setcolor.o mapkey.o mapio.o mapstr.o mapscrn.o vidi.o \
	   $(CCHANOBJ) mapchan.o trchan.o

SOURCES =  scompat.c setcolor.c mapkey.c mapio.c mapstr.c mapscrn.c vidi.c \
	   convert.c display.c lex.yy.c oops.c deflt.c

FLATFILE = keys strings

FLATFILE1 = screens

FONTS = font8x8 font8x16 font8x14

LIB=libdos.a
DOSOBJECTS=$(CFILES:.c=.o)
CMDS=doscat doscp dosdir dosformat dosls dosmkdir dosrm dosrmdir
UTILS=doschain dosinfo fatlist fdread loc_dir

all:	$(MAINS) $(LIB) $(CMDS) $(FONTS)

scompat:		scompat.o 
	$(CC) $(CFLAGS) -o scompat scompat.o $(LDFLAGS)

setcolor:		setcolor.o 
	$(CC) $(CFLAGS) -o setcolor setcolor.o $(LDFLAGS)

mapkey:		mapkey.o  mapio.o
	$(CC) $(CFLAGS) -o mapkey mapkey.o mapio.o $(LDFLAGS)

mapstr:		mapstr.o 
	$(CC) $(CFLAGS) -o mapstr mapstr.o $(LDFLAGS)

mapscrn:		mapscrn.o 
	$(CC) $(CFLAGS) -o mapscrn mapscrn.o $(LDFLAGS)

mapchan:	mapchan.o  $(CCHANOBJ)
	$(CC) $(CFLAGS) -o mapchan mapchan.o $(CCHANOBJ) $(LDFLAGS)

trchan:	trchan.o  $(CCHANOBJ)
	$(CC) $(CFLAGS) -o trchan trchan.o $(CCHANOBJ) $(LDFLAGS)

lex.yy.c:  lex.l defs.h
	$(LEX) lex.l

vidi:	vidi.o $(DEODX) $(FONTS)
	$(CC) $(CFLAGS) -o vidi vidi.o $(LDFLAGS)

$(DEODX):
	cd $(ROOT)/usr/src/cmd/layers; \
	$(MAKE) -f layers.mk deodx

scompat.o:	 $(INC)/sys/types.h $(INC)/sys/kd.h \
		 $(INC)/stdio.h $(INC)/fcntl.h $(INC)/sys/at_ansi.h \
		 $(INC)/errno.h $(INC)/string.h $(INC)/sys/param.h 

setcolor.o:	 $(INC)/sys/types.h $(INC)/sys/kd.h \
		 $(INC)/stdio.h $(INC)/fcntl.h $(INC)/sys/at_ansi.h \
		 $(INC)/errno.h $(INC)/string.h $(INC)/sys/param.h 

font8x8:	font8x8.src
	$(DEODX) <font8x8.src | dd conv=swab | \
	dd ibs=1 count=`grep bytecount font8x8.src | \
	sed -e 's/.*= *//'` > font8x8


font8x14:	font8x14.src
	$(DEODX) <font8x14.src | dd conv=swab | \
	dd ibs=1 count=`grep bytecount font8x14.src | \
	sed -e 's/.*= *//'` > font8x14

font8x16:	font8x16.src
	$(DEODX) <font8x16.src | dd conv=swab | \
	dd ibs=1 count=`grep bytecount font8x16.src | \
	sed -e 's/.*= *//'` > font8x16

# The subshell in the following is because the accursed 'od' scrunches
# identical lines into a single '*'
nfont8x16.src:
	OBJ=font8x16; \
	od -x $$OBJ | \
	( \
	flag=0 ;\
	oldaddr=0 ;\
	while read addr line ;\
	do \
		if [ "$$flag" -eq 1 -a "$$addr" != '*' ] ;\
		then \
                        i=`echo "ibase=8; ($$addr - $$oldaddr)/020; quit" | bc` ;\
			while [ "$$i" -gt 1 ] ;\
			do \
                                echo "$$oldaddr" "$$oldline" ;\
                                i=`expr $$i - 1` ;\
			done ;\
			flag=0 ;\
		elif [ "$$addr" = '*' ] ;\
		then \
			flag=1 ;\
			continue ;\
		fi ;\
		oldaddr="$$addr" ;\
		oldline="$$line" ;\
		echo "$$addr" "$$line" ;\
	done; ) | \
	sed -e '/^[^ ]*$$/d' -e 's/^[0-7]* //' -e 's/ //g' | \
	( echo "#ident\t\"$(PCT)W$(PCT)\"\n\n# bytecount=`wc -c <$$OBJ`"; \
	  cat - ) >$@ ; \
	echo You must rename $@ to $${OBJ}.src before you pdelta

nfont8x14.src:
	OBJ=font8x14; \
	od -x $$OBJ | \
	( \
	flag=0 ;\
	oldaddr=0 ;\
	while read addr line ;\
	do \
		if [ "$$flag" -eq 1 -a "$$addr" != '*' ] ;\
		then \
                        i=`echo "ibase=8; ($$addr - $$oldaddr)/020; quit" | bc` ;\
			while [ "$$i" -gt 1 ] ;\
			do \
                                echo "$$oldaddr" "$$oldline" ;\
                                i=`expr $$i - 1` ;\
			done ;\
			flag=0 ;\
		elif [ "$$addr" = '*' ] ;\
		then \
			flag=1 ;\
			continue ;\
		fi ;\
		oldaddr="$$addr" ;\
		oldline="$$line" ;\
		echo "$$addr" "$$line" ;\
	done; ) | \
	sed -e '/^[^ ]*$$/d' -e 's/^[0-7]* //' -e 's/ //g' | \
	( echo "#ident\t\"$(PCT)W$(PCT)\"\n\n# bytecount=`wc -c <$$OBJ`"; \
	  cat - ) >$@ ; \
	echo You must rename $@ to $${OBJ}.src before you pdelta

nfont8x8.src:
	OBJ=font8x8 ; \
	od -x $$OBJ | \
	( \
	flag=0 ;\
	oldaddr=0 ;\
	while read addr line ;\
	do \
		if [ "$$flag" -eq 1 -a "$$addr" != '*' ] ;\
		then \
                        i=`echo "ibase=8; ($$addr - $$oldaddr)/020; quit" | bc` ;\
			while [ "$$i" -gt 1 ] ;\
			do \
                                echo "$$oldaddr" "$$oldline" ;\
                                i=`expr $$i - 1` ;\
			done ;\
			flag=0 ;\
		elif [ "$$addr" = '*' ] ;\
		then \
			flag=1 ;\
			continue ;\
		fi ;\
		oldaddr="$$addr" ;\
		oldline="$$line" ;\
		echo "$$addr" "$$line" ;\
	done; ) | \
	sed -e '/^[^ ]*$$/d' -e 's/^[0-7]* //' -e 's/ //g' | \
	( echo "#ident\t\"$(PCT)W$(PCT)\"\n\n# bytecount=`wc -c <$$OBJ`"; \
	  cat - ) >$@ ; \
	echo You must rename $@ to $${OBJ}.src before you pdelta

$(INSDIR) $(INSDIR2) $(INSDIR3) $(INSDIR4) $(INSDIR5):
	-mkdir $@
	-$(CH) chmod 755 $@
	-$(CH) chown bin $@
	-$(CH) chgrp bin $@

CFILES=MS-DOS.c \
	add_device.c \
	alloc_clust.c \
	basename.c \
	chain_clust.c \
	close_device.c \
	critical.c \
	del_label.c \
	disp_vol.c \
	dos_fil_size.c \
	doslabel.c \
	dos_mod_date.c \
	dos_mod_time.c \
	fix_slash.c \
	free_space.c \
	get_assign.c \
	get_label.c \
	is_dir_empty.c \
	loc_free_dir.c \
	locate.c \
	lookup_dev.c \
	lookup_drv.c \
	make_label.c \
	mkdir.c \
	my_fgets.c \
	next_cluster.c \
	next_sector.c \
	open_device.c \
	parse_name.c \
	read_file.c \
	read_sector.c \
	rm_file.c \
	scan_dos_dir.c \
	strupr.c \
	ud_copy.c \
	uu_copy.c \
	write_fat.c \
	write_sector.c


dosall:	$(LIB) $(CMDS)

doschain:	$(LIB) chain_clust.c
	$(CC) $(CFLAGS) $(LDFLAGS) doschain -DSTANDALONE chain_clust.c $(LIB) $(LDFLAGS)

doscat:		$(LIB) doscat.c
	$(CC) $(CFLAGS) -o doscat doscat.c $(LIB) $(LDFLAGS)

doscp:		$(LIB) doscp.c
	$(CC) $(CFLAGS) -o doscp doscp.c $(LIB) $(LDFLAGS)

dosdir:		$(LIB) dosls.c
	$(CC) $(CFLAGS) -o dosdir -DDOSDIR dosls.c $(LIB) $(LDFLAGS)

dosformat:	$(LIB) dosformat.c
	$(CC) $(CFLAGS) -o dosformat dosformat.c $(LIB) $(LDFLAGS)

dosinfo:	$(LIB) dosinfo.c
	$(CC) $(CFLAGS) -o dosinfo dosinfo.c $(LIB) $(LDFLAGS)

dosls:		$(LIB) dosls.c
	$(CC) $(CFLAGS) -o dosls dosls.c $(LIB) $(LDFLAGS)

dosmkdir:	$(LIB) dosmkdir.c
	$(CC) $(CFLAGS) -o dosmkdir dosmkdir.c $(LIB) $(LDFLAGS)

dosrm:		$(LIB) dosrm.c
	$(CC) $(CFLAGS) -o dosrm dosrm.c $(LIB) $(LDFLAGS)

dosrmdir:	$(LIB) dosrm.c
	$(CC) $(CFLAGS) -o dosrmdir -DDOSRMDIR dosrm.c $(LIB) $(LDFLAGS)

fatlist:	$(LIB) fatlist.c
	$(CC) $(CFLAGS) -o fatlist fatlist.c $(LIB) $(LDFLAGS)

fdread:		$(LIB) fdread.c
	$(CC) $(CFLAGS) -o fdread fdread.c $(LIB) $(LDFLAGS)

libdos.a:	$(DOSOBJECTS)
	$(AR) rv $(LIB) $?

loc_dir:	$(LIB) loc_free_dir.c
	$(CC) $(CFLAGS) -DSTANDALONE -o loc_dir loc_free_dir.c $(LIB) $(LDFLAGS)

$(DOSOBJECTS):	MS-DOS.h

utils:	$(UTILS)

install:	$(INSDIR) $(INSDIR2) $(INSDIR3) $(INSDIR4) $(INSDIR5) all
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin trchan
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin mapchan
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin setcolor
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin scompat
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin mapkey
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin mapstr
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin mapscrn
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin vidi
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin doscat
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin doscp
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin dosdir
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin dosformat
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin dosls
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin dosmkdir
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin dosrm
	$(INS) -f $(INSDIR) -m 0711 -u bin -g bin dosrmdir
	$(INS) -f $(INSDIR2) -m 0644 -u bin -g bin msdos
	$(INS) -f $(INSDIR5) -m 0644 -u bin -g bin font8x8
	$(INS) -f $(INSDIR5) -m 0644 -u bin -g bin font8x14
	$(INS) -f $(INSDIR5) -m 0644 -u bin -g bin font8x16
	-@ for i in $(FLATFILE) ; do \
		rm -f $(INSDIR4)/$$i ;\
		if [ -f $$i ] ; then \
			cat $$i | sed -e '/ident/d' > $(INSDIR4)/$$i ;\
			chmod 644 $(INSDIR4)/$$i ;\
			chgrp bin $(INSDIR4)/$$i ;\
			chown bin $(INSDIR4)/$$i ;\
		fi ;\
	done 
	-@ for i in $(FLATFILE1) ; do \
		rm -f $(INSDIR3)/$$i ;\
		if [ -f $$i ] ; then \
			cat $$i | sed -e '/ident/d' > $(INSDIR3)/$$i ;\
			chmod 633 $(INSDIR3)/$$i ;\
			chgrp bin $(INSDIR3)/$$i ;\
			chown bin $(INSDIR3)/$$i ;\
		fi ;\
	done 
 
clean:
	rm -f $(OBJECTS) lex.yy.c
	rm -f $(DOSOBJECTS)

clobber: clean
	rm -f $(OBJECTS) $(MAINS) $(DOSOBJECTS) $(DEODX)
	rm -f $(CMDS) $(LIB) $(UTILS)
