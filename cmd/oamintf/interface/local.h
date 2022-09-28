/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/local.h	1.2.1.2"
#define MENU 0		/* menu line in object */
#define HELP 1		/* help line in object */
#define LIFE 2		/* lifetime line in object */
#define STAR 3		/* star ident line line object */
#define ROWS 4		/* rows line in object */
#define ERR_RET	-1	/* error return code from get_desc() */

#define MENUPFX		"Menu."		/* menu filename prefix */
#define FORMPFX		"Form."		/* form filename prefix */
#define TEXTPFX		"Text."		/* text filename prefix */
#define MENUDESC	"menu="		/* menu descriptor within menu object */
#define FORMDESC	"form="		/* form descriptor within form object */
#define TEXTDESC	"text="		/* text descriptor within text object */

#define PFXSIZ		(sizeof(MENUPFX)-1)
