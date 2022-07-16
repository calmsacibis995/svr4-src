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

#ident	"@(#)mbus:uts/i386/master.d/mb2stai/space.c	1.3"

#include <sys/types.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/mps.h>
#include <sys/mb2stai.h>

#define NO_ENDPS 28
#define MB2_DEFRCV_HIWAT 16*64  /* the weighted size of 16 64 byte messages */
#define MB2_DEFRCV_LOWAT 128
#define MB2_DEFSND_HIWAT 16*64  /* the weighted size of 16 64 byte messages */
#define MB2_DEFSND_LOWAT 128
#define MAXMSGS	 1024


int 	mb2t_dbg_level = 0;

int mb2t_no_endps = NO_ENDPS;
int mb2t_defrcv_hiwat = MB2_DEFRCV_HIWAT;
int mb2t_defrcv_lowat = MB2_DEFRCV_LOWAT;
int mb2t_defsnd_hiwat = MB2_DEFSND_HIWAT;
int mb2t_defsnd_lowat = MB2_DEFSND_LOWAT;
ulong mb2t_max_send_hiwat = 0xf000;
ulong mb2t_max_recv_hiwat = 0xf000;
ushort mb2t_first_port = 0x200;
ulong mb2t_maxmsgs = MAXMSGS;
ulong mb2t_max_rsvp_tout = 0xffff;
ulong mb2t_def_rsvptout = 2000;
ulong mb2t_max_frag_tout = 0xffff;
ulong mb2t_def_fragtout = 2000;


end_point mb2t_endps[NO_ENDPS];
ushort mb2t_ports[NO_ENDPS];
struct mb2_prim_bind mb2t_pbind[MAXMSGS];
