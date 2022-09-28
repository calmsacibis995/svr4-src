/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/ufsrestore/utilities.c	1.2.3.1"

#include "restore.h"

/*
 * Insure that all the components of a pathname exist.
 */
pathcheck(name)
	char *name;
{
	register char *cp;
	struct entry *ep;
	char *start;

	start = index(name, '/');
	if (start == 0)
		return;
	for (cp = start; *cp != '\0'; cp++) {
		if (*cp != '/')
			continue;
		*cp = '\0';
		ep = lookupname(name);
		if (ep == NIL) {
			ep = addentry(name, psearch(name), NODE);
			newnode(ep);
		}
		ep->e_flags |= NEW|KEEP;
		*cp = '/';
	}
}

/*
 * Change a name to a unique temporary name.
 */
mktempname(ep)
	register struct entry *ep;
{
	char oldname[MAXPATHLEN];

	if (ep->e_flags & TMPNAME)
		badentry(ep, "mktempname: called with TMPNAME");
	ep->e_flags |= TMPNAME;
	(void) strcpy(oldname, myname(ep));
	freename(ep->e_name);
	ep->e_name = savename(gentempname(ep));
	ep->e_namlen = strlen(ep->e_name);
	renameit(oldname, myname(ep));
}

/*
 * Generate a temporary name for an entry.
 */
char *
gentempname(ep)
	struct entry *ep;
{
	static char name[MAXPATHLEN];
	struct entry *np;
	long i = 0;

	for (np = lookupino(ep->e_ino); np != NIL && np != ep; np = np->e_links)
		i++;
	if (np == NIL)
		badentry(ep, "not on ino list");
	(void) sprintf(name, "%s%d%d", TMPHDR, i, ep->e_ino);
	return (name);
}

/*
 * Rename a file or directory.
 */
renameit(from, to)
	char *from, *to;
{
	if (rename(from, to) < 0) {
		fprintf(stderr, "Warning: cannot rename %s to %s", from, to);
		(void) fflush(stderr);
		perror("");
		return;
	}
	vprintf(stdout, "rename %s to %s\n", from, to);
}

/*
 * Create a new node (directory).
 */
newnode(np)
	struct entry *np;
{
	char *cp;

	if (np->e_type != NODE)
		badentry(np, "newnode: not a node");
	cp = myname(np);
	if (mkdir(cp, 0777) < 0) {
		np->e_flags |= EXISTED;
		fprintf(stderr, "Warning: ");
		(void) fflush(stderr);
		perror(cp);
		return;
	}
	vprintf(stdout, "Make node %s\n", cp);
}

/*
 * Remove an old node (directory).
 */
removenode(ep)
	register struct entry *ep;
{
	char *cp;

	if (ep->e_type != NODE)
		badentry(ep, "removenode: not a node");
	if (ep->e_entries != NIL)
		badentry(ep, "removenode: non-empty directory");
	ep->e_flags |= REMOVED;
	ep->e_flags &= ~TMPNAME;
	cp = myname(ep);
	if (rmdir(cp) < 0) {
		fprintf(stderr, "Warning: ");
		(void) fflush(stderr);
		perror(cp);
		return;
	}
	vprintf(stdout, "Remove node %s\n", cp);
}

/*
 * Remove a leaf.
 */
removeleaf(ep)
	register struct entry *ep;
{
	char *cp;

	if (ep->e_type != LEAF)
		badentry(ep, "removeleaf: not a leaf");
	ep->e_flags |= REMOVED;
	ep->e_flags &= ~TMPNAME;
	cp = myname(ep);
	if (unlink(cp) < 0) {
		fprintf(stderr, "Warning: ");
		(void) fflush(stderr);
		perror(cp);
		return;
	}
	vprintf(stdout, "Remove leaf %s\n", cp);
}

/*
 * Create a link.
 */
linkit(existing, new, type)
	char *existing, *new;
	int type;
{

	if (type == SYMLINK) {
		if (symlink(existing, new) < 0) {
			fprintf(stderr,
				"Warning: cannot create symbolic link %s->%s: ",
				new, existing);
			(void) fflush(stderr);
			perror("");
			return (FAIL);
		}
	} else if (type == HARDLINK) {
		if (link(existing, new) < 0) {
			fprintf(stderr,
				"Warning: cannot create hard link %s->%s: ",
				new, existing);
			(void) fflush(stderr);
			perror("");
			return (FAIL);
		}
	} else {
		panic("linkit: unknown type %d\n", type);
		return (FAIL);
	}
	vprintf(stdout, "Create %s link %s->%s\n",
		type == SYMLINK ? "symbolic" : "hard", new, existing);
	return (GOOD);
}

