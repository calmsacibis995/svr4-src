/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexecon/common/Status.h	1.2"
#ifndef Status_h
#define Status_h

#include	"Link.h"
#include	"prioctl.h"

struct Process;

class Status: public Link {
	int		inform;
	Process *	process;
	int		tracker_pid;
	Status *	next()	{ return (Status *)Link::next();	}
	friend class	Monitor;
public:
			Status( Process * );
			~Status();
	int		get_status( prstatus & );
	int		track();
};

#endif

// end of Status.h

