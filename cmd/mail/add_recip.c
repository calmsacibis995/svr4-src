/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:add_recip.c	1.5.3.1"
/*
    NAME
	add_recip, madd_recip - add recipients to recipient list

    SYNOPSIS
	int add_recip(reciplist *plist, char *name, int checkdups)
	int madd_recip(reciplist *plist, char *name, int checkdups)

    DESCRIPTION
	add_recip() adds the name to the recipient linked list.
	If checkdups is set, it first checks to make certain that
	the name is not in the list.

	madd_recips() is given a list of names separated by white
	space. Each name is split off and passed to add_recips.
*/

#include "mail.h"

add_recip (plist, name, checkdups)
reciplist	*plist;
char		*name;
int		checkdups;
{
	char		*p;
	static char	pn[] = "add_recip";
	recip		*r = &plist->recip_list;

	if ((name == (char *)NULL) || (*name == '\0')) {
		Tout(pn, "translation to NULL name ignored\n");
		return(0);
	}

	p = name;
	while (*p && !isspace(*p)) {
		p++;
	}
	if (*p != '\0') {
	    Tout(pn, "'%s' not added due to imbedded spaces\n", name);
	    return(0);
	}

	if (checkdups == TRUE) {
	    while (r->next != (struct recip *)NULL) {
		r = r->next;
		if (strcmp(r->name, name) == 0) {
			Tout(pn, "duplicate recipient '%s' not added to list\n",
									name);
			return(0);
		}
	    }
	}

	if ((p = malloc (sizeof(struct recip))) == (char *)NULL) {
		errmsg(E_MEM,"first malloc failed in add_recip()");
		done(1);
	}
	plist->last_recip->next = (struct recip *)p;
	r = plist->last_recip = plist->last_recip->next;
	if ((r->name = malloc (strlen(name)+1)) == (char *)NULL) {
		errmsg(E_MEM,"second malloc failed in add_recip()");
		done(1);
	}
	strcpy (r->name, name);
	r->next = (struct recip *)NULL;
	Tout(pn, "'%s' added to recipient list\n", name);

	return(1);
}

madd_recip (plist, namelist, checkdups)
reciplist	*plist;
char		*namelist;
int		checkdups;
{
	char	*name;
	for (name = strtok(namelist, " \t"); name; name = strtok((char*)0, " \t"))
		add_recip(plist, name, checkdups);
}
