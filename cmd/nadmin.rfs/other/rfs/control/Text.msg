#ident	"@(#)nadmin.rfs:other/rfs/control/Text.msg	1.4"
title="$ARG1"
columns=50
`readfile $INTFBASE/display.msg | message -p`

text="`readfile $ARG2`"
