/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:names.c	1.10.5.1"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Handle name lists.
 */

#include "rcv.h"

static struct name	*nalloc();
static int		isfileaddr();
static int		lengthof();
static struct name	*gexpand();
static char		*norm();
static struct name	*put();

/*
 * Allocate a single element of a name list,
 * initialize its name field to the passed
 * name and return it.
 */

static struct name *
nalloc(str)
	char str[];
{
	register struct name *np;

	np = (struct name *) salloc(sizeof *np);
	np->n_flink = NIL;
	np->n_blink = NIL;
	np->n_type = -1;
	np->n_full = savestr(str);
	np->n_name = skin(np->n_full);
	return(np);
}

/*
 * Find the tail of a list and return it.
 */

struct name *
tailof(name)
	struct name *name;
{
	register struct name *np;

	np = name;
	if (np == NIL)
		return(NIL);
	while (np->n_flink != NIL)
		np = np->n_flink;
	return(np);
}

/*
 * Extract a list of names from a line,
 * and make a list of names from it.
 * Return the list or NIL if none found.
 */

struct name *
extract(line, arg_ntype)
	char line[];
{
	short ntype = (short)arg_ntype;
	register char *cp;
	register struct name *top, *np, *t;
	char nbuf[BUFSIZ], abuf[BUFSIZ];
	int comma;

	if (line == NOSTR || strlen(line) == 0)
		return(NIL);
	comma = docomma(line);
	top = NIL;
	np = NIL;
	cp = line;
	while ((cp = yankword(cp, nbuf, comma)) != NOSTR) {
		if (np != NIL && equal(nbuf, "at")) {
			strcpy(abuf, nbuf);
			if ((cp = yankword(cp, nbuf, comma)) == NOSTR) {
				strcpy(nbuf, abuf);
				goto normal;
			}
			strcpy(abuf, np->n_name);
			strcat(abuf, "@");
			strcat(abuf, nbuf);
			np->n_name = savestr(abuf);
			continue;
		}
normal:
		t = nalloc(nbuf);
		t->n_type = ntype;
		if (top == NIL)
			top = t;
		else
			np->n_flink = t;
		t->n_blink = np;
		np = t;
	}
	return(top);
}

/*
 * Turn a list of names into a string of the same names.
 */

char *
detract(np, ntype)
	register struct name *np;
{
	register int s;
	register char *cp, *top;
	register struct name *p;

	if (np == NIL)
		return(NOSTR);
	s = 0;
	for (p = np; p != NIL; p = p->n_flink) {
		if ((ntype && (p->n_type & GMASK) != ntype)
		 || (p->n_type & GDEL))
			continue;
		s += strlen(p->n_full) + 2;
	}
	if (s == 0)
		return(NOSTR);
	top = salloc((unsigned)(++s));
	cp = top;
	for (p = np; p != NIL; p = p->n_flink) {
		if ((ntype && (p->n_type & GMASK) != ntype)
		 || (p->n_type & GDEL))
			continue;
		cp = copy(p->n_full, cp);
		*cp++ = ',';
		*cp++ = ' ';
	}
	*cp = 0;
	return(top);
}

struct name *
outpre(to)
struct name *to;
{
	register struct name *np;

	for (np = to; np; np = np->n_flink)
		if (any(*np->n_name, "+|"))
			np->n_type |= GDEL;
	return to;
}

/*
 * For each recipient in the passed name list with a /
 * in the name, append the message to the end of the named file
 * and remove him from the recipient list.
 *
 * Recipients whose name begins with | are piped through the given
 * program and removed.
 */

