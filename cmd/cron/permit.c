/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)cron:permit.c	1.6.1.3"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include "cron.h"

struct stat globstat;
#define	exists(file)	(stat(file,&globstat) == 0)
#define ROOT	"root"

int per_errno;	/* status info from getuser */


/****************/
char
*getuser(uid)
/****************/
uid_t uid;
{
	struct passwd *nptr;

	if ((nptr=getpwuid(uid)) == NULL) {
		per_errno=1;
		return(NULL); }
	if ((strcmp(nptr->pw_shell,SHELL)!=0) &&
	    (strcmp(nptr->pw_shell,"")!=0)) {
		per_errno=2;
		/* return NULL if you want crontab and at to abort
		   when the users login shell is not /usr/bin/sh otherwise
		   return pw_name
		*/
		return(nptr->pw_name);
	}
	return(nptr->pw_name);
}


/**********************/
allowed(user,allow,deny)
/**********************/
char *user,*allow,*deny;
{
	if ( exists(allow) ) {
		if ( within(user,allow) ) return(1);
		else return(0); }
	else if ( exists(deny) ) {
		if ( within(user,deny) ) return(0);
		else return(1); }
	else if ( strcmp(user,ROOT)==0 ) return(1);
		else return(0);
}


/************************/
within(username,filename)
/************************/
char *username,*filename;
{
	char line[UNAMESIZE];
	FILE *cap;
	int i;

	if((cap = fopen(filename,"r")) == NULL)
		return(0);
	while ( fgets(line,UNAMESIZE,cap) != NULL ) {
		for ( i=0 ; line[i] != '\0' ; i++ ) {
			if ( isspace(line[i]) ) {
				line[i] = '\0';
				break; }
		}
		if ( strcmp(line,username)==0 ) {
			fclose(cap);
			return(1); }
	}
	fclose(cap);
	return(0);
}
