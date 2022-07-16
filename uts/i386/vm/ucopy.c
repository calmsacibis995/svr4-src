#ident	"@(#)ucopy.c	1.2	92/05/13	JPB"
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-vm:ucopy.c	1.2.1.1"

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/vmsystm.h"
#include "vm/faultcatch.h"

/*
 * This is a collection of routines which access user addresses.
 * They provide protection against user address page fault errors using
 * CATCH_FAULTS, and validate user permissions.
 */

int
ucopyin(src, dst, cnt, catch_flags)
	caddr_t	src, dst;
	u_int	cnt, catch_flags;
{
	if (cnt == 0)		/* Explicit check for 0 count,		   */
		return 0;	/*   since it will fail valid_usr_range(). */
	if (RF_SERVER())
		return rcopyin(src, dst, cnt, 0);
	if (!valid_usr_range(src, cnt))
		return EFAULT;
	CATCH_FAULTS(catch_flags | CATCH_UFAULT) {
		bcopy(src, dst, cnt);
	}
	return END_CATCH();
}

int
copyin(src, dst, cnt)
	caddr_t	src, dst;
	u_int	cnt;
{
	return ucopyin(src, dst, cnt, 0) ? -1 : 0;
}


int
ucopyout(src, dst, cnt, catch_flags)
	caddr_t	src, dst;
	u_int	cnt, catch_flags;
{
	if (cnt == 0)		/* Explicit check for 0 count,		   */
		return 0;	/*   since it will fail valid_usr_range(). */
	if (RF_SERVER())
		return rcopyout(src, dst, cnt, NULL);
	if (!writeable_usr_range(dst, cnt))
		return EFAULT;
	CATCH_FAULTS(catch_flags | CATCH_UFAULT) {
		bcopy(src, dst, cnt);
	}
	return END_CATCH();
}

int
copyout(src, dst, cnt)
	caddr_t	src, dst;
	u_int	cnt;
{
	return ucopyout(src, dst, cnt, 0) ? -1 : 0;
}


int
lfubyte(addr)
	caddr_t	addr;
{
	u_int	val;

	if (!valid_usr_range(addr, sizeof *addr))
		return -1;
	CATCH_FAULTS(CATCH_UFAULT)
		val = (u_int)*addr;
	if (END_CATCH() != 0)
		return -1;
	return (int)val;
}

int
fubyte(addr)
	caddr_t	addr;
{
	if (RF_SERVER())
		return rfubyte(addr);
	return lfubyte(addr);
}

int
fuibyte(addr)
	caddr_t	addr;
{
	return fubyte(addr);
}


int
lfuword(addr)
	int	*addr;
{
	int	val;

	if (!valid_usr_range((caddr_t)addr, sizeof *addr))
		return -1;
	CATCH_FAULTS(CATCH_UFAULT)
		val = *addr;
	if (END_CATCH() != 0)
		return -1;
	return val;
}

int
fuword(addr)
	int	*addr;
{
	if (RF_SERVER())
		return rfuword(addr);
	return lfuword(addr);
}

int
fuiword(addr)
	int	*addr;
{
	return fuword(addr);
}


int
lsubyte(addr, val)
	caddr_t	addr;
	char	val;
{
	if (!writeable_usr_range(addr, sizeof *addr))
		return -1;
	CATCH_FAULTS(CATCH_UFAULT)
		*addr = val;
	if (END_CATCH() != 0)
		return -1;
	return 0;
}

#if defined(__STDC__)
int
subyte(caddr_t addr, char val)
#else
int
subyte(addr, val)
	caddr_t	addr;
	char	val;
#endif
{
	if (RF_SERVER())
		return rsubyte(addr, val);
	return lsubyte(addr, val);
}

int
suibyte(addr, val)
	caddr_t	addr;
	char	val;
{
	return subyte(addr, val);
}


int
lsuword(addr, val)
	int	*addr;
	int	val;
{
	if (!writeable_usr_range((caddr_t)addr, sizeof *addr))
		return -1;
	CATCH_FAULTS(CATCH_UFAULT)
		*addr = val;
	if (END_CATCH() != 0)
		return -1;
	return 0;
}

int
suword(addr, val)
	int	*addr;
	int	val;
{
	if (RF_SERVER())
		return rsuword(addr, val);
	return lsuword(addr, val);
}

int
suiword(addr, val)
	int	*addr;
	int	val;
{
	return suword(addr, val);
}


void
addupc(pc, ticks)
	void	(*pc)();
	int	ticks;
{
	register int	offset;
	register u_short *bucket_p;

	if ((offset = (int)pc - (int)u.u_prof.pr_off) < 0)
		return;
	offset = upc_scale(offset, u.u_prof.pr_scale);
	if (offset >= u.u_prof.pr_size / sizeof (u_short))
		return;
	bucket_p = u.u_prof.pr_base + offset;
	if (!writeable_usr_range((caddr_t)bucket_p, sizeof (u_short))) {
		u.u_prof.pr_scale = 0;
		return;
	}
	CATCH_FAULTS(CATCH_UFAULT) {
		*bucket_p += ticks;
	}
	if (END_CATCH() != 0)
		u.u_prof.pr_scale = 0;
}


int
upath(from, to, maxbufsize)
	caddr_t	from, to;
	size_t	maxbufsize;
{
	int	len;

	if (!valid_usr_range(from, 1))
		return -1;
	CATCH_FAULTS(CATCH_UFAULT) {
		len = spath(from, to, maxbufsize);
	}
	if (END_CATCH() != 0)
		return -1;
	return len;
}


int
userstrlen(str)
	caddr_t	str;
{
	int	len;

	if (!valid_usr_range(str, 1))
		return -1;
	CATCH_FAULTS(CATCH_UFAULT) {
		len = strlen(str);
	}
	if (END_CATCH() != 0)
		return -1;
	return len;
}


/*
 * Zero a user buffer with protection against user page fault errors.
 * A fault error in any pageable user address will cause a non-zero
 * errno to be returned.
 */
int
uzero(dst, cnt)
	caddr_t	dst;
	u_int	cnt;
{
	CATCH_FAULTS(CATCH_UFAULT)
		bzero(dst, cnt);
	return END_CATCH();
}


/* fc_jmpjmp -- Standard handler for catching page fault errors
 * At startup time, u.u_fault_catch.fc_func gets set to this (see faultcatch.h)
 *
 * NOTE: This function should really be in a different file, something like
 * a pagefault.c if and when such exists.
 */

void
fc_jmpjmp()
{
	longjmp(&u.u_fault_catch.fc_jmp);
}
