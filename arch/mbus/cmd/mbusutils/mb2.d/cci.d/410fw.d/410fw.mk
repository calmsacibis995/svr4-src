#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1990  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/410fw.d/410fw.mk	1.3.1.1"

# Since 410.fw is delivered to AT&T as a binary file,
# it needs to be converted into a hex dump source file in order
# to be maintained under sccs.
# To create a new version of the hex dump source (410.fwsrc) from
# the delivered binary file (410.fw), do the following:
# copy in the new binaries, then use the make target 'new<binary>src'
# for each new binary.
# Copy the result to '<binary>src', then make sure a make using
# the binary as target produces the correct result.
#
# This makefile recreates 410.fw from 410.fwsrc using the 'deodx'
# program that is used for a similar purpose in the layers component.
# Note that the bytes must be swapped due to the fact that od -x interprets
# 2 byte words, and the byte ordering on VAX and i386 is reversed from
# the character ordering.
# NOTE: if this is ported to a 3b2 or Amdahl, don't swap the bytes

DASHO = -O

DEODX = $(ROOT)/usr/src/cmd/layers/deodx

PCT = %

SBIN = $(ROOT)/sbin

ULIB = $(ROOT)/usr/lib

BIN = $(ULIB)/cci

INC = $(ROOT)/usr/include

CFLAGS = $(DASHO) -I$(INC) -DMBIIU $(MORECPP)

CC = cc

SRCS = lckld.c ttyswitch.c utils.c

OBJS = $(SRCS:.c=.o)

MAINS = lckld ttyswitch

BINARIES = 410.fw 450.fw mix386.fw run450.fw mix450.bt mix.stg2

BINSRCS = 410.fwsrc 450.fwsrc mix386.fwsrc run450.fwsrc mix450.btsrc mix.stg2src

NEWBINSRCS = new410.fwsrc new450.fwsrc newmix386.fwsrc newrun450.fwsrc newmix450.btsrc newmix.stg2src

.PRECIOUS: $(SRCS) $(BINSRCS)

install: all dirs
	for i in $(MAINS)                                ;\
	do                                                \
		cpset $$i $(SBIN)/$$i 0700 root sys       ;\
	done
	for i in $(BINARIES)                                ;\
	do                                                \
		cpset $$i $(BIN)/$$i 0700 root sys       ;\
	done

all: $(MAINS) $(BINARIES) 

dirs:
	-@[ -d $(SBIN) ]  || mkdir $(SBIN)
	-@[ -d $(ULIB) ] || mkdir $(ULIB)
	-@[ -d $(BIN) ]  || mkdir $(BIN)

$(MAINS):
	$(CC) -o $@ $? $(LDFLAGS) -lmb2 $(SHLIBS)

$(DEODX):
	cd $(ROOT)/usr/src/cmd/layers; \
	$(MAKE) -f layers.mk deodx

$(BINARIES): $(DEODX)
	$(DEODX) <$@src | dd conv=swab | \
	dd ibs=1 count=`grep bytecount $@src | \
	sed -e 's/.*= *//'` >$@
	
# The subshell in the following is because the accursed 'od' scrunches
# identical lines into a single '*'
$(NEWBINSRCS):
	OBJ=`expr $@ : 'new\(.*\)src'` ; \
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
	echo You must rename $@ to $${OBJ}src before you pdelta

clean :
	rm -f $(OBJS)

clobber :
	rm -f $(OBJS) $(MAINS) $(BINARIES) $(DEODX)

lckld.o : lckld.c common.h

utils.o : utils.c common.h

lckld : lckld.o utils.o 

ttyswitch : ttyswitch.o 

410.fw : 410.fwsrc

450.fw : 450.fwsrc

mix386.fw : mix386.fwsrc

run450.fw : run450.fwsrc

mix450.bt : mix450.btsrc

mix.stg2 : mix.stg2src
