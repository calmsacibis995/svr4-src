/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:ioctl.c	1.1"

/*
 *	@(#) ioctl.c 1.1 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1986.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "defs.h"
#include "emap.h"

myioctl(fd, cmd, buf)
int fd, cmd;
char buf[1024];
{
	struct stat statbuf;
	FILE *f;
	char name[100];

	if (fstat(fd, &statbuf) < 0)
		oops("cannot fstat\n");
	sprintf(name, "channels/%d", statbuf.st_ino);
	if (cmd == LDSMAP) {
		if ((f = fopen(name, "w")) == NULL)
			oops("cannot fopen\n");;
		fwrite(buf, 1024, 1, f);
		fclose(f);
		return(0);
	} else if (cmd == LDGMAP) {
		if ((f = fopen(name, "r")) == NULL)
			oops("cannot fopen\n");;
		fread(buf, 1024, 1, f);
		fclose(f);
		if (strncmp(buf, "no map", 6) == 0) {
			errno = ENAVAIL;
			return(-1);
		}
		return(0);
	} else if (cmd == LDNMAP) {
		if ((f = fopen(name, "w")) == NULL)
			oops("cannot fopen\n");;
		fputs("no map", f);	
		fclose(f);
		return(0);
	} else
		return(-1);	
}
