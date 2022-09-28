/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/netstat/inet.c	1.1.3.1"

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
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>

#include <netinet/in.h>
#include <net/route.h>
#include <netinet/in_systm.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_seq.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_debug.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <netdb.h>

struct inpcb    inpcb;
struct tcpcb    tcpcb;
extern int      kmem;
extern int      Aflag;
extern int      aflag;
extern int      nflag;

static int      first = 1;
char           *inetname();

/*
 * Print a summary of connections related to an Internet protocol.  For TCP,
 * also give state of connection. Listening processes (aflag) are suppressed
 * unless the -a (all) flag is specified. 
 */
protopr(off, name)
	off_t           off;
	char           *name;
{
	struct inpcb    cb;
	register struct inpcb *prev, *next;
	int             istcp;

	if (off == 0) {
		return;
	}
	istcp = strcmp(name, "tcp") == 0;
	readmem(off, 1, 0, &cb, sizeof(struct inpcb), "inpcb");
	inpcb = cb;
	prev = (struct inpcb *) off;
	while (inpcb.inp_next != (struct inpcb *) off) {

		next = inpcb.inp_next;
		readmem(next, 1, 0, &inpcb, sizeof(inpcb), "inpcb");
		if (inpcb.inp_prev != prev) {
			printf("corrupt control block chain\n");
			break;
		}
		if (!aflag &&
		    inet_lnaof(inpcb.inp_laddr) == INADDR_ANY) {
			prev = next;
			continue;
		}
		if (istcp) {
			readmem(inpcb.inp_ppcb, 1, 0, &tcpcb, sizeof(tcpcb),
				"tcpcb");
		}
		if (first) {
			printf("Active Internet connections");
			if (aflag)
				printf(" (including servers)");
			putchar('\n');
			if (Aflag)
				printf("%-8.8s ", "PCB");
			printf(Aflag ?
			    "%-5.5s %-6.6s %-6.6s  %-18.18s %-18.18s %s\n" :
			     "%-5.5s %-6.6s %-6.6s  %-22.22s %-22.22s %s\n",
			       "Proto", "Recv-Q", "Send-Q",
			     "Local Address", "Foreign Address", "(state)");
			first = 0;
		}
		if (Aflag)
			printf("%-8x ", next);
		printf("%-5.5s %6d %6d ", name, istcp ? tcpcb.t_iqsize : 0,
		       istcp ? tcpcb.t_qsize : 0);
		inetprint(&inpcb.inp_laddr, inpcb.inp_lport, name);
		inetprint(&inpcb.inp_faddr, inpcb.inp_fport, name);
		if (istcp) {
			if (tcpcb.t_state < 0 || tcpcb.t_state >= TCP_NSTATES)
				printf(" %d", tcpcb.t_state);
			else
				printf(" %s", tcpstates[tcpcb.t_state]);
		}
		putchar('\n');
		prev = next;
	}
}

/*
 * Dump TCP statistics structure. 
 */
