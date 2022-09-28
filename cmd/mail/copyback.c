/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:copyback.c	1.6.3.1"
/*
    NAME
	copyback - copy temp or whatever back to /var/mail

    SYNOPSIS
	void copyback()

    DESCRIPTION
	Copy the reduced contents of lettmp back to
	the mail file. First copy any new mail from
	the mail file to the end of lettmp.
*/

#include "mail.h"
void copyback()
{
	register int	i, n;
	int		new = 0;
	mode_t		mailmode, omask;
	struct stat	stbuf;
	void (*hstat)(), (*istat)(), (*qstat)();

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	lock(my_name);
	stat(mailfile, &stbuf);
	mailmode = stbuf.st_mode;

	/*
		Has new mail arrived?
	*/
	if (stbuf.st_size != let[nlet].adr) {
		malf = doopen(mailfile, "r",E_FILE);
		fseek(malf, let[nlet].adr, 0);
		fclose(tmpf);
		tmpf = doopen(lettmp,"a",E_TMP);
		/*
			Append new mail assume only one new letter
		*/
		if (!copystream(malf, tmpf)) {
			fclose(malf);
			tmperr();
			done(0);
		}
		fclose(malf);
		fclose(tmpf);
		tmpf = doopen(lettmp,"r+",E_TMP);
		if (nlet == (MAXLET-2)) {
			errmsg(E_SPACE,"");
			done(0);
		}
		let[++nlet].adr = stbuf.st_size;
		new = 1;
	}

	/*
		Copy mail back to mail file
	*/
	omask = umask(0117);

	/*
		The invoker must own the mailfile being copied to
	*/
	if ((stbuf.st_uid != my_euid) && (stbuf.st_uid != my_uid)) {
		errmsg(E_OWNR,"");
		done(0);
	}

	/* 
		If user specified the '-f' option we dont do 
		the routines to handle :saved files.
		As we would (incorrectly) restore to the user's
		mailfile upon next execution!
	*/
	if (flgf) {
		strcpy(savefile,mailfile);
	} else {
		cat(savefile,mailsave,my_name);
	}

	if ((malf = fopen(savefile, "w")) == NULL) {
		if (!flgf) {
			errmsg(E_FILE,"Cannot open savefile");
		} else {
			errmsg(E_FILE,"Cannot re-write the alternate file");
		}
		done(0);
	}

	if (chown(savefile,mf_uid,mf_gid) == -1) {
		errmsg(E_FILE,"Cannot chown savefile");
		done(0);
	}
	umask(omask);
	n = 0;

	for (i = 0; i < nlet; i++) {
		/*
			Note: any action other than an undelete, or a 
			plain read causes the letter acted upon to be 
			deleted
		*/
		if (let[i].change == ' ') {
			if (copylet(i, malf, ORDINARY) == FALSE) {
				errmsg(E_FILE,"Cannot copy mail to savefile");
				fprintf(stderr, "%s: A copy of your mailfile is in '%s'\n",program,lettmp);
				done(1);	/* keep temp file */
			}	
			n++;
		}
	}
	fclose(malf);

	if (!flgf) {
		if (unlink(mailfile) != 0) {
			errmsg(E_FILE,"Cannot unlink mailfile");
			done(0);
		}
		chmod(savefile,mailmode);
#ifdef SVR4
		if (rename(savefile, mailfile) != 0) {
			errmsg(E_FILE,"Cannot rename savefile to mailfile");
			done(0);
		}
#else
		if (link(savefile,mailfile) != 0) {
			errmsg(E_FILE,"Cannot link savefile to mailfile");
			done(0);
		}
		if (unlink(savefile) != 0) {
			errmsg(E_FILE,"Cannot unlink save file");
			done(0);
		}
#endif
	}

	/*
		Empty mailbox?
	*/
	if (n == 0) {
		delempty(stbuf.st_mode, mailfile);
	}

	if (new && !flgf) {
		printf("New mail arrived\n");
	}

	unlock();
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
}
