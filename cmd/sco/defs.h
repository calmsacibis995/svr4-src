/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:defs.h	1.1"

/*
 *	@(#) defs.h 1.1 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1984, 1985, 1986, 1987.
 *	Copyright (C) Microsoft Corporation, 1984, 1985, 1986, 1987.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */

#include <stdio.h>

#define LEXEOF  -1
#define INPUT   -2
#define OUTPUT  -3
#define DEAD    -4
#define COMPOSE -5
#define BEEP    -6
#define NULL_KEYWORD -7
#define NEWLINE -8
#define TOOBIG  -9
#define ERROR   -10

#define bool  char
#define FALSE 0
#define TRUE  1

#define SHIFT { --argc; ++argv; }
#define byte unsigned char
#define BNULL (byte) 0
#define CNULL (char) 0

/*
 * a little macro to extract an integer stuffed into a character array
 */
#define GET(i)  (buf[i] + buf[i+1]*256)

#define NL '\n'
#define SP ' '

#define MAP_DIR  "/usr/lib/mapchan"
#define MAPCHAN_DEF "/etc/default/mapchan"

#define NSECS   5

#define BAD_CHANNEL   (char *) 0

#define NULL_MAP_FILE 0
#define BAD_MAP_FILE  1
#define OKAY_MAP_FILE 2
