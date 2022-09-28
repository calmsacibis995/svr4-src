/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/err.h	55.1"
/* err.h */

#ifdef	__STDC__
/* prototype forces strings into read-only space */
extern void cerror(const char *, ...);
extern void uerror(const char *, ...);
extern void werror(const char *, ...);
extern void ulerror(int, const char *, ...);
extern void wlerror(int, const char *, ...);
extern void yyerror(const char *);
extern int er_getline(void);
#ifdef LINT
extern char *er_curname(void);
#endif
#ifndef NODBG
extern void dprintf(const char *, ...);
#endif
#else	/* def __STDC__ */
extern void cerror(), uerror(), werror(), ulerror(), wlerror(), yyerror();
extern int er_getline();
#ifdef LINT
extern char *er_curname();
#endif
#ifndef NODBG
extern void dprintf();
#endif
#endif	/* def __STDC__ */

extern void er_filename();
extern void er_markline();

#ifndef LINT
#define	UERROR uerror
#define	WERROR werror
#define ULERROR ulerror
#define WLERROR wlerror
#else
#define	UERROR luerror
#define	WERROR lwerror
#define ULERROR lulerror
#define WLERROR lwlerror
#endif

#define	DPRINTF dprintf


extern int nerrors;			/* number of errors so far */

#ifdef	NODBG
#define	DEBUG(cond, print)
#else
#define	DEBUG(cond, print) if (cond) DPRINTF print
#endif
