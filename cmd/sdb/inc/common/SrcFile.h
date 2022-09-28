/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/SrcFile.h	1.4"
#ifndef SrcFile_h
#define SrcFile_h

#include	"Vector.h"
#include	<stdio.h>

class SrcFile {
	Vector	vector;
	FILE *	fptr;
	int	hi;
	int	not_last;
public:
	char *	name;
		SrcFile( int );
		~SrcFile();
	char *	filename();
	char *	line( int );
	long	num_lines();
};

SrcFile *	find_srcfile( char * );

#endif
