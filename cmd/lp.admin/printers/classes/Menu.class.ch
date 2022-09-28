#ident	"@(#)lp.admin:printers/classes/Menu.class.ch	1.2.1.2"

menu=Choices
lifeterm=shortterm
multiselect=true
framemsg="MARK choices then press ENTER"

`set -l name_1="/tmp/lp.cl$VPID";
ls /etc/lp/classes > $name_1;
if [ -s $name_1 ];
then
	echo "all" >> $name_1;
else
	echo "init=false";
	message -b "There are no classes available";
	/usr/bin/rm  $name_1;
fi`

close=`/usr/bin/rm  $name_1;
	unset -l $name_1`

done=`getitems " "|set -l Form_Choice`close

`/usr/bin/sort $name_1 | regex '^(.*)$0$' 'name=$m0'`
