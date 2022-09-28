/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/scratch/lib/dtors..c	1.2"
#line 1 "../../lib/static/dtors.c"

/* <<cfront 1.2.4 8/23/87>> */
/* < ../../lib/static/dtors.c */
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../lib/static/dtors.c"

#line 3 "../../lib/static/dtors.c"
typedef char (*PFV )();

#line 5 "../../lib/static/dtors.c"
char dtors ()
#line 6 "../../lib/static/dtors.c"
{ 
#line 7 "../../lib/static/dtors.c"
extern PFV _dtors [];
static int _st1_ddone ;
if (_st1_ddone == 0 ){ 
#line 10 "../../lib/static/dtors.c"
_st1_ddone = 1 ;
{ PFV *_au2_pf ;

#line 11 "../../lib/static/dtors.c"
_au2_pf = _dtors ;
while (*_au2_pf )_au2_pf ++ ;
while (_dtors < _au2_pf )(*(*(-- _au2_pf )))( ) ;
}
}
}
;

/* the end */
