#ident	"@(#)ucbsendmail:cf/compat.m4	1.1"


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

##########################################################
#  General code to convert back to old style UUCP names
S5
R$+<@LOCAL>		$@ $D!$1			name@LOCAL => sun!name
R$+<@$-.LOCAL>		$@ $2!$1			u@h.LOCAL => h!u
R$+<@$+.uucp>		$@ $2!$1			u@h.uucp => h!u
R$+<@$*>		$@ $2!$1			u@h => h!u
# Route-addrs do not work here.  Punt til uucp-mail comes up with something.
R<@$+>$*		$@ @$1$2			just defocus and punt
R$*<$*>$*		$@ $1$2$3			Defocus strange stuff
