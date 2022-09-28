/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldgetname.c	1.7"
#include <stdio.h>
#include "filehdr.h"
#include "syms.h"
#include "synsyms.h"
#include "ldfcn.h"

char *
ldgetname(ldptr, symentry)
	register LDFILE *ldptr;
	register const SYMENT *symentry;
{
	static char buffer[BUFSIZ + SYMNMLEN];
	extern int vldldptr();
	extern char *strncpy();
#ifdef FLEXNAMES
	extern char *strcpy();
	extern char *malloc();
	extern void free();
	static char *strtab = NULL;
	static int last_fnum_ = 0;
	static long last_offset = -1L;
	static long length;
#endif

	if (vldldptr(ldptr) != SUCCESS)
		return (NULL);
#ifdef FLEXNAMES
	if (symentry->n_zeroes == 0L)	/* stored in string table */
	{
		/*
		* Different archive members are noted by the OFFSET change.
		* Otherwise, normal other ldptr's are distinguished by
		* the different _fnum_'s.
		*/
		if (last_offset != OFFSET(ldptr) ||
			last_fnum_ != ldptr->_fnum_)
		{
			long string_begin, home;

			if (strtab != NULL)
				free(strtab);
			string_begin = HEADER(ldptr).f_symptr +
				HEADER(ldptr).f_nsyms * SYMESZ;
			home = FTELL(ldptr);
			if (FSEEK(ldptr, string_begin, BEGINNING) != OKFSEEK ||
				FREAD((char *)&length, sizeof(long), 1, ldptr)
				!= 1 ||
				(strtab = malloc((unsigned)length)) == NULL ||
				FREAD(strtab + sizeof(long),(int)( sizeof(char)),
				(int)(length - sizeof(long)), ldptr)
				!= length - sizeof(long) ||
				fseek(IOPTR(ldptr), home, 0) != OKFSEEK ||
				strtab[length - 1] != '\0')
			{
				/*
				* Try to have no side effects if an error
				* occurs somewhere...
				*/
				(void)fseek(IOPTR(ldptr), home, 0);
				return (NULL);
			}
			last_fnum_ = ldptr->_fnum_;
			last_offset = OFFSET(ldptr);
		}
		if (length <= symentry->n_offset)
			return (NULL);
		return (strcpy(buffer, strtab + symentry->n_offset));
	}
	else	/* still in old COFF location */
#endif
	{
		(void)strncpy(buffer, symentry->n_name, SYMNMLEN);
		buffer[SYMNMLEN] = '\0';
		return (buffer);
	}
}