int
outof(names, fo)
	struct name *names;
	FILE *fo;
{
	register int c;
	register struct name *np;
	long now;
	char *date, *fname, *shell;
	FILE *fout, *fin;
	int ispipe, s;
	char line[BUFSIZ];
	int nout = 0;

	for (np = names; np != NIL; np = np->n_flink) {
		if (!isfileaddr(np->n_name) && np->n_name[0] != '|')
			continue;
		nout++;
		ispipe = np->n_name[0] == '|';
		if (ispipe)
			fname = np->n_name+1;
		else
			fname = safeexpand(np->n_name);

		/*
		 * See if we have copied the complete message out yet.
		 * If not, do so.
		 */

		if (image < 0) {
			if ((fout = fopen(tempEdit, "a")) == NULL) {
				perror(tempEdit);
				senderr++;
				goto cant;
			}
			image = open(tempEdit, O_RDWR);
			unlink(tempEdit);
			if (image < 0) {
				perror(tempEdit);
				senderr++;
				goto cant;
			}
			else {
				rewind(fo);
				time(&now);
				date = ctime(&now);
				fprintf(fout, "From %s %s", myname, date);
				while ((c = getc(fo)) != EOF)
					putc(c, fout);
				rewind(fo);
				fflush(fout);
				if (ferror(fout))
					perror(tempEdit);
				fclose(fout);
			}
		}

		/*
		 * Now either copy "image" to the desired file
		 * or give it as the standard input to the desired
		 * program as appropriate.
		 */

		if (ispipe) {
			wait(&s);
			switch (fork()) {
			case 0:
				sigchild();
				sigset(SIGHUP, SIG_IGN);
				sigset(SIGINT, SIG_IGN);
				sigset(SIGQUIT, SIG_IGN);
				close(0);
				dup(image);
				close(image);
				lseek(0, 0L, 0);
				if ((shell = value("SHELL")) == NOSTR || *shell=='\0')
					shell = SHELL;
				execlp(shell, shell, "-c", fname, (char *)0);
				perror(shell);
				exit(1);
				break;

			case (pid_t)-1:
				perror("fork");
				senderr++;
				goto cant;
			}
		}
		else {
			if ((fout = fopen(fname, "a")) == NULL) {
				perror(fname);
				senderr++;
				goto cant;
			}
			fin = Fdopen(image, "r");
			if (fin == NULL) {
				fprintf(stderr, "Can't reopen image\n");
				fclose(fout);
				senderr++;
				goto cant;
			}
			rewind(fin);
			putc(getc(fin), fout);
			while (fgets(line, sizeof line, fin)) {
				if (!strncmp(line, "From ", 5))
					putc('>', fout);
				fputs(line, fout);
			}
			putc('\n', fout);
			if (ferror(fout))
				senderr++, perror(fname);
			fclose(fout);
			fclose(fin);
		}
cant:
		/*
		 * In days of old we removed the entry from the
		 * the list; now for sake of header expansion
		 * we leave it in and mark it as deleted.
		 */

#ifdef CRAZYWOW
		if (np == top) {
			top = np->n_flink;
			if (top != NIL)
				top->n_blink = NIL;
			np = top;
			continue;
		}
		x = np->n_blink;
		t = np->n_flink;
		x->n_flink = t;
		if (t != NIL)
			t->n_blink = x;
		np = t;
#endif

		np->n_type |= GDEL;
	}
	if (image >= 0) {
		close(image);
		image = -1;
	}
	return(nout);
}

/*
 * Determine if the passed address is a local "send to file" address.
 * If any of the network metacharacters precedes any slashes, it can't
 * be a filename.  We cheat with .'s to allow path names like ./...
 */
static int
isfileaddr(name)
	char *name;
{
	register char *cp;

	if (any('@', name))
		return(0);
	if (*name == '+')
		return(1);
	for (cp = name; *cp; cp++) {
		if (*cp == '.')
			continue;
		if (any(*cp, metanet))
			return(0);
		if (*cp == '/')
			return(1);
	}
	return(0);
}

/*
 * Map all of the aliased users in the invoker's mailrc
 * file and insert them into the list.
 * Changed after all these months of service to recursively
 * expand names (2/14/80).
 */

struct name *
usermap(names)
	struct name *names;
{
	register struct name *new, *np, *cp;
	struct grouphead *gh;
	register int metoo;

	new = NIL;
	np = names;
	metoo = (value("metoo") != NOSTR);
	while (np != NIL) {
		if (np->n_name[0] == '\\') {
			cp = np->n_flink;
			new = put(new, np);
			np = cp;
			continue;
		}
		gh = findgroup(np->n_name);
		cp = np->n_flink;
		if (gh != NOGRP)
			new = gexpand(new, gh, metoo, np->n_type);
		else
			new = put(new, np);
		np = cp;
	}
	return(new);
}

/*
 * Recursively expand a group name.  We limit the expansion to some
 * fixed level to keep things from going haywire.
 * Direct recursion is not expanded for convenience.
 */

