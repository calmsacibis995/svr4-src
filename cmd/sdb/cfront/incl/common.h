/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/common.h	1.1"
/*ident	"@(#)cfront:incl/common.h	1.4"*/

#ifndef COMMONH
#define COMMONH

class istream;
class ostream;

struct common {
	virtual int	cmp(common& oo) { return this==&oo; }
		/* cmp performs a three way comparison if an ordering exists:
			this==arg: return 0,
			this<arg:  return negative,
			this>arg:  return positive
		   otherwise
			this==arg: return 0,
			this!=arg: return non-zero
		*/
	virtual int operator==(common& oo) { return this==&oo; }
	virtual char*	swho() { return 0; }
	virtual int	iwho() { return 0; }
	virtual int	size() { return sizeof(common); }
	virtual ostream&	write(ostream& s) { return s;}
	virtual istream&	read(istream& s) { return s; }
		common() {}
};

#endif
