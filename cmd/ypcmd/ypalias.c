/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ypcmd:ypalias.c	1.2.4.1"

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
#include <limits.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "ypsym.h"

/*
 *	Given a domain name, return its system v alias.
 *	If there is no alias name in the alias file,
 *	create one. Rule of creation is to take the 1st 
 *	NAME_MAX-4 characters and concatenate the last 4 characters.
 *	If the alias in the file is too long, trim off the end.
 */
/*
*void
*mkdomain_alias(name, result)
*char *name, *result;
*{
*	int retval;
*	char tmpbuf[MAXNAMLEN] = {NULL};
*
*	retval = yp_getalias(name, result, NAME_MAX);
*	if (retval == -1) {
*		if ((int)strlen(name) > NAME_MAX) {
*			strncpy(result, name, NAME_MAX-4);
*			strncpy(&result[NAME_MAX-4], 
*			    &name[strlen(name)-4], 4);
*			result[NAME_MAX]= '\0';
*		} else
*			strcpy(result, name);
*	} else if ((int)strlen(result) > NAME_MAX) {
*		strncpy(tmpbuf, result, NAME_MAX);
*		strcpy(result, tmpbuf);
*	}
*}
*/

/*
 *	Given a map name, return its system v alias .
 *	If there is no alias name in the alias file,
 *	create one. Rule of creation is to take the 1st 
 *	MAXALIASLEN-4 characters and concatenate the last 4 characters.
 *	If the alias in the file is too long, trim off the end.
 */
char *
mkmap_alias(name,result)
char *name, *result;
{
	int retval;
	char tmpbuf[MAXNAMLEN] = {NULL};

	retval = yp_getalias(name, result, MAXALIASLEN);

	if (retval == -1) {
		if ((int)strlen(name) > MAXALIASLEN) {
			(void)strncpy(result, name, MAXALIASLEN-4);
			(void)strncpy(&result[MAXALIASLEN-4], 
			    &name[strlen(name)-4], 4);
			result[MAXALIASLEN]= '\0';
		} else
			(void)strcpy(result, name);
	} else if ((int)strlen(result) > MAXALIASLEN) {
		(void)strncpy(tmpbuf, result, MAXALIASLEN);
		(void)strcpy(result, tmpbuf);
	} 
}

#ifdef MAIN
main(argc,argv)
char **argv;
{
	char result[MAXNAMLEN] = {NULL};


	if (strcmp(argv[1], "-d") == 0)
		mkmap_alias(argv[2], (char *)&result);
	else
		mkmap_alias(argv[1], (char *)&result);
	(void)printf("%s",result);
	return(0);
}
#endif MAIN
