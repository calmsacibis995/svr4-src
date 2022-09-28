/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexecon/common/Core.h	1.4"
// Core.h -- provides access to core files,
// both old and new (ELF) format

#ifndef Core_h
#define Core_h

#include <sys/elf.h>
#include "Reg.h"

class Process;
struct CoreData;	// opaque to clients

#include <sys/types.h>
#include <sys/user.h>
#include <sys/procfs.h>

class Core {
	CoreData *data;
public:
	Core( int corefd );
	~Core();

	// includes all segments, whether actually present in corefile, or not
int		 numsegments();
Elf32_Phdr	*segment( int which );	//  which = [0..numsegments()-1]

long		 statusbase();		// seek addr of prstatus struct
long		 fpregbase();		// seek addr of fp register set

prstatus_t	*getstatus();
fpregset_t	*getfpregs();		// 0 if no floating point

void		 update_reg( RegRef, long *, int size );
};

#endif
