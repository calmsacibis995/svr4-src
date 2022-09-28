#ident	"@(#)face:OBJECTS/dos/Form.mod	1.3"

form="Modify MS-DOS Programs"

done=`indicator -w;message "";
$VMSYS/bin/delserve "${ARG1}" "${ARG3}";
$VMSYS/bin/creaserve "$F1" "$F2" "$F3" "$F4" "$F5" "${ARG3}" "dos"`close OBJECTS/dos/Form.mod OBJECTS/dos/Menu.list

help=OPEN TEXT OBJECTS/Text.h "$TITLE" dos/T.hmod"$ITEM"

`set -l TERML="${TERMINFO:-/usr/lib/terminfo}";
fmlgrep TERM= ${ARG2} | fmlcut -d= -f2 | fmlcut -d";" -f1 | set -l OF1;
set -l OF2="${ARG1}";
if fmlgrep '^eval' ${ARG2} > /dev/null;
then
	fmlgrep '^eval' ${ARG2} | fmlcut -d" " -f4- | set -l OF3;
else
	tail -1 ${ARG2} | fmlcut -d" " -f3- | set -l OF3;
fi;
fmlgrep '^cd' ${ARG2} | fmlcut -d" " -f2 | set -l OF4;
if fmlgrep '^echo' ${ARG2} > /dev/null;
then
	set -l OF5=yes;
else
	set -l OF5=no;
fi`

name=Terminal Type:
show=false
nrow=1
ncol=1
rows=1
columns=14
frow=1
fcol=24
lininfo=`set -l TITLE="Terminal Type" -l ITEM=1;message -f "Enter the correct Terminal type needed for the command invoked."`
value=const "${OF1}"
valid=`echo ${F1} | fmlcut -c1 | set -l TDIR;
if [ -z "${F1}" ];
then
	set -l IMSG="You must enter a value for this field.";
	echo false;
elif [ -f "${TERML}/${TDIR}/${F1}" -a -s "${TERML}/${TDIR}/${F1}" ];
then
	echo true;
else
	set -l IMSG="${F1} is not a valid terminal on your system.";
	echo false;
fi`
invalidmsg="${IMSG}"

name=Program Menu Name:
nrow=2
ncol=1
rows=1
columns=45
frow=2
fcol=24
lininfo=`set -l TITLE="Program Menu Name" -l ITEM=2;message -f "Enter a name, then press SAVE when you complete the form."`
value=const "${OF2}"
valid=`indicator -w;
if [ -z "${F2}" ];
then
	set -l IMSG="You must enter a value for this field.";
	echo false;
elif [ "${F2}" = "${OF2}" ];
then
	echo true;
elif echo "${F2}"|fmlgrep '^.*;.*$' > /dev/null;
then
	set -l IMSG="Semi-colons are not allowed in this field.";
	echo false;
elif fmlgrep "name=\"${F2}\"" $HOME/pref/services > /dev/null 2> /dev/null;
then
	set -l IMSG="${F2} already exists.";
	echo false;
elif fmlgrep "name=\"${F2}\"" $VMSYS/lib/services > /dev/null 2> /dev/null;
then
	set -l IMSG="${F2} already exists.";
	echo false;
else
	echo true;
fi`
invalidmsg="${IMSG}"


name=Drive Number and Full Pathname of Command:
nrow=3
ncol=1
rows=1
columns=45
frow=4
fcol=24
lininfo=`set -l TITLE="Name of Command" -l ITEM=3;message -f "Enter a command name, then press SAVE when you complete the form."`
value=const "${OF3}"
valid=`indicator -w;
echo "${F3}"|fmlcut -f1 -d" "|set -l NF3;
if [ -z "${F3}" ];
then
	set -l IVAL=false -l IMSG="A value must be entered for this field.";
elif regex -v "${NF3}" '^[a-zA-Z]:/.*$' > /dev/null;
then
	set -l IVAL=true;
else
	set -l IVAL=false -l IMSG="${NF3} contains an illegal character.";
fi`${IVAL}
invalidmsg=${IMSG}
scroll=true

name=Working Directory:
nrow=5
ncol=1
rows=1
columns=45
frow=5
fcol=24
lininfo=`set -l TITLE="Working Directory" -l ITEM=4;message -f "Enter a directory name, then press SAVE when you complete the form."`
value=const "${OF4}"
valid=`test -d $F4`
invalidmsg=const "The Path entered must be a valid directory"
wrap=true

name=Prompt for Arguments:
nrow=6
ncol=1
rows=1
columns=3
frow=6
fcol=24
lininfo=`set -l TITLE="Prompt for Arguments" -l ITEM=5;message -f "Press CHOICES to select, then press SAVE when you complete the form."`
value=const "${OF5}"
rmenu={ yes no }
menuonly=true
invalidmsg="The only valid responses are yes and no."

name=RESET
button=8
action=reset