static struct name *
gexpand(nlist, gh, metoo, arg_ntype)
	struct name *nlist;
	struct grouphead *gh;
{
	short ntype = (short)arg_ntype;
	struct mgroup *gp;
	struct grouphead *ngh;
	struct name *np;
	static int depth;
	char *cp;

	if (depth > MAXEXP) {
		printf("Expanding alias to depth larger than %d\n", MAXEXP);
		return(nlist);
	}
	depth++;
	for (gp = gh->g_list; gp != NOGE; gp = gp->ge_link) {
		cp = gp->ge_name;
		if (*cp == '\\')
			goto quote;
		if (strcmp(cp, gh->g_name) == 0)
			goto quote;
		if ((ngh = findgroup(cp)) != NOGRP) {
			nlist = gexpand(nlist, ngh, metoo, ntype);
			continue;
		}
quote:
		np = nalloc(cp);
		np->n_type = ntype;
		/*
		 * At this point should allow to expand
		 * to self if only person in group
		 */
		if (gp == gh->g_list && gp->ge_link == NOGE)
			goto skip;
		if (!metoo && samebody(myname, gp->ge_name))
			np->n_type |= GDEL;
skip:
		nlist = put(nlist, np);
	}
	depth--;
	return(nlist);
}

/*
 * Normalize a network name for comparison purposes.
 */
static char *
norm(user, ubuf, nbangs)
	register char *user, *ubuf;
{
	register char *cp;

	while (*user++ == '!');
	user--;
	if (!strchr(user, '!')) {
		sprintf(ubuf, "%s!%s", host, user);
		user = ubuf;
	}
	if (nbangs) {
		cp = user + strlen(user);
		while (nbangs--)
			while (cp > user && *--cp != '!');
		user = (cp > user) ? ++cp : cp;
	}
	return user;
}

/*
 * Implement allnet options.
 */
samebody(user, addr)
	register char *user, *addr;
{
	char ubuf[500], abuf[500];
	char *allnet = value("allnet");
	int nbangs = allnet ? !strcmp(allnet, "uucp") ? 2 : 1 : 0;

	user = norm(user, ubuf, nbangs);
	addr = norm(addr, abuf, nbangs);
	return strcmp(user, addr) == 0;
}

/*
 * Compute the length of the passed name list and
 * return it.
 */
static int
lengthof(name)
	struct name *name;
{
	register struct name *np;
	register int c;

	for (c = 0, np = name; np != NIL; c++, np = np->n_flink)
		;
	return(c);
}

/*
 * Concatenate the two passed name lists, return the result.
 */

struct name *
cat(n1, n2)
	struct name *n1, *n2;
{
	register struct name *tail;

	if (n1 == NIL)
		return(n2);
	if (n2 == NIL)
		return(n1);
	tail = tailof(n1);
	tail->n_flink = n2;
	n2->n_blink = tail;
	return(n1);
}

/*
 * Unpack the name list onto a vector of strings.
 * Return an error if the name list won't fit.
 */

char **
unpack(np)
	struct name *np;
{
	register char **ap, **top;
	register struct name *n;
	char hbuf[10];
	int t, extra;
#ifdef DELIVERMAIL
	int metoo;
#endif

	n = np;
	if ((t = lengthof(n)) == 0)
		panic("No names to unpack");

	/*
	 * Compute the number of extra arguments we will need.
	 * We need at least 2 extra -- one for "mail" and one for
	 * the terminating 0 pointer.
	 * Additional spots may be needed to pass along -r and -f to 
	 * the host mailer.
	 */

	extra = 2;

	if (rflag != NOSTR)
		extra += 2;
#ifdef DELIVERMAIL
	extra++;
	metoo = value("metoo") != NOSTR;
	if (metoo)
		extra++;
#endif /* DELIVERMAIL */
	if (hflag)
		extra += 2;
	top = (char **) salloc((t + extra) * sizeof (char *));
	ap = top;
	*ap++ = "mail";
	if (rflag != NOSTR) {
		*ap++ = "-r";
		*ap++ = rflag;
	}
#ifdef DELIVERMAIL
	*ap++ = "-i";
	if (metoo)
		*ap++ = "-m";
#endif /* DELIVERMAIL */
	if (hflag) {
		*ap++ = "-h";
		sprintf(hbuf, "%d", hflag);
		*ap++ = savestr(hbuf);
	}
	while (n != NIL) {
		if (n->n_type & GDEL) {
			n = n->n_flink;
			continue;
		}
		*ap++ = n->n_name;
		n = n->n_flink;
	}
	*ap = NOSTR;
	return(top);
}

