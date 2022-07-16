#ident	"@(#)extn.c	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/extn.c	1.3.1.1"
/* @(#)dis_extn.c	10.2 */

#include "sys/types.h"
#include "../db_as.h"
#include "dis.h"
#include "structs.h"

/*
 *	This file contains those global variables that are used in more
 *	than one source file. This file is meant to be an
 *	aid in keeping track of the variables used.  Note that in the
 *	other source files, the global variables used are declared as
 *	'static' to make sure they will not be referenced outside of the
 *	containing source file.
 */

unsigned short	curbyte;	/* for storing the results of 'fetchbyte()' */
unsigned short	cur1byte;	/* for storing the results of 'get1byte()' */
unsigned short	cur2bytes;	/* for storing the results of 'get2bytes()' */
#ifdef AR32WR
	unsigned long	cur4bytes;		/* for storing the results of 'get4bytes()' */
#else
	long	cur4bytes;
#endif
char	bytebuf[4];

as_addr_t loc;		/* byte location in section being disassembled	*/
			/* IMPORTANT: remember that loc is incremented	*/
			/* only by the fetchbyte routine		*/
char	mneu[NLINE];	/* array to store mnemonic code for output	*/
