#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/add.sh	1.1.1.1"
if [ "$7" = "" ]
then	echo 1
	exit
fi

name="$1"
input=`echo $2 | tr ' ' ,`
output=`echo $3 | tr ' ' ,`
if echo "$4" | grep any > /dev/null
then	ptype=any
else	
	ptype=`echo $4 | tr ' ' ,`
fi

if echo "$5" | grep any > /dev/null
then	pname=any
else
	pname=`echo $5 | tr ' ' ,`
fi

if [ "$6" = Yes ]
then	ftype=slow
else	ftype=fast
fi

command="$7"

echo "Input types: $input
Output types: $output
Printer types: $ptype
Printers: $pname
Filter type: $ftype
Command: $7" > /usr/tmp/add$$

#check if invoked by Change Filters
if [ -f /usr/tmp/tf.$VPID ]
then
	echo "ptype=`echo $4`
prt=`echo $5`
in=`echo $2`
out=`echo $3`
slow=$6
command=$7" > /usr/tmp/newval$$
fi
shift 7

oparg=`echo $*`    #absorb blanks in "$*"
if [ "$oparg" = "" ]
then	
	#check if anything has been changed
	if [ -s /usr/tmp/tf.$VPID ]
	then diff /usr/tmp/newval$$ /usr/tmp/tf.$VPID > /dev/null
	     if [ $? = 0 ]
	     then echo 2
		  rm -f /usr/tmp/newval$$ /usr/tmp/add$$
		  exit
	     else rm -f /usr/tmp/newval$$
	     fi
	fi
	/usr/lib/lpfilter -f $name -F /usr/tmp/add$$
	rm -f /usr/tmp/add$$
	echo 0
	exit
else
	echo "Options: \c" >> /usr/tmp/add$$
	echo "INPUT
OUTPUT
TERM 
CPI 
LPI
LENGTH
WIDTH
PAGES
CHARSET
FORM
COPIES" > /usr/tmp/key$$

	#assign options to keywords
	for i in $*
	do
		for j in `cat /usr/tmp/key$$`
		do
		if [ "$1" != "" ]
		then	
			if [ $# -gt 12 ]	#last 12 args are mode options
			then
				opt=-$1*
				nopt=$1
				shift
				args=`echo $*`
				if [ "$args" = "" ]
				then
					echo "$j * = $opt">>/usr/tmp/add$$
					if [ -s /usr/tmp/tf.$VPID ]
					then echo "$j=$nopt">>/usr/tmp/newval$$
					     diff /usr/tmp/newval$$ /usr/tmp/tf.$VPID > /dev/null
					     if [ $? = 0 ]
					     then echo 2
						  rm -f /usr/tmp/newval$$ /usr/tmp/add$$ /usr/tmp/key$$
						  exit
					     else rm -f /usr/tmp/newval$$
					     fi
					fi
					/usr/lib/lpfilter -f $name -F /usr/tmp/add$$
					rm -f /usr/tmp/add$$ /usr/tmp/key$$
					echo 0
					exit
				else
					echo "$j * = $opt,\c">>/usr/tmp/add$$
					if [ -s /usr/tmp/tf.$VPID ]
					then echo "$j=$nopt">>/usr/tmp/newval$$
					fi
				fi
			else
				n=0
				for k in $*   #options for modes
				do
					if [ "$1" != "" ]
					then 	
						n=`expr $n + 1`
						kword="$1"
						kmode="$2"
						shift 2
						args=`echo $*`
						if [ "$args" = "" ]
						then 
							echo "MODES $kword = -$kmode" >> /usr/tmp/add$$
							if [ -s /usr/tmp/tf.$VPID ]
							then echo "MODE$n=$kword
opt$n=$kmode" >> /usr/tmp/newval$$
							     diff /usr/tmp/newval$$ /usr/tmp/tf.$VPID > /dev/null
							     if [ $? = 0 ]
							     then echo 2
								  rm -f /usr/tmp/newval$$ /usr/tmp/add$$ /usr/tmp/key$$
								  exit
							     else rm -f /usr/tmp/newval$$
							     fi
							fi
							/usr/lib/lpfilter -f $name -F /usr/tmp/add$$
							rm -f /usr/tmp/add$$ /usr/tmp/key$$
							echo 0
							exit
						else
							echo "MODES $kword = -$kmode,\c" >> /usr/tmp/add$$
							if [ -s /usr/tmp/tf.$VPID ]
							then echo "MODE$n=$kword
opt$n=$kmode" >> /usr/tmp/newval$$
							fi
						fi
					else
						shift 2
					fi
				done
			fi
		else
			shift
		fi
		done
	done
fi
