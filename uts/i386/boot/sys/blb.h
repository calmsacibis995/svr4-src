/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)boot:boot/sys/blb.h	1.1.2.1"

#ifndef lint
#pragma pack(01)
#endif
struct	blb	{
	unsigned long	flags;			/* bootstrap flags */
	caddr_t			DIB_off;		/* DIB offset */
	unsigned short	DIB_sel;		/* DIB selector */
	void			(*BEH_off)();	/* Boot Error Handler offset */
	unsigned short	BEH_sel;		/* Boot Error Handler selector */
	void			(*FOE_off)();	/* File Open Entry offset */
	unsigned short	FOE_sel;		/* File Open Entry selector */
	void			(*FRE_off)();	/* File Read Entry offset */
	unsigned short	FRE_sel;		/* File Read Entry selector */
	void			(*FCE_off)();	/* File Close Entry offset */
	unsigned short	FCE_sel;		/* File Close Entry selector */
	caddr_t			BMU_off;		/* Boot Memory Used offset */
	unsigned short	BMU_sel;		/* Boot Memory Used selector */
#ifdef MB1
	caddr_t			FNM_off;		/* File Name offset */
	unsigned short	FNM_sel;		/* File Name selector */
#endif
};
#ifndef lint
#pragma pack()
#endif

struct bsl_mem_used_list {
	paddr_t	block_start;
	ulong	block_length;
};
