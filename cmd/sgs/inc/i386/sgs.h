/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xenv:i386/sgs.h	1.2.15.1"
/*
 */

#define	SGS	""

/*	The symbol I386MAGIC is defined in filehdr.h	*/

#define MAGIC	I386MAGIC
#define TVMAGIC (MAGIC+1)

#define ISMAGIC(x)	(x ==  MAGIC)


#ifdef ARTYPE
#define	ISARCHIVE(x)	( x ==  ARTYPE)
#define BADMAGIC(x)	((((x) >> 8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))
#endif


/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default
 *		PAGEMAGIC  : configured for paging
 */

#define AOUT1MAGIC 0407
#define AOUT2MAGIC 0410
#define PAGEMAGIC  0413
#define LIBMAGIC   0443

/* The first few .got and .plt entries are reserved
 *	PLT[0]	jump to dynamic linker (indirect through GOT[2])
 *
 *	GOT[0]	address of _DYNAMIC
 *	GOT[1]	link map address
 *	GOT[2]	address of rtbinder in rtld
 */
#define PLT_XRTLD	0	/* plt index for jump to rtld */
#define PLT_XNumber	1

#define GOT_XDYNAMIC	0	/* got index for _DYNAMIC */
#define GOT_XLINKMAP	1	/* got index for link map */
#define GOT_XRTLD	2	/* got index for rtbinder */
#define GOT_XNumber	3

#define	SGSNAME	""
#define PLU_PKG "Standard C Development Environment "
#define PLU_REL "(SCDE) 5.0  09/24/90"
#define CPL_PKG "Standard C Development Environment "
#define CPL_REL "(SCDE) 5.0  09/24/90"
#define SGU_PKG "Standard C Development Environment "
#define SGU_REL "(SCDE) 5.0  09/24/90"
#define ACU_PKG "Standard C Development Environment "
#define ACU_REL "(SCDE) 5.0  09/24/90"
#define ESG_PKG "Standard C Development Environment "
#define ESG_REL "(SCDE) 5.0  09/24/90"
#define CPPT_PKG "Standard C Development Environment "
#define CPPT_REL "(SCDE) 5.0  09/24/90"
