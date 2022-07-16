#ident	"@(#)e503mac.c	1.2	92/12/21	JPB"
static char SysVr3TCPID[] = "@(#)e503mac.c	3.4 Lachman System V STREAMS TCP source";
/*
 *	System V STREAMS TCP - Release 3.0
 *
 *	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */
#include "sys/types.h"
#include "sys/param.h"		/* for KERNADDR used in page.h */
#include "sys/sysmacros.h"	/* for ctob */
#include "sys/page.h"		/* for virtual to physical translation */
#include "sys/stream.h"
#ifdef M_XENIX
#include "sys/assert.h"
#endif
#include "sys/socket.h"
#include "net/if.h"
#include "sys/e503.h"
#include "sys/log.h"
#include "sys/strlog.h"

extern struct ifstats *ifstats;

int e503hwinited;

#ifdef DYNAMIC
/* receive buffer management */

/* home rolled version of streams allocb for static allocations */
mblk_t *
myallocb(unit)
{
	register int i;
	register mblk_t *p;

	for (i=0; i < RX_MBLKS; i++) {
		p = e503device[unit].mblk[i];
		if (p->b_datap->db_ref == 1) {
			return(dupb(p));
		}
	}
	return(allocb(RX_BUFSZ, BPRI_MED));
}
#define	e503allocb(unit)	myallocb(unit)
#else
#define	e503allocb(unit)	allocb(RX_BUFSZ, BPRI_MED)
#endif

static unsigned int e503imsk[] =	{
					/* vec no 0 */ 0,
					/* vec no 1 */ 0,
					/* vec no 2 */ IRQ2,
					/* vec no 3 */ IRQ3,
					/* vec no 4 */ IRQ4,
					/* vec no 5 */ IRQ5,
					0,0,0,0,0,
					0,0,0,0,0,
					0,0,0,0,0,
					0,0,0,0,IRQ2
					};

#define intlev_to_msk(l)	(e503imsk[l])

static unsigned int e503dmask[] = {DRQ1, DRQ1, DRQ2, DRQ3};
#define dchan_to_msk(c)		(e503dmask[c])

e503io_base(unit)
register unit;
{
	return(e503iobase[unit]);
}

/* probe routine for 3c503 - it's rude and crude but works */

int
e503present(unit,e)
int unit;
unsigned char e[];
{
	register int i;
	register struct e503device *device = &e503device[unit];
	struct e503localstats *e503stats = &(device->e503stats);

	/* set the io address of the board */
	E503IOBASE = e503io_base(unit);
	/* reset ethernet controller */
	outb(CTRL, XSEL|SRST);		/* initialize gate array */
	outb(CTRL, XSEL);		/* toggle software reset bit */
	outb(CTRL, EALO|XSEL);		/* enable Ethernet address prom */

	for (i = 0; i < 6; i++) {
		e[i] = inb(EADDR+i);
	}

	if ((e[0] != 0x02) || (e[1] != 0x60) || (e[2] != 0x8c))
		return 0;

	return 1;
}

