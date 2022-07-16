#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucblint:lint.sh	1.1.3.1"
#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

# lint command for BSD compatibility package:
#
#	BSD compatibility package header files (/usr/ucbinclude)
#	are included before SVr4 default (/usr/include) files but 
#       after any directories specified on the command line via 
#	the -I option.  Thus, the BSD header files are included
#	next to last, and SVr4 header files are searched last.
#	
#	BSD compatibility package libraries are searched first.
#
#	Because the BSD compatibility package C lint library does not 
#	contain all the C library routines of /usr/ccs/lib/llib-lc, 
#	the BSD package C library is named /usr/ucblib/llib-lucb
#	and is passed explicitly to lint.  This ensures that llib-lucb
#	will be searched first for routines and that 
#	/usr/ccs/lib/llib-lc will be searched afterwards for routines 
#	not found in /usr/ucblib/llib-lucb.  Also because sockets is    
#       provided in libc under BSD, /usr/lib/llib-lsocket and 
#	/usr/lib/llib-lnsl are also included as default libraries.
#	
#	Note: Lint does not allow you to reset the search PATH for
# 	libraries. The following uses the -L option to point to
#	/usr/ucblib. There are however some combinations of options
#	specified by the user that could overrule the intended path.
#

/usr/ccs/bin/lint -L/usr/ucblib $@ -I/usr/ucbinclude -L/usr/ucblib -lucb -lsocket -lnsl
