#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)xinstall:custom.sh	1.1.1.1"

#
# CUSTOM - Xenix System V installation tool
#
# Executable dependencies:
#	awk cat comm df env expr find fixperm fsck getopt
#	mount mv pr rm rmdir sed sh sort tr tar umount
#
#set -x
PATH=/usr/bin:/usr/sbin:/sbin

# Return code definitions for init scripts
: ${OK=0} ${FAIL=1} ${STOP=10} ${HALT=11}

# set up default values
last=1	lst=1				# last menu item on set menu
quit=quit				# q option printed after prompt
tmp=/tmp/cus$$				# temporary file name prefix
dev=/dev/install			# device for mounting filesystems
rdev=/dev/rinstall			# device for extracting tar volumes
lib=/usr/lib/custom			# home of removal scripts
pfx=./tmp/_lbl				# software label prefix
init="./once/init.*[!%] ./tmp/init.*[!%]" # home of initialization scripts
ser="./tmp/*.ser"			# location of serial number files
					# created by init scripts
prep="./tmp/perms/prep.*[!%]"		# home of preparation scripts
retn="\007\nand press <RETURN> "	# commonly used in prompts
perms="\*|inst|rts|ext|dsmd|soft|tpmd|text" # known permlist names
AWK=awk

# Remember functions as they are defined
set -h
[ $? = 0 ] || {
	echo "$0: must be invoked with System V shell" >&2
	exit 1
}

# Print an error message
error() {
	echo "\nError: $*" >&2
	return 1
}

# Remove temp files (including those created by multi-product serialization)
# and exit with the status passed as argument
cleanup() {
	trap '' 1 2 3 15
	rm -f $init $prep $tmp* $ser
	exit $1
}

# Initializes the list of available packages in the current set
getsetlist() {
	[ "$notty" ] && return 0
	echo > $tmp.$sid "
	Name	Inst	Size	$set packages
	--------------------------------------------------------------------"
	fixperm -iv $ignorepkgs $perm |
	sed "s/^\(.*\)	.*$/s:^#!\1[ 	][ 	]*:	&	:p/" > $tmp.fl
	sed -n -f $tmp.fl $perm >> $tmp.$sid
}

# Updates the list of packages after a package installation or removal
# Can't use $ignorepkgs here (much as I'd like to) since fixperm considers
# having a -u AND a -d a fatal error, so I've got $lpkgs which indicates 
# exactly those things pertinent to the report of "No, Part, All" status
updatesetlist() {
	[ "$notty" -o ! "$perm" ] && return 0
	fixperm -iv $lpkgs $perm |
		sed "s/^\(.*\)	.*$/s:\1	[^ 	]*:&:/" > $tmp.fl
	sed -f $tmp.fl $tmp.$sid > $tmp.tl
	mv $tmp.tl $tmp.$sid
}

# Usage:  logit [status]
# Adds a record to the system installation history file
logit() {
	[ "$nolog" = y ] && return
	trap 'intr=true' 1 2 3 15
	if [ "$status" ]; then
		status="Failed: $status"
	else
		status="Successful"
	fi
	[ "$*" ] && status="$*"
	echo "date=\"`date`\" \\
prd=$prd rel=$rel upd=$upd typ=$typ set=\"$set\" \\
action=$action status=\"$status\" \\
pkgs=\"$spkgs\"
" >> $lib/history
	status=
	[ "$intr" = true ] && cleanup 1
	trap 'cleanup 1' 1 2 3 15
}

# Sets trap so that interrupts are logged in addition to any other
# status info (spkgs are pkgs for status log)
setlogtrap() {
	status=				# status of current action (inst/rmv)
	nolog=				# boolean for logging. true=quit->nolog
	spkgs=				# names of pkgs/files being manip.
	action=				# action taken (Installation or Removal)
	intr=				# flag for interrupts during logit exec
	trap 'nolog= ; logit "$status Interrupted"; cleanup 1' 1 2 3 15
}

