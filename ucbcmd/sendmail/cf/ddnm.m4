#ident	"@(#)ucbsendmail:cf/ddnm.m4	1.1"


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
#
#		DDN Mailer specification
#
#	Send mail on the Defense Data Network
#	   (such as Arpanet or Milnet)

Mddn,	P=[TCP], F=msDFMuCX, S=22, R=22, A=TCP $h, E=\r\n

# map containing the inverse of mail.aliases
DZmail.byaddr

S22
R$*<@LOCAL>$*		$:$1
R$-<@$->		$:$>3${Z$1@$2$}			invert aliases
R$*<@$+.$*>$*		$@$1<@$2.$3>$4			already ok
R$+<@$+>$*		$@$1<@$2.$m>$3			tack on our domain
R$+			$@$1<@$m>			tack on our domain
