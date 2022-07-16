/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:s5/s5getsz.c	1.2"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/vnode.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5fblk.h"
#include "sys/fs/s5filsys.h"
#include "sys/fs/s5ino.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/s5macros.h"
