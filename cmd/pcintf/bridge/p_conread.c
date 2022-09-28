/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_conread.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_conread.c	3.9	LCC);	/* Modified: 15:16:49 5/18/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include <fcntl.h>
#include <termio.h>
#include <errno.h>
#include "pci_types.h"

/* --- system calls --- */
extern  long lseek();

/* --- global routines --- */
extern  int swap_in();                  /* Swap-in file context */

/* --- local routines --- */
int  lineread();

/* --- globals --- */
extern  int errno;                      /* Global errno from system calls */
extern  char err_class, err_action, err_locus;


pci_conread(vdescriptor, dosfunc, flsh, bytes, addr, request)
    int		vdescriptor;		/* PCI virtual file descriptor */
    int         dosfunc;                /* DOS function 1,6,7,8,a,b */
    int         flsh;                   /* Flush tty before read */
    int         bytes;                  /* Max number of chars to read */
    register	struct	output	*addr;	/* Pointer to response buffer */
    int		request;		/* Contains DOS request number */
{
    register int adesc;         /* Actual UNIX descriptor */
    int filstat;                /* File status flags */
    int wasblking;              /* boolean: Originally i/o set for blocking */
    int status;			/* return value from system calls */

    /* Swap-in UNIX descriptor */
    if ((adesc = swap_in(vdescriptor, DONT_CARE)) < 0)
    {
	addr->hdr.res = (adesc == NO_FDESC) ? FILDES_INVALID : FAILURE;
	return;
    }

    do
	filstat = fcntl(adesc, F_GETFL, 0);       /* file status flags */
    while (filstat == -1 && errno == EINTR);
    wasblking =  ( (filstat & O_NDELAY) == 0);

    debug(0, ("conread adesc=%#x func=%#x flsh=%d bc=%d wb=%d\n",
	adesc, dosfunc, flsh, bytes, wasblking));

    /* dos functions: */
    /* 1 = read 1 char and claim was echoed to stdout when isatty(blocking) */
    /* 6 = read 1 char (non-blocking) */
    /* 7 = read 1 char (blocking) */
    /* 8 = read 1 char (blocking) */
    /* a = read line from stdin */
    /* b = tell if char is available at stdin */
    /* 'attr' indicates if preflushing on ttys is to be done */
    /* 'bytes' is only used for func a */

    /* in the send packet, these items are significant : */
    /* hdr.attr = 1 if echo performed by server, else 0. */
    /* hdr.b_cnt = dosfunc 1,6,7,8,a : Number of chars returned.
    /*             dosfunc b : 0x00ff when char is available, else 0. */
    /* hdr.t_cnt actual number of bytes in text field. */

    if (flsh)
    {
	if (isatty(adesc))
	{
	    debug(0, ("Flush\n"));
	    /* flush input */
#ifdef	BERKELEY42
	    while (ioctl(adesc, TIOCFLUSH, 0) == -1 && errno == EINTR)
#else	/* !BERKELEY42 */
	    while (ioctl(adesc, TCFLSH, 0) == -1 && errno == EINTR)
#endif	/* !BERKELEY42 */
		;
	}
    }

    switch (dosfunc)
    {
    case 0x0a:  /* line read */
	if (lineread(adesc, bytes, addr))
	{
	    /* when lineread() fails it doesn't corrupt errno */
	    err_handler(&addr->hdr.res, request, NULL);
	    return;
	}
	debug(0, ("read %d '%s'\n",addr->hdr.b_cnt,addr->text));
	if (isatty(adesc)) /* when is a tty, claim it was echoed */
	    addr->hdr.attr = 1;
	break;
    case 6:     /* non-blocking read of a char */
    case 0x0b:  /* is char avail? */
	/* make sure don't block */
	if (wasblking)   /* if blocking, then */
	{
	    do
		status = fcntl(adesc, F_SETFL, filstat | O_NDELAY);
	    while (status == -1 && errno == EINTR);
	    if (status == -1)
	    {
		err_handler(&addr->hdr.res, request, NULL);
		return;
	    }
	}

	if (dosfunc == 6)       /* read a char */
	{
	    do
		addr->hdr.b_cnt = read(adesc, addr->text, 1);
	    while (addr->hdr.b_cnt == -1 && errno == EINTR);
	    if (addr->hdr.b_cnt < 0)
	    {
		err_handler(&addr->hdr.res, request, NULL);
		return;
	    }
	    addr->hdr.t_cnt = addr->hdr.b_cnt;
	}
	else /* func B: see if more chars */
	{
	    debug(0, ("Always say there is a char\n"));
	    addr->hdr.b_cnt = 0x00ff;
	}

	if (wasblking)  /* set back to blocking */
	{
	    do
		status = fcntl(adesc, F_SETFL, filstat);
	    while (status == -1 && errno == EINTR);
	    if (status == -1)
	    {
		err_handler(&addr->hdr.res, request, NULL);
		return;
	    }
	}
	break;
    case 1:     /* read and 'echo' a char (blocking) */
    case 7:     /* read a char (blocking) */
    case 8:     /* read a char (blocking) */
	do
	    addr->hdr.b_cnt = read(adesc, addr->text, 1);
	while (addr->hdr.b_cnt == -1 && errno == EINTR);
	if (addr->hdr.b_cnt < 0)
	{
	    err_handler(&addr->hdr.res, request, NULL);
	    return;
	}
	debug(0, ("Read %d '%c'\n", addr->hdr.b_cnt, *(addr->text)));

	addr->hdr.t_cnt = addr->hdr.b_cnt;
	if (dosfunc == 1)
	    if (isatty(adesc)) /* when is a tty, claim it was echoed */
		addr->hdr.attr = 1;
	break;
    default:
	debug(0, ("No read.\n"));
	break;

    }/*endswitch*/
#if FAST_LSEEK

	/*
	 * Pass the NEW current offset back to the bridge so that it can
	 * keep the current file pointer accurate.
	 */

	addr->hdr.offset = lseek(adesc, 0L, 1);
#endif	/* FAST_LSEEK */
}

int lineread(adesc, maxc, addr)
int adesc;              /* file descriptor */
int maxc;               /* max chars to put in buffer, incl. the dog */
struct  output  *addr;  /* Pointer to response buffer */
{
    register int numb, ii, limit;
    char *bufp;
    char cb;

    limit = maxc - 1;
    numb = 0;
    bufp = addr->text;

    while (1)
    {
	do
	    ii = read(adesc, &cb, 1);
	while (ii == -1 && errno == EINTR);
	/* if we fail, return without corrupting errno */
	if (ii != 1)
	{
	    if (ii == -1)
		return 1;
	    break;
	}
	if (cb == 0x0d)         /* throwaway dogs */
	    continue;
	if (cb == 0x0a)         /* asses indicate eol */
	    break;

	if (numb < limit)
	{
	    numb++;
	    *bufp++ = cb;
	}

    }/*endwhile*/

    *bufp = 0x0d;
    addr->hdr.b_cnt = numb;
    addr->hdr.t_cnt = numb + 1;
    return 0;
}
