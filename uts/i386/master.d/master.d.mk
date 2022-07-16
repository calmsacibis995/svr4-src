#ident	"@(#)master.d.mk	1.2	91/09/15	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)master:master.d.mk	1.3.2.1"

BUS=AT386
ARCH=AT386
NET=
NET2=
NET3=
LCPP = $(CPP) -P $(MORECPP)

COMMODS =\
	nfs	\
	fp	\
	gentty	\
	kernel	\
	weitek	\
	mem	\
	merge	\
	osm	\
	async	\
	ldterm	\
	ansi	\
	char	\
	sad	\
	events	\
	nmi	\
	shm	\
	sem	\
	ipc	\
	msg	\
	pic	\
	specfs	\
	fifofs	\
	fdfs	\
	kma	\
	kmacct	\
	hrt	\
	nfa	\
	prf	\
	sxt	\
	nsxt	\
	xt	\
	nxt	\
	cpyrt	\
	pipemod	\
	ttcompat	\
	s5	\
	ufs	\
	xnamfs	\
	RFS	\
	namefs	\
	bfs	\
	elf	\
	coff	\
	xout	\
	intp	\
	i286x	\
	dosx	\
	rt	\
	ts	\
	clist	\
	connld	\
	gendisp	\
	proc	\
	rmc	\
	xque	\
	ws	\
	sysmsg	\
	vx	\
	raio	\
	app	\
	arp	\
	clone	\
	des	\
	icmp	\
	ip	\
	klm	\
	krpc	\
	ktli	\
	llcloop	\
	log	\
	pckt	\
	ptem	\
	ptm	\
	pts	\
	ramd	\
	rawip	\
	sockmod	\
	tcp	\
	ticlts	\
	ticots	\
	ticotsor	\
	timod	\
	tirdwr	\
	udp	\
	gdebugger	\
	kdb	\
	kdb-util

IDDIR =\
	init.d	\
	mfsys.d	\
	node.d	\
	rc.d	\
	sd.d	\
	mdevice.d	\
	sdevice.d	\
	sfsys.d

CFFILES =\
	mdevice	\
	sassign	\
	stune	\
	mtune	\
	init.base

STD_CONF_FILES =\
	mfsys	\
	sfsys	\
	node	\
	rc	\
	sd	\
	init

CONF_FILES =\
	$(STD_CONF_FILES) \
	mdev	\
	sdev

all:
	@ echo "\ncopying master.d/* to $(CONF) build tree.....\n"

	-@ for i in $(IDDIR) pack.d cf.d				;\
	do								\
		[ -d $(CONF)/$$i ] || mkdir $(CONF)/$$i			;\
	done
	-@ for i in $(IDDIR)						;\
	do								\
		rm -f $(CONF)/$$i/*					;\
	done
	-@ for i in $(CFFILES)						;\
	do								\
		rm -f $(CONF)/cf.d/$$i					;\
		if [ -f $$i ]						;\
		then							 \
			sed 's/^# /+/' $$i > $$i.c			;\
			$(LCPP) $$i.c	|				 \
			sed -e 's/^+/# /' -e '/^$$/d' >$(CONF)/cf.d/$$i	;\
			rm -r $$i.c					;\
		fi							;\
	done
	-@ for i in $(COMMODS)                                          ;\
	do								 \
	echo  "$$i \c"							;\
	[ -d $(CONF)/pack.d/$$i ] || mkdir $(CONF)/pack.d/$$i           ;\
	[ -f $$i/*.c ] && cp $$i/*.c $(CONF)/pack.d/$$i			;\
	for f in $(STD_CONF_FILES)					;\
	do								 \
		[ -f $$i/$$f ] && $(LCPP) $$i/$$f |			 \
		sed -e '/^[\*#]ident/d' -e '/^$$/d' > $(CONF)/$$f.d/$$i	;\
	done								;\
	[ -f $$i/mdev ] && ($(LCPP) $$i/mdev |				 \
	  sed -e '/^[\*#]ident/d' -e '/^$$/d' > $(CONF)/mdevice.d/$$i	;\
	  cat $(CONF)/mdevice.d/$$i >> $(CONF)/cf.d/mdevice)		;\
	[ -f $$i/sdev ] && $(LCPP) $$i/sdev |				 \
	  sed -e '/^[\*#]ident/d' -e '/^$$/d' > $(CONF)/sdevice.d/$$i	;\
	[ -f $(CONF)/pack.d/$$i/* ] && chmod +w $(CONF)/pack.d/$$i/*	;\
	done
	$(MAKE) -f master.$(ARCH).mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" "CPP=$(CPP)"	;\
	echo

clean:

clobber:	clean
	for i in $(COMMODS) ;\
	do						 \
		rm -rf $(CONF)/pack.d/$$i		;\
	done
	-@ for i in $(IDDIR)				;\
	do						 \
		rm -f $(CONF)/$$i/*			;\
	done
	$(MAKE) -f master.$(ARCH).mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" "CPP=$(CPP)" clobber;\
