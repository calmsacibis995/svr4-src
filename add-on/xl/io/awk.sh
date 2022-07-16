#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)xl:io/awk.sh	1.3"
{
printf( "%s\n", $0 )
if ( $1 ~ /^		\/encm/ ){
	f1 = substr($1, length($1), 1 )
	printf( "\t\txor	%d(%%edi),%%e%sx	/eax = new r0 bytes\n", $4 * 1024, f1 )
	printf( "\t\txor	%%e%sx,%%e%sx		/set up edx for later\n", $2, $3 )
	printf( "\t\tmovb	%%%sl,%%bl 		/multiply 4 bytes by c0\n", f1 )
	printf( "\t\txorb	(%%ebx,%%ebp),%%%sl		/ and xor to ecx\n", $2 )
	printf( "\t\tmovb	%%%sh,%%bl 		/ ecx will be new r2\n", f1 )
	printf( "\t\txorb	(%%ebx,%%ebp),%%%sh\n", $2 )
	printf( "\t\trol	$16,%%e%sx\n", f1 )
	printf( "\t\trol	$16,%%e%sx\n", $2 )
	printf( "\t\tmovb	%%%sl,%%bl\n", f1 )
	printf( "\t\txorb	(%%ebx,%%ebp),%%%sl\n", $2 )
	printf( "\t\tmovb	%%%sh,%%bl\n", f1 )
	printf( "\t\txorb	(%%ebx,%%ebp),%%%sh\n", $2 )
	printf( "\t\trol	$16,%%e%sx\n", f1 )
	printf( "\t\trol	$16,%%e%sx\n", $2 )
	printf( "\t\txor	%%e%sx,%%e%sx		/edx = new r1\n", $2, $3 )
}
}