tcp_stats(off, name)
	off_t           off;
	char           *name;
{
	struct tcpstat  tcpstat;

	if (off == 0) {
		return;
	}
	readmem(off, 1, 0, &tcpstat, sizeof(tcpstat), "tcpstat");
	printf("%s:\n", name);
	printf("\tconnections initiated: %d\n",tcpstat.tcps_connattempt);
	printf("\tconnections accepted: %d\n",tcpstat.tcps_accepts);
	printf("\tconnections established: %d\n",tcpstat.tcps_connects);
	printf("\tconnections dropped: %d\n",tcpstat.tcps_drops);
	printf("\tembryonic connections dropped: %d\n",tcpstat.tcps_conndrops);
	printf("\tconn. closed (includes drops): %d\n",tcpstat.tcps_closed);
	printf("\tsegs where we tried to get rtt: %d\n",tcpstat.tcps_segstimed);
	printf("\ttimes we succeeded: %d\n",tcpstat.tcps_rttupdated);
	printf("\tdelayed acks sent: %d\n",tcpstat.tcps_delack);
	printf("\tconn. dropped in rxmt timeout: %d\n",tcpstat.tcps_timeoutdrop);
	printf("\tretransmit timeouts: %d\n",tcpstat.tcps_rexmttimeo);
	printf("\tpersist timeouts: %d\n",tcpstat.tcps_persisttimeo);
	printf("\tkeepalive timeouts: %d\n",tcpstat.tcps_keeptimeo);
	printf("\tkeepalive probes sent: %d\n",tcpstat.tcps_keepprobe);
	printf("\tconnections dropped in keepalive: %d\n",tcpstat.tcps_keepdrops);
	printf("\ttotal packets sent: %d\n",tcpstat.tcps_sndtotal);
	printf("\tdata packets sent: %d\n",tcpstat.tcps_sndpack);
	printf("\tdata bytes sent: %d\n",tcpstat.tcps_sndbyte);
	printf("\tdata packets retransmitted: %d\n",tcpstat.tcps_sndrexmitpack);
	printf("\tdata bytes retransmitted: %d\n",tcpstat.tcps_sndrexmitbyte);
	printf("\tack-only packets sent: %d\n",tcpstat.tcps_sndacks);
	printf("\twindow probes sent: %d\n",tcpstat.tcps_sndprobe);
	printf("\tpackets sent with URG only: %d\n",tcpstat.tcps_sndurg);
	printf("\twindow update-only packets sent: %d\n",tcpstat.tcps_sndwinup);
	printf("\tcontrol (SYN|FIN|RST) packets sent: %d\n",tcpstat.tcps_sndctrl);
	printf("\ttotal packets received: %d\n",tcpstat.tcps_rcvtotal);
	printf("\tpackets received in sequence: %d\n",tcpstat.tcps_rcvpack);
	printf("\tbytes received in sequence: %d\n",tcpstat.tcps_rcvbyte);
	printf("\tpackets received with cksum errs: %d\n",tcpstat.tcps_rcvbadsum);
	printf("\tpackets received with bad offset: %d\n",tcpstat.tcps_rcvbadoff);
	printf("\tpackets received too short: %d\n",tcpstat.tcps_rcvshort);
	printf("\tduplicate-only packets received: %d\n",tcpstat.tcps_rcvduppack);
	printf("\tduplicate-only bytes received: %d\n",tcpstat.tcps_rcvdupbyte);
	printf("\tpackets with some duplicate data: %d\n",tcpstat.tcps_rcvpartduppack);
	printf("\tdup. bytes in part-dup. packets: %d\n",tcpstat.tcps_rcvpartdupbyte);
	printf("\tout-of-order packets received: %d\n",tcpstat.tcps_rcvoopack);
	printf("\tout-of-order bytes received: %d\n",tcpstat.tcps_rcvoobyte);
	printf("\tpackets with data after window: %d\n",tcpstat.tcps_rcvpackafterwin);
	printf("\tbytes rcvd after window: %d\n",tcpstat.tcps_rcvbyteafterwin);
	printf("\tpackets rcvd after \"close\": %d\n",tcpstat.tcps_rcvafterclose);
	printf("\trcvd window probe packets: %d\n",tcpstat.tcps_rcvwinprobe);
	printf("\trcvd duplicate acks: %d\n",tcpstat.tcps_rcvdupack);
	printf("\trcvd acks for unsent data: %d\n",tcpstat.tcps_rcvacktoomuch);
	printf("\trcvd ack packets: %d\n",tcpstat.tcps_rcvackpack);
	printf("\tbytes acked by rcvd acks: %d\n",tcpstat.tcps_rcvackbyte);
	printf("\trcvd window update packets: %d\n",tcpstat.tcps_rcvwinupd);
}

/*
 * Dump UDP statistics structure. 
 */
udp_stats(off, name)
	off_t           off;
	char           *name;
{
	struct udpstat  udpstat;

	if (off == 0) {
		return;
	}
	readmem(off, 1, 0, &udpstat, sizeof(udpstat), "udpstat");
	printf("%s:\n\t%d incomplete header%s\n", name,
	       udpstat.udps_hdrops, plural(udpstat.udps_hdrops));
	printf("\t%d bad data length field%s\n",
	       udpstat.udps_badlen, plural(udpstat.udps_badlen));
	printf("\t%d bad checksum%s\n",
	       udpstat.udps_badsum, plural(udpstat.udps_badsum));
}

/*
 * Dump IP statistics structure. 
 */
ip_stats(off, name)
	off_t           off;
	char           *name;
{
	struct ipstat   ipstat;

	if (off == 0) {
		return;
	}
	readmem(off, 1, 0, &ipstat, sizeof(ipstat), "ipstat");
	printf("%s:\n\t%d total packets received\n", name,
	       ipstat.ips_total);
	printf("\t%d bad header checksum%s\n",
	       ipstat.ips_badsum, plural(ipstat.ips_badsum));
	printf("\t%d with size smaller than minimum\n", ipstat.ips_tooshort);
	printf("\t%d with data size < data length\n", ipstat.ips_toosmall);
	printf("\t%d with header length < data size\n", ipstat.ips_badhlen);
	printf("\t%d with data length < header length\n", ipstat.ips_badlen);
	printf("\t%d fragment%s received\n",
	       ipstat.ips_fragments, plural(ipstat.ips_fragments));
	printf("\t%d fragment%s dropped (dup or out of space)\n",
	       ipstat.ips_fragdropped, plural(ipstat.ips_fragdropped));
	printf("\t%d fragment%s dropped after timeout\n",
	       ipstat.ips_fragtimeout, plural(ipstat.ips_fragtimeout));
	printf("\t%d packet%s forwarded\n",
	       ipstat.ips_forward, plural(ipstat.ips_forward));
	printf("\t%d packet%s not forwardable\n",
	       ipstat.ips_cantforward, plural(ipstat.ips_cantforward));
	printf("\t%d redirect%s sent\n",
	       ipstat.ips_redirectsent, plural(ipstat.ips_redirectsent));
}

