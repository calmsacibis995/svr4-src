/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/scratch/lib/_main..c	1.2"
#line 1 "../../lib/static/_main.c"

/* <<cfront 1.2.4 8/23/87>> */
/* < ../../lib/static/_main.c */
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../lib/static/_main.c"

#line 2 "../../lib/static/_main.c"
extern char _main ()
#line 3 "../../lib/static/_main.c"
{ 
#line 10 "../../lib/static/_main.c"
typedef char (*PFV )();

#line 5 "../../lib/static/_main.c"
extern PFV _ctors [];
PFV *_au1_pf ;

#line 6 "../../lib/static/_main.c"
for(_au1_pf = _ctors ;*_au1_pf ;_au1_pf ++ ) { 
#line 7 "../../lib/static/_main.c"
(*(*_au1_pf ))( ) ;
(*_au1_pf )= 0 ;
}
}
;

/* the end */
