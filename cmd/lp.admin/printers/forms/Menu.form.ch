#ident	"@(#)lp.admin:printers/forms/Menu.form.ch	1.2"

menu=Choices
lifeterm=shortterm
multiselect=true
framemsg="MARK choices then press ENTER"

`set -l name_1="/tmp/lp.fm1$VPID";
shell "ls /etc/lp/forms" > $name_1;
if [ -s $name_1 ];
then
	echo "all" >> $name_1;
else
	echo "init=false";
	message -b "There are no forms available";
	rm -f $name_1;
fi`

close=`rm -f $name_1;
	unset -l $name_1`

done=`getitems " "|set -l Form_Choice`close

`/usr/bin/sort $name_1 | regex '^(.*)$0$' 'name=$m0'`
