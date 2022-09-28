/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libsymbol/common/Tagcheck.h	1.1"
#ifndef Tagcheck_h
#define Tagcheck_h

int stacktag(long);
int vartag(long);
int typetag(long);
int addrtag(long);

#endif

// end of Tagcheck.h

