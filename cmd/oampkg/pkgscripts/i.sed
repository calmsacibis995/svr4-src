#ident	"@(#)oampkg:pkgscripts/i.sed	1.5"

error=no
while read src dest
do
	[ "$src" = /dev/null ] && continue

	echo "Modifying $dest"
	savepath=$PKGSAV/sed${dest}
	dirname=`dirname $savepath`
	if [ $? -ne 0 ]
	then
		error=yes
		continue
	fi

	if [ ! -d $dirname ]
	then
		# ignore return since mkdir has bug
		mkdir -p $dirname
	fi

	cp $src $savepath &&
		/usr/sadm/install/scripts/cmdexec /bin/sed install $savepath $dest 

	if [ $? -ne 0 ]
	then
		error=yes
	fi
done
[ "$error" = yes ] &&
	exit 2
exit 0
