/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Siglist.h	1.4"
#ifndef Siglist_h
#define Siglist_h

#include	"prioctl.h"
#include	<signal.h>

class Assoccmds;

class Siglist {
	sigset_t	_sigset;
	Assoccmds *	assoccmd[ NSIG ];
public:
			Siglist();
			~Siglist();
	int		add( int, Assoccmds * = 0 );
	int		set_sigset( sigset_t );
	int		ignored( int );	// boolean result
	int		remove( int );
	int		disable( int );
	int		enable( int );
	sigset_t	sigset();
	long		sig_mask();
	Assoccmds *	assoccmds( int );
};

#endif

// end of Siglist.h

