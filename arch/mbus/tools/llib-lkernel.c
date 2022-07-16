/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* LINTLIBRARY */
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1987 Intel Corporation
*/

#ident	"@(#)mbus:tools/llib-lkernel.c	1.3"

/*LINTLIBRARY*/
/*PROTOLIB1*/
#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/uio.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/elog.h"
#include "sys/alttbl.h"
#include "sys/fdisk.h"
#include "sys/ivlab.h"
#include "sys/vtoc.h"
#include "sys/inline.h"
#include "sys/bbh.h"
#include "sys/cred.h"
#include "sys/kmem.h"
#include "sys/stream.h"
#include "sys/map.h"
#include "sys/ddi.h"
#undef geterror	/* XXX DDI BUG */
#undef wakeup	/* XXX DDI BUG */

/*
 *	To create a lint library:
 *		lint -c -D_KERNEL llib-lkernel.c
 *		mv llib-lkernel.ln /usr/ccs/lib
 *	To use the lint library:
 *		lint -I.. -DMB1 -n -x -lkernel i546.c ../master.d/i546/space.c
 *	Alternatively, just:
 *		lint -I.. -DMB1 -n -x llib-lkernel.c i546.c ../master.d/i546/space.c
*/
/*
 *	If a function has no return value, it is declared void, not int.
*/

/*
 *	Functions for STREAMS drivers
*/
/*ARGSUSED*/ int adjmsg(mp, len) mblk_t *mp; int len; { return 0; }
/*ARGSUSED*/ mblk_t *allocb(size, pri) int size; uint pri; { return 0; }
/*ARGSUSED*/ queue_t *backq(q) queue_t *q; { return 0; }
/*ARGSUSED*/ int bcanput(q, pri) queue_t *q; unchar pri; { return 0; }
/*ARGSUSED*/ int bufcall(siz,pri,func,arg) uint siz; void (*func)(); long arg; { return 0; }
/*ARGSUSED*/ int canput(q) queue_t *q; { return 0; }
mblk_t *copyb(bp) mblk_t *bp; { return bp; }
mblk_t *copymsg(mp) mblk_t *mp; { return mp; }
/*ARGSUSED*/ int datamsg(type) int type; { return 0; }
mblk_t *dupb(bp) mblk_t *bp; { return bp; }
mblk_t *dupmsg(mp) mblk_t *mp; { return mp; }
/*ARGSUSED*/ void enableok(q) queue_t *q; {}
/*ARGSUSED*/ mblk_t *esballoc(base, size, pri, fp) unchar *base; frtn_t *fp; { return 0; }
/*ARGSUSED*/ void flushband(q, pri, flag) queue_t *q; unchar pri; {}
/*ARGSUSED*/ void flushq(q, flag) queue_t *q; int flag; { }
/*ARGSUSED*/ void freeb(bp) mblk_t *bp; { }
/*ARGSUSED*/ void freemsg(bp) mblk_t *bp; { }
/*ARGSUSED*/ mblk_t *getq(q) queue_t *q; { return 0; }
/*ARGSUSED*/ int insq(q, emp, nmp) queue_t *q; mblk_t *emp, *nmp; { return 0; }
/*ARGSUSED*/ void linkb(mp1, mp2) mblk_t *mp1, *mp2; { }
/*ARGSUSED*/ int msgdsize(mp) mblk_t *mp; { return 0; }
/*ARGSUSED*/ void noenable(q) queue_t *q; {}
queue_t *OTHERQ(q) queue_t *q; { return q; }
/*ARGSUSED*/ int pullupmsg(mp, len) mblk_t mp; int len; { return 0; }
/*ARGSUSED*/ int putbq(q, mp) queue_t *q; mblk_t *mp; { return 0; }
/*ARGSUSED*/ int putctl(q, typ) queue_t *q; int typ; { return 0; }
/*ARGSUSED*/ int putctl1(q, typ, b) queue_t *q; int typ; int b; { return 0; }
/*ARGSUSED*/ int putnext(q, mp) queue_t *q; mblk_t *mp; { return 0; }
/*ARGSUSED*/ int putq(q, mp) queue_t *q; mblk_t *mp; { return 0; }
/*ARGSUSED*/ void qenable(q) queue_t *q; {}
/*ARGSUSED*/ void qreply(q, mp) queue_t *q; mblk_t *mp; {}
/*ARGSUSED*/ int qsize(q) queue_t *q; { return 0; }
queue_t *RD(q) queue_t *q; { return q; }
/*ARGSUSED*/ mblk_t *rmvb(mp, bp) mblk_t *mp, *bp; { return mp; }
/*ARGSUSED*/ void rmvq(q, mp) queue_t q; mblk_t *mp; {}
/*ARGSUSED*//* VARARGS5 */ void strlog(mid, sid, level, flags, fmt) short mid, sid; char level; ushort flags; char *fmt; {}
/*ARGSUSED*/ int strqget(q, what, pri, valp) queue_t *q; qfields_t what; unchar pri; long *valp; { return 0; }
/*ARGSUSED*/ int strqset(q, what, pri, val) queue_t *q; qfields_t what; unchar pri; long val; { return 0; }
/*ARGSUSED*/ int testb(size, pri) int size, pri; { return 0; }
mblk_t *unlinkb(bp) mblk_t *bp; { return bp; }
queue_t *WR(q) queue_t *q; { return q; }

