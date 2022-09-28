/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nserve:nsswitch.c	1.1.21.1"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/tiuser.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "nserve.h"
#include "nsaddr.h"
#include "nsdb.h"
#include "stdns.h"
#include "nslog.h"
#include <unistd.h>

#ifdef LOGGING
#define	USAGE "usage: %s [-krv][-l level] [-n tp:tpmaster[,tp:tpmaster]] [-f logfile] [ -p tp:tpaddr[,tp:tpaddr]][-t sec] tp1,tp2,...\n"
#define	OPTSTR	"krvl:f:t:n:p:"
#else
#define	USAGE "usage: %s [-kv] [-n tp:tpmaster[,tp:tpmaster]] [-t sec] [ -p tp:tpaddr[,tp:tpaddr]] tp1,tp2,...\n"
#define	OPTSTR	"kvt:n:p:"
#endif

/*
 * array of structures defining the available name servers:
 */
struct	nsvr
{
	pid_t	pid;			/* process ID */
	char	tpname[15];		/* transport provider name */
	char	*primary;		/* primary's address string */
	char	*netmaster;		/* primary's network master file */
	struct	address	pipeaddr;	/* address of pipe */
} nslist[256];

int kflg = 0;				/* kill parent */
int rflg = 0;				/* turn off recovery */
int vflg = 0;				/* turn of verification */
int death = 0;				/* dead child */
char *Myaddress = NULL;
char *level = NULL;
char *logfile = NULL;
char *polltime = NULL;
char *primaries = NULL;			/* primary addresses (by hand) */
char *netmasters = NULL;		/* network mster files (by hand) */
char *tplist;

