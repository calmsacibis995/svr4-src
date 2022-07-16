#ident	"@(#)ucbsendmail:cf/sunbase.m4	1.1"


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

#################################################
#
#	General configuration information

# local domain names
#
# These can now be set from the domainname system call.
# If your YP domain is different from the domain name you would like to have
# appear in your mail headers, edit them to be your mail domain name.
# Note that the first component of the YP domain name is stripped off unless
# it begins with a dot or a plus sign.
# DmPodunk.EDU
# CmPodunk.EDU
#
# The Dm value is what is set in outgoing mail.  The Cm value is what is
# accepted in incoming mail.  usually these are the same, but you might
# want to have more than one Cm line to recognize more than one domain 
# name during a transition.

# known hosts in this domain are obtained from gethostbyname() call

include(base.m4)

#######################
#   Rewriting rules

# special local conversions
S6
R$*<@$*$=m>$*		$1<@$2LOCAL>$4			convert local domain

include(localm.m4)
include(etherm.m4)
