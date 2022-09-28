/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Stmt.h	1.3"
#ifndef Stmt_h
#define Stmt_h

struct Stmt {
	long	line;
	int	pos;

		Stmt()	{ line = 0; pos = 0; }

	int	is_unknown()	{	return ( line == 0 );	}
	void	unknown()	{	line = 0; pos = 0;	}
	int	operator==( Stmt & st )	{ return (line == st.line) &&
						 (pos == st.pos);	}
};

#endif

// end of Stmt.h
