/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:myregexpr.c	1.4.3.1"
#include <string.h>
#include "myregexpr.h"
int regerrno;
#define INIT		register char *sp = instring;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return c
#define ERROR(c)	return ((char*)(regerrno = (c)), (char*)0)

#define braslist	_Sbraslist
#define braelist	_Sbraelist
#define compile		_Scompile
#define step		_Sstep
#define advance		_Sadvance

#ifdef __STDC__
static int getrnge(char *str);
extern int _Sadvance(char *lp, char *ep);
extern int _Sstep(char *p1, char *p2); 
extern char *_Scompile(char *instring, char *ep, char *endbuf, int seof);
#else
static int getrnge();
extern int _Sadvance();
extern int _Sstep(); 
extern char *_Scompile();
#endif

#include <regexp.h>

#undef braslist
#undef braelist
#undef compile
#undef step
#undef advance

char **braslist = &_Sbraslist[0];
char **braelist = &_Sbraelist[0];

#ifdef __STDC__
char *compile(char *instring, char *ep, char *endbuf)
#else
char *compile(instring, ep, endbuf)
register char *ep;
char *instring, *endbuf;
#endif
{
    char *ret;
    char *svep = ep;

    /* If there's a buffer to store the compiled string into, */
    /* let's use it. */
    if (ep)
	ret = _Scompile(instring, ep+1, endbuf, 0);

    else
	{
	/* Let's malloc space to hold the compiled string, */
	/* using an estimate of twice the pattern size. */
	int insize = strlen(instring);
	unsigned int compsize = insize;
	extern char *malloc(), *realloc();

	if (!insize)
		compsize = insize = 2;

	/* regerrno == 50 means that the compiled string is */
	/* too large for the buffer or the buffer cannot be allocated. */
	regerrno = 50;
	ret = 0;

	/* Loop until we succeed with a buffer large enough. */
	while (!ret && regerrno == 50)
	    {
	    compsize += insize;
	    ep = ep ? realloc(ep, compsize) : malloc(compsize);
	    if (!ep)
		    return 0;
	    ret = _Scompile(instring, ep+1, ep+compsize, 0);
	    }
	}

    /* The first byte gets an indication of the */
    /* pattern beginning with a circumflex (^) */
    if (ret)
	*ep = circf;

    loc1 = ret;
    return svep ? ret : ep;
}

#ifdef __STDC__
int step(char *lp, char *ep)
#else
int step(lp, ep)
register char *lp, *ep;
#endif
{
    circf = *ep++;
    return _Sstep(lp, ep);
}

#ifdef __STDC__
int advance(char *lp, char *ep)
#else
int advance(lp, ep)
register char *lp, *ep;
#endif
{
    circf = *ep++;
    return _Sadvance(lp, ep);
}
