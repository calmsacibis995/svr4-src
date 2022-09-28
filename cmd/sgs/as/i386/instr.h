/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/instr.h	1.1"

#define INSTR_NAME_LEN 12	/* large enough? */
typedef	struct instag
	{
		char	name[INSTR_NAME_LEN];
		BYTE	tag;
		BYTE	val;
		BYTE	nbits;
		long	opcode;
		struct instag *next;
	} instr;

#define INSTR sizeof(instr);

extern void mk_hashtab();
extern instr * instlookup();
