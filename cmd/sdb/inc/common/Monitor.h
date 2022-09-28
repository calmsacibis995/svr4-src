/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Monitor.h	1.2"
#ifndef Monitor_h
#define Monitor_h

#include	"Link.h"
#include	"Vector.h"

class Process;
class Status;

class Monitor: public Link {
	Status *	next()	{ return (Status*)Link::next();	}
public:
			Monitor();
			~Monitor();
	int		add( Process * );
	int		remove( Process * );
	int		track( Process * );
	int		check_status();
};

extern Monitor	e_monitor;

#endif

// end of Monitor.h

