/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/state.c	1.1"
#include <sccs.h>

SCCSID("@(#)state.c	1.2	11:02:30	1/30/90");

/*
   (c) Copyright 1985 by Locus Computing Corporation.  ALL RIGHTS RESERVED.

   This material contains valuable proprietary products and trade secrets
   of Locus Computing Corporation, embodying substantial creative efforts
   and confidential information, ideas and expressions.	 No part of this
   material may be used, reproduced or transmitted in any form or by any
   means, electronic, mechanical, or otherwise, including photocopying or
   recording, or in connection with any information storage or retrieval
   system without permission in writing from Locus Computing Corporation.
*/

/*--------------------------------------------------------------------------
 *
 *	state.c - handle an open file's state change
 *
 *	routines included:
 *		rlockState()
 *
 *	comments:
 *		this file is present to resolve external references.  if
 *		the caller needs to handle special circumstances that are
 *		affected by the state of a file that is opened by this
 *		package, he will have to write an alternate version of
 *		this function.
 *
 *--------------------------------------------------------------------------
 */

/* 
 *	rlockState() - handle an open file's state change
 *
 *	input:	openEntry - index into the global open file table
 *		state - the state change itself
 *
 *	proc:	this is simply a stub function, used to resolve external
 *		references.
 *
 *	output:	(void) - none
 *
 *	global:	(none)
 */

void
rlockState(openEntry, state)
int openEntry, state;
{
}
