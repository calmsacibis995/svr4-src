/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:async/space.c	1.3"

#include "sys/types.h"
#include "sys/user.h"
#include "sys/param.h"
#include "sys/proc.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/vnode.h"
#include "sys/cred.h"
#include "sys/priocntl.h"
#include "sys/events.h"
#include "sys/evsys.h"
#include "sys/file.h"
#include "sys/asyncio.h"
#include "sys/asyncsys.h"

#include "config.h" /* for tunable parameters */

aioreq_t	aio[NAIOSYS];
int	aio_size = NAIOSYS;
int	min_aio_servers = MINAIOS;
int	max_aio_servers = MAXAIOS;
int	aio_server_timeout = AIOTIMEOUT;
short	naioproc = NAIOPROC;

