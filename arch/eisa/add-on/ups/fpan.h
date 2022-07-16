/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)eisa:add-on/ups/fpan.h	1.3"

/*
 *	Copyright (C) Ing. C. Olivetti & C. S.p.a., 1989.
 *
 *	fpan.h:
 *
 *	front_panel include file.
 *
 */

#define	FPAN		('P' << 8)
#define FPAN_STATUS	(FPAN |	1)	/* ioctl panel status command	*/
#define	FPAN_PWOFF	(FPAN | 2)	/* switch the power off		*/
#define	FPAN_SHTPRG	(FPAN | 3)	/* set shutdown in progress bit */
#define	FPAN_SHTCMP	(FPAN | 4)	/* shutdown complete		*/
#define	FPAN_UPSSUP	(FPAN | 5)	/* system support for UPS	*/
#define	FPAN_DSPRST	(FPAN | 6)	/* reset the display after write*/

#define	FPAN_T		('T' << 8)
#define FPAN_TESTo	(FPAN_T | 1)

#define UPS_PRESENT	0x02		/* tests ups bit in sys. status	*/
#define PW_FAIL		0x08		/* running on battery power	*/
#define HIGH_TEMP	0x10		/* high temperature		*/
#define	BAT_LEVEL	0x04		/* low battery level		*/
#define	DISPLAY_TYPE	0x01		/* display type 16/24 chars	*/
