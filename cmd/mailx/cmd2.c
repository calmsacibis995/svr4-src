/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:cmd2.c	1.6.5.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * More user commands.
 */

static int	delm();
static int	igshow();
static int	igcomp();
static int	save1();
static int	Save1();
static void	savemsglist();

/*
 * If any arguments were given, go to the next applicable argument
 * following dot, otherwise, go to the next applicable message.
 * If given as first command with no arguments, print first message.
 */

next(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, *ip2;
	int list[2], mdot;

	if (*msgvec != NULL) {

		/*
		 * If some messages were supplied, find the 
		 * first applicable one following dot using
		 * wrap around.
		 */

		mdot = dot - &message[0] + 1;

		/*
		 * Find the first message in the supplied
		 * message list which follows dot.
		 */

		for (ip = msgvec; *ip != NULL; ip++)
			if (*ip > mdot)
				break;
		if (*ip == NULL)
			ip = msgvec;
		ip2 = ip;
		do {
			mp = &message[*ip2 - 1];
			if ((mp->m_flag & MDELETED) == 0) {
				dot = mp;
				goto hitit;
			}
			if (*ip2 != NULL)
				ip2++;
			if (*ip2 == NULL)
				ip2 = msgvec;
		} while (ip2 != ip);
		printf("No messages applicable\n");
		return(1);
	}

	/*
	 * If this is the first command, select message 1.
	 * Note that this must exist for us to get here at all.
	 */

	if (!sawcom)
		goto hitit;

	/*
	 * Just find the next good message after dot, no
	 * wraparound.
	 */

	for (mp = dot+1; mp < &message[msgCount]; mp++)
		if ((mp->m_flag & (MDELETED|MSAVED)) == 0)
			break;
	if (mp >= &message[msgCount]) {
		printf("At EOF\n");
		return(0);
	}
	dot = mp;
hitit:
	/*
	 * Print dot.
	 */

	list[0] = dot - &message[0] + 1;
	list[1] = NULL;
	return(type(list));
}

/*
 * Save a message in a file.  Mark the message as saved
 * so we can discard when the user quits.
 */
save(str)
	char str[];
{
	return(save1(str, 1));
}

/*
 * Copy a message to a file without affected its saved-ness
 */
copycmd(str)
	char str[];
{
	return(save1(str, 0));
}

/*
 * Save/copy the indicated messages at the end of the passed file name.
 * If mark is true, mark the message "saved."
 */
static int
save1(str, mark)
	char str[];
{
	char *file, *cmd;
	int f, *msgvec;

	cmd = mark ? "save" : "copy";
	msgvec = (int *) salloc((msgCount + 2) * sizeof *msgvec);
	if ((file = snarf(str, &f, 0)) == NOSTR)
		file = Getf("MBOX");
	if (f==-1)
		return(1);
	if (!f) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == NULL) {
			printf("No messages to %s.\n", cmd);
			return(1);
		}
		msgvec[1] = NULL;
	}
	if (f && getmsglist(str, msgvec, 0) < 0)
		return(1);
	if ((file = expand(file)) == NOSTR)
		return(1);
	savemsglist(file, msgvec, mark);
	return(0);
}

Save(msgvec)
int *msgvec;
{
	return(Save1(msgvec, 1));
}

Copy(msgvec)
int *msgvec;
{
	return(Save1(msgvec, 0));
}

/*
 * save/copy the indicated messages at the end of a file named
 * by the sender of the first message in the msglist.
 */
static int
Save1(msgvec, mark)
int *msgvec;
{
	char recfile[128];

	getrecf(nameof(&message[*msgvec-1]), recfile, 1);
	savemsglist(safeexpand(recfile), msgvec, mark);
	return(0);
}

/*
 * save a message list in a file
 */
static void
savemsglist(file, msgvec, mark)
	char *file;
	int *msgvec;
{
	register int *ip, mesg;
	register struct message *mp;
	char *disp;
	FILE *obuf;
	struct stat statb;
	long lc, cc, t;
	int bnry;

	printf("\"%s\" ", file);
	flush();
	if (stat(file, &statb) >= 0)
		disp = "[Appended]";
	else
		disp = "[New file]";
	if ((obuf = fopen(file, "a")) == NULL) {
		perror("");
		return;
	}
	lc = cc = 0;
	bnry = 0;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		if (!mp->m_text) {
			bnry = 1;
		}
		if ((t = send(mp, obuf, 0)) < 0) {
			perror(file);
			fclose(obuf);
			return;
		}
		touch(mesg);
		lc += t;
		cc += mp->m_size;
		if (mark)
			mp->m_flag |= MSAVED;
	}
	fflush(obuf);
	if (ferror(obuf))
		perror(file);
	fclose(obuf);
	if (!bnry) {
		printf("%s %ld/%ld\n", disp, lc, cc);
	} else {
		printf("%s binary/%ld\n", disp, cc);
	}
}

