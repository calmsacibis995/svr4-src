/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/RegAccess.h	1.5"
#ifndef RegAccess_h
#define RegAccess_h

#include	"oslevel.h"
#include	"Itype.h"
#include	"Reg.h"

class Core;

class RegAccess {
	Key		key;
	int		corefd;
	long		fpbase;
	long		gpbase;
	gregset_t	gpreg;
	fpregset_t	fpreg;
	int		fpcurrent;
	Core *		core;
	int		readlive( RegRef, long * );
	int		readcore( RegRef, long * );
	int		writelive( RegRef, long * );
	int		writecore( RegRef, long * );
public:
			RegAccess();
	int		setup_live( Key );
	int		setup_core( int cfd, Core * );
	int		update( prstatus & );
	Iaddr		top_a_r();
	Iaddr		getreg( RegRef );
	int		readreg( RegRef, Stype, Itype & );
	int		writereg( RegRef, Stype, Itype & );
	int		display_regs( int );
};

#endif

// end of RegAccess.h
