/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libyp:yp_getalias.c	1.5.4.1"

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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "ypsym.h"
/* this is 14 less the space for temps pids and .pag more or less*/

#define YPDBDIR		"/var/yp"
#define ALIASLIST	"/var/yp/aliases"
#define issep(c) (c == ' ' || c == '\t')
#define isvar_sysv() (wasitsysv)

static int wasitsysv = TRUE;
static int first_time = TRUE;
static listofnames *list = NULL;
void sysv_exit();
extern listofnames *names();
extern void exit();
extern void free_listofnames();

/*
 * Setup alias file, check /var/yp filesystem type
 */
void *
sysvconfig()
{
	struct statvfs statbuf;
	int count=0;

        sigset(SIGCHLD,SIG_IGN);
	/*
	 * if neccesary free previous list, then read in aliaslist
	 */

	if (!first_time)
		free_listofnames(list);
	else
		first_time = FALSE;

	list = names(ALIASLIST, count);

	/*
	 *	Check if YP database directory is in a system v filesystem
	 */
/*
*	if (statvfs(YPDBDIR, &statbuf) != 0) {
*		fprintf(stderr,"Cannot stat %s\n", YPDBDIR);
*		exit(-1);
*	} else {
*		if (statbuf.f_namemax == 14)
*			wasitsysv = TRUE;
*		else
*			wasitsysv = FALSE;
*	}
*/
        sigset(SIGCHLD,(void (*)())sysvconfig);
	return;
}

/*
 * Match key to alias
 */
int
yp_getalias(key, key_alias, maxlen)
char *key;
char *key_alias;
int maxlen;
{
	listofnames *entry;
	char *longname;
	char *alias;
	char name[256];

	/* sysvconfig must be run before this routine */
/*
*	if (key == NULL || first_time)
*		return(-1);
*
*
*	if (!isvar_sysv()) {
*		fprintf(stderr,"\nNsys5:\nkey_alias=$s\nkey=%s\n", key_alias, key);
*		strcpy(key_alias, key);
*		return(0);
*	}
*/
	for (entry=list, strcpy(name, entry->name); entry; entry=entry->nextname, strcpy(name,entry->name)) {

		longname = strtok(name," \t");
		alias = strtok(NULL," \t\n");
		if (longname == NULL || alias == NULL) {
			continue;
		}
		if (strcmp(longname, key) == 0) {
			if ((int)strlen(alias) > (maxlen)) {
				strncpy(key_alias, alias, (maxlen));
				key_alias[maxlen]= '\0';
			} else {
				strcpy(key_alias, alias);
			}
			return(0);
		} 
	}
	if ((int) strlen(key) > (maxlen)) {
		/* alias not found in alias file and length of key is too long*/
		return(-1);
	} else {
		/* alias not found in alias file BUT the lenght of key is OK*/
		strcpy(key_alias, key);
		return(0);
	}
}

/*
 * Match alias to key
 */
int
yp_getkey(key_alias, key, maxlen)
char *key_alias;
char *key;
int maxlen;
{
	listofnames *entry;
	char *longname;
	char *alias;
	char name[256];
/*
*	if (key_alias == NULL || first_time) {
*		return(-1);
*	}
*
*	if (!isvar_sysv()) {
*		strcpy(key, key_alias);
*		return(0);
*	}
*/

	for (entry=list, strcpy(name, entry->name); entry; entry=entry->nextname, strcpy(name,entry->name)) {

		longname = strtok(name," \t");
		alias = strtok(NULL," \t\n");
		if (alias == NULL || longname == NULL) {
			continue;
		}
		if ((strcmp(alias, key_alias) == 0) ||
		    (strncmp(alias, key_alias, maxlen) == 0)) {
			strcpy(key, longname);
			return(0);
		} 
	}
	/* key not found */
	return(-1);
}
