/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/scratch/lib/_delete..c	1.2"
#line 1 "../../lib/new/_delete.c"

/* <<cfront 1.2.4 8/23/87>> */
/* < ../../lib/new/_delete.c */
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../lib/new/_delete.c"

#line 2 "../../lib/new/_delete.c"
extern int free ();
char _delete (_au0_p )char *_au0_p ;
{ 
#line 5 "../../lib/new/_delete.c"
if (_au0_p )free ( ((char *)_au0_p )) ;
}
;

/* the end */
