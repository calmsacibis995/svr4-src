/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/edlina/space.c	1.3.1.1"

#include "sys/enet.h"
#include "sys/edlina.h"
#include "config.h"

#define N_BOARDS		1	/* only one board currently supported */
#define	EDL_MAX_BUFS_POSTED	16

int edl_major_to_board[N_BOARDS]	= {0,	};
int edl_boards				= {N_BOARDS};
int edl_max_bufs_posted			= EDL_MAX_BUFS_POSTED;

/*
 * "edl_ifname" is used to name the internet statistics structure.  It should
 *  match the interface prefix specified in the strcf(4) file and ifconfig(1M)
 *  command used in rc.inet.
 */
int  edl_inetstats = 1;		/* 1 = keep inet stats - 0 = don't */
char *edl_ifname = "emd";

struct_edlina_t	edl_board_struct[N_BOARDS];
