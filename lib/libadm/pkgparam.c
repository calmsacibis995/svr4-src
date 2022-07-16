/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:pkgparam.c	1.1.3.1"

#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkginfo.h>
#include <pkglocs.h>

extern int	errno;
extern void	*calloc(), *realloc();
extern void	free();

#define VALSIZ	128
#define NEWLINE	'\n'
#define ESCAPE	'\\'

static char sepset[] =	":=\n";
static char qset[] = 	"'\"";

char *pkgdir = PKGLOC;
char *pkgfile = NULL;

char *
fpkgparam(fp, param)
FILE	*fp;
char	*param;
{
	char	ch, buffer[VALSIZ];
	char	*mempt, *copy;
	int	c, n, escape, begline, quoted;

	if(param == NULL) {
		errno = ENOENT;
		return(NULL);
	}

	mempt = NULL;

	for(;;) {
		copy = buffer;
		n = 0;
		while((c = getc(fp)) != EOF) {
			ch = (char) c;
			if(strchr(sepset, ch))
				break;
			if(++n < VALSIZ)
				*copy++ = ch;
		}
		if(c == EOF) {
			errno = EINVAL;
			return(NULL); /* no more entries left */
		} else if(c == NEWLINE)
			continue;
		*copy = '\0';
		if(buffer[0] == '#')
			copy = NULL;
		else {
			if(param[0] == '\0') {
				(void) strcpy(param, buffer);
				copy = buffer;
			} else if(strcmp(param, buffer))
				copy = NULL;
			else
				copy = buffer;
		}

		n = quoted = escape = 0;
		begline = 1;
		while((c = getc(fp)) != EOF) {
			ch = (char) c;
			if(begline && ((ch == ' ') || (ch == '\t')))
				continue; /* ignore leading white space */

			if(ch == NEWLINE) {
				if(!escape)
					break; /* end of entry */
				if(copy) {
					if(escape) {
						copy--; /* eat previous esc */
						n--;
					}
					*copy++ = NEWLINE;
				}
				escape = 0;
				begline = 1; /* new input line */
			} else {
				if(!escape && strchr(qset, ch)) {
					/* handle quotes */
					if(begline) {
						quoted++;
						begline = 0;
						continue;
					} else if(quoted) {
						quoted = 0;
						continue;
					}
				}
				if(ch == ESCAPE)
					escape++;
				else if(escape)
					escape = 0;
				if(copy) *copy++ = ch;
				begline = 0;
			}

			if(copy && ((++n % VALSIZ) == 0)) {
				if(mempt) {
					mempt = (char *) realloc(mempt, 
						(n+VALSIZ)*sizeof(char));
					if(!mempt)
						return(NULL);
				} else {
					mempt = calloc(2*VALSIZ, sizeof(char));
					if(!mempt)
						return(NULL);
					strncpy(mempt, buffer, n);
				}
				copy = &mempt[n];
			}
		}
		if(quoted) {
			if(mempt)
				(void) free(mempt);
			errno = EFAULT; /* missing closing quote */
			return(NULL);
		}
		if(copy) {
			*copy = '\0';
			break;
		}
		if(c == EOF) {
			errno = EINVAL; /* parameter not found */
			return(NULL);
		}
	}

	if(!mempt)
		mempt = strdup(buffer);
	else
		mempt = realloc(mempt, (strlen(mempt)+1)*sizeof(char));
	return(mempt);
}

char *
pkgparam(pkg, param)
char *pkg;
char *param;
{
	static char lastfname[PATH_MAX];
	static FILE *fp = (FILE *)0;
	char *pt, *copy, *value, line[128];

	if(!pkgdir)
		pkgdir = PKGLOC;

	if(!pkg) {
		/* request to close file */
		if(fp) {
			(void) fclose(fp);
			fp = (FILE *)0;
		}
		return(NULL);
	}

	if(!param) {
		errno = ENOENT;
		return(NULL);
	}

	if(pkgfile)
		(void) strcpy(line, pkgfile); /* filename was passed */
	else
		(void) sprintf(line, "%s/%s/pkginfo", pkgdir, pkg);

	if(fp && strcmp(line, lastfname)) {
		/* different filename implies need for different fp */
		(void) fclose(fp);
		fp = (FILE *)0;
	} 
	if(!fp) {
		(void) strcpy(lastfname, line);
		if((fp = fopen(lastfname, "r")) == NULL)
			return(NULL);
	}

	/* if parameter is a null string, then the user is requesting us
	 * to find the value of the next available parameter for this
	 * package and to copy the parameter name into the provided string;
	 * if it is not, then it is a request for a specified parameter, in
	 * which case we rewind the file to start search from beginning
	 */
	if(param[0]) {
		/* new parameter request, so reset file position */
		if(fseek(fp, 0L, 0))
			return(NULL);
	}

	if(pt = fpkgparam(fp, param)) {
		if(!strcmp(param, "ARCH") || !strcmp(param, "CATEGORY")) {
			/* remove all whitespace from value */
			value = copy = pt;
			while(*value) {
				if(!isspace(*value))
					*copy++ = *value;
				value++;
			}
			*copy = '\0';
		}
		return(pt);
	}
	return(NULL);
}
