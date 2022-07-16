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

#ident	"@(#)kern-os:bs.c	1.3"

/*
 * bs.c:  Parse the bootstring and set rootdev, swapdev, pipedev, and dumpdev.
 *	  Pass all unrecognized portions of the bootstring on to main().
 *
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/bootinfo.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/vfs.h"
#include "sys/ddi.h"

#define equal(a,b)	(bs_strcmp(a, b) == 0)


#define	BASE8	0		/* bases for bs_stratoi */
#define BASE10	1
#define	BASE16	2

char *bs_strchr();

/*
 * bootarg_parse() returns a 0 if ok to continue, a 1 if a panic should 
 * be performed.
 */

int	margc = 0;			/* argc to main() */
char	*margv[B_MAXARGS] = {0};	/* argv[] to main() */
int 	autoboot = 0;			/* non-zero implies we autobooted */
extern	dev_t rootdev, pipedev, swapdev, dumpdev;

/* boot string variables */
#define	MAXPARMS	2		/* number of device parms */


bootarg_parse()
{
	register int i;
	char	 *s;
	char	 hold_arg[B_STRSIZ];	/* tmp hold area for bargv[] entry */


	/*
 	 * Read bootflags to see if we were booted from the floppy.
 	 */
	if (bootinfo.bootflags & BF_FLOPPY) { /* we booted from floppy */
		rootdev = makedevice(1,132);
		pipedev = makedevice(1,132);
		swapdev = NODEV;
	}


	/*
	 * Parse the bootstrap args passed in bootinfo.bargv[].  Note that
	 * bootinfo.bargv[0] contains the name of the kernel booted.  We
	 * don't bother to parse this.  It gets passed along to main(), though.
	 */

	margv[0] = &bootinfo.bargv[0][0];  /* argv for main() */
	margc = 1;			   /* argc for main() */

	for (i = 1; i < bootinfo.bargc; i++) {	/* parse args */
		bcopy(&bootinfo.bargv[i][0], hold_arg, B_STRSIZ);
		if (*hold_arg && (bs_doarg(hold_arg) != 0)) {
			/* 
			 * We did not handle the boot arg here.  Pass it 
			 * along to main() in margv[].  
			 */

			margv[margc++] = &bootinfo.bargv[i][0];
		}
	}

	return(0);
}

/*	bs_doarg(arg) - determine argument and further analyze parameters
 *		returns	-1 if error
 *			 0 if arg correct
 *
 *	Currently, we only care about arguments that have the following form:
 *	Everything else gets passed onto main(argc, argv).
 *
 * 	  case 0 AUTOBOOT={yes,no}
 *	  case 1 rootfs[type]=<fstype>
 * 	  case 2 <keyword>=<device>(<parm>[,<parm>])  {root[dev], pipe[dev],
 *							swap[dev],dump[dev]}
 *
 *	Note that '<>' surround user supplied values; '[]' surround
 *	optional extensions; "parms" are words containing numeric characters;
 *	"devices" are device mnemonics (from bdevsw/cdevsw d_name); and all 
 *	other symbols are literal.
 *
 */

bs_doarg(s)
char *s;
{
	int  n, maj, parms[MAXPARMS];
	char  *p, *bs_lexnum();

	/* skip over keyword to '=' */
	if ((p = bs_strchr(s, '=')) == (char *)0)  
		return(oem_doarg(s));

	*p++ = '\0';				/* delimit keyword */

	/* case 0 */
	if (equal(s, "AUTOBOOT")) {
		if (equal(p, "yes") || equal(p, "YES") || equal(p, "Yes")) {
			autoboot = 1;
		}
		return(0);			/* arg was handled */
	}

	/* case 1 */
	if (equal(s, "rootfs") || equal(s, "rootfstype")) {
		strncpy(rootfstype, p, ROOTFS_NAMESZ);
		return(0);			/* arg was handled */
	}

	/* case 2 */
	n = bs_lexparms(p, parms, MAXPARMS);
						/* look in bdevsw[] */
	if ((n > 0  &&  n <= MAXPARMS) && ((maj = bs_find_bmaj(p)) != -1)) {

						/* make rootdev */
		if (equal(s, "root") || equal(s, "rootdev")) {
			rootdev = makedevice(maj, parms[0]);
		}
		else					/* make pipedev */
		if (equal(s, "pipe") || equal(s, "pipedev")) {
			pipedev = makedevice(maj, parms[0]);
		}
		else					/* make swapdev */
		if (equal(s, "swap") || equal(s, "swapdev")) {
			swapdev = makedevice(maj, parms[0]);
		}
		else					/* make dumpdev */
		if (equal(s, "dump") || equal(s, "dumpdev")) {
			dumpdev = makedevice(maj, parms[0]);
		}
		else {					/* unknown dev */
			
			return(-1);		 	/* arg NOT handled */
		}

		return(0);				/* arg was handled */
	}

	/* unknown argument */

	return(oem_doarg(s));				/* arg NOT handled */
}


