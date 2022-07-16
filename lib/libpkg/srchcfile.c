/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:srchcfile.c	1.7.3.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <pkgstrct.h>

#define ERROR(s) \
	{ \
		errstr = (s); \
		(void) getend(fpin); \
		return(-1); \
	}

extern void	free();
extern void	*calloc();

static int	getstr(), 
		getnum(), 
		getend(), 
		eatwhite();

static char	mypath[PATH_MAX];
static char	mylocal[PATH_MAX];

srchcfile(ept, path, fpin, fpout)
struct cfent *ept;
char	*path;
FILE	*fpin, *fpout;
{
	struct pinfo *pinfo, *lastpinfo;
	long	pos;
	char	*pt, 
		pkgname[PKGSIZ+1],
		classname[CLSSIZ+1];
	int	c, n, rdpath, anypath;
	
	/* this code uses goto's instead of nested
	 * subroutines because execution time of this
	 * routine is especially critical to installation
	 */

	errstr = NULL;
	ept->volno = 0;
	ept->ftype = BADFTYPE;
	(void) strcpy(ept->class, BADCLASS);
	ept->path = NULL;
	ept->ainfo.local = NULL;
	ept->ainfo.mode = BADMODE;
	(void) strcpy(ept->ainfo.owner, BADOWNER);
	(void) strcpy(ept->ainfo.group, BADGROUP);
	ept->cinfo.size = ept->cinfo.cksum = ept->cinfo.modtime = BADCONT;

	/* free up list of packages which reference this entry */
	while(ept->pinfo) {
		pinfo = ept->pinfo->next;
		free(ept->pinfo);
		ept->pinfo = pinfo;
	}
	ept->pinfo = NULL;
	ept->npkgs = 0;

	/* if path to search for is "*", then we will return
	 * the first path we encounter as a match, otherwise
	 * we return an error
	 */
	anypath = 0;
	if(path && (path[0] != '/')) {
		if(!strcmp(path, "*"))
			anypath++;
		else {
			errstr = "illegal search path specified";
			return(-1);
		}
	}

	rdpath = 0;
	for(;;) {
		if(feof(fpin))
			return(0); /* no more entries */

		/* save current position in file */
		pos = ftell(fpin);

		/* grab path from first entry */
		c = getc(fpin);
		if(c != '/') {
			/* we check for EOF inside this if statement
			 * to reduce normal execution time
			 */
			if(c == EOF)
				return(0); /* no more entries */
			else if(isspace(c) || (c == '#') || (c == ':')) {
				/* line is a comment */
				(void) getend(fpin);
				continue;
			}

			/* we need to read this entry in the
			 * format which specifies
			 *	ftype class path
			 * so we set the rdpath variable and
			 * immediately jump to the code which
			 * will parse this format.  When done,
			 * that code will return to Path_Done below
			 */
			ungetc(c, fpin);
			rdpath = 1; 
			break;
		}

		/* copy first token into path element of passed structure */
		pt = mypath;
		do {
			if(strchr("= \t\n", c))
				break;
			*pt++ = (char) c;
		} while((c = getc(fpin)) != EOF);
		*pt = '\0';

		if(c == EOF)
			ERROR("incomplete entry")
		ept->path = mypath;

Path_Done:
		/* determine if we have read the pathname which
		 * identifies the entry we are searching for
		 */
		if(anypath) 
			n = 0; /* any pathname will do */
		else if(path)
			n = strcmp(path, ept->path);
		else
			n = 1; /* no pathname will match */

		if(n == 0) {
			/* we want to return information about this
			 * path in the structure provided, so
			 * parse any local path and jump to code
			 * which parses rest of the input line
			 */
			if(c == '=') {
				/* parse local path specification */
				if(getstr(fpin, NULL, PATH_MAX, mylocal))
					ERROR("unable to read local/link path")
				ept->ainfo.local = mylocal;
			}
			break; /* scan into a structure */
		} else if(n < 0) {
			/* the entry we want would fit BEFORE the
			 * one we just read, so we need to unread
			 * what we've read by seeking back to the
			 * start of this entry
			 */
			if(fseek(fpin, pos, 0)) {		
				errstr = "failure attempting fseek";
				return(-1);
			}
			return(2); /* path would insert here */
		}

		if(fpout) {
			/* copy what we've read and the rest of this
			 * line onto the specified output stream
			 */
			(void) fprintf(fpout, "%s%c", ept->path, c);
			if(rdpath) {
				(void) fprintf(fpout, "%c %s", ept->ftype,
					ept->class);
			}
			while((c = getc(fpin)) != EOF) {
				putc(c, fpout);
				if(c == '\n')
					break;
			}
		} else {
			/* since this isn't the entry we want, just read
			 * the stream until we find the end of this entry
			 * and then start this search loop again
			 */
			while((c = getc(fpin)) != EOF) {
				if(c == '\n')
					break;
			}
			if(c == EOF)
				ERROR("missing newline at end of entry")
		}
	}

	if(rdpath < 2) {
		/* since we are processing an oldstyle entry and
		 * we have already read ftype, class, and path
		 * we just jump into reading the other info
		 */

		switch(c = eatwhite(fpin)) {
		  case EOF:
			errstr = "incomplete entry";
			return(-1);

		  case '0':
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		  case '7':
		  case '8':
		  case '9':
			ERROR("volume number not expected")

		  case 'i':
			ERROR("ftype <i> not expected")

		  case '?':
		  case 'f':
		  case 'v':
		  case 'e':
		  case 'l':
		  case 's':
		  case 'p':
		  case 'c':
		  case 'b':
		  case 'd':
		  case 'x':
			ept->ftype = (char) c;
			if(getstr(fpin, NULL, CLSSIZ, ept->class))
				ERROR("unable to read class token")
			if(!rdpath)
				break; /* we already read the pathname */

			if(getstr(fpin, "=", PATH_MAX, mypath))
				ERROR("unable to read pathname field")
			ept->path = mypath;

			c = getc(fpin);
			rdpath++;
			goto Path_Done;

		  default:
			errstr = "unknown ftype";
	Error:
			(void) getend(fpin);
			return(-1);
		}
	}

	if(strchr("sl", ept->ftype) && (ept->ainfo.local == NULL))
		ERROR("no link source specified");

	if(strchr("cb", ept->ftype)) {
		ept->ainfo.major = BADMAJOR;
		ept->ainfo.minor = BADMINOR;
		if(getnum(fpin, 10, (long *)&ept->ainfo.major, BADMAJOR) ||
		   getnum(fpin, 10, (long *)&ept->ainfo.minor, BADMINOR))
			ERROR("unable to read major/minor device numbers")
	}

	if(strchr("cbdxpfve", ept->ftype)) {
		/* mode, owner, group should be here */
		if(getnum(fpin, 8, (long *)&ept->ainfo.mode, BADMODE) ||
		   getstr(fpin, NULL, ATRSIZ, ept->ainfo.owner) ||
		   getstr(fpin, NULL, ATRSIZ, ept->ainfo.group))
			ERROR("unable to read mode/owner/group")

		/* don't have to check (mode < 0) since '-' is not a legal */
		if((ept->ainfo.mode != BADMODE) && ((ept->ainfo.mode > 07777) ||
		(strchr("cbdxp", ept->ftype) && (ept->ainfo.mode > 02000))))
			ERROR("illegal value for mode")
	}

	if(strchr("ifve", ept->ftype)) {
		/* look for content description */
		if(getnum(fpin, 10, (long *)&ept->cinfo.size, BADCONT) ||
		getnum(fpin, 10, (long *)&ept->cinfo.cksum, BADCONT) ||
		getnum(fpin, 10, (long *)&ept->cinfo.modtime, BADCONT))
			ERROR("unable to read content info")
	}

	if(ept->ftype == 'i') {
		if(getend(fpin)) {
			errstr = "extra tokens on input line";
			return(-1);
		}
		return(1);
	}

	/* determine list of packages which reference this entry */
	lastpinfo = (struct pinfo *)0;
	while((c = getstr(fpin, ":\\", PKGSIZ, pkgname)) <= 0) {
		if(c < 0)
			ERROR("package name too long")
		else if(c == 0) {
			/* a package was listed */
			pinfo = (struct pinfo *)calloc(1, sizeof(struct pinfo));
			if(!pinfo)
				ERROR("no memory for package information")
			if(!lastpinfo)
				ept->pinfo = pinfo; /* first one */
			else
				lastpinfo->next = pinfo; /* link list */
			lastpinfo = pinfo;

			if(strchr("-+*~!", pkgname[0])) {
				pinfo->status = pkgname[0];
				(void) strcpy(pinfo->pkg, pkgname+1);
			} else
				(void) strcpy(pinfo->pkg, pkgname);

			/*pkg/[:[ftype][:class] */
			c = getc(fpin);
			if(c == '\\') {
				/* get alternate ftype */
				pinfo->editflag++;
				c = getc(fpin);
			}

			if(c == ':') {
				/* get special classname */
				(void) getstr(fpin, "", 12, classname);
				(void) strcpy(pinfo->aclass, classname);
				c = getc(fpin);
			}
			ept->npkgs++;

			if((c == '\n') || (c == EOF))
				return(1);
			else if(!isspace(c))
				ERROR("bad end of entry")
		}
	}

	if(getend(fpin) && ept->pinfo) {
		errstr = "extra token(s) on input line";
		return(-1);
	}
	return(1);
}

