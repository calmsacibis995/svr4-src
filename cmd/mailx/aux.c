/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:aux.c	1.14.6.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Auxiliary functions.
 */

static char	*phrase();
static char	*ripoff();

/*
 * Return a pointer to a dynamic copy of the argument.
 */

char *
savestr(str)
	char *str;
{
	register char *cp, *cp2, *top;

	for (cp = str; *cp; cp++)
		;
	top = salloc((unsigned)(cp-str + 1));
	if (top == NOSTR)
		return(NOSTR);
	for (cp = str, cp2 = top; *cp; cp++)
		*cp2++ = *cp;
	*cp2 = 0;
	return(top);
}

/*
 * Announce a fatal error and die.
 */

panic(str)
	char *str;
{
	fprintf(stderr,"mailx: Panic - %s\n",str);
	exit(1);
	/* NOTREACHED */
}

/*
 * Touch the named message by setting its MTOUCH flag.
 * Touched messages have the effect of not being sent
 * back to the system mailbox on exit.
 */

void
touch(mesg)
{
	register struct message *mp;

	if (mesg < 1 || mesg > msgCount)
		return;
	mp = &message[mesg-1];
	mp->m_flag |= MTOUCH;
	if ((mp->m_flag & MREAD) == 0)
		mp->m_flag |= MREAD|MSTATUS;
}

/*
 * Test to see if the passed file name is a directory.
 * Return true if it is.
 */

isdir(name)
	char name[];
{
	struct stat sbuf;

	if (stat(name, &sbuf) < 0)
		return(0);
	return((sbuf.st_mode & S_IFMT) == S_IFDIR);
}

/*
 * Count the number of arguments in the given string raw list.
 */

argcount(argv)
	char **argv;
{
	register char **ap;

	for (ap = argv; *ap != NOSTR; ap++)
		;	
	return(ap-argv);
}

/*
 * Given a file address, determine the
 * block number it represents.
 */

blockof(off)
	off_t off;
{
	off_t a;

	a = off >> 9;
	a &= 077777;
	return((int) a);
}

/*
 * Take a file address, and determine
 * its offset in the current block.
 */

offsetofaddr(off)
	off_t off;
{
	off_t a;

	a = off & 0777;
	return((int) a);
}

/*
 * Return the desired header line from the passed message
 * pointer (or NOSTR if the desired header field is not available).
 */

char *
hfield(field, mp, add)
	char field[];
	struct message *mp;
	char *(*add)();
{
	register FILE *ibuf;
	char linebuf[LINESIZE];
	register long lc;
	char *r = NOSTR;

	ibuf = setinput(mp);
	if ((lc = mp->m_lines) <= 0) 
		return(NOSTR);
	if (readline(ibuf, linebuf) < 0)
		return(NOSTR);
	lc--;
	while ((lc = gethfield(ibuf, linebuf, lc)) >= 0)
		if (ishfield(linebuf, field))
			r = (*add)(r, hcontents(linebuf));
	return r;
}

/*
 * Return the next header field found in the given message.
 * Return > 0 if something found, <= 0 elsewise.
 * Must deal with \ continuations & other such fraud.
 */

gethfield(f, linebuf, rem)
	register FILE *f;
	char linebuf[];
	register long rem;
{
	char line2[LINESIZE];
	register char *cp, *cp2;
	register int c;

	for (;;) {
		if (rem <= 0)
			return(-1);
		if (readline(f, linebuf) < 0)
			return(-1);
		rem--;
		if (strlen(linebuf) == 0)
			return(-1);
		if (isspace(linebuf[0]))
			continue;
		if (!headerp(linebuf))
			return(-1);
		
		/*
		 * I guess we got a headline.
		 * Handle wraparounding
		 */
		
		for (;;) {
			if (rem <= 0)
				break;
			c = getc(f);
			ungetc(c, f);
			if (!isspace(c) || c == '\n')
				break;
			if (readline(f, line2) < 0)
				break;
			rem--;
			cp2 = line2;
			for (cp2 = line2; *cp2 != 0 && isspace(*cp2); cp2++)
				;
			if (strlen(linebuf) + strlen(cp2) >= LINESIZE-2)
				break;
			cp = &linebuf[strlen(linebuf)];
			while (cp > linebuf &&
			    (isspace(cp[-1]) || cp[-1] == '\\'))
				cp--;
			*cp++ = ' ';
			for (cp2 = line2; *cp2 != 0 && isspace(*cp2); cp2++)
				;
			strcpy(cp, cp2);
		}
		if ((c = strlen(linebuf)) > 0) {
			cp = &linebuf[c-1];
			while (cp > linebuf && isspace(*cp))
				cp--;
			*++cp = 0;
		}
		return(rem);
	}
	/* NOTREACHED */
}

