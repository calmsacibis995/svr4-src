/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/p1allo.h	1.2"
/* p1allo.h */

/* Declarations for storage allocation. */

extern BITOFF al_struct();		/* allocate struct storage */
extern void al_auto();			/* allocate automatic var. */
extern void al_s_param();
extern void al_param();			/* allocate function param. offset */
extern OFFSET al_g_param();		/* get current arg. offset */
extern char * al_g_regs();		/* get current reg. array */
extern char * al_g_maxreg();		/* registers used in entire function */
extern OFFSET al_g_maxtemp();		/* get max. temp offset */
extern void al_e_param();
extern void al_begf();
extern void al_s_fcode();
extern void al_e_expr();
extern OFFSET al_endf();
extern void al_s_block();
extern void al_e_block();
extern void al_regupdate();		/* update register current usage list */
extern void al_call();
#ifndef	FAT_ACOMP
extern int al_reg();
#define al_call(p)			/* do nothing for SAT */
#endif

/* Align offset "off" by alignment "align" */
#define	AL_ALIGN(off,align) (((off + align - 1) / align) * align)
				/* BITOFF AL_ALIGN(BITOFF off, int align) */
#ifdef FAT_ACOMP
/* Style of register allocation. */
#define	RA_GLOBAL	1	/* Global */
#define	RA_OLDSTYLE	2	/* the Old way, lexically, with "register" */
#define	RA_NONE		0	/* No register allocation */

#ifndef	RA_DEFAULT
#define	RA_DEFAULT RA_GLOBAL	/* Default == global. */
#endif
extern int al_regallo;		/* register allocation style */
#endif
