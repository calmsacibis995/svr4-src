/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:usermgmt/lsgrp.c	1.1.1.2"

#include <stdio.h>
#include <pwd.h>
#include <grp.h>

struct group *getgrent();
struct passwd *getpwent();
char *strcat();
void setpwent();

#define MINUSRID	100

main()
{
	struct group *gstruct;		/* ret from getgrnam */
	struct passwd *pstruct;		/* ret from getpwent */
	char prmgrps[BUFSIZ];		/* buffer for primary group members */
	char **pp;			/* ptr to supplememtary grp member */
	int added;			/* check flag for group added */
	int firsttime;			/* first time checker */
	int x;				/* throw away variable */
	

	/* Get group entry and check for user ID (greater than 99) */
	while((gstruct = getgrent()) != NULL) {
		if(gstruct->gr_gid < MINUSRID) {
			continue;
		}

		/* Clear primary group buffer */
		for (x=0; x < BUFSIZ; prmgrps[x++]='\0')
		added = 0;

		/* Buffer primary group members */
		while((pstruct = getpwent()) != NULL) {
			if (pstruct->pw_gid == gstruct->gr_gid) {
				if(added) strcat(prmgrps, ",");
				strcat(prmgrps, pstruct->pw_name);
				added = 1;
			}
		}

		/* rewind password file */
		setpwent();

		printf("Group Name:              %s\n",gstruct->gr_name);
		printf("Group ID:                %d\n",gstruct->gr_gid);
		printf("Primary Members:         %s\n",prmgrps);
    		printf("Supplementary members:   ");
		
		firsttime=1;
		for (pp = gstruct->gr_mem; *pp != (char *) NULL; pp++) {
			if (*pp != (char *) NULL && firsttime) {
    				printf("%s",*pp);
				firsttime=0;
			}
			else if (*pp != (char *) NULL && !firsttime) {
    				printf(",%s",*pp);
			}
		}	
    		printf("\n\n");
	}
}
