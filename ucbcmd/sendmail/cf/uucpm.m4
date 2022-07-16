#ident	"@(#)ucbsendmail:cf/uucpm.m4	1.1"


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

ifdef(`m4COMPAT',, `include(compat.m4)')
#	UUCP Mailer specification

Muucp,	P=/usr/bin/uux, F=msDFMhuU, S=13, R=23,
	A=uux - -r $h!rmail ($u)

# Convert uucp sender (From) field
S13
R$+			$:$>5$1				convert to old style
R$=w!$+			$2				strip local name
R$+			$:$w!$1				stick on real host name

# Convert uucp recipient (To, Cc) fields
S23
R$+			$:$>5$1				convert to old style
