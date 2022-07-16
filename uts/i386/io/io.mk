#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-io:io.mk	1.3.3.1"

INCRT = ..
CFLAGS = -O -I$(INCRT) -D_KERNEL -DSYSV -DSVR40 $(MORECPP)
FRC =
NET=
NET2=
NET3=
BUS=AT386
ARCH=AT386

TCOOSED = /bin/sed -e s/ticots/ticotsord/g -e s/tco/tcoo/g -e s/TCO/TCOO/g

NETFILES =\
	$(CONF)/pack.d/sockmod/Driver.o	\
	$(CONF)/pack.d/ticots/Driver.o	\
	$(CONF)/pack.d/ticotsor/Driver.o	\
	$(CONF)/pack.d/ticlts/Driver.o	\
	$(CONF)/pack.d/timod/Driver.o   \
	$(CONF)/pack.d/tirdwr/Driver.o   \
	$(CONF)/pack.d/pckt/Driver.o   \
	$(CONF)/pack.d/ptem/Driver.o


BASEFILES =\
	$(CONF)/pack.d/clist/Driver.o	\
	$(CONF)/pack.d/weitek/Driver.o	\
	$(CONF)/pack.d/gentty/Driver.o	\
	$(CONF)/pack.d/kernel/io.o	\
	$(CONF)/pack.d/kmacct/Driver.o   \
	$(CONF)/pack.d/hrt/Driver.o	\
	$(CONF)/pack.d/mem/Driver.o	\
	$(CONF)/pack.d/osm/Driver.o	\
	$(CONF)/pack.d/sad/Driver.o	\
	$(CONF)/pack.d/ldterm/Driver.o	\
	$(CONF)/pack.d/ansi/Driver.o	\
	$(CONF)/pack.d/char/Driver.o	\
	$(CONF)/pack.d/xt/Driver.o	\
	$(CONF)/pack.d/nxt/Driver.o	\
	$(CONF)/pack.d/sxt/Driver.o	\
	$(CONF)/pack.d/nsxt/Driver.o	\
	$(CONF)/pack.d/cpyrt/Driver.o   \
	$(CONF)/pack.d/sysmsg/Driver.o   \
	$(CONF)/pack.d/ramd/Driver.o   \
	$(CONF)/pack.d/connld/Driver.o   \
	$(CONF)/pack.d/pipemod/Driver.o   \
	$(CONF)/pack.d/ttcompat/Driver.o   \
	$(CONF)/pack.d/ws/Driver.o   \
	$(CONF)/pack.d/xque/Driver.o   \
	$(CONF)/pack.d/raio/Driver.o   \
	$(CONF)/pack.d/prf/Driver.o   \
	$(CONF)/pack.d/clone/Driver.o   \
	$(CONF)/pack.d/log/Driver.o   \
	$(CONF)/pack.d/ptm/Driver.o   \
	$(CONF)/pack.d/pts/Driver.o

DFILES = \
	hrtimers.o \
	ladd.o \
	lsub.o \
	lmul.o \
	ldivide.o \
	lshiftl.o \
	lsign.o


all:	$(BASEFILES) $(NETFILES) archmk

nonet: $(BASEFILES) archmk

archmk:
	$(MAKE) -f io.$(ARCH).mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" "CFLAGS=$(CFLAGS)"

clean:
	$(MAKE) -f io.$(ARCH).mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" clean
	cd ws; $(MAKE) -f ws.mk clean "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)"  "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)" "CFLAGS=$(CFLAGS) -I../.."
	-rm -f *.o

clobber:	clean
	$(MAKE) -f io.$(ARCH).mk "BUS=$(BUS)" "ARCH=$(ARCH)" "ROOT=$(ROOT)" clobber
	cd ws; $(MAKE) -f ws.mk clobber "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)"  "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)" "CFLAGS=$(CFLAGS) -I../.."
	-rm -f $(COMFILES) ticotsord.c

FRC:

$(CONF)/pack.d/hrt/Driver.o:	$(DFILES)
	$(LD) -r -o $@ $(DFILES)

$(CONF)/pack.d/gentty/Driver.o:gentty.o
	$(LD) -r -o $@ gentty.o

$(CONF)/pack.d/kernel/io.o:	emap.o partab.o physdsk.o stream.o iosubr.o dmacheck.o
	$(LD) -r -o $@ emap.o partab.o physdsk.o stream.o iosubr.o dmacheck.o

