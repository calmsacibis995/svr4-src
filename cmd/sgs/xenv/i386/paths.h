/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xenv:i386/paths.h	1.1.1.3"
/*
 *
 *
 *	Pathnames for i386
 */

/*
 *	Default directory search path for link-editor
 *	YLDIR says which directory in LIBPATH is replaced
 * 	by the -YL option to cc and ld.  YUDIR says
 *	which directory is replaced by -YU.
 */
#define LIBPATH	"I386LIBPATH"
#define YLDIR 1
#define YUDIR 2
/*	Directory containing library files and executables
 *	not accessed directly by users ("lib") files
 */
#define LIBDIR	"I386LIBDIR"

/*
 *	Directory containing executable ("bin") files
 */
#define BINDIR	"I386BINDIR"

/*	Absolute pathnames for dynamic shared library targets
 */
#define	LDSO_NAME "/usr/lib/ld.so.1"
#define	LIBCSO_NAME "/usr/lib/libc.so.1"

/*	Directory containing minimal abi library
 */
#define ABILIBDIR  "I386ABILIBDIR"
/*
 *	Directory containing include ("header") files for building tools
 */

#define INCDIR	"I386INCDIR"

/*
 *	Directory for "temp"  files
 */
#define TMPDIR	"I386TMPDIR"

/*
 *	Default name of output object file
 */
#define A_OUT	"SGSa.out"

/*
 *	The following pathnames will be used by the "cc" command
 *
 *	i386 cross compiler
 */
#define CPP	"I386CPP"
/*
 *	Directory containing include ("header") files for users' use
 */
#define INCLDIR	"-II386INCDIR"
#define COMP	"I386LIBDIR/comp"
#define C0	"I386LIBDIR/front"
#define C1	"I386LIBDIR/back"
#define OPTIM	"I386LIBDIR/optim"
/*
 *	i386 cross assembler
 */
#define AS	"I386BINDIR/SGSas"
#define AS1	"I386LIBDIR/SGSas1"	/* assembler pass 1 */
#define AS2	"I386LIBDIR/SGSas2"	/* assembler pass 2 */
#define M4	"I386BINDIR/SGSm4"	/* macro preprocessor */
#define CM4DEFS	"I386LIBDIR/cm4defs"	/* C interface macros */
#define CM4TVDEFS "I386LIBDIR/cm4tvdefs"	/* C macros with 'tv' call */
/*
 *	i386 link editor
 */
#define LD	"I386BINDIR/SGSld"
#define LD2	"I386LIBDIR/SGSld2"	/* link editor pass 2 */
