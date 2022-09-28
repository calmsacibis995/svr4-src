/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/scratch/lib/_vec..c	1.2"
#line 1 "../../lib/new/_vec.c"

/* <<cfront 1.2.4 8/23/87>> */
/* < ../../lib/new/_vec.c */
char *_new(); char _delete(); char *_vec_new(); char _vec_delete();

#line 1 "../../lib/new/_vec.c"

#line 2 "../../lib/new/_vec.c"
typedef char *PV ;
typedef char (*PF )();
typedef char (*PFI )();

#line 6 "../../lib/new/_vec.c"
PV _vec_new (_au0_op , _au0_n , _au0_sz , _au0_f )PV _au0_op ;

#line 6 "../../lib/new/_vec.c"
int _au0_n ;

#line 6 "../../lib/new/_vec.c"
int _au0_sz ;

#line 6 "../../lib/new/_vec.c"
PV _au0_f ;

#line 11 "../../lib/new/_vec.c"
{ 
#line 13 "../../lib/new/_vec.c"
register char *_au1_p ;
register int _au1_i ;

#line 12 "../../lib/new/_vec.c"
if (_au0_op == 0 )_au0_op = (((char *)(((char *)_new ( (long )((sizeof (char ))* (_au0_n * _au0_sz ))) ))));
_au1_p = (((char *)_au0_op ));
if (_au0_f )for(_au1_i = 0 ;_au1_i < _au0_n ;_au1_i ++ ) (*(((char (*)())_au0_f )))( ((char *)(_au1_p + (_au1_i * _au0_sz )))) ;
return (((char *)_au1_p ));
}
;
char _vec_delete (_au0_op , _au0_n , _au0_sz , _au0_f , _au0_del )PV _au0_op ;

#line 18 "../../lib/new/_vec.c"
int _au0_n ;

#line 18 "../../lib/new/_vec.c"
int _au0_sz ;

#line 18 "../../lib/new/_vec.c"
PV _au0_f ;

#line 18 "../../lib/new/_vec.c"
int _au0_del ;
{ 
#line 20 "../../lib/new/_vec.c"
if (_au0_op == 0 )return ;
if (_au0_f ){ 
#line 22 "../../lib/new/_vec.c"
register char *_au2_p ;
register int _au2_i ;

#line 22 "../../lib/new/_vec.c"
_au2_p = (((char *)_au0_op ));
for(_au2_i = 0 ;_au2_i < _au0_n ;_au2_i ++ ) (*(((char (*)())_au0_f )))( ((char *)(_au2_p + (_au2_i * _au0_sz ))), (int )0 ) ;
}
if (_au0_del )_delete ( _au0_op ) ;
}
;

/* the end */
