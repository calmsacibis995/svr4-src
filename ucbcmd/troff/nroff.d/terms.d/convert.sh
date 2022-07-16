#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucbtroff:nroff.d/terms.d/convert.sh	1.1.2.1"
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

# This shell script will attempt to convert an old-style nroff terminal
# description file to a new-style nroff terminal table.
# 
# To use this script, make a copy of the complete (i.e. all #include lines
# expanded) C source file for the description of a terminal (say "mytty"),
# and make the filename the same as the terminal name (mytty).
# N.B.  If you use /lib/cpp to expand the #include lines, be sure to use the
# "-C" and "-P" options.
# 
# 		sh convert.sh mytty
# 
# will produce the discardable files mytty.h, mytty.1, mytty.2 and mytty.3,
# as well as the final output file tab.mytty.  The file should be examined
# for errors (and corrected), even in the absence of any "XXX...XXX" error
# lines from the conversion script.
# 
# The first section creates the first 25 items in the terminal table
# by removing the comment indicators from the old identifications, and
# by substituting numbers for values calculated using "#define INCH 240".
# 
echo $1 | cat - $1 >$1.h
ed - $1.h <<- '!'
	2,/struct termtable/d
	/codetab/,$c

	.
	2,$-1v,/\*,s/$/XXXBAD FORMATXXX
	g,/\*,s,,,
	g,\*/,s,,,
	2,$-1v/,$/s/$/XXXBAD FORMATXXX
	g/,$/s///
	g,INCH/60,s//4/
	g,240/60,s//4/
	g,INCH/48,s//5/
	g,240/48,s//5/
	g,INCH/17,s//14/
	g,240/17,s//14/
	g,INCH/16,s//15/
	g,240/16,s//15/
	g,INCH/12,s//20/
	g,240/12,s//20/
	g,INCH/10,s//24/
	g,240/10,s//24/
	g,INCH/8,s//30/
	g,240/8,s//30/
	g,INCH/6,s//40/
	g,240/6,s//40/
	g,INCH/5,s//48/
	g,240/5,s//48/
	g/INCH/s/$XXXERRORXXX
	g/		*/s//	/
	w
	q
!
#
# The next section isolates the width and terminal-code specificiations of
# the /*codetab*/ part of the terminal table, from special character \hy on.
#
cp $1 $1.1
ed - $1.1 <<- '!'
	1,/bullet/-2d
	$g/^};/.-1s/"[ 	]\//"}; \//
	$g/^};/d
	v/^"\\.0/s/$/XXXBAD FORMAT AT BEGINNINGXXX
	g/^"\\.0/s///
	g/^./s//& /
	$g/"};/s//",/
	v/",[ 	]/s/$/XXXBAD FORMAT AT ENDXXX
	g/",[ 	].*/s///
	w
	q
!
#
# The next section combines the character descriptions produced in the
# preceding part with their names (contained in file "template"), and
# removes some unused characters, and adds some that were missing,
# reordering some of the characters to match more nearly the sequence
# given in the list of special characters in the nroff user's guide.
#
paste -d" " template $1.1 > $1.2
cp $1.2 $1.3
ed - $1.3 <<- '!'
	g/XXXDELETE/d
	1t1
	2s/hy/\\-/
	/em /m0
	/fm /m?dg ?
	/\*w/a
	*A 1 A
	*B 1 B
	.
	/\*D/a
	*E 1 E
	*Z 1 Z
	*Y 1 H
	.
	.1a
	*I 1 I
	*K 1 K
	.
	.1a
	*M 1 M
	*N 1 N
	.
	.1a
	*O 1 O
	.
	.1a
	*R 1 P
	.
	.2s/0 \\0/1 T/
	/\*F/a
	*X 1 X
	.
	/sr /m/ts /
	/mi /m/sc /-1
	/eq /m/mi /
	/pl /m/mi /-1
	/ct /m/fm /
	/rg /,/co /m/ct /
	/\*\* /m/eq /
	0a
	charset
	.
	w
	q
!
#
# Finally, the header and table parts are combined to form the final file.
#
cat $1.h $1.3 > tab.$1
