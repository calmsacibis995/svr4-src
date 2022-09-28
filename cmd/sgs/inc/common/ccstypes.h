/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:common/ccstypes.h	1.3"

#ifndef _SYS_TYPES_H
typedef	unsigned short uid_t;
typedef unsigned short gid_t;
typedef short pid_t;
typedef unsigned short mode_t;
typedef short nlink_t;
#endif
#ifdef uts
#	ifndef _SIZE_T
#		define _SIZE_T
#		ifndef size_t
#			define size_t	unsigned int
#		endif
#	endif
#endif
