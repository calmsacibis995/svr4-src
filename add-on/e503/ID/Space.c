#ident	"@(#)Space.c	1.1	92/09/30	JPB"

static char SysVr3TCPID[] = "@(#)Space.c	3.3 Lachman System V STREAMS TCP source";
/*
 *	System V STREAMS TCP - Release 3.0
 *
 *	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */
#include "sys/types.h"
#include "sys/stream.h"
#include "sys/socket.h"
#include "net/if.h"
#include "sys/e503.h"

#include "config.h"

#define N3C503UNIT	E503_UNITS
#define N3C503MIN	E503_UNITS

unsigned int n3c503unit = N3C503UNIT;
unsigned int n3c503min = N3C503MIN;

e_addr e503eaddr[N3C503UNIT];
struct e503device e503device[N3C503UNIT];
struct en_minor e503_em[N3C503MIN*N3C503UNIT];

unsigned int e503afterboot[N3C503UNIT];
unsigned int e503inited[N3C503UNIT];
unsigned int e503nopens[N3C503UNIT];

/* set if using external Xcvr */
unsigned int e503xcvr[N3C503UNIT] = {0};	

/* interrupt level per board */
unsigned int e503intl[N3C503UNIT] = {E503_0_VECT};

unsigned int e503iobase[N3C503UNIT] = {E503_0_SIOA};

/* initialized in software */
unsigned int e503major[N3C503UNIT];		