$(CONF)/pack.d/mem/Driver.o:	mem.o
	$(LD) -r -o $@ mem.o

$(CONF)/pack.d/clist/Driver.o:	tty.o tt1.o clist.o
	$(LD) -r -o $@ tty.o tt1.o clist.o

$(CONF)/pack.d/osm/Driver.o:	osm.o
	$(LD) -r -o $@ osm.o

$(CONF)/pack.d/ansi/Driver.o:	ansi.o
	$(LD) -r -o $@ ansi.o

$(CONF)/pack.d/char/Driver.o:	char.o
	$(LD) -r -o $@ char.o

$(CONF)/pack.d/xque/Driver.o:	xque.o
	$(LD) -r -o $@ xque.o

$(CONF)/pack.d/sad/Driver.o:	sad.o
	$(LD) -r -o $@ sad.o

$(CONF)/pack.d/sockmod/Driver.o:	sockmod.o
	[ -d $(CONF)/pack.d/sockmod ] || mkdir $(CONF)/pack.d/sockmod; \
	$(LD) -r -o $@ sockmod.o

$(CONF)/pack.d/ticots/Driver.o:		ticots.o
	[ -d $(CONF)/pack.d/ticots ] || mkdir $(CONF)/pack.d/ticots; \
	$(LD) -r -o $@ ticots.o

$(CONF)/pack.d/ticotsor/Driver.o:	ticotsord.o
	[ -d $(CONF)/pack.d/ticotsor ] || mkdir $(CONF)/pack.d/ticotsor; \
	$(LD) -r -o $@ ticotsord.o

$(CONF)/pack.d/ticlts/Driver.o:		ticlts.o
	[ -d $(CONF)/pack.d/ticlts ] || mkdir $(CONF)/pack.d/ticlts; \
	$(LD) -r -o $@ ticlts.o

$(CONF)/pack.d/ldterm/Driver.o:	ldterm.o
	$(LD) -r -o $@ ldterm.o

$(CONF)/pack.d/raio/Driver.o:	raio.o
	$(LD) -r -o $@ raio.o

$(CONF)/pack.d/prf/Driver.o:	prf.o
	$(LD) -r -o $@ prf.o

$(CONF)/pack.d/sxt/Driver.o:	sxt.o
	$(LD) -r -o $@ sxt.o

$(CONF)/pack.d/nsxt/Driver.o:	nsxt.o
	$(LD) -r -o $@ nsxt.o

$(CONF)/pack.d/xt/Driver.o:	xt.o
	$(LD) -r -o $@ xt.o

$(CONF)/pack.d/nxt/Driver.o:	nxt.o
	$(LD) -r -o $@ nxt.o

$(CONF)/pack.d/weitek/Driver.o:	weitek.o ctl87.o
	$(LD) -r -o $@ weitek.o ctl87.o

$(CONF)/pack.d/cpyrt/Driver.o:	cpyrt.o
	$(LD) -r -o $@ cpyrt.o

$(CONF)/pack.d/ramd/Driver.o:	ramd.o
	[ -d $(CONF)/pack.d/ramd ] || mkdir $(CONF)/pack.d/ramd; \
	$(LD) -r -o $@ ramd.o

$(CONF)/pack.d/connld/Driver.o:	connld.o
	[ -d $(CONF)/pack.d/connld ] || mkdir $(CONF)/pack.d/connld; \
	$(LD) -r -o $@ connld.o

$(CONF)/pack.d/kmacct/Driver.o:	kmacct.o
	[ -d $(CONF)/pack.d/kmacct ] || mkdir $(CONF)/pack.d/kmacct; \
	$(LD) -r -o $@ kmacct.o

$(CONF)/pack.d/sysmsg/Driver.o:	sysmsg.o
	[ -d $(CONF)/pack.d/sysmsg ] || mkdir $(CONF)/pack.d/sysmsg; \
	$(LD) -r -o $@ sysmsg.o

$(CONF)/pack.d/pipemod/Driver.o:	pipemod.o
	$(LD) -r -o $@ pipemod.o

$(CONF)/pack.d/ttcompat/Driver.o:	ttcompat.o
	$(LD) -r -o $@ ttcompat.o

