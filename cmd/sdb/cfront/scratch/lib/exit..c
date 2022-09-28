/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/scratch/lib/exit..c	1.2"
#line 1 "../../lib/static/exit.c"

/* <<cfront 1.2.4 8/23/87>> */
/* < ../../lib/static/exit.c */
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../lib/static/exit.c"

#line 3 "../../lib/static/exit.c"
extern char _exit ();
extern char _cleanup ();
extern char dtors ();

#line 7 "../../lib/static/exit.c"
extern char exit (_au0_i )int _au0_i ;
{ 
#line 9 "../../lib/static/exit.c"
dtors ( ) ;
_cleanup ( ) ;
_exit ( _au0_i ) ;
}
;

/* the end */