# Prompt with arguments, return non-zero on q
# 	-x and +x toggle debug mode
# 	! gets you a shell escape (can modify current shell)
# 	anything else is returned in $cmd
prompt() {
	while	echo "\n${*}or enter q to $quit: \c" >&2
		read cmd
	do	case $cmd in
		+x|-x)	set $cmd			;;
		Q|q)	return 1			;;
		!*)	eval `expr "$cmd" : "!\(.*\)"`	;;
		*)	return 0			;;
		esac
	done
}

# Mount a filesystem and clean if necessary
# Returns non-zero on failure
mntvol() {
	until	mount -r $dev /mnt 2> /dev/null
	do	[ $? = 2 ] && fsck -y $dev || {
			status="$status Extract"
			error "corrupted or non-filesystem volume"
			return 1
		}
	done
}

# Usage: getsetvals sid [vkey|perm]
# Set all values needed for specified set - called frequently.
# Can be used to associate a permlist with a keyletter and vice-versa.
# Default keyletter (vkey) will be used for os, ds, or tp if not specified
getsetvals() {
	# clear all values
	set= perm= mnt= lbl= vkey= typ= ver= prd= upd= rel=
	ignorepkgs="-uINIT -uPERM"
	case $1 in
	0)	eval $k0 perm=$k0perm
		;;
	1)	# New add-on products have permlist in ./tmp/perms
		set="distribution" perm="./tmp/perms"
		lbl=$set
		;;
	[1-$lst]|[$tens][$ones])
		# Additional supported products already installed
		eval eval \$k$1
		eval perm=\$k$1perm
		lbl=$set
		;;
	*)	return 1
		;;
	esac
	return 0
}

# Usage: getperms
# $perm should be set correctly on entry to this routine.
# If any member of $perm is not found then try to install it.
# Return non-zero on errors or failure to install a permlist.
getperms() {
	for h in $perm
	do	[ -f $h ] && continue
		# use getsetvals to find the right vkey
		getsetvals $sid $h
		echo "\nInstalling custom data files ... \c" >&2
		vol=${vkey}01
		until	prompt "\nInsert $lbl volume ${vkey}1$retn" || return 1
			chkswlabel noperm
		do	case $? in
			1)	continue			;;
			4)	echo "\nThis volume is not custom installable."
				echo "Please consult your installation notes."
				return 1			;;
			*)	error "getperms $?"
				return 1			;;
			esac
		done
		extract $h || return 1
		# $h might be a directory for new prod additions
		[ -d $h ] && return 0
		fixperm -c -dPERM $h
	done
}

# Check that release, version, etc. exist in each permlist
# sid must be set correctly on entry
checkperms() {
	# use getsetvals to find the permlist names
	getsetvals $sid
	for i in $perm
	do	# use getsetvals to find the vkey
		getsetvals $sid $i
		eval k$sid$vkey='`getpermvals $i`'
	done
}

# Usage: getpermvals files
# Extracts certain lines beginning with #name=value and prints
# the name=value pair on the standard output
getpermvals() {
	sed -n '/^#set=/s/#//p
		/^#prd=/s/#//p
		/^#ver=/s/#//p
		/^#typ=/s/#//p
		/^#rel=/s/#//p
		/^#mnt=/s/#//p
		/^#upd=/s/#//p' $*
}

# Set values from the label as local variables; ignore errors.
getswlabel() {
	# clear all known values
	_prd= _ver= _upd= _typ= _rel= _vol=
	eval 'case $vol in
	'${mnt:-!}')	mntvol || return 1
		eval `find /mnt/$pfx -type f -print | pathtovals`
		umount $dev						;;
	*)	eval `tar tnf $rdev $pfx 2>/dev/null | pathtovals`	;;
	esac'
}

# Converts a path of the form ./tmp/_lbl/name=val/name=val ...
# to shell variables with leading underscores and prints on stdout.
pathtovals() {
	sed -n "s|^.*$pfx||
		s|/| _|gp"
}

