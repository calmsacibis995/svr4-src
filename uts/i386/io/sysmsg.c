/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:sysmsg.c	1.3"

/*
** Non DSG sysmsg driver.
*/

#include "sys/errno.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/sysmacros.h"
#include "sys/sysmsg.h"
#include "sys/cram.h"
#include "sys/termio.h"
#include "sys/bootinfo.h"
#include "sys/sysi86.h"
#include "sys/stream.h"
#include "sys/termio.h"
#include "sys/asy.h"

int smsgputchar();
int smsggetchar();
int smsg_check_cmos_baud();

#ifdef AT386
int kdputchar();
int asyputchar();
int asyputchar2();
int kdgetchar();
int asygetchar2();
unsigned char CMOSread();

extern struct smsg_flags smsg_flags;

int	smsg_have_cmos;
int	smsg_init = 0;

struct conssw conssw =
{
	kdputchar,
	0,
	kdgetchar
};

#endif

smsgopen(dev)
dev_t dev;
{
	if (minor(dev) != 0)
	{
		return (ENXIO);
	}
}

smsgwrite()
{
	unsigned char c;
	while (u.u_count) {
		if (copyin((char *)u.u_base, &c, 1)) {
			return (EFAULT);
		}
		u.u_count--;
		u.u_base++;
		(*conssw.co)(c);
	}
}

smsgioctl(dev, cmd, arg)
dev_t dev;
int cmd;
struct smsg_flags *arg;
{
	u.u_error = EFAULT;
	return (EFAULT);
}

smsginit()
{
	return (EFAULT);
}

int
smsgputchar(c)
unsigned char c;
{
	(*conssw.co)(c);
}

int
smsggetchar()
{
	return ((*conssw.ci)());
	
}

int
smsg_check_bios()
{
	if ((bootinfo.id[0] == 'I') &&
	    (bootinfo.id[1] == 'D') &&
	    (bootinfo.id[2] == 'N') &&
	    (bootinfo.id[3] == 'O'))
	{
		if ((bootinfo.id[4] == C2) ||
		    (bootinfo.id[4] == C3) ||
		    (bootinfo.id[4] == C4))
			return(1);
	}
	/* not a machine with console byte in the BIOS */
	return(0);
}

int
smsg_check_cmos_baud(port, default_baud)
int port, default_baud;
{
	return(default_baud);
}

smsg_program_asy_port(port)
int port;
{
}