static int
getnum(fp, base, d, bad)
FILE *fp;
int base;
long *d;
long bad;
{
	int c;

	/* leading white space ignored */
	c = eatwhite(fp);
	if(c == '?') {
		*d = bad;
		return(0);
	}

	if((c == EOF) || (c == '\n') || !isdigit(c)) {
		(void) ungetc(c, fp);
		return(1);
	}

	*d = 0;
	while(isdigit(c)) {
		*d = (*d * base) + (c & 017);
		c = getc(fp);
	}
	(void) ungetc(c, fp);
	return(0);
}

static int
getstr(fp, sep, n, str)
FILE *fp;
int n;
char *sep, *str;
{
	int c;

	/* leading white space ignored */
	c = eatwhite(fp);
	if((c == EOF) || (c == '\n')) {
		(void) ungetc(c, fp);
		return(1); /* nothing there */
	}

	/* fill up string until space, tab, or separator */
	while(!strchr(" \t", c) && (!sep || !strchr(sep, c))) {
		if(n-- < 1) {
			*str = '\0';
			return(-1); /* too long */
		}
		*str++ = (char) c;
		c = getc(fp);
		if((c == EOF) || (c == '\n'))
			break; /* no more on this line */
	}
	*str = '\0';
	(void) ungetc(c, fp);
	return(0);
}

static int
getend(fp)
FILE *fp;
{
	int c;
	int n;

	n = 0;
	do {
		if((c = getc(fp)) == EOF)
			return(n);
		if(!isspace(c))
			n++;
	} while(c != '\n');
	return(n);
}
	
static int
eatwhite(fp)
FILE *fp;
{
	int c;

	/* this test works around a side effect of getc() */
	if(feof(fp))
		return(EOF);
	do
		c = getc(fp);
	while((c == ' ') || (c == '\t'));
	return(c);
}
