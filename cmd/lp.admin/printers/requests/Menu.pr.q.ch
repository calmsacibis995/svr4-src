#ident	"@(#)lp.admin:printers/requests/Menu.pr.q.ch	1.3.3.1"

menu=Choices
multiselect=true
framemsg="MARK choices then press ENTER"


`set -l name_1="/tmp/lp.n1$VPID";
if [ -n "$TFADMIN" ]; then $TFADMIN lpstat -oall | fmlcut -d'-' -f1  > $name_1;
else lpstat -oall | fmlcut -d'-' -f1  > $name_1; fi;
if [ -s $name_1 ];
then
	:;
else
	echo "init=false";
	message -b "There are no printers with print requests";
	rm  $name_1;
fi`

close=`rm  $name_1;
	unset -l $name_1`

done=`getitems " "|set -l Form_Choice`close

`/usr/bin/sort -u $name_1 | regex '^(.*)$0$' 'name=$m0'`
