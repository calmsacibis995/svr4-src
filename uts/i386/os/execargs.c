/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:execargs.c	1.1"

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/vmsystm.h"
#include "vm/faultcatch.h"


/*
	Compute the number of arguments and total size of strings
	in an argument list.

	int arglistsz(argv, argc_p, argsize_p, maxsize);

	where argv is the argument list ptr (a user address)
	and maxsize is the string size limit.

	Returns -2 if the string size is too long,
	-1 if a bad user address is supplied,
	otherwise returns 0 with *argc_p = argument count, and
	*argsize_p = total string size.
*/

int
arglistsz(argv, argc_p, argsize_p, maxsize)
	char	**argv;
	int	*argc_p;
	int	*argsize_p;
	int	maxsize;
{
	int	argc = *argc_p;
	int	argsize = *argsize_p;
	char	*argp;
	int	len;

	if (!valid_usr_range((caddr_t)argv, sizeof *argv))
		return -1;

	CATCH_FAULTS(CATCH_UFAULT) {
		while ((argp = *argv++) != NULL) {
			if (!valid_usr_range(argp, 1)) {
				END_CATCH();
				return -1;
			}
			if ((len = strlen(argp)) >= maxsize - argsize) {
				END_CATCH();
				return -2;
			}
			argsize += len + 1;
			++argc;
		}
	}
	if (END_CATCH() != 0)
		return -1;

	*argc_p = argc;
	*argsize_p = argsize;

	return 0;
}


/*
	Copy an argument list and its string
	into a compact form at an interim address.
	Although we can assume the interim addresses are OK,
	the pointers in the array must be rechecked because
	their image could be in shared writable pages

	int copyarglist(argc, from_argv, pdelta, to_argv, to_argp, from_kernel);

	where argc is the argument count.
	from_argv is the "from" argument pointer (a user address
	if from_kernel is zero, or a kernel/uarea address if from_kernel is 1),
	pdelta is the pointer delta to apply to the values
	in the new pointer list, to_argv is the "to" argument
	pointer (a user address), and to_argp is the "to"
	string pointer.

	The strings pointed to by the from pointer list
	are copied to the "to" string space and the new pointer
	list is constructed with pointers to the string starts
	offset by pdelta.  The reason for pdelta is that
	exec first builds the stack frame at the wrong place
	(since it cannot clobber existing data), then the image
	is moved to the right place virtually in the new address space.

	The size of the copied strings is returned on success
	and -1 is returned on failure.
*/

int
copyarglist(argc, from_argv, pdelta, to_argv, to_argp, from_kernel)
	int	argc;
	char	**from_argv;
	int	pdelta;
	char	**to_argv;
	char	*to_argp;
	int	from_kernel;
{
	char	*to_base = to_argp;
	char	*from_argp;

	/*
		Note that the from_argv has already been checked
		but the string pointers can get clobbered because
		they may be in shared pages that change.
		Thus the pointers must get checked.
		But the strings need not get checked for length now.
		This relies on there being no virtual neighbor at the end
		of the temporary image virtual space.
		Thus, there would be a fault that fails.
		Of course, implementations on other machines must make sure
		that an overrun will not be processed as a stack growth.
		That affects the code that choses the virtual hole to use.
	*/

	CATCH_FAULTS(CATCH_UFAULT) {
		while (argc-- > 0) {
			from_argp = *from_argv++;
			if (!from_kernel && !valid_usr_range(from_argp, 1)) {
				END_CATCH();
				return -1;
			}
			*to_argv++ = to_argp + pdelta;
			to_argp += strlen(strcpy(to_argp, from_argp)) + 1;
		}
	}
	if (END_CATCH() != 0)
		return -1;
	return to_argp - to_base;
}
