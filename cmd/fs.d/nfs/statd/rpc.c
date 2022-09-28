/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/statd/rpc.c	1.2.4.1"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */
/*
 * this file consists of routines to support call_rpc();
 * client handles are cached in a hash table;
 * clntudp_create is only called if (site, prog#, vers#) cannot
 * be found in the hash table;
 * a cached entry is destroyed, when remote site crashes
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <string.h>
#include <sys/param.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

extern char *calloc();
extern char *malloc();
extern int t_errno, errno;

static struct rpc_call_private {
	int	valid;			/* Is this entry valid ? */
	CLIENT	*client;		/* Client handle */
	u_long	prognum, versnum;	/* Program, version */
	char	host[MAXHOSTNAMELEN];	/* Servers host */
	char	nettype[32];		/* Network type */
} *rpc_call_private;


#define MAX_HASHSIZE 100

char *malloc();
char *xmalloc();
extern int debug;
int HASH_SIZE = MAX_HASHSIZE;
extern void nlm_prog(), klm_prog(), priv_prog();

struct cache {
	char *host;
	int prognum;
	int versnum;
	int sock;
	CLIENT *client;
	struct cache *nxt;
};

struct cache *table[MAX_HASHSIZE];
int cache_len = sizeof (struct cache);

hash(name)
	char *name;
{
	int len;
	int i, c;

	c = 0;
	len = strlen(name);
	for (i = 0; i< len; i++) {
		c = c +(int) name[i];
	}
	c = c %HASH_SIZE;
	return (c);
}

/*
 * find_hash returns the cached entry;
 * it returns NULL if not found;
 */
struct cache *
find_hash(host, prognum, versnum)
	char *host;
	int prognum, versnum;
{
	struct cache *cp;

	if (debug)
		printf("enter find_hash() ...\n");

	cp = table[hash(host)];
	while ( cp != (struct cache *)NULL) {
		if (strcmp(cp->host, host) == 0 &&
		 cp->prognum == prognum && cp->versnum == versnum) {
			/* found */
			return (cp);
		}
		cp = cp->nxt;
	}
	return (NULL);
}

struct cache *
add_hash(host, prognum, versnum)
	char *host;
	int prognum, versnum;
{
	struct cache *cp;
	int h;

	if (debug)
		printf("enter add_hash() ...\n");

	if ((cp = (struct cache *) xmalloc(cache_len)) == (struct cache *)NULL ) {
		return (NULL);	/* malloc error */
	}
	if ((cp->host = xmalloc(strlen(host)+1)) == (char *)NULL ) {
		if (cp != NULL) free(cp);
		return (NULL);	/* malloc error */
	}
	(void) strcpy(cp->host, host);
	cp->prognum = prognum;
	cp->versnum = versnum;
	h = hash(host);
	cp->nxt = table[h];
	table[h] = cp;
	return (cp);
}

void
delete_hash(host)
	char *host;
{
	struct cache *cp;
	struct cache *cp_prev = (struct cache *)NULL;
	struct cache *next;
	int h;

	if (debug)
		printf("enter delete_hash() ...\n");

	/*
	 * if there is more than one entry with same host name;
	 * delete has to be recurrsively called
	 */

	h = hash(host);
	next = table[h];
	while ((cp = next) != (struct cache *)NULL) {
		next = cp->nxt;
		if (strcmp(cp->host, host) == 0) {
			if (cp_prev == (struct cache *)NULL) {
				table[h] = cp->nxt;
			}
			else {
				cp_prev->nxt = cp->nxt;
			}
			if (debug)
				printf("delete hash entry (%x), %s \n", cp, host);
			if (cp->client)
				clnt_destroy(cp->client);
			if (cp->host != NULL) free(cp->host);
			if (cp != NULL) free(cp);
		}
		else {
			cp_prev = cp;
		}
	}
}


/*
 * This is the simplified interface to the client rpc layer.
 * The client handle is not destroyed here and is reused for
 * the future calls to same prog, vers, host and nettype combination.
 *
 * The total time available is 25 seconds.
 */
enum clnt_stat
rpc_call(host, prognum, versnum, procnum, inproc, in, outproc, out, nettype,
	 sec, usec)
	char *host;			/* host name */
	u_long prognum;			/* program number */
	u_long versnum;			/* version number */
	u_long procnum;			/* procedure number */
	xdrproc_t inproc, outproc;	/* in/out XDR procedures */
	char *in, *out;			/* recv/send data */
	char *nettype;			/* nettype */
	int sec;
	int usec;
{
	register struct rpc_call_private *rcp = rpc_call_private;
	enum clnt_stat clnt_stat;
	struct timeval tottimeout;

	if (rcp == (struct rpc_call_private *)NULL) {
		rcp = (struct rpc_call_private *)calloc(1, sizeof (*rcp));
		if (rcp == (struct rpc_call_private *)NULL) {
			rpc_createerr.cf_stat = RPC_SYSTEMERROR;
			rpc_createerr.cf_error.re_errno = errno;
			return (rpc_createerr.cf_stat);
		}
		rpc_call_private = rcp;
	}
	if ((nettype == NULL) || (nettype[0] == NULL))
		nettype = "netpath";
	if (!(rcp->valid && rcp->prognum == prognum
		&& rcp->versnum == versnum
		&& (!strcmp(rcp->host, host))
		&& (!strcmp(rcp->nettype, nettype)))) {
		int fd;
		struct t_info tinfo;

		rcp->valid = 0;
		if (rcp->client)
			CLNT_DESTROY(rcp->client);
		/*
		 * Using the first successful transport for that type
		 */
		rcp->client = clnt_create(host, prognum, versnum, nettype);
		if (rcp->client == (CLIENT *)NULL)
			return (rpc_createerr.cf_stat);
		(void) CLNT_CONTROL(rcp->client, CLGET_FD, &fd);
		if (t_getinfo(fd, &tinfo) != -1) {
			if (tinfo.servtype == T_CLTS) {
				struct timeval timeout;

				/*
				 * Set time outs for connectionless case
				 */
				timeout.tv_usec = 0;
				timeout.tv_sec = 5;
				(void) CLNT_CONTROL(rcp->client,
					CLSET_RETRY_TIMEOUT, &timeout);
			}
		} else {
			rpc_createerr.cf_stat = RPC_TLIERROR;
			rpc_createerr.cf_error.re_terrno = t_errno;
			return (rpc_createerr.cf_stat);
		}
		rcp->prognum = prognum;
		rcp->versnum = versnum;
		(void) strcpy(rcp->host, host);
		(void) strcpy(rcp->nettype, nettype);
		rcp->valid = 1;
	} /* else reuse old client */		
	tottimeout.tv_sec = sec;
	tottimeout.tv_usec = usec;
	clnt_stat = CLNT_CALL(rcp->client, procnum, inproc, in, outproc,
				out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		rcp->valid = 0;
	return (clnt_stat);
}
