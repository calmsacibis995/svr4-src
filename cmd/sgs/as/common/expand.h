/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/expand.h	1.3"

#include "expand2.h"

/*	structure for span-dependent instruction entry
 */

typedef struct {
	long	sd_addr;	/* minimum address for this sdi */
	symbol	*sd_sym;	/* ptr to label in operand */
	short	sd_off;		/* value of constant in operand */
	ITYPE	sd_itype;	/* type, IT_... */
	char	sd_flags;	/* SDF_... */
} SDI;


#define SDF_BACK	1
#define SDF_BYTE	2
#define SDF_HALF	4
#define SDF_DONE	8


#ifndef	NSDI
#define NSDI	1000
#endif


typedef struct SDIBOX	SDIBOX;
struct	SDIBOX
{
	SDI	*b_end;		/* next free b_buf[] entry */
	SDIBOX	*b_link;
	SDI	b_buf[NSDI];
};


extern SDIBOX	*Sdibox;
extern void	deflab();
extern SDI	*Sdi;
extern SDI	*shortsdi();
