/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Label.h	1.3"

//	class Label -- maintains stack of label parts, with push/pop
//	access and automatic storage management.
//
//	class Labelpart -- private to class Label, one per "part".
//	A "part" is the unit of pushing/popping (normally an id or
//	operator).

#ifndef LABEL_H
#define LABEL_H

#include "Link.h"

class Labelpart : public Link {
	char		*p;
	int		 len;
public:
	   Labelpart( char * );
	  ~Labelpart();
Labelpart *next()	{ return (Labelpart *) Link::next(); }
Labelpart *prev()	{ return (Labelpart *) Link::prev(); }
char	  *str()	{ return p; }
int	   length()	{ return len; }	// doesn't count terminating NULL
};

class Label {
	char		*buf;
	int		 buflen;
	int		 len;		// (virtual) strlen(buf)
	Labelpart	*first;
public:
		 Label();
		 Label( Label& );
		~Label();
	Label	&operator=( Label& );
	void	 push( char * );
	void	 pop( int count = 1 );
	char	*str();		// concatenation of all current parts
	int	 length() { return len; } // doesn't count terminating NULL
	void	 clear();
	operator char *() { return str(); }
};

#endif /* LABEL_H */
