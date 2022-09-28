/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef	DEBUG_H
#define	DEBUG_H
/*==================================================================*/
/*
**
*/
#ident	"@(#)lp:include/debug.h	1.6.4.1"

#ifdef	DEBUG
#include	<stdio.h>

#define	_MAX_NEST_COUNT	256

extern	int	_nestCount;
extern	char	*_FnNames [];
extern	char	*_FnNamep;
extern	char	*_Unknownp;
extern	char	*_Nullp;
extern	FILE	*_DebugFilep;

#ifdef	__STDC__
#define	DEFINE_FNNAME(fnName) \
	static	char	FnName [] = #fnName;
#else
#define	DEFINE_FNNAME(fnName) \
	static	char	FnName [] = "fnName";
#endif

#define SET_DEBUG_PATH(pathp)  _SetDebugPath (pathp);
#define	OPEN_DEBUG_FILE(pathp) _OpenDebugFile (pathp);

#define _OPEN_DEBUG_FILE _OpenDebugFile ((char *)0);

#define DEBUGs(s1)      \
	_OPEN_DEBUG_FILE \
	(void)  fprintf (_DebugFilep, "%s\n",	\
	((char *) s1 == NULL ? _Nullp : (char *) s1));\
	(void)	fflush (stderr);

#define DEBUGss(s1, s2)	\
	_OPEN_DEBUG_FILE \
	(void)  fprintf (_DebugFilep, "%s\t= %s\n",	\
	((char *) s1 == NULL ? _Nullp : (char *) s1), \
	((char *) s2 == NULL ? _Nullp : (char *) s2)); \
	(void)	fflush (_DebugFilep);

#define DEBUGsd(s1, i1)	\
	_OPEN_DEBUG_FILE \
	(void)  fprintf (_DebugFilep, "%s\t= %d\n",	\
	((char *) s1 == NULL ? _Nullp : (char *) s1),	\
	(int) i1);					\
	(void)	fflush (_DebugFilep);

#define DEBUGssx(s1, s2, i1)	\
	_OPEN_DEBUG_FILE \
	(void)  fprintf (_DebugFilep, "%s/%s\t= 0x%x\n",	\
	((char *) s1 == NULL ? _Nullp : (char *) s1),	\
	((char *) s2 == NULL ? _Nullp : (char *) s2),	\
	(int) i1);					\
	(void)	fflush (_DebugFilep);

#define DEBUGssd(s1, s2, i1)	\
	_OPEN_DEBUG_FILE \
	(void)  fprintf (_DebugFilep, "%s/%s\t= %d\n",	\
	((char *) s1 == NULL ? _Nullp : (char *) s1),	\
	((char *) s2 == NULL ? _Nullp : (char *) s2),	\
	(int) i1);					\
	(void)	fflush (_DebugFilep);

#define DEBUGsss(s1, s2, s3)	\
	_OPEN_DEBUG_FILE \
	(void)  fprintf (_DebugFilep, "%s/%s\t= %s\n",	\
	((char *) s1 == NULL ? _Nullp : (char *) s1),	\
	((char *) s2 == NULL ? _Nullp : (char *) s2),	\
	((char *) s3 == NULL ? _Nullp : (char *) s3));\
	(void)	fflush (_DebugFilep);

#define	DUMP_BYTES(p, length) \
{ \
	unsigned int i, c; \
	unsigned char *cp; \
	_OPEN_DEBUG_FILE \
	for (i=0, cp=(unsigned char *)(p); i < length; i++, cp++) { \
		c = (unsigned int)*cp; \
		(void) fprintf (_DebugFilep, "[%02d] 0x%02x  ", i, c); \
		if (c & 0x80) (void) fprintf (_DebugFilep, "   "); \
		else { \
		c &= 0x7f; \
		if (c >= 0 && c <= 0x1f) \
		(void) fprintf(_DebugFilep,"^%c ", c+0x40); \
		else if (c == 0x20) (void) fprintf(_DebugFilep, "SP ");\
		else if (c == 0x7f) (void) fprintf(_DebugFilep, "DE ");\
		} \
		if (((i+1)%5) == 0) \
			(void)	fprintf (_DebugFilep, "\n"); \
	} \
	(void)	fprintf (_DebugFilep, "\n"); \
	(void)	fflush (_DebugFilep); \
}

#define	TRACEP(label)	\
	_OPEN_DEBUG_FILE \
	(void)	fprintf (_DebugFilep, "%s/%s\n", FnName,	\
	((char *) label == NULL ? "position" : (char *) label));\
	(void)	fflush (_DebugFilep);

#ifdef	__STDC__
#define	TRACE	TRACEx

#define	TRACEx(variable)	\
	DEBUGssx(FnName, #variable, variable)

#define	TRACEd(variable)	\
	DEBUGssd(FnName, #variable, variable)

#define	TRACEs(variable)	\
	DEBUGsss(FnName, #variable, variable)

#define	TRACEb(variable, length)	\
	DEBUGssx(FnName, #variable, variable);	\
	DUMP_BYTES(variable, length)

#else
#define	TRACE	TRACEx

#define	TRACEx(variable)	\
	DEBUGssx(FnName, "variable", variable)

#define	TRACEd(variable)	\
	DEBUGssd(FnName, "variable", variable)

#define	TRACEs(variable)	\
	DEBUGsss(FnName, "variable", variable)

#define	TRACEb(variable, length)	\
	DEBUGssx(FnName, "variable", variable);	\
	DUMP_BYTES(variable, length)

#endif

/*
#define	ENTRYP(fnNamep)	\
	if (_nestCount == _MAX_NEST_COUNT) \
		_FnNamep = fnNamep; \
	else \
		_FnNames [_nestCount++] = _FnNamep = fnNamep; \
	TRACEP("entry-point")

#define	EXITP \
	TRACEP("exit-point") \
	if (_nestCount == _MAX_NEST_COUNT || _nestCount == 0) \
		_FnNamep = _Unknownp; \
	else \
		_FnNamep = _FnNames [--_nestCount];
*/
#define	ENTRYP \
	TRACEP ("**ENTRY-POINT**")

#define	EXITP \
	TRACEP ("**EXIT-POINT**")

#ifdef	__STDC__
int	_OpenDebugFile (char *);
int	_SetDebugPath (char *);
#else
int	_OpenDebugFile ();
int	_SetDebugPath ();
#endif

#else

#define	DEFINE_FNNAME(fnName)
#define SET_DEBUG_PATH(pathp)
#define	OPEN_DEBUG_FILE(pathp)
#define DEBUGs(s1)
#define DEBUGss(s1, s2)
#define DEBUGsd(s1, i1)
#define DEBUGssx(s1, s2, i1)
#define DEBUGssd(s1, s2, i1)
#define DEBUGsss(s1, s2, s3)
#define	DUMP_BYTES(p, length)
#define	TRACEP(label)
#define TRACE(variable)
#define TRACEx(variable)
#define TRACEd(variable)
#define TRACEs(variable)
#define TRACEb(variable, length)
#define	ENTRYP
#define	EXITP

#define	_SetDebugPath(pathp)
#define	_OpenDebugFile(pathp)

#endif  /*  DEBUG  */
/*==================================================================*/

#endif	/*  DEBUG_H  */

