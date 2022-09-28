/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Elfbuild.h	1.1"
#ifndef Elfbuild_h
#define Elfbuild_h

#include	"Fetalrec.h"
#include	"Reflist.h"

struct Syminfo;

class Elfbuild {
	int		fd;
	Fetalrec	fetalrec;
	long		losym, hisym;
	long		lostr, histr;
	char *		symptr;
	char *		strptr;
	Reflist		reflist;
	Attribute *	build_record( long );
public:
			Elfbuild( int );
	long		first_symbol();
	int		get_syminfo( long, Syminfo & );
	Attribute *	make_record( long );
	char *		get_name( long );
};

#endif

// end of Elfbuild.h
