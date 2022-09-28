/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:doFopt.c	1.6.3.1"
#include "mail.h"
/*
	Handles installing and removing of forwarding
*/
void doFopt()
{
	static char pn[] = "doFopt";
	static char	SP[2] = " ";
	char		*p = uval, *q;
	char		hold[1024];
	struct		stat statb;

	if (error != E_FLGE) {
		errmsg(E_FILE,
		     "Cannot install/remove forwarding without empty mailfile");
		done(0);
	}

	error = 0;
	lock(my_name);
	createmf(my_uid, mailfile);

	hold[0] = '\0';
	if (p[0] != '\0') {
		/*
			Remove excess blanks/tabs from uval
			Accept comma or space as delimiter
		*/
		while ((q = strpbrk(p,", \t")) != (char *)NULL) {
			*q = '\0';
			if (!notme(p, my_name)) {
				errmsg(E_SYNTAX,
					"Cannot install forwarding to oneself");
				done(0);
			}
			strcat(hold,SP);
			strcat(hold,p);
			p = q+1 + strspn(q+1,", \t");
			if (*p == '|') {
				strcat(hold,SP);
				strcat(hold,p);
				*p = '\0';
				break;
			}
		}
		if (*p != '\0') {
			if (!notme(p, my_name)) {
				errmsg(E_SYNTAX,
					"Cannot install forwarding to oneself");
				done(0);
			}
			strcat(hold,SP);
			strcat(hold,p);
		}
		malf = doopen(mailfile, "w",E_FILE);
		printf("Forwarding to%s\n",hold);
		fprintf(malf, "Forward to%s\n",hold);
                fclose(malf);
		/* Turn on setgid bit on mailfile */
		stat(mailfile, &statb);
		statb.st_mode |= S_ISGID;
		chmod (mailfile, statb.st_mode);
	} else {
		if (areforwarding(mailfile)) {
			malf = doopen(mailfile, "w",E_FILE);
			fclose(malf);
			printf("Forwarding removed\n");
		} else {
			fprintf(stderr,"%s: No forwarding to remove\n",program);
		}
		/* Turn off setgid bit on mailfile if set */
		stat(mailfile, &statb);
		statb.st_mode &=  ~(S_ISGID);
		if (!delempty(statb.st_mode, mailfile))
			chmod(mailfile, statb.st_mode);
	}
	stamp();
	unlock();
	done(0);
}