/*ARGSUSED*/ void biodone(bp) struct buf *bp; {}
/*ARGSUSED*/ int biowait(bp) struct buf *bp; { return 0; /* u_area, does spl0 */ }
/*ARGSUSED*/ void brelse(bp) struct buf *bp; {}
ulong btoc(bytes) ulong bytes; { return bytes; }
ulong btop(bytes) ulong bytes; { return bytes; }
ulong btopr(bytes) ulong bytes; { return bytes; }
/*ARGSUSED*/ void clrbuf(bp) struct buf *bp; {}
ulong ctob(bytes) ulong bytes; { return bytes; }
/*ARGSUSED*/ void delay(ticks) {}
/*ARGSUSED*/ void dma_breakup(strat, bp) void (*strat)(); struct buf *bp; {}	/* u_area */
/*ARGSUSED*/ int drv_getparm(parm, valuep) ulong parm; ulong *valuep; { return -1; }
clock_t drv_hztousec(hz) clock_t hz; { return hz; }
/*ARGSUSED*/ int drv_setparm(parm, value) ulong parm; ulong value; { return -1; }
clock_t drv_usectohz(usec) ulong usec; { return usec; }
/*ARGSUSED*/ void drv_usecwait(usec) ulong usec; {}
/*ARGSUSED*/ int emajor(dev) dev_t dev; { return 0; }	/* XXX major_t */
/*ARGSUSED*/ int eminor(dev) dev_t dev; { return 0; }	/* XXX minor_t */
/*ARGSUSED*/ void freerbuf(bp) buf_t *bp; {}
struct buf *geteblk() { return 0; }
/*ARGSUSED*/ int geterror(bp) struct buf *bp; { return 0; }
/*ARGSUSED*/ buf_t *getrbuf(flag) { return 0; }
/*ARGSUSED*/ unsigned hat_getkpfnum(addr) caddr_t addr; { return 0; }
/*ARGSUSED*/ int itoemajor(imaj) major_t imaj; { return 0; }	/* major_t */
/*ARGSUSED*/ _VOID *kmem_alloc(nbytes, slpflg) size_t nbytes; int slpflg; { return 0; }
/*ARGSUSED*/ void kmem_free(cp, nbytes) caddr_t cp; size_t nbytes; {}
/*ARGSUSED*/ _VOID *kmem_zalloc(nbytes, slpflg) size_t nbytes; int slpflg; { return 0; }
/*ARGSUSED*/ paddr_t kvtophys(caddr) caddr_t caddr; { return 0; }
/*ARGSUSED*/ int major(dev) dev_t dev; { return 0; }
/*ARGSUSED*/ dev_t makedev(majno, minno) int majno, minno; { return 0; }
/*ARGSUSED*/ int max(i, j) int i, j; { return i; }
/*ARGSUSED*/ int min(i, j) int i, j; { return i; }
/*ARGSUSED*/ int minor(dev) dev_t dev; { return 0; }
/*ARGSUSED*/ struct page *page_numtopp(pfn) unsigned pfn; { return 0; }
/*ARGSUSED*/ unsigned page_pptonum(pp) struct page *pp; { return 0; }
/*ARGSUSED*/ int physiock(strat,bp,dev,rw,nblks,uiop) void (*strat)(); struct buf *bp; dev_t dev; daddr_t nblks; struct uio *uiop; { return 0; }
ulong ptob(pages) ulong pages; { return pages; }
/*ARGSUSED*/ u_long rmalloc(mp, size) struct map *mp; size_t size; { return 0; }
/*ARGSUSED*/ void rmfree(mp, size, index) struct map *mp; size_t size;u_long index; {}
/*ARGSUSED*/ void rminit(mp, size) struct map *mp; int size; {}
/*ARGSUSED*/ void rmsetwant(mp) struct map *mp; {}
/*ARGSUSED*/ ulong rmwant(mp) struct map *mp; { return 0; }	/* XXX really? */
/*ARGSUSED*/ void signal(pgrp, sig) int pgrp, sig; {}
/*ARGSUSED*/ int sleep(addr, prio) caddr_t addr; int prio; { return 0; }
int spl0() { return 0; }
int spl1() { return 1; }
int spl4() { return 4; }
int spl5() { return 5; }
int spl6() { return 6; }
int spl7() { return 8; }
int splbuf() { return 6; }
int splcli() { return 8; }
int splhi() { return 8; }
int splstr() { return 5; }
int spltty() { return 8; }
/*ARGSUSED*/ void splx(oldpri) int oldpri; {}
/*ARGSUSED*/ int timeout(func, arg, ticks) void (*func)(); caddr_t arg; long ticks; { return 0; }
/*ARGSUSED*/ int uiomove(addr, bytes, rw, uiop) caddr_t addr; long bytes; enum uio_rw rw; struct uio *uiop; { return -1; }
/*ARGSUSED*/ int untimeout(id) int id; { return 0; }
/*ARGSUSED*/ int ureadc(c, uiop) int c; struct uio *uiop; { return EFAULT; }
/*ARGSUSED*/ int useracc(addr, count, rw) caddr_t addr; int count, rw; { return 0; }
/*ARGSUSED*/ int uwritec(uiop) struct uio *uiop; { return -1; }
/*ARGSUSED*/ paddr_t vtop(vaddr, p) caddr_t vaddr; struct proc *p; { return 0; }
/*ARGSUSED*/ void wakeup(addr) caddr_t addr; {}

