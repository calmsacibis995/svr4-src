/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rfsetup:rfsetup.c	1.8.7.2"


#include "fcntl.h"
#include "string.h"
#include "stdio.h"
#include "errno.h"
#include "sys/stropts.h"
#include "nserve.h"
#include "sys/rf_cirmgr.h"
#include <sys/types.h>
#include "sys/rf_sys.h"
#include "pn.h"
#include "sys/hetero.h"
#include "sys/conf.h"
#include <sys/stat.h>

#ifdef LOGGING
#define FPRINTF	fprintf
#define FFLUSH	fflush
#else
#define FPRINTF	no_fn
#define FFLUSH	no_fn
#endif

#define DEBUG_FILE "/usr/net/servers/rfs/rfs.log"
#define MAX_DB_SIZE 100000

static	FILE	*Debugfd = stderr;
static	pid_t	Mypid;
static	int	op;
static	char	canonbuf[20];
static	int	flags = 0;

extern	int	errno;
extern	int	t_errno;
extern	ndata_t	ndata;
extern	pntab_t sw_tab[];

pnhdr_t pnhdr;

main()
{
	int fd;
	int sfd;
	char	ns_pipe[256];
	struct gdpmisc gdpmisc;
	extern char *getenv();

	fd = 0;			/* transport endpoint */
	(void) t_sync(fd);
	gdpmisc.hetero = gdpmisc.version = gdpmisc.ngroups_max = 0;
	logsetup();
#ifdef LOGGING
	logrequest();
#endif
	if (banter(fd) < 0)
		d_exit(fd);
	switch (op) {
	case RF_NS:
	/*
	 * Pass fd to name server process and exit.
	 * if pass fails, close stream and exit.
	 */
		/* select TP-dependent pipe */
		(void) sprintf(ns_pipe, TPNS_PIPE, getenv("NLSPROVIDER"));
		if ((sfd = open(ns_pipe, O_RDWR)) < 0) {
			FPRINTF(Debugfd,
			   "(%5d) cannot open ns channel, errno=%d\n",
			   Mypid, errno);
			FFLUSH(Debugfd);
			d_exit(fd);
		}
		(void) poptli(fd);
		if (ioctl(fd, I_PUSH, "tirdwr") < 0) {
			FPRINTF(Debugfd,
			   "(%5d) can't push TIRDWR, errno=%d\n",
			   Mypid,errno);
			FFLUSH(Debugfd);
			d_exit(fd);
		}
		if (ioctl(sfd, I_SENDFD, fd) < 0) {
			FPRINTF(Debugfd,
			   "(%5d) can't I_SENDFD fd to ns, errno=%d\n",
			   Mypid,errno);
			FFLUSH(Debugfd);
			d_exit(fd);
		}
		(void) exit(0);
	case RF_RF:
	/* read token of client, along with rest of negotiation data */
	/* here we would also read specific du opcode and switch (i.e. case MNT) */
		if ((gdpmisc.version =
		    negotiate(fd, NULL, SERVER, &gdpmisc.ngroups_max)) < 0 ) {
			FPRINTF(Debugfd,"(%5d) negotiations failed\n",
				Mypid);
			FFLUSH(Debugfd);
			d_exit(fd);
		}
		gdpmisc.hetero = ndata.n_hetero;
		ndata.n_token.t_id = SERVER;
		(void) poptli(fd);
		if (rfsys(RF_FWFD, fd, &ndata.n_token, &gdpmisc) <0) {
			FPRINTF(Debugfd,
			   "(%5d) can't do FWFD, errno=%d\n",Mypid,errno);
			FFLUSH(Debugfd);
			d_exit(fd);
		}
		uidmap(0, (char *)NULL, (char *)NULL, &ndata.n_netname[0], 0);
		uidmap(1, (char *)NULL, (char *)NULL, &ndata.n_netname[0], 0);
		(void) exit(0);
	default:
		FPRINTF(Debugfd,"(%5d) invalid opcode %d\n",Mypid,op);
		FFLUSH(Debugfd);
		(void) exit(1);
	} /* end switch */
}

