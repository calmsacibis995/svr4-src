/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:sysent.h	1.1"

/* the syscalls table specifies how the arguments and return value
 * of each system call are to be processed.
 */

#define NARGMAX 6       /* max number of system call args */
#define NSYSENT 128     /* number of system call entries */

/* argument types */
#define INT     1
#define UINT    2
#define PTR     3
#define LONG    4
#define ZERO    5
#define SPECIAL 6

struct sysent {
	int (*routine)();       /* system call processing routine */
	char types[NARGMAX+1];  /* return value types */
};

extern struct sysent sysent[];