e503hwinit(unit, eaddr) 
register unit;
unchar *eaddr;
{
	register struct e503device *device = &e503device[unit];
	struct e503localstats *e503stats = &(device->e503stats);
	struct ifstats *ifs = &(device->ifstats);
	register int i;
	int	     s;

	/* set the io address of the board */

	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503hwinit");
	/* check if a valid interrupt is supplied */
	if (intlev_to_msk(e503intl[unit]) == 0) {
		printf("3COM: specified interrupt is not supported\n");
		return(NOT_OK);
	}
	E503IOBASE = e503io_base(unit);

	bzero(e503stats,sizeof(struct e503localstats));
	if (!(e503afterboot[unit]++)) {
		bzero((char *)ifs, sizeof(struct ifstats));
		ifs->ifs_name = "e3B";
		ifs->ifs_unit = unit;
		ifs->ifs_next = ifstats;
		ifs->ifs_mtu = E503ETHERMTU;
		ifstats = ifs;
	}
	ifs->ifs_active = 1;    /* flag as up */

	/* reset ethernet controller */

	outb(CTRL, XSEL|SRST);		/* initialize gate array */
	outb(CTRL, XSEL);		/* toggle software reset bit */
	outb(CTRL, EALO|XSEL);		/* enable Ethernet address prom */

	/* Grab the ethernet address */

#ifdef E503_DEBUG
	printf("3Com503 Ethernet Address = "); 
#endif
	for (i=0; i < 6; i++) {
		eaddr[i] = inb(EADDR+i);
#ifdef E503_DEBUG
		printf("%x", (unchar) eaddr[i]); 
		if (i != 5)
			printf(":"); 
#endif
	}
#ifdef E503_DEBUG
	printf("\n"); 
#endif

#ifdef DYNAMIC
	/* statically allocate input message buffers */

	for (i=0; i < RX_MBLKS; i++) {
		if (!(device->mblk[i] = allocb(RX_BUFSZ, BPRI_MED))) {
			while (--i >= 0)
				freeb(device->mblk[i]);
			printf("3COM: no streams buffers available\n");
			return(NOT_OK);
		}
	}
#endif

	if (e503xcvr[unit]) 
		outb(CTRL, 0);		/* external transceiver */
	else
		outb(CTRL, XSEL);	/* on-board transceiver */
	outb(PSTR, RX_BUFBASE);
	outb(PSPR, RX_BUFLIM);		/* 6.5K of receive space */
					/* Set interrupt level and DMA channel */
	outb(IDCFR, intlev_to_msk(e503intl[unit]));	
	outb(DQTR, 8);			/* 8 bytes/DMA transfer */
	outb(DAMSB, TX_BUFBASE);	/* 1.5K transmit buffer */
	outb(DALSB, 0); 
	s = splstr();
	e503strtnic(unit);		/* start NIC */
	e503hwinited = 1;
	splx(s);

#ifdef E503_DEBUG
	printf("3Com Ethernet Initialization Is Now Complete\n");
#endif
	return(OK);
}

e503watchdog(unit)
{
	register struct e503device *device = &e503device[unit];

#ifdef E503_DEBUG
	printf("ARF! ");
#endif
	device->tid = 0;
	device->e503stats.missintr++;
	e503restrtnic(unit,0);
	if (!(device->flags & E503BUSY))	/* should be BUSY */
		return;
	outb(TCR, 0);			/* retry current transmission */
	device->tid = timeout(e503watchdog, unit, TX_TIMEOUT);  /* chip bug */
	outb(CR, RD2|TXP|STA);		/* transmit */
}

e503restrtnic(unit, flag)
{
	register struct e503device *device = &e503device[unit];
	static int restarting;

	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "Restarting 3c503 NIC");
#ifdef E503_DEBUG
	printf("3C503 NIC restart");
#endif
	if (restarting)
		return;
	restarting = 1;
	device->e503stats.restarts++;
	outb(CR, RD2|STP);			/* stop NIC first */
/*MAF
	while (!(inb(ISR) & IRST))
		;
*/
#ifdef E503_DEBUG
	printf("ed\n");
#endif
	if (flag)
		e503hwinited = 0;		/* force full init */
	e503strtnic(unit);
	e503hwinited = 1;
	restarting = 0;
}

e503strtnic(unit)
{
	register struct e503device *device = &e503device[unit];
	unchar *eaddr = e503eaddr[unit].octet;
	int	 i;

	outb(CR, RD2|STP);		/* page 0 registers */ 
	outb(DCR, FT1|BMS);		/* 8 bytes/DMA burst */ 
	outb(RCR, AB|AM);		/* accept  broadcast and multicast */
	outb(TCR, LB0);			/* loopback mode */
	if (!e503hwinited)
		outb(BNRY, RX_BUFLIM-1);	/* CURR - 1 */
	outb(PSTART, RX_BUFBASE);	/* same as PSTR */
	outb(PSTOP, RX_BUFLIM);		/* same as PSTOP */
	outb(ISR, 0xff);		/* reset interrupt status */
	outb(IMR, E503IMASK);		/* enable interrupts */
	outb(CR, PS0|RD2|STP);		/* page 1 registers */
	for (i=0; i<E3COM_ADDR; i++)
		outb(PAR0+i, eaddr[i]);    /* set ethernet address */
	if (!e503hwinited)
		outb(CURR, RX_BUFBASE);		/* current page */
	outb(CR, RD2|STP);		/* page 0 registers */
	outb(CR, RD2|STA);		/* activate NIC */ 
	outb(TCR, 0);			/* exit loopback mode */
}

