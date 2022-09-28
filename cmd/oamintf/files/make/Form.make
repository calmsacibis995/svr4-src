#ident	"@(#)filemgmt:make/Form.make	1.1.3.1"
Form="Create A File System (make)"
help=OPEN TEXT $INTFBASE/Text.itemhelp $LININFO
close=unset -l XRET -l LABEL -l MNTPT -l FSTYPE -l DEVICE

framemsg=`readfile $INTFBASE/form.msg`

done=`indicator -w;
	if [  ! -b "$F1" ];
	then
		/usr/bin/devattr "$F1" bdevice 2> /dev/null | set -l DEVICE;
	else
		set -l DEVICE=$F1;
	fi;
	set -l  FSTYPE=$F2;
	if [ "$F3" ];
	then
		set -l LABEL=$F3;
	else
		set -l LABEL="NULL";
	fi;
	if [ "$F4" = "yes" ];
	then
		set -l MNTPT=$F5;
	else
		unset -l MNTPT;
	fi;
	$OAMBASE/bin/call_fsck $F2 $DEVICE | set -l XRET;
	if [ "$XRET" = "0" ];
	then
		set -l CMD="OPEN FORM $OBJ_DIR/Form.make2";
	else
		set -l CMD="OPEN FORM $OBJ_DIR/Form.$FSTYPE";
	fi;
	`$CMD
	

name="Device that will contain the file system:"
lininfo=Form.make:F1
nrow=1
ncol=1
frow=1
fcol=43
rows=1
columns=20
rmenu=OPEN MENU $OBJ_DIR/../Menu.fsdevch
#rmenu={ `$OAMBASE/bin/dev cdevice` }
value="diskette1"
valid=`$OAMBASE/bin/validdev "$F1" `
invalidmsg="Error - $F1 is not a valid device name"

name="File system type:"
lininfo=Form.make:F2
nrow=2
ncol=1
frow=2
fcol=19
rows=1
columns=20
value=`if [ "$F1" ];
	then
		$OAMBASE/bin/fstyp_special "$F1";
	fi;`
rmenu=OPEN MENU $OBJ_DIR/Menu.mkfsch
valid=`if [ -f /etc/fs/$F2/mkfs ];
	then
		echo true;
	else
		echo false;
	fi`
invalidmsg="Press CHOICES to select valid response"

name="Label for the  file system:"
lininfo=Form.make2:F3
nrow=3
ncol=1
frow=3
fcol=29
rows=1
columns=6
valid=`regex -v "$F3" '^[0-9A-Za-z]{0,6}$'`
invalidmsg=const 'Must be 1 to 6 alphanumeric characters (e.g. fsys01)'

name="Once created, should the new file system be mounted?"
lininfo=Form.make2:F2
nrow=4
ncol=1
frow=4
fcol=54
rows=1
columns=3
value=yes
rmenu={ yes no }
menuonly=true
invalidmsg="Press CHOICES to select valid response."

name="File system name when mounted:"
lininfo=Form.mntpt:F1
nrow=5
ncol=1
frow=5
fcol=32
rows=1
columns=14
show=`[ "$F4" = "yes" ]`
value=`$OAMBASE/bin/invfstab "$F1" | set -l RVAL2;
		if [ "$RVAL2" = "true" ];
		then 
			/usr/bin/cut -d" " -f2  /tmp/invfstab;
		fi`
valid=`[ -d $F5 ]`
invalidmsg="Error -- $F5 not a valid mount point"
