/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)boot:boot/at386/default.c	1.1.4.1"

#include "../sys/boot.h"
#include "sys/sysmacros.h"
#include "../sys/libfm.h"

int		autoboot;
int		timeout;
char		defbootstr[B_STRSIZ];
char		bootmsg[B_STRSIZ];
char		bootprompt[B_STRSIZ];
char		initprog[B_STRSIZ];
char		mreqmsg1[B_STRSIZ];
char		mreqmsg2[B_STRSIZ];
struct 	bootmem	memrng[B_MAXARGS];
int		memrngcnt;
int		memreq;
extern int	memsz;

extern 	char	*getdef();

#define DEFERROR(x)	printf("\nboot: %s argument missing or incorrect\n", x)

#define SET_BY_BOOT	(B_MEM_BOOTSTRAP|B_MEM_KTEXT|B_MEM_KDATA)

bdefault(path)
char	*path;
{
	off_t	offset;
	int	line;
	int	n;
	char	buf[B_STRSIZ];
	struct	bootmem	*mp;
	char	*p, *q;
	unsigned long	t;

	/* 
	 * Initialize default parameters to reasonable values 
	 * (bss is not zeroed) 
	 */

	autoboot = TRUE;
	timeout = 0;
	memreq = 0;		/* if 0, check disabled */
	strncpy(defbootstr, "/unix", B_STRSIZ);
	strncpy(bootmsg, "Booting the UNIX System... ", B_STRSIZ);
	strncpy(bootprompt, "Enter name of program to boot: ", B_STRSIZ);
	strncpy(initprog, "/etc/initprog/at386", B_STRSIZ);
	strncpy(mreqmsg1, "Insufficient memory for a useful system", B_STRSIZ);
	strncpy(mreqmsg2, " ", B_STRSIZ);
	memrng[0].base = 0;
	memrng[0].extent = 16*1024*1024;  /* 16M */ 
	memrng[0].flags = 0;
	memrngcnt = 1;

	/* 'open' the defaults file */
	 debug(printf("Entering bdefault path = %s\n",path)); 

	if (open(path) <= 0) {
		printf("\nboot: Cannot open defaults file: %s\n", path);
		halt();
	}
	/* 
	 * search for valid options; this is inefficient, but
	 * at least it's relatively clean
	 */

	line = 1;
	offset = 0;

	binfo.bargc = 1;  /* Start with 1 to reserve space for prog name */

	for( ; ((n = bfgets(buf, B_STRSIZ, offset)) != 0) && (binfo.bargc < B_MAXARGS); offset += n+1, line++ ) {

		/* 
		 * Copy buffer to argv before doing anything else, 
		 * but only increment argc if it's OK.
		 * Thus, we can stomp on buf during the parsing.
		 */

		strncpy(binfo.bargv[binfo.bargc], buf, B_STRSIZ);

		/* if it's a comment, skip it */

		if ( buf[0] == '#' ) {
			continue;

		} else if ( p = getdef( buf, "AUTOBOOT") ) {
			if ( strncmp(p, "YES", 3) == 0 )
				autoboot = TRUE;
			else if ( strncmp(p, "NO", 2) == 0 )
				autoboot = FALSE;
			else
				DEFERROR("AUTOBOOT");
				
		} else if ( p = getdef( buf, "TIMEOUT") ) {
			if ( (t = atol(p)) < 0 )
				DEFERROR("TIMEOUT");
			else
				timeout = (int)t;

		} else if ( p = getdef( buf, "DEFBOOTSTR") ) {
			strncpy(defbootstr, p, B_STRSIZ);
			debug(printf("defbootstr = %s\n", defbootstr)); 

		} else if ( p = getdef( buf, "BOOTMSG") ) {
			strncpy(bootmsg, p, B_STRSIZ);

		} else if ( p = getdef( buf, "BOOTPROMPT") ) {
			strncpy(bootprompt, p, B_STRSIZ);

		} else if ( p = getdef( buf, "INITPROG") ) {
			strncpy(initprog, p, B_STRSIZ);
			debug(printf("initprog = %s\n", initprog));

		} else if ( p = getdef( buf, "MREQMSG1") ) {
			strncpy(mreqmsg1, p, B_STRSIZ);

		} else if ( p = getdef( buf, "MREQMSG2") ) {
			strncpy(mreqmsg2, p, B_STRSIZ);

		} else if ( p = getdef( buf, "MEMREQ") ) {
			if ( (t = atol(p)) < 0 )
				DEFERROR("MEMREQ");
			else
				memreq = (int)t;

		} else if ( p = getdef( buf, "MEMRANGE") ) {

			memrngcnt = 0;
			q = strtok( p, "-");

			do {	
				unsigned long basemem;
				mp = &memrng[memrngcnt];

				/* start of the range */

				if ( q == NULL ) {
					DEFERROR("MEMRANGE");
					break;
				}
				if ( (t = atol(q)) == -1L) {
					DEFERROR("MEMRANGE");
					break;
				}
				mp->base = (paddr_t)ctob( btoct(t) );

				/* end of the range */

				if ( (q = strtok( NULL, ":")) == NULL ) {
					DEFERROR("MEMRANGE");
					break;
				}
				if ( (t = atol(q)) == -1L) {
					DEFERROR("MEMRANGE");
					break;
				}
				debug(printf("mp->base = 0x%x\n", mp->base));
				if ((long) (mp->base) == 0L) {
				   basemem = ctob((memsz*1024) / NBPP);
				   debug(printf ("0-? range in /etc/default 0x%x\n", t));
				   debug(printf ("0-? range for int 12 0x%x\n", basemem));
				   t = (t < basemem) ? t : basemem;
				   debug(printf("selected range 0-0x%x\n", t));
				}
				mp->extent = ctob( btoc(t - (long)mp->base) );

				/* flags */

				if ( (q = strtok( NULL, ",\n")) == NULL) {
					DEFERROR("MEMRANGE");
					break;
				}
				if ( (t = atol(q)) == -1L) {
					DEFERROR("MEMRANGE");
					break;
				}
				mp->flags = (char)t & ~SET_BY_BOOT;

				memrngcnt++;
				q = strtok( NULL, "-");

			} while ( (q != NULL) && (memrngcnt < B_MAXARGS) );

		/*
		 * If the string doesn't seem to be well formed, 
		 * punt silently, so that we don't yell 
		 * when processing /etc/TIMEZONE shell scripts.
		 */

		} else if ( getdef( buf, NULL) == NULL ) {
			debug(printf("bdefault: unknown option\n")); 
			/* continue; */
			break;
		} 

		/* copy the argument string to bootinfo */

		
		binfo.bargc++;
	}

	if ( binfo.bargc >= B_MAXARGS )
		printf("\nboot: Too many lines in defaults file; extra lines ignored\n");
}