banter(fd)
int fd;
{
	int	i;
	pnhdr_t rpnhdr;

	/*
	 * get header from client - 
	 *
	 * format of header is	4 char opcode
	 *			canon long hi version
	 *			canon long lo version
	 */

	if ((i =rf_rcv(fd, canonbuf, CANON_CLEN, &flags)) != CANON_CLEN) {
		FPRINTF(Debugfd,
			"(%5d) t_rcv opcode failed, t_errno=%d, errno=%d\n",
			Mypid,t_errno,errno);
		if (i != CANON_CLEN)
			FPRINTF(Debugfd, "(%5d) t_rcv only returned %d bytes\n",
				Mypid, i);
		FFLUSH(Debugfd);
		return(-1);
	}
	if (fcanon(CANONSTR, canonbuf, &pnhdr) == 0) {
		FPRINTF(Debugfd,"(%5d) fcanon returned 0\n",Mypid);
		FFLUSH(Debugfd);
		return(-1);
	}
	/* check on version mapping */
	if((pnhdr.pn_lo > HI_VER) || (pnhdr.pn_hi < LO_VER )) {
		FPRINTF(Debugfd,
		   "(%5d) bad version local hi=%d lo=%d, rem hi=%d,lo=%d\n",
		   Mypid,HI_VER,LO_VER,pnhdr.pn_lo,pnhdr.pn_hi);
		FFLUSH(Debugfd);
		rpnhdr.pn_hi = -1;
	} else  {
		if(pnhdr.pn_hi < HI_VER)
			rpnhdr.pn_lo = rpnhdr.pn_hi = pnhdr.pn_hi;
		else
			rpnhdr.pn_lo = rpnhdr.pn_hi = HI_VER;
	}
	op = get_opcode(&pnhdr);
#ifdef LOGGING
	logopcode();
#endif
	if (op < 0) {
		FPRINTF(Debugfd, "(%5d) invalid opcode\n",Mypid);
		FFLUSH(Debugfd);
		return(-1);
	}
	/* send back version we are talking in */
	strncpy(&rpnhdr.pn_op[0],sw_tab[RF_AK].sw_opcode,OPCODLEN);
	if ((i = tcanon(CANONSTR,&rpnhdr, canonbuf)) == 0) {
		FPRINTF(Debugfd,"(%5d) version fcanon returned 0\n",Mypid);
		FFLUSH(Debugfd);
		return(-1);
	}
	if (t_snd(fd, canonbuf, i, 0) != i) {
		FPRINTF(Debugfd,
		    "(%5d) response t_snd failed, t_errno=%d, errno=%d\n",
		    Mypid,t_errno,errno);
		FFLUSH(Debugfd);
		return(-1);
	}
	return(rpnhdr.pn_hi);
}

int
get_opcode(ptr)
pnhdr_t *ptr;
{
	int i;

	for (i = 0; i < NUMSWENT; i++)
		if (strcmp(ptr->pn_op, sw_tab[i].sw_opcode) == 0)
			break;
	if (i >= NUMSWENT)
		return(-1);
	return(sw_tab[i].sw_idx);
}

d_exit(fd)
int fd;
{
	char modname[FMNAMESZ];

	FPRINTF(Debugfd,"(%5d) exit\n",Mypid);
	FFLUSH(Debugfd);
	if (ioctl(fd, I_LOOK, modname) >= 0) {
		if (strcmp(modname, TIMOD) == 0)
			(void) t_snddis(fd);
	}
	(void) exit(2);
}

#ifdef LOGGING

int
logrequest()
{
	char	*getenv();
	char	*addr = getenv("NLSADDR");
	char	*provider = getenv("NLSPROVIDER");

	FPRINTF(Debugfd,"(%5d) Rcvd req from addr=%s, provider=%s\n",
		Mypid,(addr)?addr:"UNK",(provider)?provider:"UNK");
	FFLUSH(Debugfd);
}

int
logopcode()
{
	char	*opcode;
	char	errval[BUFSIZ];

	switch (op) {
	case RF_NS:
		opcode="RFS name service";
		break;
	case RF_RF:
		opcode="RFS remote mount";
		break;
	default:
		opcode=errval;
		sprintf(opcode,"UNKNOWN(%d)",op);
		break;
	}
	FPRINTF(Debugfd,"(%5d) request = %s\n",Mypid,opcode);
	FFLUSH(Debugfd);
}

#endif

int
logsetup()
{
	struct stat	sbuf;
	int tempfd;
	char *openflag;

	Mypid = getpid();

	(void) close(2);

#ifndef LOGGING

	(void) close(1);
	return;

#else
	openflag = "w";
	if (stat(DEBUG_FILE,&sbuf) == 0) {
		/* if file is too big, truncate it and start over */
		if (sbuf.st_size <= MAX_DB_SIZE)
			openflag = "a";
	}

	/* freopen does auto close of open stream */

	if ((Debugfd = freopen(DEBUG_FILE,openflag,stdout)) == NULL) {
		if ((Debugfd = fopen("/dev/null","w")) == NULL) {
			Debugfd = stderr;
			return;
		}
	}
	if ((tempfd = dup(1)) != 2) {
		if (tempfd >= 1)
			(void) close(tempfd);
		(void) close(2);
	}
#endif
}

int
poptli(fd)
int fd;
{
#ifdef i386
	char modname[FMNAMESZ+1];
#else
	char modname[FMNAMESZ];
#endif

	if (ioctl(fd, I_LOOK, modname) >= 0) {
		if (strcmp(modname, TIMOD) == 0) {
#ifdef i386
			if (ioctl(fd, I_POP, 0) < 0) {
#else
			if (ioctl(fd, I_POP) < 0) {
#endif
				FPRINTF(Debugfd, "(%5d) can't pop TIMOD, errno=%d\n",
					Mypid,errno);
				FFLUSH(Debugfd);
			}
		}
	}
	return(0);
}

/* VARARGS */

no_fn() {} /* dummy function for FPRINTF and FFLUSH */
