/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldopen.c	1.10"
/*
* ldopen - get LDFILE, header info for object file.
*		if it is an archive, get the first file from the archive.
*		if it is an already opened archive, assume ldclose() set
*		up everything already.
*
* #ifdef PORTAR		printable ascii header archive version
* #else #ifdef PORT5AR	UNIX 5.0 semi-portable archive version
* #else			pre UNIX 5.0 (old) archive version
* #endif
*/
#include <stdio.h>
#include <ar.h>
#include "filehdr.h"
#include "synsyms.h"
#include "ldfcn.h"

LDFILE *
ldopen(filename, ldptr)
	const char *filename;
	LDFILE *ldptr; 
{
	extern int strncmp();
	extern int vldldptr();
	extern LDFILE *allocldptr();
	extern int freeldptr();
	FILE *ioptr;
	unsigned short type;
#ifdef PORTAR
	struct ar_hdr arbuf;
	char buf[SARMAG];
#else
#ifdef PORT5AR
	extern long sgetl();
	struct ar_hdr arbuf;
	long nsyms;
#else
	unsigned short atype = 0;
#endif
#endif

	if (vldldptr(ldptr) == FAILURE)
	{
		if ((ioptr = fopen(filename, "r")) == NULL)
			return (NULL);
#ifdef PORTAR
		if (fread(buf, sizeof(char) * SARMAG, 1, ioptr) != 1)
			buf[0] = '\0';
		(void) fseek(ioptr, 0L, 0);
#else
#ifdef PORT5AR
		if (fread((char *)&arbuf, sizeof(arbuf), 1, ioptr) != 1)
			arbuf.ar_magic[0] = '\0';
		(void) fseek(ioptr, 0L, 0);
#endif
#endif
		if (fread((char *)&type,(int) (sizeof(type)), 1, ioptr) != 1 ||
			(ldptr = allocldptr()) == NULL)
		{
			(void) fclose(ioptr);
			return (NULL);
		}
#ifdef PORTAR
		if (strncmp(buf, ARMAG, SARMAG) == 0)
		{
			long ar_size;

			TYPE(ldptr) = USH_ARTYPE;
			if (fseek(ioptr, (long)(sizeof(char) * SARMAG), 0) ==
				OKFSEEK &&
				fread((char *)&arbuf,(int)(sizeof(arbuf)), 1, ioptr)
				== 1 &&
				!strncmp(arbuf.ar_fmag, ARFMAG,
				(int)(sizeof(arbuf.ar_fmag))) &&
				arbuf.ar_name[0] == '/' &&
				sscanf(arbuf.ar_size, "%ld", &ar_size) == 1)
			{
				OFFSET(ldptr) = sizeof(char) * SARMAG +
					2 * sizeof(struct ar_hdr) +
					((ar_size + 01) & ~01);
			}
			else
				OFFSET(ldptr) = sizeof(char) * SARMAG +
					sizeof(struct ar_hdr);
		}
		else
		{
			TYPE(ldptr) = type;
			OFFSET(ldptr) = 0L;
		}
#else
#ifdef PORT5AR
		if (strncmp(arbuf.ar_magic, ARMAG, SARMAG) == 0)
		{
			TYPE(ldptr) = USH_ARTYPE;
			nsyms = sgetl(arbuf.ar_syms);

			OFFSET(ldptr) = (nsyms * sizeof(struct ar_sym)) +
				sizeof(struct arf_hdr) + sizeof(arbuf);
		}
		else
		{
			TYPE(ldptr) = type;
			OFFSET(ldptr) = 0L;
		}
#else
		if (sizeof(ARTYPE) == sizeof(type) ||
			fread((char *)&atype, sizeof(atype), 1, ioptr) != 1)
		{
			atype = type;
		}
		if (atype == USH_ARTYPE ||
			type == USH_ARTYPE)
		{
			TYPE(ldptr) = USH_ARTYPE;
			OFFSET(ldptr) = ARCHSZ + sizeof(ARMAG);
		}
		else
		{
			TYPE(ldptr) = type;
			OFFSET(ldptr) = 0L;
		}
#endif
#endif
		IOPTR(ldptr) = ioptr;
		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
			FREAD((char *)&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1)
		{
			return (ldptr);
		}
	}
	else if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK)
		return (ldptr);
	(void) fclose(IOPTR(ldptr));
	(void) freeldptr(ldptr);
	return (NULL);
}

