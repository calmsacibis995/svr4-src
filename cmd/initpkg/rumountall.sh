#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rumountall.sh	1.12.10.1"

if u3b2 || i386
then
echo "#!/sbin/sh
#	Unmounts remote file resources
#	Note: written to depend on as few commands as possible.

ULIST=/tmp/ulist\$\$

trap 'rm -f \${ULIST}' 0 1 2 3 15

kill=
while [ \$# -ge 1 ]
do
	case \"\$1\" in
	-k )
		if [ ! -x /usr/sbin/fuser ]
		then
			echo >&2 \"\$0:  WARNING!!!  /usr/sbin/fuser not found.\"
		else
			kill=yes
		fi
		;;
	* )
		echo >&2 \"Usage:  \$0 [ -k ]
-k	kill processes with files open in each file system before unmounting.\"
		exit 1
	esac
	shift
done
#		kill queued mounts
 
/usr/bin/rm -f /etc/rfs/rmnttab
 
if [ \${kill} ]
then
	>\${ULIST}
	/sbin/mount  |
		/usr/bin/sort -r  |
		while read fs dummy1 dev mode1 mode2 dummy2
		do
			if [ \`echo \${mode1}\${mode2} | grep remote\` ]
			then
				echo  \"\${dev} \\\c\" >>\${ULIST}
			fi
		done 
	klist=\`cat \${ULIST}\`
	if [ \"\${klist}\" = \"\" ]
	then
		exit
	fi
	/usr/sbin/fuser -k \${klist} >/dev/null 2>&1
	for dev in \${klist}
	do
		/sbin/umount -d \${dev}
	done 
else
	/sbin/mount  |
		/usr/bin/sort -r  |
		while read fs dummy1 dev mode1 mode2 dummy2
		do
			if [ \`echo \${mode1}\${mode2} | /usr/bin/grep remote\` ]
			then
				/sbin/umount -d \${dev}
			fi
		done 
fi" >rumountall
fi
