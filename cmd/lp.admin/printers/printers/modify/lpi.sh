#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp.admin:printers/printers/modify/lpi.sh	1.2"
#!/sbin/sh

infocmp $1 | grep lpi| sed -e 's/^/}/' -e 's/$/{/' -e 's/}[^{]*{/\
/g' > /tmp/lpi
cat /tmp/lpi| sed  -e '/^$/d' > /tmp/LPI;
rm -f /tmp/lpi;