#ifdef _i386
int inb(port) ushort port; { return 1; }
int inw(port) ushort port; { return 1; }
int inl(port) ushort port; { return 1; }
void intr_disable() {}
void intr_restore() {}
void outb(port, value) ushort port; char value; {}
void outw(port, value) ushort port; short value; {}
void outl(port, value) ushort port; int value; {}
void tenmicrosec() {}
#endif

#ifdef MB2
#include "sys/mb2tran.h"
#include "sys/mb2sup.h"
#include "sys/ics.h"

/* Interconnect Space */
int ics_cpunum();

ushort mps_lhid();		/* Local Host-id */
unchar ics_myslotid();	/* Local Slot-id */
unchar pc16;		/* Is mb2-at  */

char *cpu_cfglist[];
int ics_agent_cmp(name_list, slot) char *name_list[]; int slot; { return 0; }

unsigned ics_find_rec(slot, record) unsigned slot; unchar record; { return 0xFFFF; }

unchar ics_read(slot, reg) ushort slot; ushort reg; { return 0; }
void ics_write(slot, reg, value) ushort slot; ushort reg; unchar value; {}

void ics_rdwr(cmd, addr, flag) int cmd; struct ics_rw_struct *addr; int flag; {}
void ics_rw(cmd,addr,flag) int cmd; struct ics_struct *addr; int flag; {/*OBSOLETE*/}

