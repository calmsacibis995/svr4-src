#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libyp:ypinit.sh	1.6.3.1"
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PROPRIETARY NOTICE (Combined)
#
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
#
#
#
#	Copyright Notice 
#
# Notice of copyright on this source code product does not indicate 
#  publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#          All rights reserved.
#

# set -xv
maps="publickey publickey.byname"
yproot_dir=/var/yp
yproot_exe=/usr/sbin/yp
hf=/tmp/ypservers.$$
XFR=${YPXFR-ypxfr}

clientp=F
masterp=F
slavep=F
host=""
def_dom=""
master=""
got_host_list=F
first_time=T
exit_on_error=F
errors_in_setup=F

PATH=/bin:/usr/bin:/usr/etc:/usr/sbin:$yproot_exe:$PATH
export PATH 

case $# in
1)	case $1 in
	-c)	clientp=T;;
	-m)	masterp=T;;
	*)	echo 'usage:'
		echo '	ypinit -c'
		echo '	ypinit -m'
		echo '	ypinit -s master_server'
		echo ""
		echo "\
where -c is used to set up a yp client, -m is used to build a master "
                echo "\
yp server data base, and -s is used for a slave data base."
		echo "\
master_server must be an existing reachable yp server."
		exit 1;;
	esac;;

2)	case $1 in
	-s)	slavep=T; master=$2;;
	*)	echo 'usage:'
		echo '	ypinit -c'
		echo '	ypinit -m'
		echo '	ypinit -s master_server'
		echo ""
		echo "\
where -c is used to set up a yp client, -m is used to build a master "
                echo "\
yp server data base, and -s is used for a slave data base."
		echo "\
master_server must be an existing reachable yp server."
		exit 1;;
	esac;;
3)	case $1 in
	-c)	clientp=T;;
	*)	echo 'usage:'
		echo '	ypinit -c'
		echo '	ypinit -m'
		echo '	ypinit -s master_server'
		echo ""
		echo "\
where -c is used to set up a yp client, -m is used to build a master "
                echo "\
yp server data base, and -s is used for a slave data base."
		echo "\
master_server must be an existing reachable yp server."
		exit 1;;
	esac;;

*)	echo 'usage:'
	echo '	ypinit -c'
	echo '	ypinit -m'
	echo '	ypinit -s master_server' 
	echo ""
	echo "\
where -c is used to set up a yp client, -m is used to build a master "
	echo "\
yp server data base, and -s is used for a slave data base."
	echo "\
master_server must be an existing reachable yp server."
	exit 1;;
esac

if [ $? -ne 0 ]
then 
	echo "\
You have to be the superuser to run this.  Please log in as root."
	exit 1
fi

host=`uname -n`

if [ $? -ne 0 ]
then 
	echo "Can't get local host's name.  Please check your path."
	exit 1
fi

if [ -z "$host" ]
then
	echo "The local host's name hasn't been set.  Please set it."
	exit 1
fi

def_dom=`domainname`

if [ $? -ne 0 ]
then 
	echo "Can't get local host's domain name.  Please check your path."
	exit 1
fi

if [ -z "$def_dom" ]
then
	echo "The local host's domain name hasn't been set.  Please set it."
	exit 1
fi

domainname $def_dom
real_def_dom=$def_dom
def_dom=`ypalias -d $def_dom`
ypservers_map=`ypalias ypservers`
domain_dir="$yproot_dir""/""$def_dom" 
binding_dir="$yproot_dir""/binding/""$def_dom"
binding_file="$yproot_dir""/binding/""$def_dom""/ypservers"

if [ ! -d $yproot_dir -o -f $yproot_dir ]
then
    echo "\
The directory $yproot_dir doesn't exist.  Restore it from the distribution."
	exit 1
fi

# add domainname and ypservers aliases to aliases file
echo ypservers $ypservers_map >> $yproot_dir/aliases
echo $real_def_dom $def_dom >> $yproot_dir/aliases
sort $yproot_dir/aliases | uniq > /tmp/.ypaliases; mv /tmp/.ypaliases $yproot_dir/aliases

if [ ! -d "$yproot_dir"/binding ]
then
	mkdir "$yproot_dir"/binding
fi

if [ ! -d  $binding_dir ]
then
	mkdir  "$binding_dir"
fi

