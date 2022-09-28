/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/stream.h	1.1"
/*ident	"@(#)cfront:incl/stream.h	1.11" */
/*
	stream.h, the header file for the C++ stream i/o system 
*/

#ifndef STREAMH
#define STREAMH

#include <stdio.h>

#define	BUFSIZE	BUFSIZ

#ifndef NULL
#define NULL	0
#endif

#ifndef EOF
#define	EOF	(-1)
#endif

#ifndef BUFSIZE
#if uts
#define BUFSIZE 4096
#else
#define BUFSIZE 1024
#endif
#endif

enum state_value { _good=0, _eof=1, _fail=2, _bad=4 };
enum open_mode { input=0, output=1, append=2 };

struct streambuf {			// a buffer for streams

	char*	base;			// pointer to beginning of buffer
	char*	pptr;			// pointer to next free byte
	char*	gptr;			// pointer to next filled byte
	char*	eptr;			// pointer to first byte following buffer
	char	alloc;			// true if buffer is allocated using "new"
	FILE*	fp;			// for stdio compatibility
	
	virtual overflow(int c=EOF);	// Empty a buffer.
					// Return EOF on error
	 				//        0 on success

	virtual int underflow();	// Fill a buffer
					// Return EOF on error or end of input
	 				//	  next character on success
	
	inline int sgetc()		// get the current character
	{
		return (gptr>=pptr) ? underflow() : *gptr&0377;
	}

	
	inline int snextc()		// get the next character
	{
		return (gptr>=(pptr-1)) ? underflow() : *++gptr&0377;
	}

	inline void stossc()		// advance to the next character
	{
		if (gptr++ >= pptr)
			underflow();
	}

	inline void sputbackc(char c)
	/*
		Return a character to the buffer (ala lookahead 1).  Since
		the user may be "playing games" the character might be 
		different than the last one returned by sgetc or snextc.
		If the user attempts to put back more characters than have
		been extracted, nothing will be put back.
		Putting back an EOF is DANGEROUS.
	*/
	{
		if (gptr > base) *--gptr = c;
	}

	inline int sputc(int c =EOF)	// put a character into the buffer
	{
		if (fp == 0)
			return (eptr<=pptr) ? overflow(c&0377) : (*pptr++=c&0377);
		else
			return putc(c, fp);
	}

	
	inline streambuf * setbuf(char *p, int len, int count =0)
	/*
		supply an area for a buffer.
		The "count" parameter allows the buffer to start in non-empty.
	*/
	{
		base = gptr = p;
		pptr = p + count;
		eptr = base + len;
		return this;
	}

	int doallocate();		// allocate some space for the buffer
	inline int allocate(){ return base==0 ? doallocate() : 0; }

	streambuf() { base = gptr = pptr = eptr = 0; alloc = 0; fp = 0; }
	streambuf(char* p, int l) { setbuf(p,l); alloc = 0; }
	~streambuf() { if (base && alloc) delete base; }
};

extern int close(int);

struct filebuf : public streambuf {	// a stream buffer for files

	int	fd;			// file descriptor
	char	opened;			// non-zero if file has been opened

	int overflow(int c=EOF);	// Empty a buffer.
					// Return EOF on error
					//	  0 on success
	
	int underflow();		// Fill a buffer.
	 				// Return EOF on error or end of input
	 				//        next character on success
	 
	filebuf* open(/*const*/ char *name, open_mode om);	// Open a file
	 						// return 0 if failure
							// return "this" if success
	int 	close() { int i = opened?::close(fd):0; opened=0; return i; }

	filebuf() { opened = 0; fp = 0; }
	filebuf(FILE* p) { fp = p; fd = p->_file; opened = 1; }
	filebuf(int nfd) { fd = nfd; opened = 1; }
	filebuf(int nfd, char* p, int l) : (p,l) { fd = nfd; opened = 1; }
	~filebuf() { close(); }
};

struct circbuf : public streambuf {	// a circular stream buffer

	int overflow(int c=EOF);	// Empty a buffer.
	 				// Return EOF on error
	 				//	  0 on success
	
	int underflow();		// Fill a buffer.
					// Return EOF on error or end of input
	 				//        next character on success
	circbuf() { }
	~circbuf() { }

};

/*
 *	This type defines white space.  Any number of whitespace
 *	characters can be used to separate two fields in an input
 *	stream.  The effect of sending an input stream to a whitespace
 *	object is to cause all whitespace in the input stream, up to the
 *	next non-whitespace character, to be discarded.  The whitespace
 *	characters are space, tab, form feed, and new line.
 */