/*
 * Check whether the passed line is a header line of
 * the desired breed.
 */

ishfield(linebuf, field)
	char linebuf[], field[];
{
	register char *cp;
	register char c;

	if ((cp = strchr(linebuf, ':')) == NOSTR)
		return(0);
	if (cp == linebuf)
		return(0);
	cp--;
	while (cp > linebuf && isspace(*cp))
		cp--;
	c = *++cp;
	*cp = 0;
	if (icequal(linebuf, field)) {
		*cp = c;
		return(1);
	}
	*cp = c;
	return(0);
}

/*
 * Extract the non label information from the given header field
 * and return it.
 */

char *
hcontents(hfield)
	char hfield[];
{
	register char *cp;

	if ((cp = strchr(hfield, ':')) == NOSTR)
		return(NOSTR);
	cp++;
	while (*cp && isspace(*cp))
		cp++;
	return(cp);
}

/*
 * Compare two strings, ignoring case.
 */

icequal(s1, s2)
	register char *s1, *s2;
{

	while (toupper(*s1++) == toupper(*s2))
		if (*s2++ == 0)
			return(1);
	return(0);
}

/*
 * Copy a string, lowercasing it as we go.
 */
void
istrcpy(dest, src)
	char *dest, *src;
{
	register char *cp, *cp2;

	cp2 = dest;
	cp = src;
	do {
		*cp2++ = tolower(*cp);
	} while (*cp++ != 0);
}

/*
 * The following code deals with input stacking to do source
 * commands.  All but the current file pointer are saved on
 * the stack.
 */

static	int	ssp = -1;		/* Top of file stack */
static struct sstack {
	FILE	*s_file;		/* File we were in. */
	int	s_cond;			/* Saved state of conditionals */
	int	s_loading;		/* Loading .mailrc, etc. */
} *sstack;

/*
 * Pushdown current input file and switch to a new one.
 * Set the global flag "sourcing" so that others will realize
 * that they are no longer reading from a tty (in all probability).
 */

source(name)
	char name[];
{
	register FILE *fi;
	register char *cp;

	if ((cp = expand(name)) == NOSTR)
		return(1);
	if ((fi = fopen(cp, "r")) == NULL) {
		printf("Unable to open %s\n",cp);
		return(1);
	}

	if ( !maxfiles ) {
		if ( (maxfiles=ulimit(4, 0)) < 0 )
#ifndef _NFILE
# define _NFILE 20
#endif
			maxfiles = _NFILE;
		sstack = (struct sstack *)calloc(maxfiles, sizeof(struct sstack));
		if ( sstack == NULL ) {
			printf("Couldn't allocate memory for sourcing stack\n");
			fclose(fi);
			return(1);
		}
	}

	sstack[++ssp].s_file = input;
	sstack[ssp].s_cond = cond;
	sstack[ssp].s_loading = loading;
	loading = 0;
	cond = CANY;
	input = fi;
	sourcing++;
	return(0);
}

/*
 * Pop the current input back to the previous level.
 * Update the "sourcing" flag as appropriate.
 */

