/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:notme.c	1.5.3.1"
#include "mail.h"
/*
	Don't allow forwarding to oneself.

	If we are sending to system!user and he has 
		"Forward to system!user" 
			or
		"Forward to user" - error
*/
notme(fto, myname)
char *fto, *myname;
{
	static char pn[] = "notme";
	char tosys[256], touser[256], work[256];
	int bangs = 0, k, l, lastbang = 0, priorbang = 0;

	Dout(pn, 0, "fto = '%s', myname = '%s'\n", fto, myname);
	if (strrchr(fto, '!') == NULL) {
		if (strcmp(fto, myname) == SAME) {
			strcpy(uval, myname);
			error = E_FRWL;
			Dout(pn, 0, "error set to %d\n", error);
			return(FALSE);
		}
	} else {
		for (k = 0; fto[k] != '\0'; k++)  {
			if (fto[k] == '!') {
				bangs++;
				priorbang=lastbang;
				lastbang=k;
			}
		}
		if (bangs > 1) {
			for (k = 0; fto[priorbang + k + 1] != '\0'; k++) {
				work[k] = fto[priorbang + k + 1];
			}
			work[k] = '\0';
		} else {
			strcpy(work, fto);
		}

		for (k = 0; work[k] != '!'; k++) {
			tosys[k] = work[k];
		}
		tosys[k++] = '\0';

		for (l = 0; work[k] != '\0'; k++) {
			touser[l++] = work[k];
		}
		touser[l] = '\0';

		/* check the system name against both */
		/* the uname and the cluster name */
		Dout(pn, 0, "touser = '%s', myname = '%s'\n", touser,myname);
		Dout(pn, 0, "tosys = '%s', nodename = '%s', thissys = '%s'\n", 
			tosys, utsn.nodename, thissys);

		if ((strcmp(touser, myname) == SAME) &&
		    ((strcmp(tosys, utsn.nodename) == SAME) ||
		     (strcmp(tosys, thissys) == SAME))) {
			strcpy(uval, myname);
			error = E_FRWL;
			Dout(pn, 0, "error set to %d\n", error);
			return(FALSE);
		}
	}
	return(TRUE);
}
