/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:optim.c	1.5.6.3"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Network name modification routines.
 */

#include "rcv.h"
#include "configdefs.h"

static char		*arpafix();
static char		*lasthost();
static char		*makeremote();
static int		mstash();
static int		mtype();
static int		netlook();
static int		nettype();
static int		ntype();
static char		*tackon();
static struct xtrahash	*xlocate();

/*
 * Map a name into the correct network "view" of the
 * name.  This is done by prepending the name with the
 * network address of the sender, then optimizing away
 * nonsense.
 */

char *
netmap(name, from)
	char name[], from[];
{
	char nbuf[BUFSIZ], ret[BUFSIZ];
	register char *cp, *oname;

	if (debug) fprintf(stderr, "netmap(name '%s', from '%s')\n", name, from);
	if (strlen(from) == 0)
		return(name);
	if (any('@', name) || any('%', name))
		return(arpafix(name, from));
	if (any('@', from) || any('%', from))
		return(unuucp(makeremote(name, from)));
	if (value("onehop") && (cp = strchr(name, '!')) && cp > name)
		strcpy(nbuf, name);
	else {
		from = tackon(host, from);
		*strrchr(from, '!') = 0;
		name = tackon(lasthost(from), name);
		while (((cp = lasthost(from)) != 0) && ishost(cp, name)) {
			oname = name;
			name = strchr(name, '!') + 1;
			if (cp == from) {
				from[strlen(from)] = '!';
				if (value("mustbang") && !strchr(name, '!'))
					name = oname;
				return(unuucp(name));
			}
			*--cp = 0;
		}
		from[strlen(from)] = '!';
		from = strchr(from, '!') + 1;
		sprintf(nbuf, "%s!%s", from, name);
	}
	strcpy(ret, nbuf);
	cp = ret;
	if (debug) fprintf(stderr, "wind up with '%s'\n", name);
	if (!icequal(name, cp))
		return(unuucp(savestr(cp)));
	return(unuucp(name));
}

/*
 * Stick a host on the beginning of a uucp
 * address if it isn't there already.
 */
static char *
tackon(sys, rest)
	char *sys, *rest;
{
	while (*rest == '!')
		rest++;
	if (!ishost(sys, rest)) {
		char *r = salloc(strlen(sys) + strlen(rest) + 2);
		sprintf(r, "%s!%s", sys, rest);
		rest = r;
	}
	return rest;
}

/*
 * Check equality of the first host in a uucp address.
 */
ishost(sys, rest)
	char *sys, *rest;
{
	while (*sys && *sys == *rest)
		sys++, rest++;
	return(*sys == 0 && *rest == '!');
}

/*
 * Return last host in a uucp address.
 */
static char *
lasthost(addr)
	char *addr;
{
	char *r = strrchr(addr, '!');
	return r ? ++r : addr;
}

/*
 * Optionally translate an old format uucp name into a new one, e.g.
 * "mach1!mach2!user" becomes "user@mach2.UUCP".  This optional because
 * some information is necessarily lost (e.g. the route it got here
 * via) and if we don't have the host in our routing tables, we lose.
 */
char *
unuucp(name)
char *name;
{
	register char *np, *hp, *cp;
	char result[100];
	char tname[300];

	if (UnUUCP==0 &&
	    ((cp = value("conv"))==NOSTR || strcmp(cp, "internet")))
		return name;
	if (debug) fprintf(stderr, "unuucp(%s)\n", name);
	strcpy(tname, name);
	np = strrchr(tname, '!');
	if (np == NOSTR)
		return name;
	*np++ = 0;
	hp = strrchr(tname, '!');
	if (hp == NOSTR)
		hp = tname;
	else
		*hp++ = 0;
	cp = strchr(np, '@');
	if (cp == NOSTR)
		cp = strchr(np, '%');
	if (cp)
		*cp = 0;
	if (debug) fprintf(stderr, "host %s, name %s\n", hp, np);
	sprintf(result, "%s@%s.UUCP", np, hp);
	if (debug) fprintf(stderr, "unuucp returns %s\n", result);
	return savestr(result);
}

