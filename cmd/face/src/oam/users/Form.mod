#ident	"@(#)face:src/oam/users/Form.mod	1.4"

form="Modify FACE Environment for a FACE User"
done=open TEXT $OBJ_DIR/Text.mod "$F1" "$F4" "$F2" "$F3" `getfrm`

close=`unset -l UID -l IMSG`

init=`if $VMSYS/bin/chkperm -l > /dev/null;
then
	echo true;
else
	message "There are no FACE users defined on this system to modify.";
	echo false;
fi`

help=open TEXT $INTFBASE/Text.itemhelp $LININFO

`fmlgrep '^vmsys:' /etc/passwd | fmlcut -f6 -d: | set -e VMSYS`

name="User's Login ID:"
nrow=1
ncol=1
rows=1
columns=8
frow=1
fcol=42
lininfo=`message -f "Enter the login ID of the user you wish to modify."`form:F1
rmenu=const { `indicator -w; $VMSYS/bin/chkperm -l | sort` }
value=""
valid=`indicator -w;
fmlgrep "^${F1}:" /etc/passwd | fmlcut -f3 -d":" | set -l UID;
if [ -z "${F1}" ];
then
	set -l IMSG="You must enter a value for this field.";
	echo false;
elif [ -z "${UID}" ];
then
	set -l IMSG="The login specified must exist in the /etc/passwd file first.";
	echo false;
elif [ "${UID}" -lt "100" ];
then
	set -l IMSG="The login specified has a uid less than 100, I will not modify this login.";
	echo false;
elif $VMSYS/bin/chkperm -v -u "${F1}";
then
	echo true;
else
	set -l IMSG="${F1} is not a FACE user, use add instead.";
	echo false;
fi`
invalidmsg="${IMSG}"

name="Invoke FACE at Login:"
nrow=2
ncol=1
rows=1
columns=3
frow=2
fcol=42
value=`test -n "${F1}" && $VMSYS/bin/chkperm -v -u "${F1}" && $VMSYS/bin/chkperm -e invoke -u "${F1}"`
lininfo=`message -f "Should FACE be invoked automatically when this user logs onto this system?"`form:F2
rmenu=const { yes no }
menuonly=true
invalidmsg="Only yes or no are valid answers."

name="Provide UNIX System Access:"
nrow=3
ncol=1
rows=1
columns=3
frow=3
fcol=42
value=`test -n "${F1}" && $VMSYS/bin/chkperm -v -u "${F1}" && $VMSYS/bin/chkperm -e unix -u "${F1}"`
lininfo=`message -f "Should this user have access to the UNIX System shell?"`form:F3
rmenu=const { yes no }
menuonly=true
invalidmsg="Only yes or no are valid answers."

name="Show System Administration in FACE menu:"
nrow=4
ncol=1
rows=1
columns=3
frow=4
fcol=42
value=`test -n "${F1}" && $VMSYS/bin/chkperm -v -u "${F1}" && $VMSYS/bin/chkperm -e admin -u "${F1}"`
lininfo=`message -f "Should this user have a System Administration entry in the top FACE menu?"`form:F4
rmenu=const { yes no }
menuonly=true
invalidmsg="Only yes or no are valid answers."
