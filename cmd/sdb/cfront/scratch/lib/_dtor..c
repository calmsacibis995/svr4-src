/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/scratch/lib/_dtor..c	1.2"
#line 1 "../../lib/static/_dtor.c"

/* <<cfront 1.2.4 8/23/87>> */
/* < ../../lib/static/_dtor.c */
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../lib/static/_dtor.c"

#line 2 "../../lib/static/_dtor.c"
typedef char (*PFV )();
extern PFV _dtors [1];
PFV _dtors [1]= { 0 } ;

/* the end */