static char    *icmpnames[] = {
			       "echo reply",
			       "#1",
			       "#2",
			       "destination unreachable",
			       "source quench",
			       "routing redirect",
			       "#6",
			       "#7",
			       "echo",
			       "#9",
			       "#10",
			       "time exceeded",
			       "parameter problem",
			       "time stamp",
			       "time stamp reply",
			       "information request",
			       "information request reply",
			       "address mask request",
			       "address mask reply",
};

/*
 * Dump ICMP statistics. 
 */
icmp_stats(off, name)
	off_t           off;
	char           *name;
{
	struct icmpstat icmpstat;
	register int    i, first;

	if (off == 0) {
		return;
	}
	readmem(off, 1, 0, &icmpstat, sizeof(icmpstat), "icmpstat");
	printf("%s:\n\t%d call%s to icmp_error\n", name,
	       icmpstat.icps_error, plural(icmpstat.icps_error));
	printf("\t%d error%s not generated 'cuz old message was icmp\n",
	       icmpstat.icps_oldicmp, plural(icmpstat.icps_oldicmp));
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_outhist[i] != 0) {
			if (first) {
				printf("\tOutput histogram:\n");
				first = 0;
			}
			printf("\t\t%s: %d\n", icmpnames[i],
			       icmpstat.icps_outhist[i]);
		}
	printf("\t%d message%s with bad code fields\n",
	       icmpstat.icps_badcode, plural(icmpstat.icps_badcode));
	printf("\t%d message%s < minimum length\n",
	       icmpstat.icps_tooshort, plural(icmpstat.icps_tooshort));
	printf("\t%d bad checksum%s\n",
	       icmpstat.icps_checksum, plural(icmpstat.icps_checksum));
	printf("\t%d message%s with bad length\n",
	       icmpstat.icps_badlen, plural(icmpstat.icps_badlen));
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_inhist[i] != 0) {
			if (first) {
				printf("\tInput histogram:\n");
				first = 0;
			}
			printf("\t\t%s: %d\n", icmpnames[i],
			       icmpstat.icps_inhist[i]);
		}
	printf("\t%d message response%s generated\n",
	       icmpstat.icps_reflect, plural(icmpstat.icps_reflect));
}

/*
 * Pretty print an Internet address (net address + port). If the nflag was
 * specified, use numbers instead of names. 
 */
inetprint(in, port, proto)
	register struct in_addr *in;
	int             port;
	char           *proto;
{
	struct servent *sp = 0;
	char            line[80], *cp, *strchr();
	int             width;

	sprintf(line, "%.*s.", (Aflag && !nflag) ? 12 : 16, inetname(*in));
	cp = strchr(line, '\0');
	if (!nflag && port)
		sp = getservbyport(port, proto);
	if (sp || port == 0)
		sprintf(cp, "%.8s", sp ? sp->s_name : "*");
	else
		sprintf(cp, "%d", ntohs((u_short) port));
	width = Aflag ? 18 : 22;
	printf(" %-*.*s", width, width, line);
}

/*
 * Construct an Internet address representation. If the nflag has been
 * supplied, give numeric value, otherwise try for symbolic name. 
 */
char           *
inetname(in)
	struct in_addr  in;
{
	register char  *cp;
	static char     line[50];
	struct hostent *hp;
	struct netent  *np;
	static char     domain[MAXHOSTNAMELEN + 1];
	static int      first = 1;
	extern char    *strchr();

	if (first && !nflag) {
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
		    (cp = strchr(domain, '.')))
			(void) strcpy(domain, cp + 1);
		else
			domain[0] = 0;
	}
	cp = 0;
	if (!nflag && in.s_addr != INADDR_ANY) {
		int             net = inet_netof(in);
		int             lna = inet_lnaof(in);

		if (lna == INADDR_ANY) {
			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		}
		if (cp == 0) {
			hp = gethostbyaddr(&in, sizeof(in), AF_INET);
			if (hp) {
				if ((cp = strchr(hp->h_name, '.')) &&
				    !strcmp(cp + 1, domain))
					*cp = 0;
				cp = hp->h_name;
			}
		}
	}
	if (in.s_addr == INADDR_ANY)
		strcpy(line, "*");
	else if (cp)
		strcpy(line, cp);
	else {
		in.s_addr = ntohl(in.s_addr);
#define C(x)	((x) & 0xff)
		sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
	}
	return (line);
}
