/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)librpc:netnamer.c	1.3.1.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
*	Copyright Notice
*
* Notice of copyright on this source code product does not indicate
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)netnamer.c 1.4 89/03/20 Copyr 1986 Sun Micro";
#endif

/*
 * netname utility routines convert from unix names to network names and
 * vice-versa This module is operating system dependent! What we define here
 * will work with any unix system that has adopted the sun yp domain
 * architecture.
 */
#include <sys/param.h>
#include <rpc/rpc.h>
#include <ctype.h>
#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>

static char    *OPSYS = "unix";
static char    *NETID = "netid.byname";
static char    *NETIDFILE = "/etc/netid";

#ifndef NGROUPS
#define	NGROUPS 16
#endif

/*
 * Convert network-name into unix credential
 */
netname2user(netname, uidp, gidp, gidlenp, gidlist)
	char netname[MAXNETNAMELEN + 1];
	uid_t *uidp;
	gid_t *gidp;
	int *gidlenp;
	gid_t *gidlist;
{
	char *p;
	int gidlen;
	uid_t uid;
	struct passwd *pwd;
	char val[1024];
	char *val1, *val2;
	char *domain;
	int vallen;
	int err;

	if (getnetid(netname, val)) {
		p = strtok(val, ":");
		if (p == NULL)
			return (0);
		*uidp = atol(val);
		p = strtok(NULL, "\n,");
		*gidp = atol(p);
		if (p == NULL) {
			return (0);
		}
		gidlen = 0;
		for (gidlen = 0; gidlen < NGROUPS; gidlen++) {
			p = strtok(NULL, "\n,");
			if (p == NULL)
				break;
			gidlist[gidlen] = atol(p);
		}
		*gidlenp = gidlen;

		return (1);
	}
	val1 = strchr(netname, '.');
	if (val1 == NULL)
		return (0);
	if (strncmp(netname, OPSYS, (val1-netname)))
		return (0);
	val1++;
	val2 = strchr(val1, '@');
	if (val2 == NULL)
		return (0);
	vallen = val2 - val1;
	if (vallen > (1024 - 1))
		vallen = 1024 - 1;
	(void) strncpy(val, val1, 1024);
	val[vallen] = 0;

	err = _rpc_get_default_domain(&domain);	/* change to rpc */
	if (err)
		return (0);

	if (strcmp(val2 + 1, domain))
		return (0);	/* wrong domain */

	if (sscanf(val, "%d", &uid) != 1)
		return (0);
	/* use initgroups method */
	pwd = getpwuid(uid);
	if (pwd == NULL)
		return (0);
	*uidp = pwd->pw_uid;
	*gidp = pwd->pw_gid;
	*gidlenp = getgroups(pwd->pw_name, gidlist);
	return (1);
}

/*
 * initgroups
 */
struct group *getgrent();

static
getgroups(uname, groups)
	char *uname;
	int groups[NGROUPS];
{
	gid_t ngroups = 0;
	register struct group *grp;
	register int i;
	register int j;
	int filter;

	setgrent();
	while (grp = getgrent()) {
		for (i = 0; grp->gr_mem[i]; i++)
			if (!strcmp(grp->gr_mem[i], uname)) {
				if (ngroups == NGROUPS) {
#ifdef DEBUG
					fprintf(stderr,
		"initgroups: %s is in too many groups\n", uname);
#endif
					goto toomany;
				}
				/* filter out duplicate group entries */
				filter = 0;
				for (j = 0; j < ngroups; j++)
					if (groups[j] == grp->gr_gid) {
						filter++;
						break;
					}
				if (!filter)
					groups[ngroups++] = grp->gr_gid;
			}
	}
toomany:
	endgrent();
	return (ngroups);
}

/*
 * Convert network-name to hostname
 */
netname2host(netname, hostname, hostlen)
	char netname[MAXNETNAMELEN + 1];
	char *hostname;
	int hostlen;
{
	int err;
	char valbuf[1024];
	char *val;
	char *val2;
	int vallen;
	char *domain;

	if (getnetid(netname, valbuf)) {
		val = valbuf;
		if ((*val == '0') && (val[1] == ':')) {
			(void) strncpy(hostname, val + 2, hostlen);
			return (1);
		}
	}
	val = strchr(netname, '.');
	if (val == NULL)
		return (0);
	if (strncmp(netname, OPSYS, (val - netname)))
		return (0);
	val++;
	val2 = strchr(val, '@');
	if (val2 == NULL)
		return (0);
	vallen = val2 - val;
	if (vallen > (hostlen - 1))
		vallen = hostlen - 1;
	(void) strncpy(hostname, val, vallen);
	hostname[vallen] = 0;

	err = _rpc_get_default_domain(&domain);	/* change to rpc */
	if (err)
		return (0);

	if (strcmp(val2 + 1, domain))
		return (0);	/* wrong domain */
	else
		return (1);
}

/*
 * reads the file /etc/netid looking for a + to optionally go to the yellow
 * pages
 */
int
getnetid(key, ret)
	char *key, *ret;
{
	char buf[1024];	/* big enough */
	char *res;
	char *mkey;
	char *mval;
	char *domain;
	int err;
	char *lookup;
	int len;
	FILE *fd;

	fd = fopen(NETIDFILE, "r");
	if (fd == (FILE *) 0) {
#ifdef YP
		res = "+";
		goto getnetidyp;
#else
		return (0);
#endif
	}
	for (;;) {
		if (fd == (FILE *) 0)
			return (0);	/* getnetidyp brings us here */
		res = fgets(buf, 1024, fd);
		if (res == 0) {
			fclose(fd);
			return (0);
		}
		if (res[0] == '#')
			continue;
		else if (res[0] == '+') {
#ifdef YP
	getnetidyp:
			err = yp_get_default_domain(&domain);
			if (err) {
				continue;
			}
			lookup = NULL;
			err = yp_match(domain, NETID, key,
				strlen(key), &lookup, &len);
			if (err) {
#ifdef DEBUG
				fprintf(stderr, "match failed error %d\n", err);
#endif
				continue;
			}
			lookup[len] = 0;
			strcpy(ret, lookup);
			free(lookup);
			fclose(fd);
			return (2);
#else	/* YP */
#ifdef DEBUG
			fprintf(stderr,
"Bad record in %s '+' -- yp not supported in this library copy\n",
				NETIDFILE);
#endif
			continue;
#endif	/* YP */
		} else {
			mkey = strtok(buf, "\t ");
			if (mkey == NULL) {
				fprintf(stderr,
		"Bad record in %s -- %s", NETIDFILE, buf);
				continue;
			}
			mval = strtok(NULL, " \t#\n");
			if (mval == NULL) {
				fprintf(stderr,
		"Bad record in %s val problem - %s", NETIDFILE, buf);
				continue;
			}
			if (strcmp(mkey, key) == 0) {
				strcpy(ret, mval);
				fclose(fd);
				return (1);

			}
		}
	}
}
