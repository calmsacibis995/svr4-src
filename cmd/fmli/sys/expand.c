/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/expand.c	1.6"

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	"wish.h"
#include	"moremacros.h"
#include	"terror.h"
#include	"sizes.h"
#include 	"sizes.h"

/*
 * Globals to maintain dynamic expansion buffer
 * (destination buffer for "variable expanded" text)
 */
static char *Destbuf;
static int  Bufsize;
static int  Bufcnt;

/*
 * These macros check that the expansion buffer is BIG enough
 * before text is copies into it.
 */
#define CHKbuf(p, q)		if (++Bufcnt >= Bufsize) { \
					growbuf(q, BUFSIZ); \
					p = Destbuf + Bufcnt - 1; \
				}

#define CHKnbuf(p, q, num)	if ((Bufcnt += num) >= Bufsize) { \
					growbuf(q, num); \
					p = Destbuf + Bufcnt - num; \
				}

/*
 * GROWBUF will allocate/grow Destbuf by BUFSIZ
 */
static void 
growbuf(buf, num)
char *buf;
int num;
{

	Bufsize += num;
	if (Destbuf == NULL) {
		if ((Destbuf = malloc(Bufsize)) == NULL)
			fatal(NOMEM, nil);
		strcpy(Destbuf, buf);
	}
 	else if ((Destbuf = realloc(Destbuf, Bufsize)) == NULL)
		fatal(NOMEM, nil);
}

/*
 * EXPAND will "expand" all environment variables in the
 * string pointed to by "src" and return a pointer to
 * the expanded text.
 */ 
char *
expand(src)
char *src;
{
	char	buf[BUFSIZ];
	char	*ret;
	char	*pexpand();

	/*
	 * Use a static 1K buffer by default ....
	 * pexpand() will create a dynamic buffer
	 * if necessary and set "Destbuf" to it.
	 */ 
	Destbuf = NULL;
	Bufsize = BUFSIZ;
	Bufcnt = 0;

	(void) pexpand(buf, src, '\0');
	if (Destbuf)		
		ret = Destbuf;		/* return malloc'd buffer */
	else
		ret = strsave(buf);	/* strsave text from static buffer */
	return(ret);
}

static char *
pexpand(buf, name, eos)
char	*buf;
char	*name;
char	eos;
{
    register char	delim;
    register char	*src;
    register char	*dst;
    register char	*file;
    char	fbuf[PATHSIZ];
    char	*anyenv();
    char	*getepenv();
    char	*savebuf;
    int	savesize;
    int	savecnt;

    dst = buf;
    src = name;
    while (*src && *src != eos) {
	if (*src == '\\') {
	    ++src;
	    CHKbuf(dst, buf);
	    *dst++ = *src++;
	}
	else if (*src == '$') {
	    register char	*start;

	    if ((delim = (*++src == '{') ? '}' : '\0'))
		start = ++src;
	    else
		start = src;
	    file = NULL;
	    if (*src == '(') {
		/*
		 * Save dynamic buffer before calling
		 * pexpand() recursively
		 */
		savebuf = Destbuf;
		savesize = Bufsize;
		savecnt = Bufcnt;

		/*
		 * Initialize globals
		 */
		Destbuf = NULL;
		Bufsize = PATHSIZ;
		Bufcnt = 0;

		src = pexpand(fbuf, ++src, ')');
		if (*src) {
		    start = ++src;
		    if (Destbuf)
			file = Destbuf;
		    else
			file = fbuf;
		}

		/*
		 * Restore previous values for 
		 * dynamic buffer and continue
		 * as usual ....
		 */
		Destbuf = savebuf;
		Bufsize = savesize;
		Bufcnt = savecnt;
	    }
	    if (isalpha(*src)) {
		register char	*p;
		register char	savechar;

		while (isalpha(*src) || isdigit(*src) || *src == '_')
		    src++;
		savechar = *src;
		*src = '\0';
		if ((p = (file ? anyenv(file, start) : getepenv(start))) == NULL) {
		    if (delim) {
			if ((*src = savechar) == ':' && *++src == '-')
			    while (*++src && *src != delim) {
				CHKbuf(dst, buf);
				*dst++ = *src;
			    }
		    }
		    else
			*src = savechar;
		}
		else {
		    *src = savechar;
		    CHKnbuf(dst, buf, (int)strlen(p)); /* EFT k16 */
		    strcpy(dst, p);
		    dst += strlen(p);
		    free(p);
		}
		if (delim)
		    while (*src && *src++ != delim)
			;
	    }
	}
	else {
	    CHKbuf(dst, buf);
	    *dst++ = *src++;
	}
    }
    CHKbuf(dst, buf);
    *dst = '\0';
    return src;
}
