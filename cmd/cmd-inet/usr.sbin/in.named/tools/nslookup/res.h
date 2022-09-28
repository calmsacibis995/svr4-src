/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.named/tools/nslookup/res.h	1.3.2.1"

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


#define TRUE	1
#define FALSE	0

#ifdef SYSV
#define	rindex		strrchr
#define	index		strchr
#define	bcmp(a,b,c)	memcmp((a),(b),(c))
#define	bzero(a,b)	memset((a), 0, (b))
#define	bcopy(a,b,c)	memcpy((b),(a),(c))
#endif /* SYSV */

/*
 *  Define return statuses in addtion to the ones defined in namserv.h
 *   let SUCCESS be a synonym for NOERROR
 *
 *	TIME_OUT	- a socket connection timed out.
 *	NO_INFO		- the server didn't find any info about the host.
 *	ERROR		- one of the following types of errors:
 *			   dn_expand, res_mkquery failed
 *			   bad command line, socket operation failed, etc.
 *	NONAUTH		- the server didn't have the desired info but
 *			  returned the name(s) of some servers who should.
 *
 */

#define  SUCCESS		0
#define  TIME_OUT		-1
#define  NO_INFO 		-2
#define  ERROR 			-3
#define  NONAUTH 		-4

/*
 *  Define additional options for the resolver state structure.
 *
 *   RES_DEBUG2		more verbose debug level 
 */

#define RES_DEBUG2	0x80000000

/*
 *  Maximum length of server, host and file names.
 */

#define NAME_LEN 80


/*
 * Modified struct hostent from <netdb.h>
 *
 * "Structures returned by network data base library.  All addresses
 * are supplied in host order, and returned in network order (suitable
 * for use in system calls)."
 */

typedef struct	{
	char	*name;		/* official name of host */
	char	**domains;	/* domains it serves */
	char	**addrList;	/* list of addresses from name server */
} ServerInfo;

typedef struct	{
	char	*name;		/* official name of host */
	char	**aliases;	/* alias list */
	char	**addrList;	/* list of addresses from name server */
	int	addrType;	/* host address type */
	int	addrLen;	/* length of address */
	ServerInfo **servers;
} HostInfo;


/*
 *  SockFD is the file descriptor for sockets used to connect with
 *  the name servers. It is global so the Control-C handler can close
 *  it. Likewise for filePtr, which is used for directing listings
 *  to a file.
 */

extern int sockFD;
extern FILE *filePtr;


/*
 *  External routines:
 */

extern int   Print_query();
extern char *Print_cdname();
extern char *Print_cdname2();	/* fixed width */
extern char *Print_rr();
extern char *DecodeType();	/* descriptive version of p_type */
extern char *DecodeError();
extern char *Calloc();
extern void NsError();
extern void PrintServer();
extern void PrintHostInfo();
extern void ShowOptions();
extern void FreeHostInfoPtr();
extern FILE *OpenFile();
extern char *inet_ntoa();
extern char *res_skip();
