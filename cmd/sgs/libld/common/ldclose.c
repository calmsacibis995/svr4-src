/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldclose.c	1.8"
/*
* ldclose - close current object file.
*		if current object file is an archive member,
*		set up for next object file from archive.
*
* #ifdef PORTAR		printable ascii headers archive version
* #else #ifdef PORT5AR	UNIX 5.0 semi-portable archive version
* #else			pre UNIX 5.0 (old) archive version
* #endif
*/
#include <stdio.h>
#include <ar.h>
#include "filehdr.h"
#include "synsyms.h"
#include "ldfcn.h"

int
ldclose(ldptr)
	LDFILE *ldptr;
{
	extern int strncmp();
	extern int vldldptr();
	extern int freeldptr();

#ifdef PORTAR
	struct ar_hdr arhdr;
	long ar_size;

	if (vldldptr(ldptr) == FAILURE)
		return (SUCCESS);
	if (TYPE(ldptr) == ARTYPE &&
		FSEEK(ldptr, -((long)sizeof(arhdr)), BEGINNING) == OKFSEEK &&
		FREAD((char *)&arhdr, sizeof(arhdr), 1, ldptr) == 1 &&
		!strncmp(arhdr.ar_fmag, ARFMAG, sizeof(arhdr.ar_fmag)) &&
		sscanf(arhdr.ar_size, "%ld", &ar_size) == 1)
	{
		/*
		* Be sure OFFSET is even
		*/
		OFFSET(ldptr) += ar_size + (ar_size & 01); /* move to location
							      of next ar header */
		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
		   	FREAD((char *)&arhdr, sizeof(arhdr), 1, ldptr) == 1 &&
			!strncmp(arhdr.ar_fmag, ARFMAG, sizeof(arhdr.ar_fmag)) &&
			sscanf(arhdr.ar_size, "%ld", &ar_size) == 1){
			
			OFFSET(ldptr) += sizeof(arhdr);

			if ( ar_size >= FILHSZ ) { /* could be a COFF file */
				if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
					FREAD((char *)&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1) 
					return (FAILURE);
			}
			else  /* assume it's a non-COFF file */
			if ( ar_size > 0L && FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK){
					(HEADER(ldptr)).f_magic = 0;
					return (FAILURE);
				}	
		}
	
	}
#else
#ifdef PORT5AR
	struct arf_hdr arhdr;
	long ar_size, nsyms;
	extern long sgetl();

	if (vldldptr(ldptr) == FAILURE)
		return (SUCCESS);
	if (TYPE(ldptr) == ARTYPE &&
		FSEEK(ldptr, -((long)sizeof(arhdr)), BEGINNING) == OKFSEEK &&
		FREAD((char *)&arhdr, sizeof(arhdr), 1, ldptr) == 1)
	{
		ar_size = sgetl(arhdr.arf_size);
		/*
		* Be sure offset is even
		*/
		OFFSET(ldptr) += ar_size + sizeof(arhdr) + (ar_size & 01);
		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK &&
			FREAD((char *)&(HEADER(ldptr)), FILHSZ, 1, ldptr) == 1) {
			return (FAILURE);
			}
	}
#else
	ARCHDR arhdr;

	if (vldldptr(ldptr) == FAILURE)
		return (SUCCESS);
	if (TYPE(ldptr) == ARTYPE &&
		FSEEK(ldptr, -((long)ARCHSZ), BEGINNING) == OKFSEEK &&
		FREAD((char *)&arhdr, ARCHSZ, 1, ldptr) == 1)
	{
		/*
		* Be sure OFFSET is even
		*/
		OFFSET(ldptr) += arhdr.ar_size + ARCHSZ + (arhdr.ar_size & 01);
		if (FSEEK(ldptr, 0L, BEGINNING) == OKFSEEK)
			FREAD((char *)&(HEADER(ldptr)), FILEHSZ, 1, ldptr) == 1)
		{
			return (FAILURE);
		}
	}
#endif
#endif
	(void) fclose(IOPTR(ldptr));
	(void) freeldptr(ldptr);
	return (SUCCESS);
}
