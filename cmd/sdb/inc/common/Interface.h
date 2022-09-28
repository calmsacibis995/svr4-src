/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Interface.h	1.5"

// Debugger interface structure.

// Enables different interfaces (e.g., screen and line mode)
// to work together.

#ifndef Interface_h
#define Interface_h

#include <stdio.h>
#include <stdarg.h>
#include "Itype.h"

class	Vector;
class	Process;

extern int interrupted;

enum	OutType	{ ot_none, ot_file, ot_vector };

enum	CmdType { NOCOM, PRCOM, DSCOM, DSICOM };

extern CmdType	lastcom;
extern Iaddr	instaddr;

struct	OutPut {
	OutType	type;
	union {
		FILE *		fp;
		Vector *	vec;
	};
};

extern	int	pushoutfile( FILE * );
extern	int	pushoutvec( Vector * );
extern	void	popout();

extern	int	vprintx( const char *, va_list );
extern	int	vprinte( const char *, va_list );

extern	int	_printx( const char * ... );
extern	int	_printe( const char * ... );

struct	Interface {
	int	(*_show_location)( Process *, Iaddr pc, int showsrc );
	int	(*_printx)( const char * ... );
	int	(*_printe)( const char * ... );
};

extern	Interface *	IP;	// points to the current interface
				// which is: (only choice, currently)
extern	Interface	ln_interface;		// line mode

#define	show_location	 (!IP->_show_location)	  ? 0 : (*IP->_show_location)
#define	printx		 (!IP->_printx)		  ? 0 : (*IP->_printx)
#define	printe		 (!IP->_printe)		  ? 0 : (*IP->_printe)

void ABORT( char *format ... );

#endif	/* Interface_h */
