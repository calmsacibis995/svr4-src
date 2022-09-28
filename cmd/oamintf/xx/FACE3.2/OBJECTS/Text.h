/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:xx/FACE3.2/OBJECTS/Text.h	1.1"

title=HELP on $ARG1
lifetime=shortterm

init="$RETVAL"
`shell test -r /usr/vmsys/HELP/$ARG2 && set -l RETVAL=true || set -l RETVAL=false; 
 regex -e -v "$RETVAL" 
	'^true$' '`message "Strike the CANCEL function key to cancel."`' 
	'^false$' '`message "No HELP text is available for this item."`'`

text="`readfile /usr/vmsys/HELP/$ARG2`"
columns=`longline`

name=""
button=1
action=nop

name="CONTENTS"
button=8
action=OPEN MENU OBJECTS/Menu.h0.toc
