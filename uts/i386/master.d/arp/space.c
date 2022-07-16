/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)//usr/src/uts/i386/master.d/arp/space.c.sl 1.1 4.0 12/08/90 13197 AT&T-USL"

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>

#define STRNET

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#define ARPTAB_BSIZ	9
#define ARPTAB_NB	19
#define ARPTAB_SIZE	(ARPTAB_BSIZ * ARPTAB_NB)

struct arptab	arptab[ARPTAB_SIZE];
int		arptab_bsiz = ARPTAB_BSIZ;
int		arptab_nb = ARPTAB_NB;
int		arptab_size = ARPTAB_SIZE;