main(argc, argv)
int argc;
char *argv[];
{
	int		c;			/* getopt() character */
	int		errflg;			/* error flag */
	int		pd;			/* port descr for pipe */
	pid_t		ppid;			/* parent's PID */
	int		ret;			/* returned size */
	int		lock_ret;		/* returned value of setnslock()  */
	int		size;			/* block size */
	char		*block = NULL;		/* input/output block */
	struct	nsvr	*nsp;			/* name server list ptr */
	int		clrnslock();		/* clear name server lock */
	int		initnss();		/* init nsvr structs */
	int		procreq();		/* process request */
	int		setnslock();		/* set name server lock */
	pid_t		startns();		/* start name server for TP */
	void		deadchild();		/* child has died */
	void		terminate();		/* called when SIGTERM */
	extern	int	optind;			/* option index */
	extern	char	*optarg;		/* option argument */
	extern	int	fclose();		/* close a sstream */
	extern	int	fflush();		/* flush a sstream */
	extern	int	getopt();		/* parse options */
	extern	int	nslisten();		/* name server listen */
	extern	int	nsread();		/* read name server */
	extern	int	nswait();		/* wait for req */
	extern	int	nswrite();		/* write to name server */
	extern	int	setlog();		/* set log level */
	extern	void	free();			/* free memory */
	extern	FILE	*fopen();		/* open a sstream */
	extern	FILE	*freopen();		/* reopen a sstream */
	extern	char	*strcpy();		/* string copy */
	
	Logstamp = getpid();
	errflg = 0;
	while((c = getopt(argc, argv, OPTSTR)) != EOF)
	{
		switch(c)
		{
			case 'n':
				netmasters = optarg;
				break;

			case 'p':
				primaries = optarg;
				break;

			case 't':
				polltime = optarg;
				break;

			case 'v':
				vflg++;
				break;

			case 'r':
				rflg++;
				break;

			case 'k':
				kflg++;
				break;

			case 'f':
				if (Logfd = fopen(optarg, "w"))
				{
					logfile = optarg;
					(void) fclose(Logfd);
					Logfd = freopen(optarg, "w", stderr);
					setbuf(Logfd, NULL);
				}
				else
					Logfd = stderr;
				break;

			case 'l':
				level = optarg;
				(void) setlog(optarg);
				break;

			case '?':
				errflg++;
				break;
		}
	}

	ppid = getppid();

	if (errflg || (optind != argc-1))
	{
		if (kflg)
			(void) kill(ppid, SIGUSR2);
		else
			(void) fprintf(stderr, USAGE, argv[0]);
		exit(2);
	}

	if (!kflg && fork())	/* place in background */
		exit(0);

	Logstamp = getpid();
	(void) sigset(SIGHUP, SIG_IGN);
	(void) sigset(SIGINT, SIG_IGN);
	(void) sigset(SIGQUIT, SIG_IGN);
	(void) sigset(SIGTERM, SIG_IGN);
	(void) sigset(SIGCLD, SIG_IGN);

	if ((lock_ret = setnslock()) != 0)
	{
		if (lock_ret == -1)
			PLOG1("Name server already running\n");
		if (kflg)
			(void) kill(ppid, SIGUSR2);
		exit(2);
	}

	if (nslisten() == -1)
	{
		PLOG1("nserve: nslisten() failed\n");
		(void) clrnslock();
		if (kflg)
			(void) kill(ppid, SIGUSR2);
		exit(1);
	}
	LOG1(L_OVER, "nserve: listener port opened\n");

	if (tplist = (char *)malloc(strlen(argv[optind]) + 1))
		strcpy (tplist, argv[optind]);
	else	LOG1(L_OVER, "nserve: malloc for tplist failed\n");
	(void) sigset(SIGTERM, terminate);
	(void) sigset(SIGCLD, deadchild);
	(void) sighold(SIGCLD);
	if (initnss(argv[optind]) == 0)
	{
		(void) clrnslock();
		if (kflg)
			(void) kill(ppid, SIGUSR2);
		exit(1);
	}


	if (kflg)
		(void) kill(ppid, SIGUSR1);

	(void) setpgrp();
	(void) fclose(stdin);
	(void) fclose(stdout);

	while(1)
	{
		ret = nswait(&pd);
		LOG3(L_OVER, "port = %d, ret = %d\n", pd, ret);
		if (ret == 1)
			continue;

		if ((size = nsread(pd, &block, 0)) < 0)
		{
			PLOG1("nsread failed\n");
			continue;
		}

		/*
		 * process dead children at this point.
		 */
		do
		{
			death = 0;
			(void) sigset(SIGCLD, deadchild);
		} while(death);
		(void) sighold(SIGCLD);

		for(nsp = nslist; *nsp->tpname; nsp++)
		{
			/* process no longer running? */
			if (!(nsp->pid)) 
				nsp->pid = startns(nsp->tpname, nsp->primary, nsp->netmaster);
		}

		/*
		 * fork a child and process the request
		 */
		sigset(SIGCLD, SIG_IGN);
		switch(fork())
		{
			case -1:
				PLOG1("could not fork()\n");
				exit(1);

			case 0:		/* child */
				Logstamp = getpid();
				(void) sigset(SIGTERM, SIG_DFL);
				if ((ret = procreq(&block, size)) == 0) 
					PLOG1("no data returned\n");
				else
					if (nswrite(pd, block, ret) < 0) 
						PLOG1("nswrite() failed to pipe\n");
				exit(0);	/* no one cares */

			default:
				break;
		}

		free(block);
		block = NULL;

#ifdef LOGGING
		fflush(Logfd);
#endif
	}

	/*NOTREACHED*/
}

int np_clean()
{}

/*
 * settp() sets the transport name part of the address in the ar records.
 */
int
settp(req, tp)
struct request *req;
char *tp;
{
	register struct	res_rec **r;
	int		alen;			/* address length */
	int		i;			/* counter */
	int		len;			/* tp name length */
	char		*p;			/* character pointer */
	extern	int	strlen();		/* string length */
	extern	char	*malloc();		/* allocate memory */
	extern	void	free();			/* free memory */
	len = strlen(tp) + 1;
	r = req->rq_ar;
	for (i = 0; i < req->rq_head->h_arcnt; i++)
	{
		alen = strlen((*r)->rr_data) + 1;
		if ((p = malloc(len + alen)) == NULL)
			return(0);

		(void) sprintf(p, "%s %s", tp, (*r)->rr_data);
		free((*r)->rr_data);
		(*r)->rr_data = p;
		r++;
	}

	return(1);
}