unstack()
{
	if (ssp < 0) {
		printf("\"Source\" stack over-pop.\n");
		sourcing = 0;
		return(1);
	}
	fclose(input);
	if (cond != CANY)
		printf("Unmatched \"if\"\n");
	cond = sstack[ssp].s_cond;
	loading = sstack[ssp].s_loading;
	input = sstack[ssp--].s_file;
	if (ssp < 0)
		sourcing = loading;
	return(0);
}

/*
 * Touch the indicated file.
 * This is nifty for the shell.
 * If we have the utime() system call, this is better served
 * by using that, since it will work for empty files.
 * On non-utime systems, we must sleep a second, then read.
 */

void
alter(name)
	char name[];
{
	int rc = utime(name, utimep);

	if (rc != 0) {
		fprintf(stderr, "Cannot utime %s in aux:alter\n", name);
		fprintf(stderr, "Errno: %d\n", errno);
	}
}

/*
 * Examine the passed line buffer and
 * return true if it is all blanks and tabs.
 */

blankline(linebuf)
	char linebuf[];
{
	register char *cp;

	for (cp = linebuf; *cp; cp++)
		if (!any(*cp, " \t"))
			return(0);
	return(1);
}

/*
 * Skin an arpa net address according to the RFC 822 interpretation
 * of "host-phrase."
 */
static char *
phrase(name, token, comma)
	char *name;
{
	register char c;
	register char *cp, *cp2;
	char *bufend;
	int gotlt, lastsp;
	char nbuf[LINESIZE];
	int nesting;

	if (name == NOSTR)
		return(NOSTR);
	gotlt = 0;
	lastsp = 0;
	bufend = nbuf;
	for (cp = name, cp2 = bufend; (c = *cp++) != 0;) {
		switch (c) {
		case '(':
			/*
				Start of a comment, ignore it.
			*/
			nesting = 1;
			while ((c = *cp) != 0) {
				cp++;
				switch(c) {
				case '\\':
					if (*cp == 0) goto outcm;
					cp++;
					break;
				case '(':
					nesting++;
					break;
				case ')':
					--nesting;
					break;
				}
				if (nesting <= 0) break;
			}
		outcm:
			lastsp = 0;
			break;
		case '"':
			/*
				Start a quoted string.
				Copy it in its entirety.
			*/
			while ((c = *cp) != 0) {
				cp++;
				switch (c) {
				case '\\':
					if ((c = *cp) == 0) goto outqs;
					cp++;
					break;
				case '"':
					goto outqs;
				}
				*cp2++ = c;
			}
		outqs:
			lastsp = 0;
			break;

		case ' ':
		case '\t':
		case '\n':
			if (token && (!comma || c == '\n')) {
			done:
				cp[-1] = 0;
				return cp;
			}
			lastsp = 1;
			break;

		case ',':
			*cp2++ = c;
			if (gotlt != '<') {
				if (token)
					goto done;
				bufend = cp2 + 1;
				gotlt = 0;
			}
			break;

		case '<':
			cp2 = bufend;
			gotlt = c;
			lastsp = 0;
			break;

		case '>':
			if (gotlt == '<') {
				gotlt = c;
				break;
			}

			/* FALLTHROUGH . . . */

		default:
			if (gotlt == 0 || gotlt == '<') {
				if (lastsp) {
					lastsp = 0;
					*cp2++ = ' ';
				}
				*cp2++ = c;
			}
			break;
		}
	}
	*cp2 = 0;
	return (token ? --cp : equal(name, nbuf) ? name : savestr(nbuf));
}

char *
skin(name)
	char *name;
{
	return phrase(name, 0, 0);
}

char *
yankword(name, word, comma)
	char *name, *word;
{
	char *cp;

	if (name == 0)
		return 0;
	while (isspace(*name))
		name++;
	if (*name == 0)
		return 0;
	cp = phrase(name, 1, comma);
	strcpy(word, name);
	return cp;
}

docomma(s)
	char *s;
{
	return s && strpbrk(s, "(<,");
}

/*
 * Fetch the sender's name from the passed message.
 */