/*
 * find lowest number file (above "start") that needs to be extracted
 */
ino_t
lowerbnd(start)
	ino_t start;
{
	register struct entry *ep;

	for ( ; start < maxino; start++) {
		ep = lookupino(start);
		if (ep == NIL || ep->e_type == NODE)
			continue;
		if (ep->e_flags & (NEW|EXTRACT))
			return (start);
	}
	return (start);
}

/*
 * find highest number file (below "start") that needs to be extracted
 */
ino_t
upperbnd(start)
	ino_t start;
{
	register struct entry *ep;

	for ( ; start > UFSROOTINO; start--) {
		ep = lookupino(start);
		if (ep == NIL || ep->e_type == NODE)
			continue;
		if (ep->e_flags & (NEW|EXTRACT))
			return (start);
	}
	return (start);
}

/*
 * report on a badly formed entry
 */
badentry(ep, msg)
	register struct entry *ep;
	char *msg;
{

	fprintf(stderr, "bad entry: %s\n", msg);
	fprintf(stderr, "name: %s\n", myname(ep));
	fprintf(stderr, "parent name %s\n", myname(ep->e_parent));
	if (ep->e_sibling != NIL)
		fprintf(stderr, "sibling name: %s\n", myname(ep->e_sibling));
	if (ep->e_entries != NIL)
		fprintf(stderr, "next entry name: %s\n", myname(ep->e_entries));
	if (ep->e_links != NIL)
		fprintf(stderr, "next link name: %s\n", myname(ep->e_links));
	if (ep->e_next != NIL)
		fprintf(stderr, "next hashchain name: %s\n", myname(ep->e_next));
	fprintf(stderr, "entry type: %s\n",
		ep->e_type == NODE ? "NODE" : "LEAF");
	fprintf(stderr, "inode number: %ld\n", ep->e_ino);
	panic("flags: %s\n", flagvalues(ep));
}

/*
 * Construct a string indicating the active flag bits of an entry.
 */
char *
flagvalues(ep)
	register struct entry *ep;
{
	static char flagbuf[BUFSIZ];

	(void) strcpy(flagbuf, "|NIL");
	flagbuf[0] = '\0';
	if (ep->e_flags & REMOVED)
		(void) strcat(flagbuf, "|REMOVED");
	if (ep->e_flags & TMPNAME)
		(void) strcat(flagbuf, "|TMPNAME");
	if (ep->e_flags & EXTRACT)
		(void) strcat(flagbuf, "|EXTRACT");
	if (ep->e_flags & NEW)
		(void) strcat(flagbuf, "|NEW");
	if (ep->e_flags & KEEP)
		(void) strcat(flagbuf, "|KEEP");
	if (ep->e_flags & EXISTED)
		(void) strcat(flagbuf, "|EXISTED");
	return (&flagbuf[1]);
}

/*
 * Check to see if a name is on a dump tape.
 */
ino_t
dirlookup(name)
	char *name;
{
	ino_t ino;

	ino = psearch(name);
	if (ino == 0 || BIT(ino, dumpmap) == 0)
		fprintf(stderr, "%s is not on tape\n", name);
	return (ino);
}

/*
 * Elicit a reply.
 */
reply(question)
	char *question;
{
	char c;

	do	{
		fprintf(stderr, "%s? [yn] ", question);
		(void) fflush(stderr);
		c = getc(terminal);
		while (c != '\n' && getc(terminal) != '\n')
			if (feof(terminal))
				return (FAIL);
	} while (c != 'y' && c != 'n');
	if (c == 'y')
		return (GOOD);
	return (FAIL);
}

/*
 * handle unexpected inconsistencies
 */
/* VARARGS1 */
panic(msg, d1, d2)
	char *msg;
	long d1, d2;
{

	fprintf(stderr, msg, d1, d2);
	if (reply("abort") == GOOD) {
		if (reply("dump core") == GOOD)
			abort();
		done(1);
	}
}
