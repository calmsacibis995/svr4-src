#ident	"@(#)filemgmt:bin/fstyp_special.	1.1.1.1"
# fstyp_special  <special>
# returns the type of special from vfstab (if any)
SPECIAL=$1
BLOCK=`devattr "$SPECIAL" bdevice  2> /dev/null`
if [ "$BLOCK" = "" ]
then
	BLOCK="$SPECIAL"
fi
while read special dummy1 mountp fstype dummy2
do
	if [ "t$BLOCK" = "t$special" ]
	then
		echo $fstype
		exit 0
	fi
done < /etc/vfstab