/*
 * Write the indicated messages at the end of the passed
 * file name, minus header and trailing blank line.
 */

swrite(str)
	char str[];
{
	register int *ip, mesg;
	register struct message *mp;
	register char *file, *disp;
	char linebuf[LINESIZE];
	int f, *msgvec;
	long lc, cc, t, clen, n;
	FILE *obuf, *mesf;
	struct stat statb;
	int hdr, bnry;

	msgvec = (int *) salloc((msgCount + 2) * sizeof *msgvec);
	if ((file = snarf(str, &f, 1)) == NOSTR)
		return(1);
	if (f==-1)
		return(1);
	if ((file = expand(file)) == NOSTR)
		return(1);
	if (!f) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == NULL) {
			printf("No messages to write.\n");
			return(1);
		}
		msgvec[1] = NULL;
	}
	if (f && getmsglist(str, msgvec, 0) < 0)
		return(1);
	printf("\"%s\" ", file);
	flush();
	if (stat(file, &statb) >= 0)
		disp = "[Appended]";
	else
		disp = "[New file]";
	if ((obuf = fopen(file, "a")) == NULL) {
		perror("");
		return(1);
	}
	lc = cc = 0L;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		mesf = setinput(mp);
		t = mp->m_lines - 2;
		readline(mesf, linebuf);
		hdr = 1;
		bnry = 0;
		while (t-- > 0) {
			fgets(linebuf, sizeof linebuf, mesf);
			if (hdr) {
				if (linebuf[0] == '\n'
				 || !isspace(linebuf[0])
				 && !strchr(linebuf, ':')) {
					hdr = 0;
				}
				if (linebuf[0] == '\n')
					continue;
			}
			if (!hdr && mp->m_text) {
				fputs(linebuf, obuf);
				cc += strlen(linebuf);
				lc++;
			}
		}
		if (!hdr && !(mp->m_text)) {
		   clen = mp->m_clen - 1;
		   n = clen < sizeof linebuf ? clen : sizeof linebuf;
		   bnry = 1;
		   for (;;) {
			if ((n = fread(linebuf, 1, n, mesf)) <= 0) {
		           fprintf(stderr, "\t(Unexpected end-of-file).\n");
			   clen = 0;
			} else {
		          if (fwrite(linebuf, 1, n, obuf) != n) {
		          fprintf(stderr, "\t Error writing to the new file.\n");
			  fflush(obuf);
			} else {
				cc += n;
				}
			}
			clen -= n;
			if (clen <= 0) {
				break;
			}
		   }
		}
		mp->m_flag |= MSAVED;
	}
	fflush(obuf);
	if (ferror(obuf))
		perror(file);
	fclose(obuf);
	if (!bnry) {
	 	printf("%s %ld/%ld\n", disp, lc, cc);
	} else {
		printf("%s binary/%ld\n", disp, cc);
	}
	return(0);
}

/*
 * Snarf the file from the end of the command line and
 * return a pointer to it.  If there is no file attached,
 * just return NOSTR.  Put a null in front of the file
 * name so that the message list processing won't see it,
 * unless the file name is the only thing on the line, in
 * which case, return 0 in the reference flag variable.
 */

char *
snarf(linebuf, flag, erf)
	char linebuf[];
	int *flag;
{
	register char *cp;
	char end;

	*flag = 1;
	cp = strlen(linebuf) + linebuf - 1;

	/*
	 * Strip away trailing blanks.
	 */
	while (*cp == ' ' && cp > linebuf)
		cp--;
	*++cp = 0;

	/*
	 * Now see if string is quoted
	 */
	if (cp > linebuf && any(cp[-1], "'\"")) {
		end = *--cp;
		*cp = '\0';
		while (*cp != end && cp > linebuf)
			cp--;
		if (*cp != end) {
			printf("Syntax error: missing %c.\n", end);
			*flag = -1;
			return(NOSTR);
		}
		if (cp==linebuf)
			*flag = 0;
		*cp++ = '\0';
		return(cp);
	}

	/*
	 * Now search for the beginning of the file name.
	 */

	while (cp > linebuf && !any(*cp, "\t "))
		cp--;
	if (*cp == '\0') {
		if (erf)
			printf("No file specified.\n");
		*flag = 0;
		return(NOSTR);
	}
	if (any(*cp, " \t"))
		*cp++ = 0;
	else
		*flag = 0;
	return(cp);
}

