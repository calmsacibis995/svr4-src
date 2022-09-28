/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/synsyms.h	1.4"
#ifdef __STDC__
#if 0
#define	ldahread	_ldahread
#define	ldclose		_ldclose
#define	ldaclose	_ldaclose 
#define	ldfhread	_ldfhread
#define	ldgetname	_ldgetname
#define	ldlread		_ldlread
#define	ldlinit		_ldlinit
#define	ldlitem		_ldlitem
#define	ldlseek		_ldlseek
#define	ldnlseek	_ldnlseek
#define	ldohseek	_ldohseek
#define	ldopen		_ldopen 
#define	ldaopen		_ldaopen
#define	ldrseek		_ldrseek
#define	ldnrseek	_ldnrseek
#define	ldshread	_ldshread
#define	ldnshread	_ldnshread
#define	ldsseek		_ldsseek
#define ldnsseek	_ldnsseek
#define	ldtbindex	_ldtbindex
#define	ldtbread	_ldtbread
#define	ldtbseek	_ldtbseek
#define	sputl		_sputl
#define	sgetl		_sgetl
#define allocldptr	_allocldptr
#define freeldptr	_freeldptr
#define vldldptr	_vldldptr
#define hdrassign	_hdrassign
#endif
#else
#define const /* for older compilers that don't recognize const */
#endif
