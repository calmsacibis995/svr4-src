#ident	"@(#)lp.admin:printers/filters/Menu.opt.ch	1.1"

Menu=Select Filter Options
lifetime=shortterm
multiselect=true

help=OPEN TEXT $INTFBASE/Text.itemhelp $LININFO


done=`
    message -w 'Saving this template.  Please wait.';
    set -e templ$templcount="$F2 $F3=$F4";
    set -l unset="$unset -e templ$templcount";
    expr $templcount + 1 | set -e templcount;
    echo UPDATE;
    set	-l templates=1;
`
`echo "" >> ;
CHARSET 
COPIES 
CPI 
FORM 
INPUT 
LENGTH 
LPI 
MODES 
OUTPUT 
PAGES 
TERM 
WIDTH 

#
#	F3
#
name=Pattern
nrow=3
ncol=11
frow=4
fcol=11
rows=1
columns=16
lininfo=Form.add.2:F3
valid=`
    regex -v "$F3"
	'[^\\][=,]'	'error1'
	'^[=,]'		'error2'
	'^.*$'		'ok'
    | set -l badval;
    if [ "$badval" = 'ok' ];
    then
	true;
    else
	false;
    fi;
`
fieldmsg='Enter the pattern to be matched.'
invalidmsg="Badval is [$badval]"
#invalidmsg='You must precede commas and equal signs with a backslash'

#
#	F4
#

name=Replacement
nrow=3
ncol=28
frow=4
fcol=28
rows=1
columns=16
lininfo=Form.add.2:F4
valid=`
    regex -v "$F4"
	'[^\\][=,]'	'error'
	'^[=,]'		'error'
	'^.*$'		'ok'
    | set -l badval;
    if [ "$badval" = 'ok' ];
    then
	true;
    else
	false;
    fi;
`
fieldmsg='Enter the string to be used when the pattern matches.'
invalidmsg='You must precede commas and equal signs with a backslash'
