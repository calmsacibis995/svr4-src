/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/data.c	2.14"
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak _iob = __iob
#endif
#include "synonyms.h"
#include <stdio.h>
#include "stdiom.h"

#if !u370

/*
 * Ptrs to start of preallocated buffers for stdin, stdout.
 * Some slop is allowed at the end of the buffers in case an upset in
 * the synchronization of _cnt and _ptr (caused by an interrupt or other
 * signal) is not immediately detected.
 */
Uchar _sibuf[BUFSIZ + _SMBFSZ], _sobuf[BUFSIZ + _SMBFSZ];
#endif

Uchar _smbuf[_NFILE + 1][_SMBFSZ] = {0};  /* shared library compatibility */

/*
 * Ptrs to end of read/write buffers for first _NFILE devices.
 * There is an extra bufend pointer which corresponds to the dummy
 * file number _NFILE, which is used by sscanf and sprintf.
 */
Uchar *_bufendtab[_NFILE+1] = { NULL, NULL, _smbuf[2] + _SBFSIZ, };

FILE _iob[_NFILE] = {
#if vax || u3b || M32 || i386
	{ 0, NULL, NULL, _IOREAD, 0},
	{ 0, NULL, NULL, _IOWRT, 1},
	{ 0, _smbuf[2], _smbuf[2], _IOWRT+_IONBF, 2},
#endif
#if pdp11
	{ NULL, 0, NULL, _IOREAD, 0},
	{ NULL, 0, NULL, _IOWRT, 1},
	{ _smbuf[2], 0, _smbuf[2], _IOWRT+_IONBF, 2},
#endif
#if u370
	{ NULL, 0, NULL, _IOREAD, 0},
	{ NULL, 0, NULL, _IOWRT, 1},
	{ NULL, 0, NULL, _IOWRT+_IONBF, 2},
#endif
};

/*
 * Ptr to end of io control blocks
 */
FILE *_lastbuf = &_iob[_NFILE];
