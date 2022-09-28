/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/netstat/main.c	1.2.4.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 *
 * Copyright 1987, 1988 Lachman Associates, Incorporated (LAI) All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */


#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <nlist.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

struct nlist    nl[] = {
#define	N_IPSTAT	0
			{"ipstat"},
#define	N_TCB		1
			{"tcb"},
#define	N_TCPSTAT	2
			{"tcpstat"},
#define	N_UDB		3
			{"udb"},
#define	N_UDPSTAT	4
			{"udpstat"},
#define	N_RAWCB		5
			{"rawcb"},
#define N_IFSTAT        6
			{"ifstats"},
#define N_RTHOST        7
			{"rthost"},
#define N_RTNET         8
			{"rtnet"},
#define N_ICMPSTAT      9
			{"icmpstat"},
#define N_RTSTAT        10
			{"rtstat"},
#define N_RTHASHSIZE    11
			{"rthashsize"},
#define N_UNIX		12
			{"so_ux_list"},
			"",
};

/* internet protocols */
extern int      protopr();
extern int      tcp_stats(), udp_stats(), ip_stats(), icmp_stats();
extern int      nsprotopr();
extern int      spp_stats(), idp_stats(), nserr_stats();

struct protox {
	u_char          pr_index;	/* index into nlist of cb head */
	u_char          pr_sindex;	/* index into nlist of stat block */
	u_char          pr_wanted;	/* 1 if wanted, 0 otherwise */
	int             (*pr_cblocks) ();	/* control blocks printing
						 * routine */
	int             (*pr_stats) ();	/* statistics printing routine */
	char           *pr_name;/* well-known name */
}               protox[] = {
	{
		                N_TCB, N_TCPSTAT, 1, protopr,
		                tcp_stats, "tcp"
	}              ,
	{
		                N_UDB, N_UDPSTAT, 1, protopr,
		                udp_stats, "udp"
	}              ,
	{
				N_RAWCB, 0, 1, protopr,
				0, "rip"
	}              ,
	{
		                (u_char) -1, N_IPSTAT, 1, 0,
		                ip_stats, "ip"
	}              ,
	{
		                (u_char) -1, N_ICMPSTAT, 1, 0,
		                icmp_stats, "icmp"
	}              ,
	{
		                (u_char) -1, (u_char) -1, 0, 0,
		                0, 0
	}
};

struct protox   nsprotox[] = {
			      {(u_char) -1, (u_char) -1, 0, 0,
			       0, 0}
};

struct mpte    *Sysmap;

char           *mysystem = "/unix";
char           *kmemf = "/dev/kmem";
int             kmem;
int             Aflag;
int             aflag;
int             iflag;
int             mflag;
int             nflag;
int             rflag;
int             sflag;
int             Sflag;
int             interval;
char           *interface;
int             unit;
char            usage[] = "[ -AaimnrsS ] [-f address_family] [-I interface] [ interval ] [ system ] [ core ]";

int             af = AF_UNSPEC;

main(argc, argv)
	int             argc;
	char           *argv[];
{
	char           *cp, *name;
	register struct protoent *p;
	register struct protox *tp;

	name = argv[0];
	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		for (cp = &argv[0][1]; *cp; cp++)
			switch (*cp) {

			case 'A':
				Aflag++;
				break;

			case 'a':
				aflag++;
				break;
			
			case 'i':
				iflag++;
				break;

			case 'm':
				mflag++;
				break;

			case 'n':
				nflag++;
				break;

			case 'r':
				rflag++;
				break;

			case 's':
				sflag++;
				break;

			case 'S':
				Sflag++;
				break;

			case 'u':
				af = AF_UNIX;
				break;

			case 'f':
				argv++;
				argc--;
				if (strcmp(*argv, "ns") == 0)
					af = AF_NS;
				else if (strcmp(*argv, "inet") == 0)
					af = AF_INET;
				else if (strcmp(*argv, "unix") == 0)
					af = AF_UNIX;
				else {
					fprintf(stderr, "%s: unknown address family\n",
						*argv);
					exit(10);
				}
				break;

			case 'I':
				iflag++;
				if (*(interface = cp + 1) == 0) {
					if ((interface = argv[1]) == 0)
						break;
					argv++;
					argc--;
				}
				for (cp = interface; isalpha(*cp); cp++);
				unit = atoi(cp);
				*cp-- = 0;
				break;

			default:
		use:
				printf("usage: %s %s\n", name, usage);
				exit(1);
			}
		argv++, argc--;
	}
	if (argc > 0 && isdigit(argv[0][0])) {
		interval = atoi(argv[0]);
		if (interval <= 0)
			goto use;
		argv++, argc--;
		iflag++;
	}
	if (argc > 0) {
		mysystem = *argv;
		argv++, argc--;
	}
	if (getnlist(mysystem, nl) < 0) {
		fprintf(stderr, "%s: no namelist\n", mysystem);
		exit(1);
	}
	if (argc > 0) {
		kmemf = *argv;
	}
	vaddrinit(kmemf, "netstat", 0);

	if (mflag) {
#ifndef LATER
		fprintf(stderr, "Memory information not currently supported\n");
		exit(1);
#else
		mbpr(nl[N_MBSTAT].n_value);
		exit(0);
#endif
	}
        if (Sflag) {
                fprintf(stderr, "Serial line configuration not currently supported\n");
                exit(1);
        }
	/*
	 * Keep file descriptors open to avoid overhead of open/close on each
	 * call to get* routines. 
	 */
	sethostent(1);
	setnetent(1);
	if (iflag) {
		intpr(interval, nl[N_IFSTAT].n_value);
		exit(0);
	}
	if (rflag) {
		if (sflag)
			rt_stats(nl[N_RTSTAT].n_value);
		else
			routepr(nl[N_RTHOST].n_value, nl[N_RTNET].n_value,
				nl[N_RTHASHSIZE].n_value);
		exit(0);
	}
	if (af == AF_INET || af == AF_UNSPEC) {
		setprotoent(1);
		setservent(1);
		while (p = getprotoent()) {

			for (tp = protox; tp->pr_name; tp++)
				if (strcmp(tp->pr_name, p->p_name) == 0)
					break;
			if (tp->pr_name == 0 || tp->pr_wanted == 0)
				continue;
			if (sflag) {
				if (tp->pr_stats)
					(*tp->pr_stats) (nl[tp->pr_sindex].n_value,
							 p->p_name);
			} else if (tp->pr_cblocks)
				(*tp->pr_cblocks) (nl[tp->pr_index].n_value,
						   p->p_name);
		}
		endprotoent();
	}
	if (af == AF_NS || af == AF_UNSPEC) {
		for (tp = nsprotox; tp->pr_name; tp++) {
			if (sflag) {
				if (tp->pr_stats)
					(*tp->pr_stats) (nl[tp->pr_sindex].n_value,
							 tp->pr_name);
			} else if (tp->pr_cblocks)
				(*tp->pr_cblocks) (nl[tp->pr_index].n_value,
						   tp->pr_name);
		}
	}
	if ((af == AF_UNIX || af == AF_UNSPEC) && !sflag)
		unixpr(nl[N_UNIX].n_value);
	exit(0);
}