/* Message Space */
mps_datbuf_t *dma_buf_join(db1, db2) mps_datbuf_t *db1; mps_datbuf_t *db2; { return 0; }
mps_datbuf_t *dma_buf_breakup(dbp, count) mps_datbuf_t *dbp; long count; { return 0; }
mps_datbuf_t *mps_get_dbuf(cnt) int cnt; { return 0; }

long mps_buf_count(dbp) mps_datbuf_t *dbp; {}
long mps_free_tid(chan, tid) long chan; unchar tid; { return 0; }

mps_msgbuf_t *mps_get_msgbuf() { return 0; }

unchar mps_get_tid(chan) long chan; { return 0; }

void mps_free_dbuf(dbp) mps_datbuf_t *dbp; {}
void mps_free_msgbuf(mbp) mps_msgbuf_t *mbp; {}
void mps_get_soldata(mbp, dptr, count) mps_msgbuf_t *mbp; caddr_t dptr; int count; {}
void mps_get_unsoldata(mbp, dptr, count) mps_msgbuf_t *mbp; caddr_t dptr; int count; {}

void mps_mk_bgrant(mbp, dsocid, lid, count) mps_msgbuf_t *mbp; mb2socid_t dsocid; unchar lid; int count; {}
void mps_mk_brdcst(mbp, dpid, dptr, count) mps_msgbuf_t *mbp; ushort dpid; unchar *dptr; int count; {}
void mps_mk_sol(mbp, dsocid, tid, dptr, count) mps_msgbuf_t *mbp; mb2socid_t dsocid; unchar tid; caddr_t dptr; int count; {}
void mps_mk_solrply(mbp, dsocid, tid, dptr, count, eotflag) mps_msgbuf_t *mbp; mb2socid_t dsocid; unchar tid; caddr_t dptr; int count; unchar eotflag; {}
void mps_mk_unsol(mbp, dsocid, tid, dptr, count) mps_msgbuf_t *mbp; mb2socid_t dsocid; unchar tid; caddr_t dptr; int count; {}
void mps_mk_unsolrply(mbp, dsocid, tid, dptr, count) mps_msgbuf_t *mbp; mb2socid_t dsocid; unchar tid; caddr_t dptr; int count; {}
void mps_msg_showmsg(mbp) mps_msgbuf_t *mbp; {}

long mps_AMPcancel(chan, dsocid, tid) ulong dsocid; unchar tid; { return -1; }
long mps_AMPreceive(chan, dsocid, omsg, ibuf) long chan; ulong dsocid; mps_msgbuf_t *omsg; mps_datbuf_t *ibuf; { return 0; }
long mps_AMPreceive_frag(chan,omsg,dsocid,tid,ibuf) long chan; mps_msgbuf_t *omsg; ulong dsocid; unchar tid; mps_datbuf_t *ibuf; { return 0; }
long mps_AMPsend(chan, mbp) long chan; mps_msgbuf_t *mbp; { return 0; }
long mps_AMPsend_data(chan, omsg, obuf) long chan; mps_msgbuf_t *omsg; mps_datbuf_t *obuf; { return 0; }
long mps_AMPsend_reply(chan, omsg, obuf) long chan; mps_msgbuf_t *omsg; mps_datbuf_t *obuf; { return 0; }
long mps_AMPsend_rsvp(chan, omsg, obuf, ibuf) long chan; mps_msgbuf_t *omsg; mps_datbuf_t *obuf, *ibuf; { return 0; }

long mps_open_chan(portid, intr, priolev) ushort portid; void (*intr)(); ushort priolev; { return -1; }
long mps_close_chan(chan) long chan; { return 0; }

#endif /* MB2 */

#ifdef _3B2
unchar getvec(baddr) long baddr; { return 0; }
int hdeeqd(dev,pdsno,edtyp) dev_t dev; daddr_t pdsno; short edtyp; {return 0;}
hdelog
#endif
