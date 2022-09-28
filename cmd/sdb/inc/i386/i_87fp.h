/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/i386/i_87fp.h	1.1"

/* 64-bit significand, 16-bit exponent */
typedef unsigned short fp_tempreal[5];

/* Save state for both the extended word 80287 and 80387 */
struct sdbfpstate_t {
	unsigned int fp_control;
	unsigned int fp_status;
	unsigned int fp_tag;
	unsigned int fp_ip;
	unsigned short fp_cs;
	unsigned short fp_ds;
	unsigned long fp_data_addr;
	unsigned long fp_unknown;
	fp_tempreal fp_stack[8];
	short new_fp_status;
	short fp_padding; /* for even longword alignment */
};

