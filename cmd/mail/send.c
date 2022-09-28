/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:send.c	1.8.3.2"
/*
    NAME
	send - send a letter to a given name

    SYNOPSIS
	send(reciplist *plist, int letnum, char *name, int level)


    DESCRIPTION
	Send letter "letnum" from the current letter file to a given
	"name". Use the recipient list "plist" for any translations
	which may occur. Keep track of the local forwarding recursion
	depth in "level". (If we go too deep, reject the letter.)

    RETURNS
	TRUE	-> mail sent (or translated to something else)
	FALSE	-> can't send mail
*/
#include "mail.h"

#ifdef __STDC__
static int safefile(char*);
#else
static int safefile();
#endif

send(plist, letnum, name, level)
reciplist	*plist;
int		letnum;
char		*name;
{
	char		file[MAXFILENAME];
	int 		rc=0, fini=0;
	int		local = 0;
	char 		tmpsendto[1024];
	char		buf[2048];
	char		*q, *tsp = tmpsendto;
	static char	pn[] = "send";
	struct hdrs	*hptr;
	uid_t		useruid;	/* uid of local user */

	recipname = name; /* set global pointer for low level routines */

	Dout(pn, 0, "entered\n");
	/*
	 * Check for special case of leading '!' in name.
	 */
	while (*name == '!') {
		name++;
	}

	local = islocal(name, &useruid);
	Tout (pn, "\nNext recipient '%s' is %slocal to %s\n", name,
		(local==1 ? "" : "NOT "), thissys);
	/*
	 * If sending back 'Failure to deliver' notification,
	 * do NOT clobber original recipient.....
	 */
	if ((error == 0 || dflag != 1) || uval[0] == '\0') {
		strcpy(uval, name);
	}
	if(level > 20) {
		error = E_UNBND;
		goback(0);
		rc = FALSE;
		goto rtrn;
	}
	if (strcmp(name, "-") == SAME) {
		rc = TRUE;
		goto rtrn;
	}

	/* SURROGATE processing: if one exists, it gets first shot. */
	switch (sendsurg(plist, letnum, DELIVER, local)) {
	case CONTINUE:	/* surrogate requests normal processing, */
			/* or no surrogate */
		break;

	case SUCCESS:	/* surrogate did it all */
		sendsurg(plist, letnum, POSTDELIVER, local);
		/* If local recipient, send positive delivery notification */
		/* if requested */
		if (local && (dflag != 1)) {
			gendeliv((FILE *)NULL, 0, name);
		}
		/* FALLTHROUGH */

	case TRANSLATE: /* name translated into something else. Will do later */
		rc = TRUE;
		goto rtrn;

	case FAILURE:
		strcpy(fwdFrom, ""); /* want to send back to REAL sender */
		if (!flgT) {
			/* Only do this if not tracing mailsurr via -T option */
			goback(0);
		}
		rc = TRUE;
		goto rtrn;
	}

	if (flgT) {
	    Tout(pn,
		"If no SUCCESS via mailsurr file, local delivery would %s\n",
		(local ? "succeed" : "fail"));
	    sendsurg(plist, letnum, POSTDELIVER, local);
	    rc = TRUE;
	    goto rtrn;
	}

	if (!local) {
		/* unknown recipient. don't attempt local delivery */
		fprintf(stderr, "%s: Can't send to %s\n",program,name);
		error = SURRerrfile ? E_SURG : E_USER;
		Dout(pn, 0, "unknown local user '%s', error set to %d\n",
			   name, error);
		goback(0);
		rc = FALSE;
		goto rtrn;
	}

	Dout(pn, 0, "Delivering locally\n");

	/*
		Check for failsafe processing. If the :saved
		directory is not there, forward to $FAILSAFE!name.
	*/
	if (failsafe && (access(SAVEDIR, A_EXIST) != A_OK)) {
		reciplist list;
		new_reciplist(&list);
		Dout(pn, 0, ":saved directory not present!\n");

		/* create an Auto-Forwarded-From: line */
		sprintf(buf,"%s!%s", thissys, name);
		pushlist(H_AFWDFROM, HEAD, buf, FALSE);

		/* Update the >To: line if one is present */
		if ((hptr = hdrlines[H_TCOPY].head) !=
		    (struct hdrs *)NULL) {
			char *pp, *qq;
			sprintf(buf, "%s!%s", failsafe, name);
			if ((pp = strchr(hptr->value,'/')) !=
			    (char *)NULL) {
				if ((qq = strchr(pp,'(')) !=
				(char *)NULL) {
					/* Do not include the */
					/* comment field */
					strncat(buf, pp, qq-pp);
				} else {
					strcat(buf, pp);
				}
			}
			pushlist(H_TCOPY, HEAD, buf, FALSE);
		}
		strcpy(fwdFrom,name);

		/* Change the name and send it. */
		sprintf(buf, "%s!%s", failsafe, name);
		Dout(pn, 0, "\tnow sending to '%s'\n", buf);
		add_recip(&list, buf, FALSE);
		rc = sendlist(&list, letnum, level + 1);

		strcpy(fwdFrom,"");
		poplist(H_AFWDFROM, HEAD);
		if (hdrlines[H_TCOPY].head != (struct hdrs *)NULL)
			poplist(H_TCOPY, HEAD);
		del_reciplist(&list);
		goto rtrn;
	}

	/*
		See if user has specified that mail is to be fowarded
	*/
	cat(file, maildir, name);
	if (areforwarding(file)) {
		strcpy(tmpsendto,sendto);
		Dout(pn, 0, "tmpsendto = '%s'\n", tsp);
		/*
		 * Pick off names one at a time for forwarding.
		 * If name starts with '|' (pipe symbol), assume rest of
		 * line is command to pipe to.
		 */
		while (*tsp && !fini) {
			/* skip leading blank(s) */
			tsp += strspn(tsp," ");
			if (*tsp == '|') {
				char	*contype = "";
				char	*subject = "";
				char	dbuf[10];
				Dout(pn, 0, "piping to '%s'\n", tsp);
				fini++;
				if ((hptr = hdrlines[H_CTYPE].head) !=
				    (struct hdrs *)NULL) {
					contype = hptr->value;
				}
				if ((hptr = hdrlines[H_SUBJ].head) !=
				    (struct hdrs *)NULL) {
					subject = hptr->value;
				}
				/*
				 * Pass any invocation debug option to
				 * the PIPER
				 */
				if (orig_dbglvl != 0) {
					sprintf(dbuf,"-x%d", orig_dbglvl);
				}
				sprintf(buf,
			  "%s %s -r \"%s\" -R \"%s\" -c \"%s\" -S \"%s\"",
				  PIPER, ((orig_dbglvl == 0) ? "" : dbuf),
				  name, Rpath, contype, subject);
				Dout(pn, 0, "PIPER exec == '%s'\n", buf);
				rc = 0;
				lock (name);
				rc = pipletr(letnum, buf, ORDINARY);
				unlock();
				Dout(pn, 0, "PIPER complete, result %d\n", rc);
				if (debug > 0) {
				    if (SURRerrfile) {
					fprintf(dbgfp,
					    "=============== Start of output from surrogate ============\n");
					rewind (SURRerrfile);
					(void) copystream(SURRerrfile, dbgfp);
					fprintf(dbgfp,
					    "\n=============== End of output from surrogate ============\n");
				    } else
					fprintf(dbgfp,
					    "=============== Surrogate output unavailable ============\n");
				}
				if (rc != 0) {
					char	*tmpstr;
					int	len;

					error = E_SURG;
					Dout(pn, 0, "PIPER command failed, error set to %d\n", error);
					surg_rc = rc;
					len = strlen(tsp) + 7; /* 7 ==> \n + 5 blanks + \0 */
					if (SURRcmdstr != (char *)NULL) {
						len += strlen(SURRcmdstr);
					}
					if ((tmpstr = malloc (len)) != (char *)NULL) {
						sprintf(tmpstr,"%s\n     %s",
						    ((SURRcmdstr == (char *)NULL) ? "" : SURRcmdstr),
						    tsp);
						if (SURRcmdstr != (char *)NULL) {
							free (SURRcmdstr);
						}
						SURRcmdstr = tmpstr;
						trimnl (SURRcmdstr);
					} else
						Dout(pn, 0, "malloc for tmpstr failed\n");
					goback(0);
					rc = FALSE;
					goto rtrn;
				}
				sendsurg (plist, letnum, POSTDELIVER, local);
				/* Send positive delivery notification */
				/* if requested */
				if (dflag != 1) {
					gendeliv((FILE *)NULL, 0, name);
				}
				continue;
			}
			/*
			 * Find end of current foward-to name.
			 */
			if ((q = strpbrk(tsp," \n")) != (char *)NULL) {
				*q = '\0';
			}
			if (strlen(tsp) == 0) {
				/* Will get here if there were trailing */
				/* blanks before the newline */
				continue;
			}
			Dout(pn, 0, "forwarding to '%s'\n", tsp);
			if (!notme(tsp, name)) {
				goback(0);
			} else {
				reciplist list;
				/* To handle recursive calling of send() */
				/* when dealing with auto-forwarding, push */
				/* new entries onto HEAD of linked lists for */
				/* various types of header lines to reflect */
				/* auto-forwarded info. After return from */
				/* recursion, pop new entry off plist to */
				/* reset for next, if any, recipient */
				if (affcnt > 20) {
					error = E_UNBND;
					goback(0);
					rc = FALSE;
					goto rtrn;
				}
				new_reciplist(&list);
				sprintf(buf,"%s!%s", thissys, name);
				pushlist(H_AFWDFROM, HEAD, buf, FALSE);
				sprintf(buf, "%s", tsp);
				if ((hptr = hdrlines[H_TCOPY].head) !=
				    (struct hdrs *)NULL) {
					char *pp, *qq;
					if ((pp = strchr(hptr->value,'/')) !=
					    (char *)NULL) {
						if ((qq = strchr(pp,'(')) !=
						(char *)NULL) {
							/* Dont include the */
							/* comment field */
							strncat(buf, pp, qq-pp);
						} else {
							strcat(buf, pp);
						}
					}
				}
				pushlist(H_TCOPY, HEAD, buf, FALSE);
				strcpy(fwdFrom,name);

				add_recip(&list, tsp, FALSE);
				sendlist(&list, letnum, level + 1);

				strcpy(fwdFrom,"");
				poplist(H_AFWDFROM, HEAD);
				poplist(H_TCOPY, HEAD);
				del_reciplist(&list);
			}
			/*
			 * Assume that the 'Forward to' line in the mailfile
			 * has a newline before the terminating NULL
			 * or this will take you to never-never land.
			 */
			tsp = q + 1;
		}
		rc = TRUE;
		goto rtrn;
	}

	lock(name);
	createmf(useruid, file);

	/* Disallow links. */
	if (!safefile(file)) {
		fprintf(stderr, "%s: '%s' must be regular or character special file with no links\n",
			program, file);
		unlock();
		error = E_FILE;
		Dout(pn, 0, "%s must be regular or character special file with no links, error set to %d\n", file, error);
		goback(0);
		rc = FALSE;
		goto rtrn;
	}

	/*
		Append letter to mail box
	*/
	if (((malf = fopen(file, "a")) == NULL) ||
	    (copylet(letnum, malf, ORDINARY) == FALSE)) {
		fprintf(stderr, "%s: Cannot append to %s\n",program,file);
		unlock();
		error = E_FILE;
		Dout(pn, 0, "cannot append to '%s', error set to %d\n", file, error);
		goback(0);
		rc = FALSE;
		goto rtrn;
	}
	fclose(malf);
	unlock();
	sendsurg (plist, letnum, POSTDELIVER, local);

	/* Send positive delivery notification if requested */
	if (dflag != 1) {
		gendeliv((FILE *)NULL, 0, name);
	}
	rc = TRUE;

    rtrn:
	if (SURRerrfile != (FILE *)NULL) {
		fclose (SURRerrfile);
		SURRerrfile = (FILE *)NULL;
	}

	if (SURRcmdstr != (char *)NULL) {
		free (SURRcmdstr);
		SURRcmdstr = (char *)NULL;
	}

	return (rc);
}

/*
   Make certain that the file has no links and is either a regular file
   or a character-special file. This latter check allows one to create
   a /dev/null node under mail for a mailbox which is just being thrown away.
*/
static int safefile(f)
	char *f;
{
	struct stat statb;

#ifdef SVR3
	if (stat(f, &statb) < 0)
#else
	if (lstat(f, &statb) < 0)
#endif
		return 1;

#ifdef S_IFLNK
	if ((statb.st_mode & S_IFMT) == S_IFLNK)
		return 0;
#endif

	if (statb.st_nlink != 1)
		return 0;

	if (((statb.st_mode & S_IFMT) != S_IFREG) &&
	    ((statb.st_mode & S_IFMT) != S_IFCHR))
		return 0;

	return 1;
}
