/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/bootpd/bp_subr.c	1.2.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *	PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 *     Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 *
 *  (c) 1986,1987,1988.1989  Sun Microsystems, Inc
 *  (c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *            All rights reserved.
 *
 */
/*
 * Subroutines that implement the bootparam services.
 */

#include "bootparam.h"
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LINESIZE	1024	

int debug = 0;

/*
 * Whoami turns a client address into a client name
*/
bp_whoami_res *
bootparamproc_whoami_1(argp)
	bp_whoami_arg *argp;
{
	static bp_whoami_res res;
	struct in_addr clnt_addr;
	struct hostent *hp;
	static char clnt_entry[LINESIZE];
	static char domain[32];

	if (argp->client_address.address_type != IP_ADDR_TYPE) {
		if (debug) {
			fprintf(stderr,
			   "Whoami failed: unknown address type %d\n",
				argp->client_address.address_type);
		}
		return (NULL);
	}
	memcpy((char *) &clnt_addr,
		(char *) &argp->client_address.bp_address.ip_addr,
	        sizeof (clnt_addr));
	hp = gethostbyaddr(&clnt_addr, sizeof clnt_addr, AF_INET);
	if (hp == NULL) {
		if (debug) {
			fprintf(stderr,
				"Whoami failed: gethostbyaddr for %s.\n",
				inet_ntoa (clnt_addr));
		}
		return (NULL);
	}
	if (bp_getclntent(hp->h_name, clnt_entry) != 0) {
		if (debug) {
			fprintf(stderr, "bp_getclntent failed.\n");
		}
		return (NULL);
	}
	res.client_name = hp->h_name;
	getdomainname(domain, sizeof domain);
	res.domain_name = domain;
	res.router_address.address_type = IP_ADDR_TYPE;
	memset((char *) &res.router_address.bp_address.ip_addr, 0,
	       sizeof (res.router_address.bp_address.ip_addr));

	return (&res);
}

/*
 * Getfile gets the client name and the key and returns its server
 * and the pathname for that key.
 */
bp_getfile_res *
bootparamproc_getfile_1(argp)
	bp_getfile_arg *argp;
{
	static bp_getfile_res res;
	static char clnt_entry[LINESIZE];
	struct hostent *hp;
	char *cp;

	if (bp_getclntkey(argp->client_name, argp->file_id, clnt_entry) != 0) {
		if (debug) {
			fprintf(stderr, "bp_getclntkey failed.\n");
		}
		return (NULL);
	}
	if ((cp = strchr(clnt_entry, ':')) == 0) {
		return (NULL);
	}
	*cp++ = '\0';
	res.server_name = clnt_entry;
	res.server_path = cp;
	if (*res.server_name == 0) {
		res.server_address.address_type = IP_ADDR_TYPE;
		memset(&res.server_address.bp_address.ip_addr, 0,
		      sizeof(res.server_address.bp_address.ip_addr));
	} else {
		if ((hp = gethostbyname(res.server_name)) == NULL) {
			if (debug) {
				fprintf(stderr,
				   "getfile_1: gethostbyname(%s) failed\n",
					res.server_name);
			}
			return (NULL);
		}
		res.server_address.address_type = IP_ADDR_TYPE;
		memcpy((char *) &res.server_address.bp_address.ip_addr,
			(char *) hp->h_addr,
		       sizeof (res.server_address.bp_address.ip_addr));
	}
	if (debug) {
		getf_printres(&res);
	}
	return (&res);
}

getf_printres(res)
	bp_getfile_res *res;
{
	fprintf(stderr, "getfile_1: file is \"%s\" %s \"%s\"\n",
		res->server_name,
		inet_ntoa (res->server_address.bp_address.ip_addr),
		res->server_path);
}