# Check volume number, release, and version; returns:
# 1 - wrong volume
# 2 - update needed but not wanted 
# 3 - update attempted
# 4 - missing volume label
chkswlabel() {
	noperm=$1
	# get set/vkey specific values from permlist
	getsetvals $sid $vkey
	# set values from the label
	getswlabel || return 1
	# if no permlist present, assume values are correct
	[ "$noperm" ] && prd=$_prd ver=$_ver typ=$_typ rel=$_rel vol=$_upd$vol
	# compare permlist and label values
	case $_prd in
	$prd)	# product matches
				;;
	xnxsv)	# forever correcting for our youth ...
		case $prd in
		xnx??)	: installing 2.1 vols	;;
		*)	error "incorrect product in drive"
			return 1		;;
		esac		;;
	"")	error "volume label not found"
		return 4	;;
	*)	error "incorrect product in drive"
		return 1	;;
	esac

	for i in $typ $ver
	do	case $i in
		$_typ|$_ver)	:		# matches, so keep a'going
				;;
		*)		error "incorrect product type in drive"
				return		
				;;
		esac
	done

	[ "$vol" != "$_vol" ] && {
		error "incorrect volume in drive"
		return 1
	}
	[ "$rel" = "$_rel" ] && return 0
	# Do not proceed if we are already updating the release
	[ "$nest" = y ] && {
		error "volume ${vkey}1 must be Release $rel"
		return 1
	}
	nest=y
	echo "\nUpdating custom data files ... \c" >&2
	# Permlists are always on volume ?01
	[ "$_vol" != "${vkey}01" ] && {
		# Volume containing permlist must be new release
		eval k$sid$vkey="rel=$_rel\ \$k$sid$vkey"
		vol=${vkey}01
		until	prompt "\nInsert $lbl volume ${vkey}1$retn" || {
				nest=
				return 2
			}
			chkswlabel
		do	case $? in
			[14])	continue		;;
			[!2])	error "chkswlabel $?"	;;
			esac
			nest=
			return 2
		done
	}
	nest=
	if [ "$sid" -lt 1 ]; then
		# only extract perms that match the current vkey
		eval extract \$$vkey || return 2
		eval fixperm -c -dPERM \$$vkey
	else
		# add on product perms live in the directory ./tmp/perms
		rm -rf ./tmp/perms
		extract ./tmp/perms && execprep $prep &&
			countperms ./tmp/perms/* || return 2
		mv ./tmp/perms/* $perm
		rm -rf ./tmp/perms
	fi
	echo >&2
	# spkgs is not applicable when only the permlist was updated.
	# spkgs gets reset using $save in the calling function.
	spkgs=
	logit "Updated custom datafiles"
	return 3
}

# Usage: execinit files
# Move files to a unique name and then execute them.
# This is trickier than it seems.  We cannot simply append our pid
# since children must remove all files matching init.* except these.
# Returns exit status of last file executed.
# init scripts are passed the argument "-c" to indicate that they
# were invoked by custom.  init scripts are executed with sh "-c"
# flag because they may be binaries.
execinit() {
	# append % to scripts prevent children from removing them
	for i in $*
	do	[ -x $i ] && cp $i $i% && ilist="$ilist $i"
	done
	j=0
	for i in $ilist
	do	sh -c "$i% -c"; j=$?
		case $j in
		$OK)	: All is well					;;
		$STOP)	status="Interrupted"; logit
			cleanup $STOP					;;
		$HALT)	error "haltsys not supported"			;;
		$FAIL)	status="Init/Prep Script"; error "$i failed"	;;
		esac
		rm -f $i%
	done
	unset ilist
	return $j
}

# Usage: execprep files
# Identical to execinit except that the files are removed from ./tmp/perms
# as they must be for countperms and setselect, etc... to work.  However
# the init files in ./tmp/ must remain because they are listed in the
# perm list as part of the package.  This is the easiest fix for 
# code that apparently never worked quite right.
execprep() {
	# append % to scripts prevent children from removing them
	for i in $*
	do	[ -x $i ] && mv $i $i% && ilist="$ilist $i"
	done
	j=0
	for i in $ilist
	do	sh -c "$i% -c"; j=$?
		case $j in
		$OK)	: All is well					;;
		$STOP)	status="Interrupted"; logit
			cleanup $STOP					;;
		$HALT)	error "haltsys not supported"			;;
		$FAIL)	status="Init/Prep Script"; error "$i failed"	;;
		esac
		rm -f $i%
	done
	unset ilist
	return $j
}

# Prompt for yes or no answer - returns non-zero for no
getyn() {
	while	read yn
	do	case $yn in
		[yY])	return 0 			;;
		[nN])	return 1			;;
		*)	error "enter either y or n" 	;;
		esac
	done
}

# Extract a list of files - returns non-zero on failure
extract() {
	case $1 in
	-F)	shift; flags=F	;;
	*)	flags=		;;
	esac
	filelist=$*
	eval 'case $vol in
	'${mnt:-!}')	mntvol || return 1
		until	(cd /mnt; tar cf$flags - $filelist) | tar xf -
		do	error "extraction failed: try again? (y/n) \c"
			getyn && continue
			umount $dev
			return 1
		done
		umount $dev	;;
	#  Give tar the p option so that init and prep files will still
	#  be executable.  Otherwise, they will lose the x bit and
	#  execinit will not try to execute them
	*)	until	tar xnpf$flags $rdev $filelist
		do	error "extraction failed: try again? (y/n) \c"
			getyn || return 1
		done
	esac'
	unset filelist
}

# Get vkey from first letter of vol and create pvol
# pvol has the leading zero stripped and is for prompting only
getvkey() {
	vkey=`expr $vol : "\(.*\).."`
	pvol=$vkey`expr $vol : "${vkey}0*\(.*\)"`
}

# countperms - passed a list of permlists as arguments
# returns ok if only one is found
countperms() {
	case $# in
	0)	error "missing custom data files"	;;
	1)	[ -f $1 ] && return 0			;;
	*)	error "multiple custom data files"	;;
	esac
	return 1
}

# converts the argument passed in from a set name to a set id
# returns either the sid or zero if no legal conversion can be made
nametosid() {
	j=$1
	for i in 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
	do	getsetvals $i || return 0
		[ "$j" = "$prd" ] && return $i
	done
	error "nametosid search overrun"
}

# checks ./etc/perms for new set to add to menu
# returns zero only if a new set was found
chkfornewsets() {
	# only check if forced (-f) or if no check done yet
	[ "$setchkdone" = y -a "$1" != -f ] && return 1
	#  The ./etc/perms directory may not exist yet.
	[ ! -d ./etc/perms ] &&  {
		setchkdone="y"
		return 1
	}

	#  Use a bogus directory to check if ./etc/perms is an empty 
	#  directory.  Used to be guaranteed not empty in XENIX.
	#  Sleasy way to avoid expansion of '*'.
	perm_files=`cd ./etc/perms; echo *`
	if [ "/att/msoft/isc/*" = "/att/msoft/isc/$perm_files" ]
	then
		return 1		#  Empty
	fi

	stat=1
	for h in $perm_files
	do 	eval "case $h in $perms) continue; esac"
		# add perm to known list and get values from it
		perms="$perms|$h" k0perm=./etc/perms/$h
		k0=`getpermvals $k0perm`
		# are the required values all there?
		getsetvals 0
		[ "$set" -a "$prd" -a "$rel" ] || {
			error "invalid custom data file: $k0perm"
			continue
		}
		# new sets are added to menu, but updates are not
		if [ "$upd" ]; then
			# find the sid of the product it updates
			if nametosid $prd; then
				error "update for unknown product: $k0perm"
			else	# add to product $perm and save update values
				eval k$?perm="\$k$?perm\ $k0perm" k$?$upd='$k0'
			fi
		else
			last=`expr $last + 1`
			eval k$last='$k0' k${last}perm=$k0perm
			lst=$last newsets="$newsets$last. $set\n\t"
		fi
		stat=0
	done
	# allow for more than ten sets to be added
	[ $last -gt 9 ] && {
		tens="1-`expr $last / 10`"
		ones="0-`expr $last % 10`"
		lst=9
	}
	setchkdone=y
	return $stat
}

listpkgs() {
	cat $tmp.$sid
	echo "\nPress <RETURN> to continue \c" >&2
	read i
}

printhelp() {
	pg -p"" $lib/help
}

installpkgs() {
	action=Installation
	cmd=$*
	[ "$cmd" ] || {
		cat $tmp.$sid
		prompt "Enter the package(s) to install\n" || { nolog=y; return; }
	}
	pkgs=	regx=	not=	save=$cmd	lpkgs=
	for i in `echo $cmd | tr "[a-z]" "[A-Z]"`
	do	case $i in
		RTS|INST) error "Re-installing $i is not supported" 	;;
		ALL)	pkgs="-uRTS -uPERM -uINIT -uINST -uSER1 -uSER2 -uSER3 \
-uSER4 -uFD48 -uFD96 -uHD1" regx=$i
			# subtract size of RTS, INST and PERM from ALL
			not='$1 ~ /^#!(RTS|INST|PERM)$/ { s -= $2 }'
			lpkgs="-uPERM -uINIT -uSER1 -uSER2 -uSER3 -uSER4 \
-uFD48 -uFD96 -uHD1" 
			spkgs="$spkgs ALL"
			break						;;
		*)	pkgs="$pkgs -d$i" regx=${regx}${regx:+"|"}$i	
			lpkgs="$pkgs"
			spkgs="$spkgs $i"				;;
		esac
	done
	[ "$pkgs" ] || { nolog=y; return; }

	# put the contents list for each volume into a separate file
	> $tmp.fl; > $tmp.tl
	fixperm -fw $pkgs $perm |
	  sort -u +1 +0 |
	    $AWK '(NF < 2) || ($2 == 0)	{ next } # missing or zero vol field
			$2 != vol { if (vol) print "EOF" vol
				  vol = $2
				  print "cat>" t vol "<<\EOF" vol
				  # put N and upgrade volumes in separate files
				  if (vol ~ /N+/)
					print vol >> ( t "tl" )
				  else if (vol ~ /U[A-Z]+/)
					print vol >> ( t "ul" )
				  else
					print vol >> ( t "fl" )
				}
				{ print $1 }
		' t=$tmp. - |
	      sh || { status="$status Fixperm/File List"; return 1; }

	[ -s $tmp.fl -o -s $tmp.tl ] || {
		error "$set has no package(s) named $cmd"
		nolog=y; return 1
	}

	size=`$AWK "\\$1 ~ /^#!($regx)$/ { s += \\$2 }
		$not
		END { print s+0 }" $perm`
	space=`df /dev/root | sed "s/.*:[ 	]*\([0-9]*\)[ 	]*.*/\1/p`
	set -- `df /dev/root` # third arg is free space
	[ "$size" -gt "$space" ] && {
		echo "
$size blocks are required and only $space blocks are available
on the root filesystem.  Do you wish to continue? (y/n) \c" >&2
		getyn || { nolog=y; return 1; }
	}
	unset size
	rm -f $init
	# non-generic (N) volumes must always be extracted last
	for vol in `cat $tmp.fl $tmp.tl`
	do	getvkey
		# get lbl string for prompt
		getsetvals $sid $vkey
		until	prompt "Insert $lbl volume $pvol$retn" || {
				nolog=y
				continue 2
			}
			chkswlabel
		do	case $? in
			1|4)	continue	;;
			2)	status="$status Improper Data File"
				return 1	;;
			3)	# evaluate new permlist and redo cmd
				checkperms; installpkgs $save; getsetlist
				return 0	;;
			esac
		done
		echo "Extracting files ... \c" >&2
		extract -F $tmp.$vol || status="$status Extract"
		echo >&2
		execinit $init
	done
	echo "Checking file permissions ... \c" >&2
	fixperm -c $pkgs $perm || status="$status Fixperm"
	updatesetlist
	echo >&2
}

removepkgs() {
	action=Removal
	cmd=$*
	[ "$cmd" ] || {
		cat $tmp.$sid
		prompt "Enter the package(s) to remove\n" || { nolog=y; return; }
	}
	pkgs=	upkgs= rmvp=
	for i in `echo $cmd | tr "[a-z]" "[A-Z]"`
	do	case $i in
		RTS|PERM|INST)	error "cannot remove $i package";;
		ALL)	pkgs="-uRTS -uPERM -uINIT -uINST -uSER1 -uSER2 -uSER3 \
-uSER4 -uFD48 -uFD96 -uHD1" 
 			upkgs="-dRTS -dINST -dPERM" rmvp=ALL
			spkgs="$spkgs ALL"
			break					;;
		*)	pkgs="$pkgs -d$i" upkgs="$upkgs -u$i"
			rmvp="$rmvp $i"				
			spkgs="$spkgs $i"			;;
		esac
	done
	[ "$pkgs" ] || { nolog=y; continue; }
	fixperm -fg $pkgs $perm | sort > $tmp.fl
	[ -s $tmp.fl ] || {
		error "$set has no package(s) named $cmd"
		nolog=y 
		continue
	}
	# need to make sure that $prd is set!
	getsetvals $sid
	# execute any product specific removal scripts (may be keyed)
	for i in $lib/$prd.rmv $lib/$prd[A-Z].rmv $lib/$prd[A-Z][A-Z].rmv
	do	[ -x $i ] || continue
		sh -c "$i $rmvp" 
		case $? in
		$OK)			;;
		$HALT|$STOP)
			echo "\nRemoval aborted."
			status="$status; Remove Script Interrupted"
			return 1	;;
		*)	status="$status Remove Script"
			error "$rmvp removal failed"	;;
		esac
	done
	unset rmvp
	# check all installed perms since some files cross set boundaries
	# skip current permlist since it has been done already.
	fixperm -fg $upkgs $perm > $tmp.tl
	for i in  2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
	do	getsetvals $i || break
		[ $i != $sid -a  "$perm" ] && cat -s $perm
	done | fixperm -fg - >> $tmp.tl
	# reset values to real sid
	getsetvals $sid

	# remove the files that are not in more than one package
	#   but do not exceed max argument list size
	sort -y $tmp.tl |
	    comm -23 $tmp.fl - |
		xargs /bin/rm -f || status="$status Removal"
	# remove the empty directories, if any
	rmdir `fixperm -D $pkgs $perm | sort -r` 2> /dev/null
	updatesetlist
}

listfiles() {
	cmd=$*
	[ "$cmd" ] || {
		cat $tmp.$sid
		prompt "Enter the package(s) to list\n" || return
	}
	pkgs=
	for i in `echo $cmd | tr "[a-z]" "[A-Z]"`
	do	case $i in
		ALL)	pkgs="-uINST"; break	;;
		*)	pkgs="$pkgs -d$i"	;;
		esac
	done
	[ "$pkgs" ] || continue
	[ -t ] && echo "\nPress <RETURN> each time the bell rings\n" >&2
	fixperm -l $pkgs $perm | pr -2prtl12 ||
		error "$set has no package(s) named $cmd"
}


installfile() {
	action=Installation
	cmd=$*
	[ "$cmd" ] || {
		prompt "Enter the pathname of the file to install\n" || {
			nolog=y; return
		}
	}
	case $cmd in
	"")	return 		;;
	./*)	file=$cmd	;;
	/*)	file=.$cmd	;;
	*)	file=./$cmd	;;
	esac
	spkgs="$file"
	vol=`fixperm -fw $perm | sed -n "s!^$file[ 	][ 	]*!!p"`
	[ "$vol" ] || {
		error "$set has no file named $file"
		nolog=y
		return 1
	}
	getvkey
	until	prompt "Insert $set volume $pvol$retn" || { nolog=y; return; }
		chkswlabel
	do	case $? in
		1|4)	continue 				;;
		2)	status="$status Improper Data File"
		 	return 1				;;
		3)	# evaluate new permlist after completing cmd
			checkperms; installfile $file; getsetlist
			return 0				;;
		esac
	done
	echo "Extracting $file ... \c" >&2
	extract $file || status="$status Extract"
	echo >&2
	unset file
}

diskusage() {
	bar="------------------------------------------------------"
	echo "\n\t\tCurrent Disk Usage\n$bar\n`df -v`\n$bar" >&2
	unset bar
}

# selects a new set by menu
# returns non-zero on q
setmenu() {
	# save old set id in case they quit without selecting a new one
	save=$sid
	chkfornewsets
	while	:
	do	prompt "
	1. Add a Supported Product
	$newsets\nSelect a set to customize "
		case $cmd in
		[1-$lst]|[$tens][$ones])
			setselect $cmd || continue
			[ -f $tmp.$sid ] || getsetlist
			return 0				;;
		[qQ])	[ "$save" ] && setselect $save
			return 1				;;
		*)	error "enter 1 through $last or q"	;;
		esac
	done
}


# usage: setselect sid
# The global set id (sid) is set to the argument passed in.
# This routine is the only place sid can be reset.
setselect() {
	sid=$1
	case $sid in
	1)	# add a supported product
		getsetvals $sid
		rm -rf $perm
		getperms && execprep $prep && countperms ./tmp/perms/* || return 1
		i=`cd $perm; echo *`
		#  ./etc/perms may not exist.  Add it if necessary.
		[ -d ./etc/perms ] ||  {
			mkdir ./etc/perms
			[ $? != 0 ] && {
				error "Can't create /etc/perms"
				return 1
			}
		}
		[ -f ./etc/perms/$i ] && {
			error "./etc/perms/$i already exists"
			return 1 
		}
		mv $perm/$i ./etc/perms/$i
		rm -rf $perm
		# chkfornewsets prints an error if perm is invalid
		chkfornewsets -f || return 1
		sid=$last
		getsetvals $sid
		;;
	[0-9]*)	chkfornewsets
		getsetvals $sid || error "invalid set number: $sid"
		;;
	*)	# set was specified by name
		chkfornewsets
		nametosid $sid && error "unknown set name: $sid" || sid=$?
		;;
	esac
	return $?
}



# main()

# Initialize variables
perm=	arg=	vkey=	set=	setflg=	cmdflg=	setchkdone=	tens=z
sid=	nest=	save=	cmd=	pvol=	argflg=	notty=		ones=z
ignorepkgs=

# Must be at root
cd /

# evaluate arguments
[ $# != 0 ] && {
	set -- `getopt irlfm:s: $*` || {
		echo "Usage: custom [-s set] [-ilr [pkgs]] [-f [file]]" >&2
		exit 1
	}
	while	case $1 in
		-s)	setflg=$2; shift				;;
		-i)	cmdflg='setlogtrap; installpkgs $arglst; logit'	;;
		-r)	cmdflg='setlogtrap; removepkgs $arglst; logit'	;;
		-l)	cmdflg='listfiles $arglst'			;;
		-f)	cmdflg='setlogtrap; installfile $arglst; logit'	;;
		-m)	dev=$2; rdev=`dirname $2`/r`basename $2`
			;;
		--)	shift;	break					;;
		esac
	do	shift
	done
	arglst="$*"
}
# Clean up and exit after signals
trap 'cleanup 1' 1 2 3 15


# is everything here?  if so we are fully non-interactive
[ "$setflg" -a "$cmdflg" -a "$arglst" ] && {
	notty=y
	setselect $setflg || exit 1
	eval $cmdflg
	cleanup $?
}

# if a set was given and it is valid, generate a list of pkgs,
# otherwise force the user to select a valid set now
[ "$setflg" ] && setselect $setflg && getsetlist || setmenu || cleanup 0

# if a cmd was given, execute it with any args
[ "$cmdflg" ] && eval $cmdflg

unset setflg cmdflg arglst
quit="return to the menu"

# central processing loop
while	echo "
	1. Install one or more packages
	2. Remove one or more packages
	3. List the available packages
	4. List the files in a package
	5. Install a single file
	6. Select a new set to customize
	7. Display current disk usage
	8. Help

Select an option or enter q to quit: \c" >&2
do	read cmd arg
	case $cmd in
	1)	setlogtrap; installpkgs $arg; logit	;;
	2)	setlogtrap; removepkgs $arg; logit	;;
	3)	listpkgs 				;;
	4)	listfiles $arg				;;
	5)	setlogtrap; installfile $arg; logit	;;
	6)	setmenu $arg				;;
	7)	diskusage				;;
	8)	printhelp				;;
	Q|q)	cleanup 0				;;
	*)	error "enter 1 through 8 or q"		;;
	esac
done

