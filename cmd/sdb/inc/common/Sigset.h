/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Sigset.h	1.3"
#ifndef Sigset_h
#define Sigset_h

#include	<signal.h>

struct Sigset {
	long mem[2];
};
#ifndef __cplusplus
typedef struct Sigset Sigset;
#endif

int	all_on( Sigset * );
int	all_off( Sigset * );
int	turn_on( int, Sigset * );
int	turn_off( int, Sigset * );
int	is_on( int, Sigset * );
int	copy_sigset( Sigset *, Sigset * );

#endif

/* end of Sigset.h */