/*	bs_lexparms - extract numeric parameters from an argument string
 *		   surrounded by braces ()
 *
 *		s = input string pointer
 *	    parms = pointer to array of ints
 *	 maxparms = size of parms array
 *
 *	returns	0 = error
 *		n = number of parameters found
 */

bs_lexparms(s, parms, maxparms)
char *s;
int *parms, maxparms;
{
	char *p, *bs_lexnum();
	int n = 0;

	if ((p = bs_strchr(s, '(')) == (char *)0)  
		return(0);
	*p = '\0';			/* delimit anything prior to '(' */
	do {
		p++;
		if (maxparms-- == 0)  
			return(0);
		p = bs_lexnum(p, &parms[n]);	/* extract number */
		if (p == (char *)0)  		/* no parm error */
			return(0);		
		if (*p != ','  &&  *p != ')')  
			return(0);
		n++;
	} while (*p == ',');			/* while more parms */
	return(n);
}


/*	bs_lexwords - extract word parameters from an argument string
 *		   surrounded by braces ()
 *
 *		s = input string pointer
 *	    parms = pointer to array of char *
 *	 maxparms = size of parms array
 *
 *	returns	0 = error
 *		n = number of parameters found
 */

bs_lexwords(s, parms, maxparms)
char *s, *parms[];
int maxparms;
{
	char *p;
	int n = 0;

	if ((p = bs_strchr(s, '(')) == (char *)0)  
		return(0);
	do {
		*p = '\0';			/* delimit previous word */
		p++;
		if (maxparms-- == 0)  
			return(0);
		parms[n] = p;
		while (*p >= '0' && *p <= '9'  ||  *p >= 'A' &&  *p <= '~') 
			p++;
		if (*p != ','  &&  *p != ')')  
			return(0);
		n++;
	} while (*p == ',');			/* while more parms */
	*p = '\0';
	return(n);
}


/*	bs_lexnum - extract number from string (works with octal, decimal, or hex)
 *
 *	     p = pointer to input string
 *	  parm = pointer to value storage
 *
 *	returns 0 = error
 *		else pointer to next non-numeric character in input string
 *
 *	octal constants are preceded by a '0'
 *	hex constants are preceded by a '0x'
 *	all others are assumed decimal
 */
char *
bs_lexnum(p, parm)
char *p;
int *parm;
{
	char *q, *bs_stratoi();

	if (*p == '0') {			/* hex or octal */
		p++;
		if (*p == 'x') {		/* hex */
			p++;
			q = bs_stratoi(p, parm, BASE16);
		} else {			/* octal */
			p--;			/* leave leading zero */
			q = bs_stratoi(p, parm, BASE8);
		}
	} else {				/* decimal */
		q = bs_stratoi(p, parm, BASE10);
	}
	if (q == (char *)0  ||  p == q)  
		return((char *)0);
	return(q);
}


/*	strstoi - convert ascii numbers to integer values
 *
 *	      p = pointer to input string
 *	   parm = pointer to value storage
 *	   base = number base for conversion
 *
 *	returns NULL = no value found
 *		else pointer to next non-numeric character
 */
static char *digit[] = {"01234567", "0123456789", "0123456789abcdef"};
static int bases[] = {8, 10, 16};

char *
bs_stratoi(p, parm, base)
char *p;
int *parm, base;
{
	char *q, *digits;

	if (base > 2)  
		return((char *)0);
	digits = digit[base];
	*parm = 0;
	while ((q = bs_strchr(digits, (*p>='A') ? *p|0x20 : *p)) != (char *)0) {
		*parm = (*parm) * bases[base] + (q - digits);
		p++;
	}
	return(p);
}


bs_find_bmaj(dev)
char *dev;
{
	register int  i;
	register char *cp;

	for (i = 0; i < bdevcnt; i++)
		if ((cp = bdevsw[i].d_name) && equal(dev, cp))  
			return(i);
	return(-1);
}


bs_find_cmaj(dev)
char *dev;
{
	register int  i;
	register char *cp;

	for (i = 0; i < cdevcnt; i++)
		if ((cp = cdevsw[i].d_name) && equal(dev, cp))  
			return(i);
	return(-1);
}


bs_syntax(s)
	char *s;
{
	cmn_err(CE_WARN, "\"%s\": bad syntax\n", s);
}


char *
bs_strchr(sp, c)
register char *sp, c;
{
	while (*sp) {
		if (*sp == c)
			return(sp);
		sp++;
	}
	return(NULL);
}


bs_strcmp(s1, s2)
register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++=='\0')
			return(0);
	return(*s1 - *--s2);
}