/*
 * merge request merges the specified request structures into a single
 * structure.
 */
struct request *
mrgreq(reqs, reqcnt)
struct	request	**reqs;				/* request structures */
int	reqcnt;					/* number of request structs */
{
	register struct	request	**rp;		/* request pointer */
	struct	request	*first;			/* first request */
	int		i;			/* index */
	int		j;			/* index */
	long		ancnt;			/* answer count */
	long		arcnt;			/* address count */
	long		nscnt;			/* nserver count */
	long		qdcnt;			/* query count */
	long		rcode;			/* return code */
	extern	void	free();			/* free allocated memory */

	first = *reqs;
	if (reqcnt == 1)
		return(first);

	/* scan headers */
	qdcnt = first->rq_head->h_qdcnt;
	ancnt = first->rq_head->h_ancnt;
	nscnt = first->rq_head->h_nscnt;
	arcnt = first->rq_head->h_arcnt;
	rcode = first->rq_head->h_rcode;
	for(i = 1, rp = &reqs[1]; i < reqcnt; i++, rp++)
	{
		if ((*rp)->rq_head->h_opcode != first->rq_head->h_opcode)
			return(first);

		/* set return code if not already set to zero */
		if (rcode)
			rcode = (*rp)->rq_head->h_rcode;

		qdcnt += (*rp)->rq_head->h_qdcnt;
		ancnt += (*rp)->rq_head->h_ancnt;
		nscnt += (*rp)->rq_head->h_nscnt;
		arcnt += (*rp)->rq_head->h_arcnt;
	}

	/* merge the request blocks */
	first->rq_head->h_rcode = rcode;

	if (rcode)
		return(first);

	if (qdcnt != first->rq_head->h_qdcnt)
	{
		struct	question **nqp;
		struct	question **qp;
		struct	question **sqp;

		/* merge QDs */
		if ((sqp = nqp = (struct question **)calloc(qdcnt, sizeof(struct question *))) == NULL)
			return(first);

		for(i = 0, rp = reqs; i < reqcnt; i++, rp++)
		{
			if ((*rp)->rq_head->h_qdcnt)
			{
				qp = (*rp)->rq_qd;
				for(j = 0; j < (*rp)->rq_head->h_qdcnt; j++)
					*nqp++ = *qp++;
				(*rp)->rq_head->h_qdcnt = 0;
			}
		}
		first->rq_head->h_qdcnt = qdcnt;
		if (first->rq_qd)
			free(first->rq_qd);
		first->rq_qd = sqp;
	}

	if (ancnt != first->rq_head->h_ancnt)
	{
		struct	res_rec **nrsp;
		struct	res_rec **rsp;
		struct	res_rec **srsp;

		/* merge QDs */
		if ((srsp = nrsp = (struct res_rec **)calloc(ancnt, sizeof(struct res_rec *))) == NULL)
			return(first);

		for(i = 0, rp = reqs; i < reqcnt; i++, rp++)
		{
			if ((*rp)->rq_head->h_ancnt)
			{
				rsp = (*rp)->rq_an;
				for(j = 0; j < (*rp)->rq_head->h_ancnt; j++)
					*nrsp++ = *rsp++;
				(*rp)->rq_head->h_ancnt = 0;
			}
		}
		first->rq_head->h_ancnt = ancnt;
		if (first->rq_an)
			free(first->rq_an);
		first->rq_an = srsp;
	}

	if (nscnt != first->rq_head->h_nscnt)
	{
		struct	res_rec **nrsp;
		struct	res_rec **rsp;
		struct	res_rec **srsp;

		/* merge QDs */
		if ((srsp = nrsp = (struct res_rec **)calloc(nscnt, sizeof(struct res_rec *))) == NULL)
			return(first);

		for(i = 0, rp = reqs; i < reqcnt; i++, rp++)
		{
			if ((*rp)->rq_head->h_nscnt)
			{
				rsp = (*rp)->rq_ns;
				for(j = 0; j < (*rp)->rq_head->h_nscnt; j++)
					*nrsp++ = *rsp++;
				(*rp)->rq_head->h_nscnt = 0;
			}
		}
		first->rq_head->h_nscnt = nscnt;
		if (first->rq_ns)
			free(first->rq_ns);
		first->rq_ns = srsp;
	}

	if (arcnt != first->rq_head->h_arcnt)
	{
		struct	res_rec **nrsp;
		struct	res_rec **rsp;
		struct	res_rec **srsp;

		/* merge QDs */
		if ((srsp = nrsp = (struct res_rec **)calloc(arcnt, sizeof(struct res_rec *))) == NULL)
			return(first);

		for(i = 0, rp = reqs; i < reqcnt; i++, rp++)
		{
			if ((*rp)->rq_head->h_arcnt)
			{
				rsp = (*rp)->rq_ar;
				for(j = 0; j < (*rp)->rq_head->h_arcnt; j++)
					*nrsp++ = *rsp++;
				(*rp)->rq_head->h_arcnt = 0;
			}
		}
		first->rq_head->h_arcnt = arcnt;
		if (first->rq_ar)
			free(first->rq_ar);
		first->rq_ar = srsp;
	}

	return(first);
}

