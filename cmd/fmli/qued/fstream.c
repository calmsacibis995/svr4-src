/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:qued/fstream.c	1.4"

#include	<stdio.h>
#include	<ctype.h>
#include        <curses.h>
#include	"wish.h"
#include	"terror.h"
#include	"token.h"
#include	"winp.h"
#include	"fmacs.h"
#include	"vtdefs.h"
#include	"ctl.h"	
#include	"attrs.h"

#define MAXSUBS		5

/* ODSH functions */
extern token singleline();
extern token multiline();
extern token editsingle();
extern token editmulti();

token (*Substream[][MAXSUBS])() = {
	{
		editsingle,
		singleline,
		NULL
	},
	{
		editsingle,
		editmulti,
		multiline,
		NULL
	},
};

token
field_stream(tok)
token tok;
{
	token stream();

	return(stream(tok, Substream[Currtype]));
}
