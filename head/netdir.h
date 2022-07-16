/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)/usr/src/head/netdir.h.sl 1.1 4.0 12/08/90 47724 AT&T-USL"

/*
 * netdir.h
 *
 * This is the include file that defines various structures and
 * constants used by the netdir routines.
 */

#ifndef _netdir_h
#define _netdir_h

struct nd_addrlist {
	int 		n_cnt;		/* number of netbufs */
	struct netbuf 	*n_addrs;	/* the netbufs */
};

struct nd_hostservlist {
	int			h_cnt;		/* number of nd_hostservs */
	struct nd_hostserv	*h_hostservs;	/* the entries */
};

struct nd_hostserv {
	char		*h_host;	/* the host name */
	char		*h_serv;	/* the service name */
};

struct nd_mergearg {
	char		*s_uaddr;	/* servers universal address */
	char		*c_uaddr;	/* clients universal address */
	char		*m_uaddr;	/* merged universal address */
};

#ifdef __STDC__		/* ANSI	*/
int netdir_options(struct netconfig *, int option, int fd, char *par);
int netdir_getbyname(struct netconfig *, struct nd_hostserv *, struct nd_addrlist **);
int netdir_getbyaddr(struct netconfig *, struct nd_hostservlist **, struct netbuf *);
void netdir_free(char *, int);
struct netbuf *uaddr2taddr(struct netconfig *, char *);
char *taddr2uaddr(struct netconfig *, struct netbuf *);
void netdir_perror(char *);
char *netdir_sperror();
struct nd_addrlist *_netdir_getbyname(struct netconfig *, struct nd_hostserv *);
struct nd_hostservlist *_netdir_getbyaddr(struct netconfig *, struct netbuf *);
struct netbuf *_uaddr2taddr(struct netconfig *, char *);
char *_taddr2uaddr(struct netconfig *, struct netbuf *);

#else

int netdir_options();
int netdir_getbyname();
int netdir_getbyaddr();
int netdir_mergeaddr();
void netdir_free();
struct netbuf *uaddr2taddr();
void netdir_perror();
char *netdir_sperror();
char *taddr2uaddr();
struct nd_addrlist *_netdir_getbyname();
struct nd_hostservlist *_netdir_getbyaddr();
char *_netdir_mergeaddr();
struct netbuf *_uaddr2taddr();
char *_taddr2uaddr();

#endif /* ANSI */

/*
 * These are all objects that can be freed by netdir_free
 */
#define ND_HOSTSERV	0
#define ND_HOSTSERVLIST	1
#define ND_ADDR		2
#define ND_ADDRLIST	3

/* 
 * These are the various errors that can be encountered while attempting
 * to translate names to addresses. Note that none of them (except maybe
 * no memory) are truely fatal unless the ntoa deamon is on its last attempt
 * to translate the name. 
 *
 * Negative errors terminate the search resolution process, positive errors
 * are treated as warnings.
 */

#define ND_BADARG	-2	/* Bad arguments passed 	*/
#define ND_NOMEM 	-1	/* No virtual memory left	*/
#define ND_OK		0	/* Translation successful	*/
#define ND_NOHOST	1	/* Hostname was not resolvable	*/
#define ND_NOSERV	2	/* Service was unknown		*/
#define ND_NOSYM	3	/* Couldn't resolve symbol	*/
#define ND_OPEN		4	/* File couldn't be opened	*/
#define ND_ACCESS	5	/* File is not accessable	*/
#define ND_UKNWN	6	/* Unknown object to be freed	*/
#define ND_NOCTRL	7       /* Unknown option passed to netdir_options */
#define ND_FAILCTRL	8       /* Option failed in netdir_options */
#define ND_SYSTEM	9       /* Other System error           */

/*
 * The following netdir_options commands can be given to the fd. These is
 * a way of providing for any transport specific action which the caller
 * may want to initiate on his transport. It is up to the trasport provider
 * to support the netdir_options he wants to support.
 */

#define ND_SET_BROADCAST      1   /* Do t_optmgmt to support broadcast*/
#define ND_SET_RESERVEDPORT   2   /* bind it to reserve address */
#define ND_CHECK_RESERVEDPORT 3   /* check if address is reserved */
#define ND_MERGEADDR 	      4   /* Merge universal address        */

/*
 *	The following special case host names are used to give the underlying
 *	transport provides a clue as to the intent of the request.
 */

#define	HOST_SELF	"\\1"
#define	HOST_ANY	"\\2"
#define	HOST_BROADCAST	"\\3"

#endif /* !_netdir_h */