void
freerp(req)
struct request *req;
{
	int		i;
	struct	header	*hp;
	void		freerlist();		/* free list of records */
	extern	void	free();			/* free memory */

	if (!req) {
		return;
	}
	hp = req->rq_head;

	/* remove resource records	*/
	freerlist(req->rq_an, hp->h_ancnt);
	freerlist(req->rq_ns,hp->h_nscnt);
	freerlist(req->rq_ar, hp->h_arcnt);

	if (req->rq_an) 
		free(req->rq_an);
	if (req->rq_ns) 
		free(req->rq_ns);
	if (req->rq_ar) 
		free(req->rq_ar);

	/* now do the query section	*/
	for (i=0; i < hp->h_qdcnt; i++) {
		free(req->rq_qd[i]->q_name);
		free(req->rq_qd[i]);
	}
	if (req->rq_qd) 
		free(req->rq_qd);

	if (hp->h_dname) 
		free(hp->h_dname);

	free(hp);
	free(req);
	return;
}

void
freerlist(recp, count)
struct res_rec **recp;
int count;
{
	extern	void	free();
	void	freerrec();
	int i;

	LOG1(L_OVER,"entering: freerlist\n"); 
	if (!recp)
		return;

	for (i = 0; i < count; i++) {
		freerrec(recp[i]);
	}
	free(*recp);
	free(recp);
	LOG1(L_OVER,"leaving: freerlist\n"); 
}
	
void
freerrec(rec)
struct res_rec *rec;
{
	extern	void	free();

	switch(rec->rr_type)
	{
		case RN:
			if (rec->rr_rn)
			{
				if (rec->rr_owner)
					free(rec->rr_owner);
				if (rec->rr_desc)
					free(rec->rr_desc);
				if (rec->rr_path)
					free(rec->rr_path);
				free(rec->rr_rn);
			}
			break;

		case DOM:
			if (rec->rr_drec)
				free(rec->rr_drec);
			if (rec->rr_dom)
				free(rec->rr_dom);
			break;

		default:
			if (rec->rr_data)
				free(rec->rr_data);
			break;
	}
}


/*
 * procreq() processes the specified request by sending it to each of
 * the known name servers.
 */