/*
 * Delete messages.
 */

delete(msgvec)
	int msgvec[];
{
	return(delm(msgvec));
}

/*
 * Delete messages, then type the new dot.
 */

deltype(msgvec)
	int msgvec[];
{
	int list[2];
	int lastdot;

	lastdot = dot - &message[0] + 1;
	if (delm(msgvec) >= 0) {
		list[0] = dot - &message[0];
		list[0]++;
		if (list[0] > lastdot) {
			touch(list[0]);
			list[1] = NULL;
			return(type(list));
		}
		printf("At EOF\n");
		return(0);
	}
	else {
		printf("No more messages\n");
		return(0);
	}
}

/*
 * Delete the indicated messages.
 * Set dot to some nice place afterwards.
 * Internal interface.
 */
static int
delm(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register *ip, mesg;
	int last;

	last = NULL;
	for (ip = msgvec; *ip != NULL; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		mp->m_flag |= MDELETED|MTOUCH;
		mp->m_flag &= ~(MPRESERVE|MSAVED|MBOX);
		last = mesg;
	}
	if (last != NULL) {
		dot = &message[last-1];
		last = first(0, MDELETED);
		if (last != NULL) {
			dot = &message[last-1];
			return(0);
		}
		else {
			dot = &message[0];
			return(-1);
		}
	}

	/*
	 * Following can't happen -- it keeps lint happy
	 */

	return(-1);
}

/*
 * Undelete the indicated messages.
 */
int
undelete(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register *ip, mesg;

	for (ip = msgvec; ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		if (mesg == 0)
			return(0);
		touch(mesg);
		mp = &message[mesg-1];
		dot = mp;
		mp->m_flag &= ~MDELETED;
	}
	return(0);
}

/*
 * Add the given header fields to the ignored list.
 * If no arguments, print the current list of ignored fields.
 */
igfield(list)
	char *list[];
{
	char field[BUFSIZ];
	register int h;
	register struct ignore *igp;
	char **ap;

	if (argcount(list) == 0)
		return(igshow());
	for (ap = list; *ap != 0; ap++) {
		if (isign(*ap))
			continue;
		istrcpy(field, *ap);
		h = hash(field);
		igp = (struct ignore *) calloc(1, sizeof (struct ignore));
		igp->i_field = calloc(strlen(field) + 1, sizeof (char));
		strcpy(igp->i_field, field);
		igp->i_link = ignore[h];
		ignore[h] = igp;
	}
	return(0);
}

/*
 * Print out all currently ignored fields.
 */
static int
igshow()
{
	register int h, count;
	struct ignore *igp;
	char **ap, **ring;

	count = 0;
	for (h = 0; h < HSHSIZE; h++)
		for (igp = ignore[h]; igp != 0; igp = igp->i_link)
			count++;
	if (count == 0) {
		printf("No fields currently being ignored.\n");
		return(0);
	}
	ring = (char **) salloc((count + 1) * sizeof (char *));
	ap = ring;
	for (h = 0; h < HSHSIZE; h++)
		for (igp = ignore[h]; igp != 0; igp = igp->i_link)
			*ap++ = igp->i_field;
	*ap = 0;
	qsort(ring, count, sizeof (char *), igcomp);
	for (ap = ring; *ap != 0; ap++)
		printf("%s\n", *ap);
	return(0);
}

/*
 * Compare two names for sorting ignored field list.
 */
static int
igcomp(l, r)
	char **l, **r;
{
	return(strcmp(*l, *r));
}

/*
 * Remove a list of fields from the ignore list.
 */
unigfield(list)
	char *list[];
{
	char **ap, field[BUFSIZ];
	register int h, count = 0;
	register struct ignore *ig1, *ig2;

	if (argcount(list) == 0) {
		for (h = 0; h < HSHSIZE; h++) {
			ig1 = ignore[h];
			while (ig1) {
				free(ig1->i_field);
				ig2 = ig1->i_link;
				free((char *) ig1);
				ig1 = ig2;
				count++;
			}
			ignore[h] = NULL;
		}
		if (count == 0)
			printf("No fields currently being ignored.\n");
		return 0;
	}
	for (ap = list; *ap; ap++) {
		istrcpy(field, *ap);
		h = hash(field);
		for (ig1 = ignore[h]; ig1; ig2 = ig1, ig1 = ig1->i_link)
			if (strcmp(ig1->i_field, field) == 0) {
				if (ig1 == ignore[h])
					ignore[h] = ig1->i_link;
				else
					ig2->i_link = ig1->i_link;
				free(ig1->i_field);
				free((char *) ig1);
				break;
			}
	}
	return 0;
}