char *
nameof(mp)
	register struct message *mp;
{
	char namebuf[LINESIZE];
	char linebuf[LINESIZE];
	register char *cp, *cp2;
	register FILE *ibuf;
	int first = 1, wint = 0;

	if (value("from")
	 && ((cp = hfield("reply-to", mp, addto)) != NOSTR
	  || (cp = hfield("from", mp, addto)) != NOSTR))
		return ripoff(cp);
	ibuf = setinput(mp);
	copy("", namebuf);
	if (readline(ibuf, linebuf) <= 0)
		return(savestr(namebuf));
newname:
	for (cp = linebuf; *cp != ' '; cp++)
		;
	while (any(*cp, " \t"))
		cp++;
	for (cp2 = &namebuf[strlen(namebuf)]; *cp && !any(*cp, " \t") &&
	    cp2-namebuf < LINESIZE-1; *cp2++ = *cp++)
		;
	*cp2 = '\0';
	for (;;) {
		if (readline(ibuf, linebuf) <= 0)
			break;
		if (substr(linebuf,"forwarded by ") != -1)
			continue;
		if (linebuf[0] == 'F')
			cp = linebuf;
		else if (linebuf[0] == '>')
			cp = linebuf + 1;
		else
			break;
		if (strncmp(cp, "From ", 5) != 0)
			break;
		if ((wint = substr(cp, "remote from ")) != -1) {
			cp += wint + 12;
			if (first) {
				copy(cp, namebuf);
				first = 0;
			} else
				strcpy(strrchr(namebuf, '!')+1, cp);
			strcat(namebuf, "!");
			goto newname;
		} else
			break;
	}
	for (cp = namebuf; *cp == '!'; cp++);
	while (ishost(host, cp))
		cp = strchr(cp, '!') + 1;
	if (value("mustbang") && !strchr(cp, '!'))
		cp = strcat(strcat(strcpy(linebuf, host), "!"), cp);
	if (cp2 = hfield("from", mp, addto))
		return(splice(cp, cp2));
	else
		return(savestr(cp));
}

/*
 * Splice an address into a commented recipient header.
 */
char *
splice(addr, hdr)
	char *addr, *hdr;
{
	char buf[LINESIZE];
	char *cp;

	if (cp = strchr(hdr, '<')) {
		strncpy(buf, hdr, cp-hdr+1);
		buf[cp-hdr+1] = 0;
		strcat(buf, addr);
		if (cp = strchr(cp, '>'))
			strcat(buf, cp);
	} else if (cp = strchr(hdr, '(')) {
		strcpy(buf, addr);
		strcat(buf, " ");
		strcat(buf, cp);
	} else
		strcpy(buf, addr);
	return savestr(ripoff(buf));
}

static char *
ripoff(buf)
	register char *buf;
{
	register char *cp;

	cp = buf + strlen(buf);
	while (--cp >= buf && isspace(*cp));
	if (cp >= buf && *cp == ',')
		cp--;
	*++cp = 0;
	return buf;
}

/*
 * Are any of the characters in the two strings the same?
 */

anyof(s1, s2)
	register char *s1, *s2;
{
	register int c;

	while ((c = *s1++) != 0)
		if (any(c, s2))
			return(1);
	return(0);
}

/*
 * See if the given header field is supposed to be ignored.
 */
isign(field)
	char *field;
{
	char realfld[BUFSIZ];
	register int h;
	register struct ignore *igp;

	istrcpy(realfld, field);
	h = hash(realfld);
	for (igp = ignore[h]; igp != 0; igp = igp->i_link)
		if (strcmp(igp->i_field, realfld) == 0)
			return(1);
	return(0);
}
/*
	This routine looks for string2 in string1.
	If found, it returns the position string2 is found at,
	otherwise it returns a -1.
*/
substr(string1, string2)
char *string1, *string2;
{
	int i,j, len1, len2;

	len1 = strlen(string1);
	len2 = strlen(string2);
	for (i=0; i < len1 - len2 + 1; i++) {
		for (j = 0; j < len2 && string1[i+j] == string2[j]; j++);
		if (j == len2) return(i);
	}
	return(-1);
}
