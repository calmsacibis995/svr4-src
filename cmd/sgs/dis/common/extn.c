/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:common/extn.c	1.5"

#include	<stdio.h>
#include	"dis.h"
#include	"structs.h"
#include	"ccstypes.h"
/*
 *	This file contains those global variables that are used in more
 *	than one source file. This file is meant to be an
 *	aid in keeping track of the variables used.  Note that in the
 *	other source files, the global variables used are declared as
 *	'static' to make sure they will not be referenced outside of the
 *	containing source file.
 */

Elf             *elf, *arf;
Elf_Arhdr       *mem_header;
Elf_Scn         *scn;
Elf32_Shdr      *shdr, *scnhdr;
Elf_Cmd         cmd;
Elf32_Ehdr      *ehdr;
Elf_Data        *data;
unsigned        char    *p_data, *ptr_line_data;
size_t		size_line;

int     	archive = 0,
		debug = 0,
		line  = 0,
		symtab = 0,
		Rel_sec = 0,
		Rela_data = 0,
		Rel_data = 0;
		


unsigned short	cur1byte;	/* for storing the results of 'get1byte()' */
unsigned short	cur2bytes;	/* for storing the results of 'get2bytes()' */
unsigned short	curbyte;

#ifdef AR32WR
	unsigned long	cur4bytes;/* for storing the results of 'get4bytes()' */
#endif

char	bytebuf[4];

/* aflag, oflag, trace, Lflag, sflag, and tflag are flags that are set
 * to 1 if specified by the user as options for the disassembly run.
 *
 * aflag	(set by the -da option) indicates that when disassembling
 *		a section as data, the actual address of the data should be 
 *		printed rather than the offset within the data section.
 * oflag	indicates that output is to be in octal rather than hex.
 * trace	is for debugging of the disassembler itself.
 * Lflag	is for looking up labels.
 * fflag	is for disassembling named functions.
 *		fflag is incremented for each named function on the command line.
 * ffunction	contains information about each named function
 * sflag	is for symbolic disassembly (VAX, U3B, N3B and M32 only)
 * tflag	(set by the -t option) used later to determine if the .rodata
		section was given as the section name to the -t option.
 * Sflag	is for forcing SPOP's to be disassembled as SPOP's (M32 only)
 * Rflag specifies to Reverse 286/386 mode for translating boot (I386 only).
 */

int	oflag = 0;
int	trace = 0;
int	Lflag = 0;
int	tflag = 0;
short	aflag = 0;
#if I386
int 	Rflag = 0;
#endif
int	fflag = 0;
int	sflag = 0;
int	Sflag = 0;
int	Fflag = 0;

NFUNC   *ffunction;

long	 loc;		/* byte location in section being disassembled	*/
			/* IMPORTANT: remember that loc is incremented	*/
			/* only by the getbyte routine			*/
char	object[NHEX];	/* array to store object code for output	*/
char	mneu[NLINE];	/* array to store mnemonic code for output	*/
char	symrep[NLINE];  /* array to store symbolic disassembly output	*/

char	*fname;		/* to save and pass name of file being processed*/	
char	*sname; 	/* to save and pass name of a section		*/

char	**namedsec;	/* contains names of sections to be disassembled */
int	*namedtype;	/* specifies whether the corresponding section
			 * is to be disassembled as text or data
			 */

int	nsecs = -1;		/* number of sections in the above arrays */

FUNCLIST	*next_function;	/* structure containing name and address  */
				/* of the next function in a text section */

SCNLIST		*sclist;	/* structure containing list of sections to
				   be disassembled */
