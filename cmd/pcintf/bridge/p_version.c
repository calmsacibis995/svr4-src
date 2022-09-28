/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_version.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_version.c	1.6	LCC);	/* Modified: 18:18:15 8/28/89 */

/***********************************************************************

	Copyright (c) 1989 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

***********************************************************************/

#include <const.h>
#include <pci_types.h>
#include <string.h>
#include <version.h>

extern char *malloc();

#if defined(JANUS) && defined(FAST_LSEEK)
/* Fast LSEEK code must be enabled statically for Merge 386 */
unsigned short bridge_ver_flags = V_FAST_LSEEK;
#else
unsigned short bridge_ver_flags = 0;
#endif	/* JANUS && FAST_LSEEK */
char *bridge_version = NULL;
char server_version[] = SERVER_VERSION;

get_version_string(bridge_str, count, addr)
	char *bridge_str;
	struct output *addr;
{
	strcpy(addr->text, server_version);
	addr->hdr.t_cnt = sizeof server_version;
	if (count) {
		bridge_ver_flags = 0;
		if (bridge_version)
			free(bridge_version);
		if (bridge_version = malloc(count)) {
			strcpy(bridge_version, bridge_str);
			log("server_version=\"%s\"\n", server_version);
			log("bridge_version=\"%s\"\n", bridge_version);
			if (feature_exists("FAST_LSEEK", NULL))
				bridge_ver_flags |= V_FAST_LSEEK;
			if (feature_exists("ERR_FILTER", NULL))
				bridge_ver_flags |= V_ERR_FILTER;
			log("bridge_ver_flags=%#x\n", (int)bridge_ver_flags & 0xffff);
			addr->hdr.res = SUCCESS;
		}
		else
			addr->hdr.res = INSUFFICIENT_MEMORY;
	}
	else
		addr->hdr.res = DATA_INVALID;
	return;
}



#if	!defined(MIN)
#define	MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#define	MAX_VER_SIZE	32

char *feature(token)
	char *token;
{
	register char *f;
	register int n;
	register int l;

	static char version[MAX_VER_SIZE];

	if (!token)
		return NULL;
	f = bridge_version;
	l = strlen(token);
	while (f && (strncmp(f, token, (n = strcspn(f, "="))) || n != l))
		if (f = strchr(f, ','))
			f++;
	if (f) {
		f += n + 1;
		if (n = strcspn(f, ",")) {
			strncpy(version, f, MIN(n, MAX_VER_SIZE));
			version[MAX_VER_SIZE - 1] = '\0';
			return version;
		}
	}
	return f;
}



int feature_exists(token, version)
	char *token;
	char *version;
{
	char *c;

	if (!(c = feature(token)))
		return 0;	/* the feature isn't listed */
	else if (!version)
		return 1;	/* caller doesn't care what version */
	else
		return !strcmp(c, version);
}

