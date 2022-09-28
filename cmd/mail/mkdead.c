/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:mkdead.c	1.6.3.1"
#include "mail.h"
/*
	Routine creates dead.letter
*/
void mkdead()
{
	static char pn[] = "mkdead";
	int aret;
	char *dotdead = &dead[1];
	malf = 0;

	/*
		Make certain that there's something to copy.
	*/
	if (!tmpf)
		return;

	/*
		Try to create dead letter in current directory
		or in home directory
	*/
	umask(umsave);
	if ((aret = legal(dotdead))) malf = fopen(dotdead, "a");
	if ((malf == NULL) || (aret == 0)) {
		/*
			try to create in $HOME
		*/
		if((hmdead = malloc(strlen(home) + strlen(dead) + 1)) == NULL) {
			fprintf(stderr, "%s: Can't malloc\n",program);
			Dout(pn, 0, "Cannot malloc.\n");
			goto out;
		}
		cat(hmdead, home, dead);
		if ((aret=legal(hmdead))) malf = fopen(hmdead, "a");
		if ((malf == NULL) || (aret == 0)) {
			fprintf(stderr,
				"%s: Cannot create %s\n",
				program,dotdead);
			Dout(pn, 0, "Cannot create %s\n", dotdead);
		out:
			fclose(tmpf);
			error = E_FILE;
			Dout(pn, 0, "error set to %d\n", error);
			umask(7);
			return;
		}  else {
			chmod(hmdead, DEADPERM);
			fprintf(stderr,"%s: Mail saved in %s\n",program,hmdead);
		}
	} else {
		chmod(dotdead, DEADPERM);
		fprintf(stderr,"%s: Mail saved in %s\n",program,dotdead);
	}

	/*
		Copy letter into dead letter box
	*/
	umask(7);
	aret = fseek(tmpf,0L,0);
	if (aret)
		errmsg(E_DEAD,"");
	if (!copystream(tmpf, malf))
		errmsg(E_DEAD,"");
	fclose(malf);
}

void savdead()
{
	static char pn[] = "savdead";
	setsig(SIGINT, saveint);
	dflag = 2;	/* do not send back letter on interrupt */
	Dout(pn, 0, "dflag set to 2\n");
	if (!error) {
		error = E_REMOTE;
		Dout(pn, 0, "error set to %d\n", error);
	}
	maxerr = error;
	Dout(pn, 0, "maxerr set to %d\n", maxerr);
}
