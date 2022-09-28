/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Vector.h	1.1"
#ifndef Vector_h
#define Vector_h

class Vector {
	int		total_bytes;
	int		bytes_used;
	char *		vector;
	void		getmemory(int);
public:
			Vector();
			Vector(Vector &);
			~Vector();
	Vector &	add(void *, int);
	Vector &	drop(int i)		{ if ( i <= bytes_used )
							bytes_used -= i;
						  return *this; }
	Vector &	take(Vector &);
	void *		ptr()			{ return vector;	}
	int		size()			{ return bytes_used;	}
	Vector &	operator= (Vector&);
	Vector &	clear()			{ bytes_used = 0; return *this;	}
	Vector &	report(char * = 0);
};

#endif

// end of Vector.h