/*
 * Turn a network machine name into a unique character
 */
static int
netlook(machine, attnet)
	char machine[];
{
	register struct netmach *np;
	register char *cp, *cp2;
	char nbuf[BUFSIZ];

	/*
	 * Make into lower case.
	 */
	for (cp = machine, cp2 = nbuf;
	     *cp && cp2 < &nbuf[BUFSIZ-1];
	     *cp2++ = tolower(*cp++))
		/*nothing*/;
	*cp2 = 0;

	/*
	 * If a single letter machine, look through those first.
	 */

	if (strlen(nbuf) == 1)
		for (np = netmach; np->nt_mid != 0; np++)
			if (np->nt_mid == nbuf[0])
				return(nbuf[0]);

	/*
	 * Look for usual name
	 */

	for (np = netmach; np->nt_mid != 0; np++)
		if (strcmp(np->nt_machine, nbuf) == 0)
			return(np->nt_mid);

	/*
	 * Look in side hash table.
	 */

	return(mstash(nbuf, attnet));
}

/*
 * Deal with arpa net addresses.  The way this is done is strange.
 * In particular, if the destination arpa net host is not Berkeley,
 * then the address is correct as stands.  Otherwise, we strip off
 * the trailing @Berkeley, then cook up a phony person for it to
 * be from and optimize the result.
 */
static char *
arpafix(name, from)
	char name[];
	char from[];
{
	register char *cp;
	register int arpamach;
	char	*newname;

	if (debug) {
		fprintf(stderr, "arpafix(%s, %s)\n", name, from);
	}
	cp = strrchr(name, '@');
	if (cp == NOSTR)
		cp = strrchr(name, '%');
	if (cp == NOSTR) {
		fprintf(stderr, "Somethings amiss -- no @ or %% in arpafix\n");
		return(name);
	}
	cp++;
	arpamach = netlook(cp, '@');
	if (debug) fprintf(stderr, "cp '%s', arpamach %o, nettypes arpamach %o LOCAL %o\n", cp, arpamach, nettype(arpamach), nettype(LOCAL));
	if (arpamach == 0) {
		if (debug)
			fprintf(stderr, "machine %s unknown, uses: %s\n", cp, name);
		return(name);
	}
	if (((nettype(arpamach) & nettype(LOCAL)) & ~AN) == 0) {
		if (debug)
			fprintf(stderr, "machine %s known but remote, uses: %s\n",
			    cp, name);
		return(name);
	}
	newname = savestr(name);
	cp = strrchr(newname, '@');
	if (cp == NOSTR)
		cp = strrchr(newname, '%');
	*cp = 0;
	if (debug) fprintf(stderr, "local address, return '%s'\n", newname);
	return(savestr(newname));
}

/*
 * We have name with no @'s in it, and from with @'s.
 * Assume that name is meaningful only on the site in from.
 */
static char *
makeremote(name, from)
	char name[];
	char from[];
{
	register char *cp;
	static char rbuf[200];

	if (debug) fprintf(stderr, "makeremote(%s, %s) returns ", name, from);
	strcpy(rbuf, name);
	cp = strrchr(from, '@');
	if (cp == NOSTR)
		cp = strrchr(from, '%');
	strcat(rbuf, cp);
	if (debug) fprintf(stderr, "%s\n", rbuf);
	return rbuf;
}

/*
 * Take a network machine descriptor and find the types of connected
 * nets and return it.
 */
static int
nettype(mid)
{
	register struct netmach *np;

	if (mid & 0200)
		return(mtype(mid));
	for (np = netmach; np->nt_mid != 0; np++)
		if (np->nt_mid == mid)
			return(np->nt_type);
	return(0);
}

/*
 * Hashing routines to salt away machines seen scanning
 * networks paths that we don't know about.
 */

#define	XHSIZE		19		/* Size of extra hash table */
#define	NXMID		(XHSIZE*3/4)	/* Max extra machines */