/*
 * e503intr - Ethernet interface interrupt
 */

e503intr(lev) 
register lev;
{
	register struct e503device *device;
	register struct e503localstats *e503stats;
	struct ifstats  *ifs;
	register mblk_t *mp;
	register unchar c, c2;
	register uint len;
	register struct en_minor *em;
	register e_frame *eh;
	register i;
	int	 nxtpkt, curpkt;
	int	 board = int_to_board(lev);	/* key off interrupt level */
	int x;

	device = &e503device[board];
	e503stats = &(device->e503stats);
	ifs = &(device->ifstats);

	e503stats->etint++;

	outb(IMR, 0);		/* disable interrupts */

	c = inb(ISR);
	outb(ISR, c);		/* clear status */
	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503intr: lev = %d, status = %x", lev, c);
	if (c & (IPTX|ITXE)) {
		(void)e503cktsr(board);
		e503strtout(board);
	}
	if (c & IRXE) {		/* Receive error */
		ifs->ifs_ierrors++;
		c2 = inb(RSR);
		STRLOG(ENETM_ID, 0, 9, SL_TRACE, "receive error status = %x", c2);
		if (c2 & CRCE)
			e503stats->crcerrs++;
		if (c2 & FAE)
			e503stats->fralignerrs++;
		if (c2 & FO)
			e503stats->fifoorun++;
		if (c2 & MPA)
			e503stats->misspkts++;
		if (c2 & ~(CRCE|FAE|FO|MPA)) 		/* Huh? */
			e503stats->badintr++;
	}
	if (c & ICNT) { 	/* Counter overflow */
		(void)inb(CNTR0);	 /* clear counters */
		(void)inb(CNTR1);
		(void)inb(CNTR2);
	}
	if (c & IRDC) 		/* Remote DMA complete */
		e503stats->rdmaint++;
	if (c & (IPRX|IOVW)) {	/* Packet received */
		if (c & IOVW) 
			outb(CR, RD2|STP);
		nxtpkt = NXT_RXBUF(inb(BNRY));
		while (nxtpkt != CURRXBUF(i)) {
			outb(DAMSB, nxtpkt);
			outb(DALSB, 0);
			outb(CTRL, (e503xcvr[board] ? 0 : XSEL)|START);
			c2 = inb(RFMSB);
			curpkt = nxtpkt;
			nxtpkt = inb(RFMSB);
			len = inw(RFMSB);
			STRLOG(ENETM_ID, 0, 9, SL_TRACE, 
				"packet received, s %x cn %x l %x", 
				 c2, (curpkt<<8)|nxtpkt, len);
			if (e503pktsft(curpkt, nxtpkt, len)) {
				STRLOG(ENETM_ID, 0, 9, SL_TRACE, 
					"shifted packet rcvd");
				outb(CTRL, (e503xcvr[board] ? 0 : XSEL));
				e503stats->sftpkts++;
				e503restrtnic(board, 1);
				return;
			}
			if (c2 & (DIS|DFR)) {
				STRLOG(ENETM_ID, 0, 9, SL_TRACE, 
				"packet rcvd erronously, status=%x", c2);
				outb(CTRL, (e503xcvr[board] ? 0 : XSEL));
				e503stats->badrstat++;
				continue;
			}
			len -= 4;	/* dump the CRC */
			STRLOG(ENETM_ID, 0, 9, SL_TRACE, 
				"allocating buffer size %d", len);
			mp = allocb(len, BPRI_HI);
			if (mp) {
				e503stats->rxct++;
				ifs->ifs_ipackets++;
				e503ioin(mp->b_rptr, len, RFMSB, STREG);
				outb(CTRL, (e503xcvr[board] ? 0 : XSEL));
			} else {
				ifs->ifs_ierrors++;
				e503stats->allocfails++;
				STRLOG(ENETM_ID, 0, 9, SL_TRACE, 
					"allocb length %d failed", len);
				outb(CTRL, (e503xcvr[board] ? 0 : XSEL));
				continue;
			}

			/* adjust length of write pointer */
			mp->b_wptr = mp->b_rptr + len;
			eh = (e_frame *) mp->b_rptr;
			em = &e503_em[board*n3c503min];
			for (i=0; i < n3c503min; em++, i++) {
				if (em->em_top && 
				    eh->e_type == em->em_sap &&
				    canput(em->em_top)) {
					putq(em->em_top, mp);
					break;
				}
			}
			if (i == n3c503min)
				freemsg(mp);
		}
		outb(BNRY, PRV_RXBUF(nxtpkt));	/* update read pointer */
	}
	if (c & IOVW) {		/* Overwrite warning */
		e503stats->overwr++;
		e503restrtnic(board,0);
		return;
	}
	outb(IMR, E503IMASK);
}

