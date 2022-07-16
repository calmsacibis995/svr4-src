#ident	"@(#)xl:uface/Text.tape	1.1"
title="Floppy Tape $ARG3"
lifetime=shortterm
altslks=true
rows=8
columns=50


`message Strike the CONT function key to continue or CANCEL to stop.`

text="
 Insert the floppy tape to be $ARG2.
 $ARG4
 Strike CONT to continue or 
 CANCEL to stop."

name=CANCEL
button=9
action=close

name=CONT
button=11
action=`/usr/lib/tape/ftape $ARG1; echo $RET | regex -e
		'^0$'  "open text $VMSYS/OBJECTS/ftape/Text.sop $ARG2 " 
	        '.*'   "open text $VMSYS/OBJECTS/ftape/Text.fop $ARG3 "   `

name=""
button=16

name=""
button=10
action=nop

name=""
button=12
action=nop

name=""
button=13
action=nop

name=""
button=14
action=nop

name=""
button=15
action=nop