int
procreq(block, size)
char **block;					/* block to send */
int size;					/* size of block */
{
	int		pd;			/* port descriptor */
	int		reqcnt;			/* request count */
	int		ret;			/* return value */
	char		*rblock;		/* read block pointer */
	struct	nsvr	*nsp;			/* pointer to nsvr list */
	struct	request	*reqs[256];		/* request structures */
	struct	request	**r;			/* request array ptr */
	int		settp();		/* set transport in addr */
	void		freerp();		/* free requests */
	struct	request	*mrgreq();		/* merge requests */
	extern	int	nsconnect();		/* conect to a name server */
	extern	int	nsrclose();		/* close name server port */
	extern	int	nsread();		/* read from name server */
	extern	int	nswrite();		/* write to name server */
	extern	char	*reqtob();		/* cnv request to block */
	extern	void	free();			/* free memory */
	extern	struct	request	*btoreq();	/* cnv block to request */

	rblock = NULL;
	reqcnt = 0;
	r = reqs;
	for(nsp = nslist; *nsp->tpname; nsp++)
	{
		if (nsp->pid <= 0)
			continue;

		if ((pd = nsconnect(&(nsp->pipeaddr))) < 0)
		{
			nsp->pid = 0;
			continue;
		}

		if (nswrite(pd, *block, size) < 0)
		{
			nsp->pid = 0;
			continue;
		}

		if ((ret = nsread(pd, &rblock, 0)) < 0)
		{
			nsp->pid = 0;
			rblock = NULL;
			continue;
		}
		(void) nsrclose(pd);

		if ((*r = btoreq(rblock, ret)) == NULL)
		{
			nsp->pid = 0;
			rblock = NULL;
			continue;
		}
		free(rblock);
		rblock = NULL;

		(void) settp(*r, nsp->tpname);
		r++;
		reqcnt++;
	}

	if (!reqcnt)
		return(0);

	(void) free(*block);
	/* convert merged request back into a block */
	if ((*block = reqtob(mrgreq(reqs, reqcnt), NULL, &ret)) == NULL)
		return(0);

	/* free generated request structures */
	for (r = reqs; reqcnt; reqcnt--, r++)
		freerp(*r);

	return(ret);
}

/*
 * initnss() will read the file containing the list of transport
 * providers and will set up the nsvr structure for each.  It will
 * also attempt to start the name servers (later).
 */
int
initnss(transports)
char *transports;				/* list of transports */
{
	register char	*tp;			/* transport name */
	int		onenet;			/* a single netmaster */
	int		oneprim;		/* a single primary */
	int		plen;			/* length of pipe name */
	int		tpcnt;			/* transport count */
	int		tpnum;			/* transport arguments count */
	int		err;			/* return code from ck_prim() */
	char		*p;			/* gp char ptr */
	struct	nsvr	*nsp;			/* name server list pointer */
	pid_t		startns();		/* start name server for TP */
	char		*getfield();		/* get field info from str */
	int		ck_prim();
	void		cleanns();		/* cleanup for ns */
	extern	int	strlen();		/* string length */
	extern	char	*malloc();		/* memory allocator */
	extern	char	*strchr();		/* string index */
	extern	char	*strcpy();		/* string copy */
	extern	char	*strtok();		/* string tokenizer */
	char		path[BUFSIZ];
	struct stat	sbuf;

	/*
	 * The following code is a disgusting hack to support both
	 * the -p netspec:address and the -p address syntaxes as well
	 * as the analogous -n netspec:netmaster and -n netmaster
	 */
	oneprim = onenet = 0;

	/* check the illegal case of -p :netspec */
	if (primaries && *primaries == ':') {
		PLOG1("Illegal primary specification\n");	
		PLOG1("usage: \"-p primary_ns_addres\" when only 1 transport provider is used.\n");
		PLOG1("       \"-p netspec:tpaddress[,netspec:tpaddress...] in the case of\n");
		PLOG1("       multiple transport providers.\n");
		return(0);
	}
	if (strchr(transports, ','))
	{
		/* multiple transports */
		if (primaries && !strchr(primaries, ':'))
		{
			/* single primary (no netspec:) */
			PLOG1("Illegal primary specification\n");	
			PLOG1("nserve: \"-p primary_ns_address\" not allowed when multiple\n");
			PLOG1("        transport providers are used.\n");
			PLOG1("        Use \"-p netspec:tpaddress[,netspec:tpaddress...]\" instead.\n");
			return(0);
		}
		if (netmasters && !strchr(netmasters, ':'))
		{
			PLOG1("Illegal netmaster specification\n");
			return(0);
		}
	}
	else
	{
		/* single transport */
		if (primaries && !strchr(primaries, ':'))
			oneprim = 1;
		if (netmasters && !strchr(netmasters, ':'))
			onenet = 1;
	}

	/* check if bad netspec specified by -p */
	err = 0;
	if ((primaries) && (strchr(primaries, ':'))) 
		err = ck_prim(primaries, transports);
	if (err)
		return(0);

	tpcnt = 0;
	tpnum = 0;
	nsp = nslist;
	plen = strlen(TPNS_PIPE) - 2;
	while(tp = strtok(transports, ","))
	{
		/* set to NULL for next call to strtok() */
		transports = NULL;
		tpnum++;
		if (tpnum > 255) {
			fprintf(stderr, "warning: only 255 transport providers are allowed. All others are ignored.\n");
			break;
		}

		/* check if tp is existing */
		sprintf (path, "/dev/%s", tp);
		if (stat (path, &sbuf) < 0) {
			PLOG2("nserve: %s does not exist\n", path);
			continue;
		}
		/* try to start name server */
		if (oneprim)
			nsp->primary = primaries;
		else
			nsp->primary = getfield(tp, primaries);

		if (onenet)
			nsp->netmaster = netmasters;
		else
			nsp->netmaster = getfield(tp, netmasters);
		cleanns(tp);
		nsp->pid = startns(tp, nsp->primary, nsp->netmaster);
		if (nsp->pid) {
			LOG2(L_COMM, "nserve: name server started for %s\n", tp);
			tpcnt++;
		}
		else
			LOG2(L_COMM, "nserve: could not start name server for %s\n", tp);

		(void)strcpy(nsp->tpname, tp);
		nsp->pipeaddr.protocol = NULL;
		nsp->pipeaddr.addbuf.len = plen + strlen(tp) + 1;
		if ((nsp->pipeaddr.addbuf.buf = malloc(nsp->pipeaddr.addbuf.len)) == NULL)
			return(0);
		(void) sprintf(nsp->pipeaddr.addbuf.buf, TPNS_PIPE, tp);
		nsp++;
	}

	*nsp->tpname = '\0';
	return(tpcnt);
}