e503cktsr(unit)
{
	register struct e503device *device = &e503device[unit];
	register struct e503localstats *e503stats = &device->e503stats;
	register struct ifstats *ifs = &device->ifstats;
	unchar c;

	while (!((c = inb(TSR)) & (PTX|ABT|FU)))
		;

	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503cktsr: status = %x", c);
	if (c & PTX) {
		e503stats->txct++;
		ifs->ifs_opackets++;
	} else
		if (c & ABT) {
			ifs->ifs_collisions += 16;
			ifs->ifs_oerrors++;
			e503stats->txerrs++;
			e503stats->excolls++;
		} else
			if (c & FU) {
				ifs->ifs_oerrors++;
				e503stats->txerrs++;
				e503stats->fifourun++;
			}
	if (c & COL) {
		int n = inb(NCR);

		ifs->ifs_collisions += n;
		e503stats->colls += n;
	}
	if (c & CRS)
		e503stats->cslost++;
	if (c & OWC) {
		ifs->ifs_collisions++;
		e503stats->owcolls++;
	}
	if (c & CDH)
		e503stats->cdheartbt++;
	return(c);
}

e503strtout(unit)
{
	register struct e503device *device = &e503device[unit];
	register struct en_minor *em;
	queue_t *q;
	int      i;
	int	 s;

	s = splstr();
	if (device->flags & E503BUSY) {
		untimeout(device->tid);		/* call off watchdog */
		device->tid = 0;
		device->flags &= ~E503BUSY;
	}
	splx(s);
	if (!(device->flags & E503WAITO))
		return;
	device->flags &= ~E503WAITO;
	em = &e503_em[unit*n3c503min];
	for (i=0; i < n3c503min; em++, i++)  /* should be round-robin or fifo */
		if (em->em_top && (q=WR(em->em_top))->q_first) {
			qenable(q);
			return;
		}
}

e503pktsft(curpkt, nxtpkt, len)
{
	curpkt += (len>>8) + 1;
	if (curpkt >= RX_BUFLIM)
		curpkt -= RX_BUFLIM - RX_BUFBASE;
	if (nxtpkt == curpkt || nxtpkt == NXT_RXBUF(curpkt))
		return(0);
	else
		return(1);
}

/*
 * e503hwput - copy to board and send packet
 *
 */

