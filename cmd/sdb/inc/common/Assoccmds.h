/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Assoccmds.h	1.3"
#ifndef Assoccmds_h
#define Assoccmds_h

#include	"Vector.h"

class Assoccmds {
	Vector			vector;
	char *			next_cmd;
	int			refcount;
	friend Assoccmds *	dispose_assoc( Assoccmds * );
public:
				Assoccmds()	{	refcount = 0;	}
	char *			get_cmd();
	void			add_cmd( char * );
	void			reset();
	char *			string();
};

Assoccmds *	dispose_assoc( Assoccmds * );
int		queue_assoc( Assoccmds * );
Assoccmds *	dequeue();

#endif

// end of Assoccmds.h

