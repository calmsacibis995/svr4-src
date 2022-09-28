/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Circlech.h	1.1"
#ifndef Circlech_h
#define Circlech_h

class Circlech {
	Circlech	*prev, *succ;
public:
	void *		item;
			Circlech( void * = 0 );
			~Circlech();
	int		unconnected();
	void		add( Circlech * );
	void		remove();
	Circlech *	next()	{	return succ;	}
	Circlech *	last()	{	return prev;	}
	void		add_prev( Circlech * );
	void		add_succ( Circlech * );
};

#endif

// end of Circlech.h