e503hwput(unit, m)
register unit;
register mblk_t *m;
{
	register struct e503device *device = &e503device[unit];
	register struct e503localstats *e503stats = &(device->e503stats);
	register uint len = 0;
	register uint tlen;
	int s;

	e503stats->ethwput++;

	s = splstr();
	if (device->flags & E503BUSY) {
		device->flags |= E503WAITO;	/* assume caller waits */
		splx(s);
		STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503hwput deferring");
		return(NOT_OK);
	}
	outb(DAMSB, TX_BUFBASE);
	outb(DALSB, 0);
	outb(CTRL, (e503xcvr[unit] ? 0 : XSEL)|START|DDIR);
	while(m) {
		len += (tlen = m->b_wptr - m->b_rptr);
		if (len > E3COM_MAXPACK)
			break;
		e503ioout(m->b_rptr, tlen, RFMSB, STREG);
		m = m->b_cont;
	}
	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503hwput, len = %d", len);
	outb(CTRL, (e503xcvr[unit] ? 0 : XSEL));
	if (len < E3COM_MINPACK || len > E3COM_MAXPACK) {
		splx(s);
		printf("3c503 bad packet size: %d\n", len);
		return(OK);	/* continue normally */
	}
	device->flags |= E503BUSY;
	outb(TCR, 0);
	outb(TPSR, TX_BUFBASE);
	outb(TBCR0, len & 0xff);
	outb(TBCR1, len >> 8);
	device->tid = timeout(e503watchdog, unit, TX_TIMEOUT);  /* chip bug */
	outb(CR, RD2|TXP|STA);		/* transmit */
	splx(s);
	return(OK);
}

e503hwclose(unit)
int unit;
{
	register struct e503device *device = &e503device[unit];
	int i;

	STRLOG(ENETM_ID, 0, 9, SL_TRACE, "e503hwclose");
	device->ifstats.ifs_active = 0;
	e503hwinited = 0;

	/* reset ethernet controller */
	outb(CR, RD2|STP);
	outb(CTRL, XSEL|SRST);

#ifdef DYNAMIC
	/* deallocate static input message buffers */

	for (i=0; i < RX_MBLKS; i++) {
		freemsg(device->mblk[i]); 
	}
#endif

#ifdef E503_DEBUG
	e503dumpstats(unit);
#endif
}

#ifdef E503_DEBUG
e503dumpstats(unit)
{
	register struct e503localstats *e503stats = &e503device[unit].e503stats;

	printf("******** e503hwclose *******\n");
	printf("allocfails      = %d  ", e503stats->allocfails);
	printf("badintr         = %d  ", e503stats->badintr);
	printf("badrstat        = %d  \n", e503stats->badrstat);
	printf("cdheartbt       = %d  ", e503stats->cdheartbt);
	printf("colls           = %d  ", e503stats->colls);
	printf("crcerrs         = %d  \n", e503stats->crcerrs);
	printf("cslost          = %d  ", e503stats->cslost);
	printf("ethwput         = %d  ", e503stats->ethwput);
	printf("etint           = %d  \n", e503stats->etint);
	printf("excolls         = %d  ", e503stats->excolls);
	printf("fifoorun        = %d  ", e503stats->fifoorun);
	printf("fifourun        = %d  \n", e503stats->fifourun);
	printf("fralignerrs     = %d  ", e503stats->fralignerrs);
	printf("missintr        = %d  ", e503stats->missintr);
	printf("misspkts        = %d  \n", e503stats->misspkts);
	printf("owcolls         = %d  ", e503stats->owcolls);
	printf("overwr          = %d  ", e503stats->overwr);
	printf("restarts        = %d  \n", e503stats->restarts);
	printf("rdmaint         = %d  ", e503stats->rdmaint);
	printf("sftpkts         = %d  ", e503stats->sftpkts);
	printf("rxct            = %d  \n", e503stats->rxct);
	printf("txct            = %d  ", e503stats->txct);
	printf("txerrs          = %d  \n", e503stats->txerrs);
}
#endif

static 
int_to_board(lev)
{
	int i;

	for (i=0; i<n3c503unit; i++)
		if (e503intl[i] == lev)
			break;
	return(i);
}

#ifndef i386
e503ioout(p, len, port, streg)
char *p;
{
	int  l;

	while (len) {
		int i;

		while (!(inb(streg) & DPRDY))
			;
		l = (len < 8 ? len : 8);
		for (i=0; i<l; i++)
			outb(port, *p++);
		len -= l;
	}
}

e503ioin(p, len, port, streg)
char *p;
{
	int  l;

	while (len) {
		int i;

		while (!(inb(streg) & DPRDY))
			;
		l = (len < 8 ? len : 8);
		for (i=0; i<l; i++)
			*p++ = inb(port);
		len -= l;
	}
}
#endif
