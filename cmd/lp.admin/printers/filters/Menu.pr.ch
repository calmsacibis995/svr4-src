#ident	"@(#)lp.admin:printers/filters/Menu.pr.ch	1.2"

menu=Choices
multiselect=true
framemsg="MARK printer choices then press ENTER"
close=`/usr/bin/rm  $name_1;
	unset -l $name_1`

done=`getitems " "|set -l Form_Choice`close


`set -l name_1="/tmp/lp.n1$VPID";
	ls /etc/lp/printers > $name_1;
if [ -s $name_1 ];
then
	echo "any" >> $name_1;
else
	echo "init=false";
	message -b "There are no printers available";
	rm  $name_1;
fi`

`/usr/bin/sort $name_1 | regex '^(.*)$0$' 'name=$m0'`