struct whitespace { };

/***************************** output: *********************************/

extern char* oct(long, int =0);
extern char* dec(long, int =0);
extern char* hex(long, int =0);

extern char* chr(int, int =0);		// chr(0) is the empty string ""
extern char* str(const char*, int =0);
extern char* form(const char* ...);		// printf format

class istream;
class common;

class ostream {
friend istream;

	streambuf* bp;
	short	state;
public:
	ostream& operator<<(const char*);	// write
	ostream& operator<<(int a) { return *this<<long(a); }
	//ostream& operator<<(unsigned a) { return *this<<form("%u",a); }
	ostream& operator<<(unsigned a) 
		{ return *this<<(unsigned long) a; }
	ostream& operator<<(unsigned long);
	ostream& operator<<(long);	// beware: << 'a' writes 97
	ostream& operator<<(double);
	ostream& operator<<(const streambuf&);
	ostream& operator<<(const whitespace&);
	ostream& operator<<(const common&);

	ostream& put(char);		// put('a') writes a
	ostream& flush() { bp->overflow(); return *this; }


		operator void*(){ return _eof<state?0:this; }
	int	operator!()	{ return _eof<state; }
	int	eof()		{ return state&_eof; }
	int	fail()		{ return _eof<state; }
	int	bad()		{ return _fail<state; }
	int	good()		{ return state==_good; }
	void	clear(state_value i =0)	{ state=i; }
	int	rdstate()	{ return state; }
	char*	bufptr()	{ return bp->base; }

		ostream(streambuf* s) { bp = s; state = 0; }
		ostream(int fd) { bp = new filebuf(fd); state = 0; }
		ostream(int size, char* p)
		{
			state = 0;
			bp = new streambuf();
			if (p == 0) p = new char[size];
			bp->setbuf(p, size);
		}
		~ostream() { flush(); }
};

/***************************** input: ***********************************/

/*
	The >> operator reads after skipping initial whitespace
	get() reads but does not skip whitespace

	if >> fails	(1) the state of the stream turns non-zero
			(2) the value of the argument does not change
			(3) non-whitespace characters are put back onto the stream

	>> get() fails if the state of the stream is non-zero
*/

class istream {
friend ostream;

	streambuf*	bp;
	ostream*	tied_to;
	char		skipws;		// if non-null, automaticly skip whitespace
	short		state;

	friend void eatwhite (istream&);
public:
	int	 skip(int i) { int ii=skipws; skipws=i; return ii; }

	/*
		formatted input: >> skip whitespace
	*/
	istream& operator>>(char*);			// string
	istream& operator>>(char&);			// single character
	istream& operator>>(short&);
	istream& operator>>(int&);
	istream& operator>>(long&);
	istream& operator>>(float&);
	istream& operator>>(double&);
	istream& operator>>(streambuf&);
	istream& operator>>(whitespace&);		// skip whitespace
	istream& operator>>(common&);

	/*
		raw input: get's do not skip whitespace
	*/
	istream& get(char*, int, char ='\n');		// string
	istream& get(streambuf& sb, char ='\n');
	istream& get(char& c)				// single character
	{
		int os = skipws;
		skipws = 0;
		*this >> c;
		skipws = os;
		return *this;
	}

	istream& putback(char c);
	ostream* tie(ostream* s) { ostream* t = tied_to; tied_to = s; return t; }

		operator void*(){ return _eof<state?0:this; }
	int	operator!()	{ return _eof<state; }
	int	eof()		{ return state&_eof; }
	int	fail()		{ return _eof<state; }
	int	bad()		{ return _fail<state; }
	int	good()		{ return state==_good; }
	void	clear(state_value i =0)	{ state=i; }
	int	rdstate()	{ return state; }
	char*	bufptr()	{ return bp->base; }	

		istream(streambuf* s, int sk =1, ostream* t =0)	// bind to buffer
		{
			state = 0;
			skipws = sk;
			tied_to = t;
			bp = s;
		}

		istream(int size, char* p, int sk =1)		// bind to string
		{
			state = 0;
			skipws = sk;
			tied_to = 0;
			bp = new streambuf();
			if (p == 0) p = new char[size];
			bp->setbuf(p, size, size);
		}

		istream(int fd, int sk =1, ostream* t =0)	// bind to file
		{
			state = 0;
			skipws = sk;
			tied_to = t;
			bp = new filebuf(fd);
		}
};


extern istream cin;	// standard input predefined
extern ostream cout;	// standard output
extern ostream cerr;	// error output

extern whitespace WS;	// predefined white space

#endif
