#ident	"@(#)lp.admin:printers/Menu.lp	1.5.4.1"

menu="Line Printer Services Configuration and Operation"

help=OPEN TEXT $INTFBASE/Text.oamhelp $!LININFO

`if [ -n "$TFADMIN" ]; then $TFADMIN uname -n |set -e lp_system;
else uname -n |set -e lp_system; fi;
if [ -z "$lp_is_down" ];
then
    set -e datafile="/tmp/lpd.dat$VPID";
    cocreate -i lpdata -R lpu.main -e "-EOT-" /usr/lib/lp/lpdata -f $datafile;
    set -e error="/tmp/lpd.err$VPID";
    set -e output="/tmp/lpd.out$VPID";
    set -e lp_was_down=unknown;
    set -l d_msg="You must start the Printer Service before you can use this function.";
fi;
set -e OAMBASE=/usr/sadm/sysadm;
if [ -n "$TFADMIN" ];
then
	if $TFADMIN lpstat -r | fmlgrep 'scheduler is running' > /dev/null;
	then
    	set -e lp_is_down=false;
	else
    	set -e lp_is_down=true;
	fi;
else
	if lpstat -r | fmlgrep 'scheduler is running' > /dev/null;
	then
    	set -e lp_is_down=false;
	else
    	set -e lp_is_down=true;
	fi;
fi`

close=`codestroy -R lpu.main lpdata;
	/usr/bin/rm -f /tmp/lpd.dat$VPID /tmp/lpd.err$VPID /tmp/lpd.out$VPID;
	unset -e lp_system -e datafile -e output -e error -l lp_is_down -l lp_was_down -l d_msg`

name=classes
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/classes \"Manage Classes of Related Printers\"" classes
description="Manage Classes of Related Printers"  
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/classes";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/classes/Menu.classes";
fi`

name=filters
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/filters \"Manage Filters for Special Processing\"" filters
description=Manage Filters for Special Processing
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/filters";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/filters/Menu.filters";
fi`

name=forms
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/forms \"Defining Pre-Printed Forms\"" forms
description=Manage Pre-Printed Forms
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/forms";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/forms/Menu.forms";
fi`

name=operations
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/operations \"Performing Daily Printer Service Operations\"" operations
description=Perform Daily Printer Service Operations
action=`set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/operations"`OPEN MENU $OAMBASE/add-ons/lp/printers/operations/Menu.opers

name=printers
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/printers \"Configuring Printers\"" printers
description=Configure Printers for the Printer Service
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/printers";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/printers/Menu.printers";
fi`

name=priorities
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/priorities \"Assigning Print Queue Priorities to Users\"" priorities
description=Assign Print Queue Priorities to Users
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/priorities";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/priorities/Menu.prioritie";
fi`

name=requests
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/requests \"Examining and Manipulating Printer Requests\"" requests
description=Manage Active Print Requests
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/requests";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/requests/Menu.requests";
fi`


name=status
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/reports \"Display Status of Printer Service\"" reports
description=Display Status of Printer Service
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/reports";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/reports/Menu.reports";
fi`

name=systems
lininfo="/usr/sadm/sysadm/add-ons/lp/printers/systems \"Configuring COnnections to Remote Systems\"" systems
description=Configure Connections to Remote Systems
show=`
    indicator -w;
	if [ -f /usr/lib/lp/lpNet ];
    then
	true;
    else
	false;
    fi;
`
action=`if [ "$lp_is_down" = "true" ];
then
	message -b "$d_msg";echo nop;
else
	set -l OBJ_DIR="$OAMBASE/add-ons/lp/printers/systems";
	echo "OPEN MENU $OAMBASE/add-ons/lp/printers/systems/Menu.systems";
fi`
