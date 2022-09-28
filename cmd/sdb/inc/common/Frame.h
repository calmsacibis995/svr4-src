/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/Frame.h	1.3"
#ifndef Frame_h
#define Frame_h

//
// NAME
//	Frame
//
// ABSTRACT
//	Represents stack frames and provides access to registers
//
// DESCRIPTION
//	A Process instance will always have at least one Frame
//	instance (the topmost frame, accessing the hardware registers).
//	It will produce others upon request, via the Frame::caller()
//	member function.  It is the client's responsibility to ensure
//	that the process has not moved before attempting to query a
//	Frame instance other than Process::topframe.  That is, no Frame
//	instance (other than topframe) remains valid after the process
//	steps or runs.  The Process class will delete all Frames other
//	than topframe whenever it starts in motion for any reason.
//
// DATA
//	(no public data)
//
// OPERATIONS
//	Frame(Process*)	Constructor - initializes the topmost frame; not public
//
//	Frame(Frame*)	Constructor - initializes the caller's frame
//			and links it into the list; not public
//
//	FrameId id()	returns a one-word struct which identifies the
//			frame; FrameId's may only be compared for equality
//
//	Frame *caller()	returns this frame's caller (constructs it if
//			necessary) or NULL if at bottom of stack
//
//	Frame *callee()	returns this frame's callee or NULL if at top of stack
//
//	int valid()	returns non-zero if frame is still valid (process has
//			not moved)
//
//	int readreg(RegRef which, Stype what, Itype& dest)
//			fetches register "which" from the frame, and
//			puts result as a "what" into "dest".
//			Returns 0 if successful, non-zero if failure.
//			May fail if combination of "which" and "what"
//			is nonsensical, such as requesting an IU reg
//			as a double.
//
//	int writereg(RegRef which, Stype what, Itype& src)
//			writes "src" as a "what" into register "which"
//			in the frame.  Returns 0 if successful, non-zero
//			if failure.  May fail if combination of "which"
//			and "what" is nonsensical, such as writing a
//			double into an IU register.
//
//	Iaddr getreg(RegRef)
//			shorthand for   { readreg(..., Saddr, itype);
//					  return itype.iaddr; }
//
//	Iint4 argword(int n)
//			returns nth argument word (0 based)
//
//	int nargwds()	returns number of words of arguments, if feasible.
//			Note that it will ALWAYS return -1 on the SPARC,
//			since it not possible to determine the actual number
//			of words of arguments passed.

#include "Link.h"
#include "Reg.h"
#include "Itype.h"


struct framedata;		// opaque to clients

struct frameid;			// opaque to clients

class Frame;

class FrameId {
	frameid *id;
public:
	FrameId(Frame * = 0);
//	FrameId()	{	id = 0;	}
	~FrameId();
int	operator==(FrameId&);
int	operator!=(FrameId&);
int	isnull()	{ return id == 0;	}
void	null() { if (id) delete id; id = 0;	}
FrameId &	operator=( FrameId & );
void	print(char * = 0 );
};

class Frame : public Link {
	friend class Process;
	framedata *data;	// all Frame data is accessed through this ptr
	int	isleaf();	// internal; called by Frame constructor
	Frame(Process*);	// only Process can use this constructor
	Frame(Frame*);		// internal
	~Frame();

	friend class FrameId;	// need to access member "data" in constructor.
public:

	FrameId	id();
	Frame	*caller();
	Frame	*callee();

	int	valid();

	int	readreg(RegRef which, Stype what, Itype& dest);
	int	writereg(RegRef which, Stype what, Itype& src);

	Iaddr	getreg(RegRef);

	Iint4	argword(int n);

	int	nargwds();

	Iaddr	pc_value();
};

#endif

// end of Frame.h
