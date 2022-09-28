/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:lock.c	1.5.5.1"
/*
 * lock a file pointer
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

lock(fp, mode, blk)
FILE	*fp;
char	*mode;
int	blk;
{
	struct flock	l;

	l.l_type = !strcmp(mode, "r") ? F_RDLCK : F_WRLCK;
	l.l_whence = 0;
	l.l_start = l.l_len = 0L;
	return fcntl(fileno(fp), blk ? F_SETLKW : F_SETLK, &l);
}
