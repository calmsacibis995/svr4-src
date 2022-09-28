/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)32ld:proto.c	2.4.1.1"

/*
 *	Error detecting protocol for 68ld
 */

#include <signal.h>
#include <stdio.h>
#include "proto.h"

struct Packet{
	char	packet[MAXPKTSIZE];
	short	size;
	short	timo;
	short	state;
};

typedef struct Packet *	Pkp_t;

#define	NULL	0
#define	WAIT	1
#define	OK	2

struct Packet	packets[NPBUFS];
int		ptype;
char		xseq;
char		freepkts;
char		nulls[MAXPKTSIZE-PKTHDRSIZE];
short		xtimo;

enum{
	badack, rack, unkack, noack, xpkts, timeout, dsize, speed, nstats
};

#define	BADACK		(int)badack
#define	RACK		(int)rack
#define	UNKACK		(int)unkack
#define	NOACK		(int)noack
#define	XPKTS		(int)xpkts
#define	TIMEOUT		(int)timeout
#define	DSIZE		(int)dsize
#define	SPEED		(int)speed
#define	NSTATS		(int)nstats

struct{
	char	*desc;
	long	count;
}stats[NSTATS]={
	{"unrecognised ack"},
	{"acks received"},
	{"unknown ack"},
	{"packets retransmitted by skipped sequence ack"},
	{"packets transmitted"},
	{"packets retransmitted by timeout"},
	{"max packet data size"},
	{"bytes/sec."},
};

#define	STATS(A)	stats[A].count++

void
pinit(lspeed, maxpktdsize, aptype)
	int	lspeed;
	int	maxpktdsize;
	int	aptype;
{
	freepkts = NPBUFS;
	xtimo = (NPBUFS*(PKTHDRSIZE+PKTCRCSIZE+PKTASIZE+maxpktdsize)+lspeed-1)/lspeed+2;
	if((ptype=aptype) == ACKON)
		ptimeout();
	stats[DSIZE].count = maxpktdsize;
	stats[SPEED].count = lspeed;
}

void
precv(c)
	char		c;
{
	register int	seq;
	register int	pseq;
	register Pkp_t	pkp;
	register int	hit = 0;

	if((c&PTYP) != ptype){
		STATS(BADACK);
		return;
	}
	seq = c & SEQMASK;
	for(pkp = packets; pkp<&packets[NPBUFS]; pkp++)
		if(pkp->state == WAIT)
			if((pseq = pkp->packet[0]&SEQMASK) == seq){
				STATS(RACK);
				pkp->state = OK;
				freepkts++;
				hit++;
			}else if(pseq<=(seq-1) && pseq>(seq-NPBUFS)){
				STATS(NOACK);
				pkp->timo = xtimo;
				Write(pkp->packet, pkp->size);
				hit++;
			}
	if(!hit)
		STATS(UNKACK);
}

int
psend(bufp, count)
	char *		bufp;
	register int	count;
{
	register Pkp_t	pkp;
	register int	i;

	if(ptype == ACKON){
		for(pkp=(Pkp_t)0, freepkts=0, i=0; i<NPBUFS; i++ )
			if(packets[i].state != WAIT){
				if(pkp == (Pkp_t)0){
					pkp = &packets[i];
					pkp->state = WAIT;
					pkp->timo = xtimo;
				}
				else
					freepkts++;
			}

		if(pkp == (Pkp_t)0)
			return -1;
	}else
		pkp = packets;
	swab(bufp, &pkp->packet[PKTHDRSIZE], count);
	pkp->packet[0] = ptype | ((xseq++)&SEQMASK);
	pkp->packet[1] = count;
	count += PKTHDRSIZE;
	if(ptype != NOCRC){
		(void)crc(pkp->packet, count);
		count += PKTCRCSIZE;
	}
	pkp->size = count;
	Write(pkp->packet, count);
	STATS(XPKTS);
	return 0;
}

void
ptimeout()
{
	register Pkp_t	pkp;
	register int	retrys;

	(void)signal(SIGALRM, SIG_IGN);
	for(pkp = packets, retrys = 0; pkp<&packets[NPBUFS]; pkp++)
		if(pkp->state == WAIT /*&& --pkp->timo <= 0 && retrys++ == 0 */){
			STATS(TIMEOUT);
			pkp->timo = xtimo;
			Write(nulls, pkp->size-PKTHDRSIZE);
			Write(pkp->packet, pkp->size);
		}
	(void)signal(SIGALRM, ptimeout);
}

void
pstats(fd)
	FILE *	fd;
{
	register int	i;
	register int	count = 0;

	for(i=0; i<NSTATS; i++ )
		if(stats[i].count){
			if(count++ == 0)
				fprintf(fd, "\nStatistics:\n");
			fprintf(fd, "\t%ld %s\n", (long)stats[i].count, stats[i].desc);
		}
}
