#ident	"@(#)ucbsendmail:cf/etherm.m4	1.1"


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

############################################################
#####
#####		Ethernet Mailer specification
#####
#####	Messages processed by this configuration are assumed to remain
#####	in the same domain.  This really has nothing particular to do
#####   with Ethernet - the name is historical.

Mether,	P=[TCP], F=msDFMuCX, S=11, R=21, A=TCP $h
S11
R$*<@$+>$*		$@$1<@$2>$3			already ok
R$+			$@$1<@$w>			tack on our hostname

S21
# None needed.