char *
getfield(tp, s)
char *tp;					/* transport name */
register char *s;				/* string */
{
	register char	*p, *q;
	int		done;
	int		len;			/* tp length */
	extern	int	strlen();		/* string length */
	extern	int	strncmp();		/* limited string compare */
	extern	char	*malloc();		/* memory allocator */
	extern	char	*strchr();		/* string index */
	extern	char	*strncpy();		/* limited string copy */

	if (!s)
		return(NULL);

	len = strlen(tp);
	done = 0;
	while((p = s) && !done)
	{
		/*
		 *	tp:tpaddr,tp:tpaddr
		 *	p  q     sp  q     s
		 */
		if ((s = strchr(p, ',')) == NULL)
		{
			done++;
			s = p + strlen(p);
		}

		if ((q = strchr(p, ':')) == NULL)
			return(NULL);

		if ((len != (q-p)) || strncmp(tp, p, len))
		{
			/* no match */
			s++;
			continue;
		}

		q++;

		if ((p = malloc(s-q+1)) == NULL)
			return(NULL);

		(void) strncpy(p, q, s-q);
		p[s-q] = '\0';
		return(p);
	}

	return(NULL);
}

int success;					/* starting TP successful? */
/*
 * startns() starts a name server for the specified transport provider.
 * This function returns pid for success, 0 for failure.
 */
