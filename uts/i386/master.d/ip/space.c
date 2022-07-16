/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)//usr/src/uts/i386/master.d/ip/space.c.sl 1.1 4.0 12/08/90 36785 AT&T-USL"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#define STRNET

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/ip_str.h>
#include <netinet/ip_var.h>

#define IPCNT	8
#define IPPROVCNT	16
#define IPSENDREDIRECTS	0

/* if IPFORWARDING is set, hosts will act as gateways */
#define IPFORWARDING	0

struct	ip_pcb ip_pcb[IPCNT];
struct 	ip_provider provider[IPPROVCNT];
int     ipcnt=IPCNT;
int	ipprovcnt=IPPROVCNT;
int	ipforwarding=IPFORWARDING;
int	ipsendredirects=IPSENDREDIRECTS;
