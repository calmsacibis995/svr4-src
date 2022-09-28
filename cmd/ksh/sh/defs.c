/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/defs.c	1.2.3.1"

#include	"defs.h"
#include	"jobs.h"
#include	"sym.h"
#include	"history.h"
#include	"edit.h"
#include	"timeout.h"


struct sh_scoped	st;
struct sh_static	sh;

#ifdef VSH
    struct	edit	editb;
#else
#   ifdef ESH
	struct	edit	editb;
#   endif /* ESH */
#endif	/* VSH */

struct history	*hist_ptr;
struct jobs	job;
int		sh_lastbase = 10; 
time_t		sh_mailchk = 600;
#ifdef TIMEOUT
    long		sh_timeout = TIMEOUT;
#else
    long		sh_timeout = 0;
#endif /* TIMEOUT */
char		io_tmpname[] = "/tmp/shxxxxxx.aaa";

#ifdef 	NOBUF
    char	_sibuf[IOBSIZE+1];
    char	_sobuf[IOBSIZE+1];
#endif	/* NOBUF */

struct fileblk io_stdin = { _sibuf, _sibuf, _sibuf, 0, IOREAD, 0, F_ISFILE};
struct fileblk io_stdout = { _sobuf, _sobuf, _sobuf+IOBSIZE, 0, IOWRT,2};
struct fileblk *io_ftable[NFILE] = { 0, &io_stdout, &io_stdout};

#ifdef MULTIBYTE
/*
 * These are default values.  They can be changed with CSWIDTH
 */

char int_charsize[] =
{
	1, CCS1_IN_SIZE, CCS2_IN_SIZE, CCS3_IN_SIZE,	/* input sizes */
	1, CCS1_OUT_SIZE, CCS2_OUT_SIZE, CCS3_OUT_SIZE	/* output widths */
};
#else
char int_charsize[] =
{
	1, 0, 0, 0,	/* input sizes */
	1, 0, 0, 0	/* output widths */
};
#endif /* MULTIBYTE */