pid_t
startns(tp, primaddr, netms)
char *tp;					/* transport name */
char *primaddr;					/* primary address string */
char *netms;					/* network master file */
{
	pid_t		pid;			/* child's process id */
	char		**ap;			/* argument pointer */
	char		*av[32];		/* argument vector */
	void		nsresult();		/* result from signal */

	ap = av;
	*ap++ = TPNSERVE;
	*ap++ = "-k";

	if (vflg)
		*ap++ = "-v";

	if (polltime)
	{
		*ap++ = "-t";
		*ap++ = polltime;
	}

	if (rflg)
		*ap++ = "-r";

	if (level)
	{
		*ap++ = "-l";
		*ap++ = level;
	}

	if (logfile)
	{
		*ap++ = "-f";
		*ap++ = logfile;
	}

	if (primaddr)
	{
		*ap++ = "-p";
		*ap++ = primaddr;
	}

	if (netms)
	{
		*ap++ = "-n";
		*ap++ = netms;
	}

	*ap++ = tp;
	*ap++ = NULL;

	(void) sigset(SIGUSR1, nsresult);
	(void) sigset(SIGUSR2, nsresult);
	(void) sighold(SIGUSR1);
	(void) sighold(SIGUSR2);
	success = 0;

	switch(pid = fork())
	{
		case -1:			/* could not fork */
			return(0);

		case 0:				/* child */
			/* (void) setpgrp();*/
			(void) sigset(SIGUSR1, SIG_DFL);
			(void) sigset(SIGUSR2, SIG_DFL);
			(void) sigset(SIGHUP, SIG_DFL);
			(void) sigset(SIGINT, SIG_DFL);
			(void) sigset(SIGQUIT, SIG_DFL);
			(void) sigset(SIGTERM, SIG_DFL);
			(void) sigset(SIGCLD, SIG_DFL);
			(void) execv(TPNSERVE, av);
			(void) kill(getppid(), SIGUSR2);
			exit(1);

		default:			/* parent */
			(void) sigrelse(SIGUSR1);
			(void) sigrelse(SIGUSR2);
			pause();
			if (success)
				return(pid);
			else
				return(0);
	}
	/* NOTREACHED */
}

/*
 * nsresult() awaits a signal from the child and sets the 'success' variable
 * appropriately.
 */
void nsresult(sig)
int sig;
{
	if (sig == SIGUSR1)
		success = 1;
	else
		success = 0;
}

/*
 * cleanns() will check to see if the name server for the specified 
 * transport is running.  If so, it will kill it.
 */
void
cleanns(tp)
char *tp;					/* transport name */
{
	int		fd;			/* pid file fildes */
	pid_t		pid;			/* process id */
	char		buf[32];		/* input buffer */
	char		pidfile[256];		/* pid file */

	LOG1(L_OVER,"enter cleanns\n");
	(void)sprintf(pidfile, TPNSPID, tp);
	if ((fd = open(pidfile, O_RDONLY, 0)) < 0)
		return;		/* not there */

	if (lockf(fd, F_TLOCK, 0L) == 0)
	{
		/* file present, by name server not running */
		(void) close(fd);
		(void) unlink(pidfile);
		return;
	}

	/* name serve is still running */
	if ((read(fd, buf, sizeof(buf))) < 0)
	{
		(void) close(fd);
		return;
	}
	(void) close(fd);

	if (pid = (pid_t)atol(buf))
	{
		(void) kill(pid, SIGTERM);
		LOG2(L_OVER, "cleanns sent SIGTERM to %d\n", pid);
		(void) sleep(2);
	}

	return;
}

/*
 * terminate is called when this process receives a SIGTERM.  This 
 * function will kill all the sub servers and clean up.
 */
void
terminate()
{
	int		clrnslock();
	register char	*tp;			/* transport name */
	void		cleanns();		/* cleanup for ns */
	extern	char	*strtok();		/* string tokenizer */

	LOG1(L_ALL, "terminate called\n");
	while(tp = strtok(tplist, ","))
	{
		/* set to NULL for next call to strtok() */
		tplist = NULL;
		cleanns(tp);
	}
	(void) clrnslock();
	exit(0);
}

/*
 * deadchild() is called for each child process that dies.  It wait()s
 * for the child and checks to see if this child is a name server
 * process.  If is it, that name server must be restarted.
 */