/* 
 * getdef():	get default entry from string buffer.
 *		returns a pointer to the arguments associated with key,
 *		NULL if key is not present or argument(s) are missing.
 */

char *
getdef(buf, key)
char	*buf;
char	*key;
{
	int 	c = 0;

	if ( (key != NULL) && (strncmp( buf, key, strlen(key)) != 0) )
		return(NULL);

	/* find beginning of arg string, and return a pointer to it */

	for ( ; buf[c]; c++ )
		if ( (buf[c] == '=') )
			return( &buf[c+1] );

	/* if not found, return NULL */

	return( NULL );
}


/* 
 * atol():	sort of, but not exactly, like the libc atol().
 *		We extract a positive integer from the string s,
 *		allowing for the 'k' (*1024) and 'm' (*1024^2)
 *		multiplier abbreviations.
 *		returns the integer if successful, (unsigned long)-1L if not.
 */

unsigned long
atol( p )
register char	*p;
{
	register unsigned long n;

	if ( *p == 0 )
		return(-1L);

	/* gobble white space */

	for ( ;; *p++ ) {
		if ( (*p == ' ') || (*p == '\t') )
			continue;
		break;
	}

	/* grab digits */

	n = 0;
	while ( (*p >= '0') && (*p <= '9') ) 
		n = n * 10 + *p++ - '0';

	/* modifiers */

	switch( *p ) {
	case ('M'):
	case ('m'):
		n *= 1024;
	case ('K'):
	case ('k'):
		n *= 1024;
		p++;
	}

	return( ((*p == '\0') || (*p == '\n')) ? n : -1L );
}