char           *
plural(n)
	int             n;
{

	return (n != 1 ? "s" : "");
}


/*
 * getnlist: 
 * read the name list and build the cache file by calling writedata.
 * thereafter, readata will return the nlist.  This is done to save
 * time.
 */

getnlist(mysystem, nl)
	char           *mysystem;
	struct nlist   *nl;
{
	if (!readata(mysystem)) {
		if (nlist(mysystem, nl) < 0)
			return -1;
		writedata();
	}
	return 0;
}

int             memfd;
#define vtop(x,y)	x

/*ARGSUSED*/
vaddrinit(mem, name, flag)
	char           *mem;
	char           *name;
{
	if ((memfd = open(mem, 0)) < 0)
		error("%s: can't open %s\n", name, mem);
}

/*ARGSUSED*/
seekmem(addr, mode, proc)
	long            addr;
	int             mode, proc;
{
	long            paddr;
	extern long     lseek();

	if (mode)
		paddr = vtop(addr, proc);
	else
		paddr = addr;
	if (paddr == -1)
		error("%x is an invalid address\n", addr);
	if (lseek(memfd, paddr, 0) == -1)
		error("seek error on address %x\n", addr);
}

/* lseek and read */
int
readmem(addr, mode, proc, buffer, size, name)
	long            addr;
	int             mode, proc;
	char           *buffer;
	unsigned        size;
	char           *name;
{
	seekmem(addr, mode, proc);
	if (read(memfd, buffer, size) != size)
		error("read error on %s\n", name);
}

error(string, arg1, arg2, arg3)
	char           *string;
	int             arg1, arg2, arg3;
{
	fprintf(stderr, string, arg1, arg2, arg3);
	exit(1);
}

/*
** hack to avoid nlist'ing all the time
** save data to a file, a la PS...
*/

#define NSDATA "/etc/netstat_data"

readata(kfile)
char	*kfile;
{
	int f;
	int cnt;
	struct stat sbuf1, sbuf2;

	if (stat(NSDATA, &sbuf1) < 0
	    || stat(kfile, &sbuf2) < 0
	    || sbuf1.st_mtime <= sbuf2.st_mtime
	    || sbuf1.st_mtime <= sbuf2.st_ctime)
		return(0);

	if ((f = open(NSDATA, O_RDONLY)) < 0)
		return 0;
	
	cnt = read(f, nl, sizeof(nl));

	if (cnt != sizeof(nl)) {
		(void) close(f);
		(void) unlink(NSDATA);
		return 0;
	}

	(void) close(f);
	return 1;
}

writedata()
{

	int f;
	int cnt;

	umask(02);
	unlink(NSDATA);
	if ((f = open(NSDATA, O_WRONLY | O_CREAT | O_EXCL , 0664)) < 0)
		return 0;
	
	cnt = write(f, nl, sizeof(nl));

	if (cnt != sizeof(nl)) {
		(void) close(f);
		(void) unlink(NSDATA);
		return 0;
	}

	(void) close(f);
	return 1;
}