struct xtrahash {
	char	*xh_name;		/* Name of machine */
	short	xh_mid;			/* Machine ID */
	short	xh_attnet;		/* Attached networks */
} xtrahash[XHSIZE];

static struct xtrahash	*xtab[XHSIZE];		/* F: mid-->machine name */

static short	midfree;			/* Next free machine id */

/*
 * Initialize the extra host hash table.
 * Called by sreset.
 */
void
minit()
{
	register struct xtrahash *xp, **tp;

	midfree = 0;
	tp = &xtab[0];
	for (xp = &xtrahash[0]; xp < &xtrahash[XHSIZE]; xp++) {
		xp->xh_name = NOSTR;
		xp->xh_mid = 0;
		xp->xh_attnet = 0;
		*tp++ = (struct xtrahash *) 0;
	}
}

/*
 * Stash a net name in the extra host hash table.
 * If a new entry is put in the hash table, deduce what
 * net the machine is attached to from the net character.
 *
 * If the machine is already known, add the given attached
 * net to those already known.
 */
static int
mstash(name, attnet)
	char name[];
{
	register struct xtrahash *xp;
	int x;

	xp = xlocate(name);
	if (xp == (struct xtrahash *) 0) {
		printf("Ran out of machine id spots\n");
		return(0);
	}
	if (xp->xh_name == NOSTR) {
		if (midfree >= XHSIZE) {
			printf("Out of machine ids\n");
			return(0);
		}
		xtab[midfree] = xp;
		xp->xh_name = savestr(name);
		xp->xh_mid = 0200 + midfree++;
	}
	x = ntype(attnet);
	if (x == 0)
		xp->xh_attnet |= AN;
	else
		xp->xh_attnet |= x;
	return(xp->xh_mid);
}

/*
 * Search for the given name in the hash table
 * and return the pointer to it if found, or to the first
 * empty slot if not found.
 *
 * If no free slots can be found, return 0.
 */

static struct xtrahash *
xlocate(name)
	char name[];
{
	register int h, q, i;
	register char *cp;
	register struct xtrahash *xp;

	for (h = 0, cp = name; *cp; h = (h << 2) + *cp++)
		;
	if (h < 0 && (h = -h) < 0)
		h = 0;
	h = h % XHSIZE;
	cp = name;
	for (i = 0, q = 0; q < XHSIZE; i++, q = i * i) {
		xp = &xtrahash[(h + q) % XHSIZE];
		if (xp->xh_name == NOSTR)
			return(xp);
		if (strcmp(cp, xp->xh_name) == 0)
			return(xp);
		if (h - q < 0)
			q += XHSIZE;
		xp = &xtrahash[(h - q) % XHSIZE];
		if (xp->xh_name == NOSTR)
			return(xp);
		if (strcmp(cp, xp->xh_name) == 0)
			return(xp);
	}
	return((struct xtrahash *) 0);
}

/*
 * Return the bit mask of net's that the given extra host machine
 * id has so far.
 */
static int
mtype(mid)
{
	register int m;

	if ((mid & 0200) == 0)
		return(0);
	m = mid & 0177;
	if (m >= midfree) {
		printf("Use made of undefined machine id\n");
		return(0);
	}
	return(xtab[m]->xh_attnet);
}

/*
 * Return the network of the separator --
 *	AN for arpa net
 *	BN for Bell labs net	(e.g. UUCP, NOT Berknet)
 *	SN for Schmidt net	(Berknet)
 *	0 if we don't know.
 */
static int
ntype(nc)
	register int nc;
{
	register struct ntypetab *np;

	for (np = ntypetab; np->nt_char != 0; np++)
		if (np->nt_char == nc)
			return(np->nt_bcode);
	return(0);
}

/*
 * Add a single character onto a string.
 */

void
stradd(str, c)
	register char *str;
	register int c;
{
	while (*str)
		str++;
	*str++ = (char)c;
	*str = '\0';
}