void
deadchild()
{
	pid_t		pid;			/* process id */
	struct	nsvr	*nsp;			/* name server struct ptr */
	int		stat_loc;

	death++;

	LOG1(L_OVER, "enter deadchild\n");
	if ((pid = wait(&stat_loc)) < 0) 
		return;

	stat_loc = stat_loc & 0x0200;
	for(nsp=nslist; *nsp->tpname; nsp++)
	{
		LOG3(L_OVER, "nsp->tpname=%s nsp->pid=%d\n",nsp->tpname,nsp->pid);
		if (nsp->pid == pid || nsp->pid == 0)
		{
			if (stat_loc) { /* process should not be
						restarted */
				nsp->pid = -1;
				return;
			}
			nsp->pid = 0;
			return;
		}
	}
}

/*
 * *** stolen from nserve.c ***
 * setnslock sets a lock file to make sure that two name servers
 * are not allowed to run at once.  It also writes this name server's
 * pid into the lock file so that rfstop can use it.
 */
static int	Lockfd = -1;	/* current lock fd	*/
static int
setnslock()
{
	char	buf[32];

	if ((Lockfd = open(NSPID, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR)) == -1)
	{
		(void) perror(NSPID);
		return(-2);
	}
	if (lockf(Lockfd,F_TLOCK,0L) == -1)
	{
		(void) close(Lockfd);
		if (errno==EACCES || errno==EAGAIN) 
			return(-1);	/* busy */

		(void) perror(NSPID);
		return(-2);
	}
	sprintf(buf,"%-7d",getpid());
	(void) write(Lockfd,buf,strlen(buf)+1);
	(void) close(Lockfd);
	return(0);
}

static int
clrnslock()
{
	if (Lockfd == -1)
		return(-1);

	(void) close(Lockfd);
	Lockfd = -1;
	return(0);
}

int
ck_prim(pr, tp)
char *pr;	/* the argument following -p */
char *tp;	/* what's in netspec file */
{
	char		prim[BUFSIZ];	/* the argument following -p */
	char		tps[BUFSIZ];	/* what's in netspec file */
	int		i, j;
	int		p_count;
	int		t_count;
	char		*tmp_tok;
	char		*prim_p;
	char		*tps_p;
	char		*p_tok[256];
	char		*t_tok[256];
	register char	*q;
	int		match;
	extern	char	*strcpy();		/* string copy */
	extern	char	*strtok();		/* string tokenizer */
	extern	char	*strchr();		/* string index */
	extern	int	strncmp();		/* limited string compare */

	strcpy(prim, pr);
	prim_p = prim;
	i = 0; 
	while (tmp_tok = strtok(prim_p, ",")) {
		/* check the situation of -p ...,:netspec:addr */
		if (*tmp_tok == ':') {
			PLOG1("usage: \"-p primary_ns_address\" when only 1 transport provider is used.\n");
			PLOG1("       \"-p netspec:tpaddress[,netspec:tpaddress...] in the case of\n");
			PLOG1("       multiple transport providers.\n");
			return (1);
		     }
		/* check the situation of -p netspec:(no_addr) */
		if ((q = strchr (tmp_tok, ':')) != NULL) {
			if (*++q == ',' || *q == '\0') {
				PLOG1("nserve: -p option given bad primary address\n");
				PLOG1("usage: \"-p primary_ns_address\" when only 1 transport provider is used.\n");
				PLOG1("       \"-p netspec:tpaddress[,netspec:tpaddress...] in the case of\n");
				PLOG1("       multiple transport providers.\n");
				return(1);
			}
		}
		prim_p = NULL;
		p_tok[i++] = tmp_tok;
	}
	p_count = i;

	strcpy(tps, tp);
	tps_p = tps;
	i = 0;
	while (tmp_tok = strtok(tps_p, ",")) {
		tps_p = NULL;
		t_tok[i++] = tmp_tok;
	}
	t_count = i;

	for (i=0; i < p_count; i++) {
		match = 0;
		for (j=0; j < t_count; j++) {
			if (strncmp(t_tok[j], p_tok[i], strlen(t_tok[j]))==0) {
				match++;
				break;
			}
		}
		if (!match) {
			PLOG2("nserve: unknown primary specification %s in -p\n", p_tok[i]);
			return(1);
		}
	}
	return(0);
}
