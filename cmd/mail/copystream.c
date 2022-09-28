/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:copystream.c	1.2.3.1"
/*
    NAME
	copystream - copy one FILE stream to another

    SYNOPSIS
	int copystream(FILE *infp, FILE *outfp)

    DESCRIPTION
	copystream() copies one stream to another. The stream
	infp must be opened for reading and the stream outfp
	must be opened for writing.

	It returns true if the stream is successively copied;
	false if any writes fail.
*/

#include "mail.h"
int copystream(infp, outfp)
register FILE *infp;
register FILE *outfp;
{
    char buffer[BUFSIZ];
    register int nread;

    while ((nread = fread(buffer, sizeof(char), sizeof(buffer), infp)) > 0)
	if (fwrite(buffer, sizeof(char), nread, outfp) != nread)
	    return 0;
    return 1;
}