/*
 * See if the user named himself as a destination
 * for outgoing mail.  If so, set the global flag
 * selfsent so that we avoid removing his mailbox.
 */

void
mechk(names)
	struct name *names;
{
	register struct name *np;

	for (np = names; np != NIL; np = np->n_flink)
		if ((np->n_type & GDEL) == 0 && samebody(np->n_name, myname)) {
			selfsent++;
			return;
		}
}

/*
 * Remove all of the duplicates from the passed name list by
 * insertion sorting them, then checking for dups.
 * Return the head of the new list.
 */

struct name *
elide(names)
	struct name *names;
{
	register struct name *np, *t, *new;
	struct name *x;

	if (names == NIL)
		return(NIL);
	new = names;
	np = names;
	np = np->n_flink;
	if (np != NIL)
		np->n_blink = NIL;
	new->n_flink = NIL;
	while (np != NIL) {
		t = new;
		while (strcmp(t->n_name, np->n_name) < 0) {
			if (t->n_flink == NIL)
				break;
			t = t->n_flink;
		}

		/*
		 * If we ran out of t's, put the new entry after
		 * the current value of t.
		 */

		if (strcmp(t->n_name, np->n_name) < 0) {
			t->n_flink = np;
			np->n_blink = t;
			t = np;
			np = np->n_flink;
			t->n_flink = NIL;
			continue;
		}

		/*
		 * Otherwise, put the new entry in front of the
		 * current t.  If at the front of the list,
		 * the new guy becomes the new head of the list.
		 */

		if (t == new) {
			t = np;
			np = np->n_flink;
			t->n_flink = new;
			new->n_blink = t;
			t->n_blink = NIL;
			new = t;
			continue;
		}

		/*
		 * The normal case -- we are inserting into the
		 * middle of the list.
		 */

		x = np;
		np = np->n_flink;
		x->n_flink = t;
		x->n_blink = t->n_blink;
		t->n_blink->n_flink = x;
		t->n_blink = x;
	}

	/*
	 * Now the list headed up by new is sorted.
	 * Go through it and remove duplicates.
	 */

	np = new;
	while (np != NIL) {
		t = np;
		while (t->n_flink!=NIL &&
		    strcmp(np->n_name, t->n_flink->n_name) == 0)
			t = t->n_flink;
		if (t == np || t == NIL) {
			np = np->n_flink;
			continue;
		}
		
		/*
		 * Now t points to the last entry with the same name
		 * as np.  Make np point beyond t.
		 */

		np->n_flink = t->n_flink;
		if (t->n_flink != NIL)
			t->n_flink->n_blink = np;
		np = np->n_flink;
	}
	return(new);
}

/*
 * Put another node onto a list of names and return
 * the list.
 */

static struct name *
put(list, node)
	struct name *list, *node;
{
	node->n_flink = list;
	node->n_blink = NIL;
	if (list != NIL)
		list->n_blink = node;
	return(node);
}


/*
 * Delete the given name from a namelist.
 */
struct name *
delname(np, name)
	register struct name *np;
	char name[];
{
	register struct name *p;

	for (p = np; p != NIL; p = p->n_flink)
		if (samebody(name, p->n_name)) {
			if (p->n_blink == NIL) {
				if (p->n_flink != NIL)
					p->n_flink->n_blink = NIL;
				np = p->n_flink;
				continue;
			}
			if (p->n_flink == NIL) {
				if (p->n_blink != NIL)
					p->n_blink->n_flink = NIL;
				continue;
			}
			p->n_blink->n_flink = p->n_flink;
			p->n_flink->n_blink = p->n_blink;
		}
	return(np);
}

/*
 * Call the given routine on each element of the name
 * list, replacing said value if need be.
 */

void
mapf(np, from)
	register struct name *np;
	char *from;
{
	register struct name *p;

	if (debug) fprintf(stderr, "mapf %lx, %s\n", (long)np, from);
	for (p = np; p != NIL; p = p->n_flink)
		if ((p->n_type & GDEL) == 0) {
			p->n_name = netmap(p->n_name, from);
			p->n_full = splice(p->n_name, p->n_full);
		}
	if (debug) fprintf(stderr, "mapf %s done\n", from);
}
