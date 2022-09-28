/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Object.h	1.3"
#ifndef Object_h
#define Object_h

#include	"Circlech.h"
#include	"Itype.h"
#include	"prioctl.h"
#include	"oslevel.h"

class Symtable;
class EventTable;
class Process;

struct Object {
	int			fdobj;
	dev_t			device;
	ino_t			inumber;
	Circlech		link;
	Symtable *		symtable;
				Object( int, dev_t, ino_t );
				~Object();
	EventTable *		etable;
	Process *		protop;
	Process *		get_proto();
};

extern Object *		first_object();
extern Symtable *	get_symtable( int );	
extern Object *		find_object( int, EventTable **, Symtable ** );
int			dispose_process( Process * );

#endif

// end of Object.h