$(CONF)/pack.d/clone/Driver.o:	clone.o
	[ -d $(CONF)/pack.d/clone ] || mkdir $(CONF)/pack.d/clone; \
	$(LD) -r -o $@ clone.o

$(CONF)/pack.d/log/Driver.o:	log.o
	[ -d $(CONF)/pack.d/log ] || mkdir $(CONF)/pack.d/log; \
	$(LD) -r -o $@ log.o

$(CONF)/pack.d/timod/Driver.o:	timod.o
	[ -d $(CONF)/pack.d/timod ] || mkdir $(CONF)/pack.d/timod; \
	$(LD) -r -o $@ timod.o

$(CONF)/pack.d/tirdwr/Driver.o:	tirdwr.o
	[ -d $(CONF)/pack.d/tirdwr ] || mkdir $(CONF)/pack.d/tirdwr; \
	$(LD) -r -o $@ tirdwr.o

$(CONF)/pack.d/pckt/Driver.o:	pckt.o
	[ -d $(CONF)/pack.d/pckt ] || mkdir $(CONF)/pack.d/pckt; \
	$(LD) -r -o $@ pckt.o

$(CONF)/pack.d/ptem/Driver.o:	ptem.o
	[ -d $(CONF)/pack.d/ptem ] || mkdir $(CONF)/pack.d/ptem; \
	$(LD) -r -o $@ ptem.o

$(CONF)/pack.d/ptm/Driver.o:	ptm.o
	[ -d $(CONF)/pack.d/ptm ] || mkdir $(CONF)/pack.d/ptm; \
	$(LD) -r -o $@ ptm.o

$(CONF)/pack.d/pts/Driver.o:	pts.o
	[ -d $(CONF)/pack.d/pts ] || mkdir $(CONF)/pack.d/pts; \
	$(LD) -r -o $@ pts.o

$(CONF)/pack.d/ws/Driver.o:	WS

WS:
	cd ws; $(MAKE) -f ws.mk "CC=$(CC)" "AS=$(AS)" "LD=$(LD)" "FRC=$(FRC)" "MORECPP=$(MORECPP)"  "BUS=$(BUS)" "ARCH=$(ARCH)" "CONF=$(CONF)" "NET=$(NET)" "NET2=$(NET2)" "NET3=$(NET3)" "CFLAGS=$(CFLAGS) -I../.."

#
# Header dependencies
#

clist.o: clist.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/var.h \
	$(FRC)

partab.o: partab.c \
	$(FRC)

physdsk.o: physdsk.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

dmacheck.o: dmacheck.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/vm/seg_kmem.h \
	$(INCRT)/vm/mp.h \
	$(INCRT)/vm/page.h \
	$(INCRT)/sys/dmaable.h \
	$(FRC)

tt1.o: tt1.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/emap.h \
	$(FRC)

tty.o: tty.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

emap.o: emap.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/emap.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/var.h \
	$(FRC)

stream.o: stream.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/map.h \
	$(FRC)

iosubr.o: iosubr.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/uio.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/v86.h \
	$(INCRT)/sys/fp.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/resource.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/rf_messg.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/mman.h \
	$(INCRT)/vm/vm_hat.h \
	$(INCRT)/vm/hat.h \
	$(INCRT)/vm/seg_vpix.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/as.h \
	$(INCRT)/vm/vm_hat.h \
	$(FRC)

gentty.o: gentty.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/seg.h \
	$(FRC)

ipc.o: ipc.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/istk.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/ipc.h \
	$(FRC)

mem.o: mem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(FRC)

msg.o: msg.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/istk.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/msg.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/page.h \
	$(INCRT)/sys/macro.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

osm.o: osm.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

prf.o: prf.c \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(FRC)

sad.o: sad.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/file.h  \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sad.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

sockmod.o: sockmod.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/socketvar.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/sockmod.h \
	$(INCRT)/sys/socket.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/sysmacros.h \
	$(FRC)

ticots.o: ticots.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
 	$(INCRT)/sys/mkdev.h \
	$(INCRT)/sys/ticots.h \
	$(FRC)
	$(CC) $(CFLAGS) -DTICOTS -c ticots.c

ticotsord.c:	ticots.c
	$(TCOOSED) < ticots.c > ticotsord.c

