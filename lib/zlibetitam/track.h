/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:track.h	1.1"
#ifndef _TRACK
#define _TRACK

#include "sys/window.h"

#define T_BEGIN		0x1
#define T_INPUT		0x2
#define T_END		0x4

#define TERR_OK		0
#define TERR_SYS	-1		/* same as EOF in <stdio.h> */
#define TERR_IOCTL	-2
#define TERR_WRITE	-3		/* if a write fails	*/

struct icon {			/* Put here to keep lint quite */
  int a;
};

typedef struct
{
	unsigned short	ti_x;		/* x position		*/
	unsigned short	ti_y;		/* y position		*/
	unsigned short	ti_w;		/* width		*/
	unsigned short	ti_h;		/* height		*/
	struct icon	*ti_icon;	/* icon			*/
	int		ti_val;		/* user value		*/
} tkitem_t;

typedef struct
{
	char		t_flags;	/* flags		*/
	char		t_scalex;	/* x & y scaling	*/
	char		t_scaley;
	short		t_lastx;
	short		t_lasty;
	struct icon	*t_bicon;	/* background icon	*/
	struct umdata	t_umdata;	/* save the mouse data	*/
	tkitem_t	*t_tkitems;	/* ptr to items		*/
	tkitem_t	*t_curi;	/* ptr to current item	*/
} track_t;

#endif /*_TRACK*/
