#ident	"@(#)lp.admin:printers/reports/Menu.usr.rq.ch	1.2.1.1"

menu="Choices" 
lifeterm=shortterm
multiselect=true
framemsg="MARK choices then press ENTER"

`set -l name_1="/tmp/lp.n1$VPID";
if [ -n "$TFADMIN" ]; then $TFADMIN lpstat -o$ARG1 | tr -s " " " " | fmlcut -d' ' -f2 > $name_1;
else lpstat -o$ARG1 | tr -s " " " " | fmlcut -d' ' -f2 > $name_1; fi;
if [ -s $name_1 ];
then
	echo "all" >> $name_1;
else
	echo "init=false";
	message -b "There are no print requests on this system";
	rm -f $name_1;
fi`

close=`rm -f $name_1;
	unset -l $name_1`

done=`getitems " "|set -l Form_Choice`close

`/usr/bin/sort -u $name_1 | regex '^(.*)$0$' 'name=$m0'`