ticotsord.o: ticotsord.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
 	$(INCRT)/sys/mkdev.h \
	$(INCRT)/sys/ticotsord.h \
	$(FRC)
	$(CC) $(CFLAGS) -DTICOTSORD -c ticotsord.c

ticlts.o: ticlts.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/kmem.h \
 	$(INCRT)/sys/mkdev.h \
	$(INCRT)/sys/ticlts.h \
	$(FRC)

sem.o: sem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/istk.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/sem.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

shm.o: shm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

sxt.o: sxt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/page.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/macro.h \
	$(INCRT)/sys/istk.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/sxt.h \
	$(INCRT)/sys/inline.h \
	$(FRC)
	$(CC) -D_STYPES -I$(INCRT) $(CFLAGS) -c sxt.c

nsxt.o: nsxt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/nsxt.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

xt.o: xt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/jioctl.h \
	$(INCRT)/sys/xtproto.h \
	$(INCRT)/sys/xt.h \
	$(INCRT)/sys/inline.h \
	$(FRC)
	$(CC) -D_STYPES -I$(INCRT) $(CFLAGS) -c xt.c

nxt.o:	nxt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/jioctl.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/nxtproto.h \
	$(INCRT)/sys/nxt.h \
	$(INCRT)/sys/eucioctl.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

ctl87.o: ctl87.c \
	$(FRC)

weitek.o: weitek.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/trap.h \
	$(INCRT)/sys/seg.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/weitek.h \
	$(FRC)

wemulate.o: wemulate.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/weitek.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/fp.h \
	$(FRC)

xque.o:	xque.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/xque.h \
	$(FRC)

hrtimers.o: hrtimers.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/callo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tss.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/evecb.h \
	$(INCRT)/sys/hrtcntl.h \
	$(INCRT)/sys/dl.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/priocntl.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/events.h \
	$(INCRT)/sys/evsys.h \
	$(INCRT)/sys/hrtsys.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

ladd.o: ladd.s \
	$(FRC)

lsub.o: lsub.s \
	$(FRC)

lmul.o: lmul.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/dl.h \
	$(FRC)

ldivide.o: ldivide.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/dl.h \
	$(FRC)

lshiftl.o: lshiftl.s \
	$(FRC)

lsign.o: lsign.s \
	$(FRC)

connld.o: connld.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strsubr.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/fifonode.h \
	$(FRC)

kmacct.o: kmacct.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/dir.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/kmacct.h \
	$(FRC)

pipemod.o: pipemod.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/file.h \
	$(FRC)

ttcompat.o: ttcompat.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/ttcompat.h \
	$(FRC)

clone.o: clone.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/vfs.h \
	$(INCRT)/sys/vnode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cred.h \
	$(FRC)

log.o: log.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/inline.h \
	$(FRC)

timod.o: timod.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cred.h \
	$(INCRT)/sys/inline.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

tirdwr.o: tirdwr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(FRC)

pckt.o: pckt.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

ptem.o: ptem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/jioctl.h \
	$(INCRT)/sys/ptem.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

ptm.o: ptm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/ptms.h \
	$(FRC)

pts.o: pts.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/ptms.h \
	$(FRC)

ansi.o:	ansi.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/ws/tcl.h \
	$(INCRT)/sys/ansi.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)

chanmux.o:	chanmux.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/char.h \
	$(INCRT)/sys/ddi.h \
	$(FRC)

char.o:	char.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/termios.h \
	$(INCRT)/sys/strtty.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/kmem.h \
	$(INCRT)/sys/ascii.h \
	$(INCRT)/sys/vt.h \
	$(INCRT)/sys/at_ansi.h \
	$(INCRT)/sys/kd.h \
	$(INCRT)/sys/ws/chan.h \
	$(INCRT)/sys/ws/ws.h \
	$(INCRT)/sys/char.h \
	$(INCRT)/sys/ddi.h \
	$(INCRT)/sys/mouse.h \
	$(FRC)

sysmsg.o: sysmsg.c \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sysmsg.h \
	$(INCRT)/sys/cram.h \
	$(INCRT)/sys/bootinfo.h \
	$(INCRT)/sys/sysi86.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/stream.h \
	$(FRC)

raio.o: raio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/ioccom.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/dmaable.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/vm/seg.h \
	$(INCRT)/vm/faultcode.h \
	$(INCRT)/vm/page.h \
	$(FRC)