if [ $slavep = F ]
then
	while [ $got_host_list = F ]; do
		echo ""
		echo "\
	In order for YP to operate sucessfully, we have to construct a list of the "
		echo "\
	YP servers.  Please continue to add the names for YP servers in order of"
		echo "\
	preference, one per line.  When you are done with the list, type a <control D>."
		if [ $masterp = T ]
		then
			echo $host > $hf
			echo "\tnext host to add:  $host"
		elif [ -f $binding_file ]
		then
			if [ $first_time = T ]
			then
				for h in `cat $binding_file`
				do
					echo $h >> $hf
					echo "\tnext host to add:  $h" 
				done
			fi
		fi

		echo  "	next host to add:  \c"

		while read h
		do
			echo  "	next host to add:  \c"
			echo $h >> $hf
		done

		echo ""
		echo "The current list of yp servers looks like this:"
		echo ""

		cat $hf
		echo ""
		echo  "Is this correct?  [y/n: y]  \c"
		read hlist_ok

		case $hlist_ok in
		n*)	got_host_list=F
			first_time=F
			rm $hf
			echo "Let's try the whole thing again...";;
		N*)	got_host_list=F
			first_time=F
			rm $hf
			echo "Let's try the whole thing again...";;
		*)	got_host_list=T;;
		esac
	done
		cp  $hf $binding_file
fi

#
# If client only, we are done
# 	our purpose was just to set up the binding file
#
if [ $clientp = T ]
then
	rm $hf
	exit 1
fi

if [ $slavep = T ]
then
	if [ $host = $master ]
	then
		echo "\
The host specified should be a running master yp server, not this machine."
		exit 1
	fi

	maps=`ypwhich -m | egrep $master$| awk '{ printf("%s ",$1) }' -`
	if [ -z "$maps" ]
	then
		echo "Can't enumerate maps from $master. Please check that it is running."
		exit 1
	fi
fi

echo ""

echo "Installing the YP database will require that you answer a few questions."
echo "Questions will all be asked at the beginning of the procedure."
echo ""
echo "Do you want this procedure to quit on non-fatal errors? [y/n: n]  \c"
read doexit

case $doexit in
y*)	exit_on_error=T;;
Y*)	exit_on_error=T;;
*)	echo "\
OK, please remember to go back and redo manually whatever fails.  If you"
	echo "\
don't, some part of the system (perhaps the yp itself) won't work.";;
esac

echo "The yp domain directory is $yproot_dir""/""$def_dom"

for dir in $yproot_dir/$def_dom
do

	if [ -d $dir ]; then
		echo  "Can we destroy the existing $dir and its contents? [y/n: n]  \c"
		read kill_old_dir

		case $kill_old_dir in
		y*)	rm -r -f $dir

			if [ $?  -ne 0 ]
			then
			echo "Can't clean up old directory $dir.  Fatal error."
				exit 1
			fi;;

		Y*)	rm -r -f $dir

			if [ $?  -ne 0 ]
			then
			echo "Can't clean up old directory $dir.  Fatal error."
				exit 1
			fi;;

		*)    echo "OK, please clean it up by hand and start again.  Bye"
			exit 0;;
		esac
	fi

	mkdir $dir

	if [ $?  -ne 0 ]
	then
		echo "Can't make new directory $dir.  Fatal error."
		exit 1
	fi

done

if [ $slavep = T ]
then
	echo "\
There will be no further questions. The remainder of the procedure should take"
	echo "a few minutes, to copy the data bases from $master."

	for dom in  $real_def_dom
	do
		for map in $maps
		do
			echo "Transferring $map..."
			$XFR -h $master -c -d $dom $map

			if [ $?  -ne 0 ]
			then
				errors_in_setup=T

				if [ $exit_on_error = T ]
				then
					exit 1
				fi
			fi
		done
	done

	echo ""
	echo  "${host}'s yellowpages data base has been set up\n"

	if [ $errors_in_setup = T ]
	then
		echo " with errors.  Please remember"
		echo "to figure out what went wrong, and fix it."
	else
		echo " without any errors."
	fi

	exit 0
else

	rm -f $yproot_dir/*.time

	echo "\
There will be no further questions. The remainder of the procedure should take"
	echo "5 to 10 minutes."

	echo "Building $yproot_dir/$def_dom/ypservers..."
	makedbm $hf $yproot_dir/$def_dom/$ypservers_map

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/$ypservers_map."
		errors_in_setup=T

		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	rm $hf

	in_pwd=`pwd`
	cd $yproot_dir
	echo  "Running \c"
	echo  $yproot_dir "\c"
	echo "/Makefile..."
	make NOPUSH=1 

	if [ $?  -ne 0 ]
	then
		echo "\
Error running Makefile."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	cd $in_pwd
	echo ""
	echo  "\
$host has been set up as a yp master server\c"

	if [ $errors_in_setup = T ]
	then
		echo " with errors.  Please remember"
		echo "to figure out what went wrong, and fix it."
	else
		echo " without any errors."
	fi

	echo ""
	echo "\
If there are running slave yp servers, run yppush now for any data bases"
	echo "\
which have been changed.  If there are no running slaves, run ypinit on"
	echo "\
those hosts which are to be slave servers."

fi
