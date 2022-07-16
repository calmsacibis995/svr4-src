#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license
#	agreement with Intel Corporation and may not be copied nor
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/i279.d/i279.mk	1.3"

# Since most of the files in this directory are binary files,
# they need to be converted into hex dump source files in order
# to be maintained under sccs.
# To create a new version of the hex dump sources from the binary files,
# copy in the new binaries, then use the make target 'new<binary>src'
# for each new binary.
# Copy the result to '<binary>src', then make sure a make using
# the binary as target produces the correct result.
#
# This makefile recreates the binaries from the respective hex dumps
# using the 'deodx' program that is used for a similar purpose in the
# layers component.
# Note that the bytes must be swapped due to the fact that od -x interprets
# 2 byte words, and the byte ordering on VAX and i386 is reversed from
# the character ordering.
# NOTE: if this is ported to a 3b2 or Amdahl, don't swap the bytes

DEODX = $(ROOT)/usr/src/cmd/layers/deodx

PCT = %

ULIB = $(ROOT)/usr/lib

BIN = $(ULIB)/i279

INC = $(ROOT)/usr/include

OBJS = menu

BINARIES = menu

BINSRCS = menusrc

NEWBINSRCS = newmenusrc

.PRECIOUS: $(BINSRCS)

install: all dirs
	for i in $(OBJS)                                 ;\
	do                                                \
		cpset $$i $(BIN)/$$i 0700 root sys       ;\
	done

all:	$(BINARIES)

dirs:
	-@[ -d $(ULIB) ] || mkdir $(ULIB)
	-@[ -d $(BIN) ]  || mkdir $(BIN)

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

menu : menusrc

clean:

clobber:
	rm -f $(BINARIES) $(DEODX)
